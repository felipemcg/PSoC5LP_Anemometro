/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include <stdlib.h>
#include <stdbool.h>
#include "project.h"
#include "ring_buffer.h"
#include "globals.h"
#include "interfaz_debug.h"

#define DEBUG_SERIAL 1

/*
 * Timeouts. Múltiplo de 2 ms.
 */
#define TIMEOUT_RECEPCION_UART_ESP8266 20000 // Espera de 2ms.    

static volatile bool b_paquete_serial_recibido = 0;
static volatile bool b_cmd_recibir_datos = 0;
static volatile uint16_t num_bytes_recibidos = 0;
static volatile uint8_t posicion_comienzo_datos = 0;
static volatile uint16_t tam_paquete_datos_tcp = 0;
static volatile uint16_t timeout_uart_esp8266; 

typedef enum  {cmd_procesando, cmd_recibiendo} cmd_status;
static cmd_status cmd_estado = cmd_procesando;

CY_ISR(contador_timeout_esp8266){
    Timer_esp8266_ReadStatusRegister();
    timeout_uart_esp8266++; 
    isr_timer_esp8266_ClearPending();
    return; 
}

void incializar_esp8266(){
    UART_ESP_Start(); 
    isr_timer_esp8266_StartEx(contador_timeout_esp8266);
    Timer_esp8266_Start();
    timeout_uart_esp8266 = 0;
    UART_ESP_ClearTxBuffer();
    UART_ESP_ClearRxBuffer();
    ENABLE_ESP_Write(1);
    CyDelay(100);
    return;
}

/*            
Funcion que revisa el buffer de recepcion serial y lee Byte por Byte los datos, para 
determinar cuando se recibio un paquete completo. Existen 2 tipos de paquetes: Los
paquetes de respuestas a comandos, los cuales terminan en donde se encuentra el 
caracter \n. El otro paquete es el de los datos, en el cual se obtiene el tamaño de 
los datos a recibir, al recibir todos estos datos debe estar presente el caracter \n 
para que el paquete sea valido. Obs: Cuando los parametros de Buffer Size dentro de la 
configuracion del componente UART_ESP en el Top Design, tiene un valor mayor a 4 Bytes
automaticamente de utiliza la interrupcion interna para pasar los Bytes del FIFO 
Hardware al FIFI Software, para mas detalles, ver el datasheet y  el link: 
https://community.cypress.com/thread/31534?start=0&tstart=0
Parametros:
    -*buffer_respuesta_comando -> Puntero al array en donde se almacenara la respuesta 
    recibida. Obs: Aqui no se almacenan datos. 
    -timeout -> Parametro para indicar cuanto tiempo se debe esperar a recibir un nuevo
    Byte. Para calcular el tiempo en segundos, multiplicar el valor de timeout por 2ms.
Retorno: 
    1   -> Todavia no se concluyo la recepcion de un paquete, los datos siguen llegando.
    0   -> Se recibio correctamente un paquete.  
    Posibles errores(unicamente valores negativos).
    -1  -> Se estaba recibiendo datos, al terminar la cantidad de Bytes, no se encontro
    el caracter \n.
    -2  -> Ocurrio un TIMEOUT.
*/
int8_t uart_espera_paquete(uint8_t *buffer_respuesta_comando, uint32_t timeout){
    #if DEBUG_SERIAL
    LED_Write(1u);
    #endif
    
    int8_t valor_retorno = 1; 
    static bool b_cmd_recibir_datos_ok = 0; 
    static bool b_par_coma = 0;
    static bool b_tam_paquete_recibido = 0;
    
    volatile static uint8_t cantidad_comas = 0;
    static uint8_t pos_primera_coma = 0;
    static uint8_t pos_segunda_coma = 0;
    static uint8_t indice_buffer_tam_paquete_datos = 0;
    
    static uint8_t buffer_tam_paquete_datos[4] = {0}; 
    static uint16_t cantidad_caracteres_paquete_datos_tcp = 0;
    
    uint8 rec_status = 0u;
    volatile char rec_data = 0;
    
    /*Verificamos que el sistema se encuentra en un estado de recepcion de datos*/
//    if(g_cmd_estado == cmd_procesando){
//        goto retorno_interrupcion;
//    }   
    
    /*Retorna la cantidad de Bytes disponibles en el FIFO Software, sin contar los 
    Bytes que se encuentran dentro del FIFO Hardware.*/
    uint16_t rx_buffer_size = UART_ESP_GetRxBufferSize();
    
    if(rx_buffer_size > 0)
    { 
        timeout_uart_esp8266 = 0;
        
        /*Se extrae el siguiente Byte recibido del FIFO Software.*/
        rec_data = UART_ESP_ReadRxData();
        
        /*Si se trata de recepcion de datos, se verfica que no hubo error*/
        if(b_cmd_recibir_datos == 1 && num_bytes_recibidos == 0 
            && rec_data == CMD_RESP_OK)
        {
            b_cmd_recibir_datos_ok = 1;
        }
        
        /*Se lleva la cuenta de las comas y se guarda la posicion de la primera 
        y segunda, despues de eso ya no entra aqui, solo cuando se termino de 
        procesar el paquete.*/
        if( (b_cmd_recibir_datos == 1) && (b_cmd_recibir_datos_ok == 1) 
            && (rec_data == ',') && (b_par_coma == 0) )
        {
        	if(cantidad_comas == 0)
            {
        		pos_primera_coma = num_bytes_recibidos;
        	}
        	if(cantidad_comas == 1)
            {
        		pos_segunda_coma = num_bytes_recibidos;
                /*Indica que ya se encontraron el par de comas*/
        		b_par_coma = 1;
        	}
    	    cantidad_comas++;
        }
        
        /*Si se trata de recepcion de datos, se almacena la cantidad de bytes 
        a recibir*/
        /*Agregar verificacion de si lo que se esta guardando es efectivamente un numero*/
        if( (b_cmd_recibir_datos == 1) && (b_cmd_recibir_datos_ok == 1) 
            && (rec_data != ',') && (cantidad_comas  > 0) && ( cantidad_comas < 2))
        {
            /*Falta agregar verificacion de que el caracter que se va almacenar 
            es efectivamente un numero.*/
            buffer_tam_paquete_datos[indice_buffer_tam_paquete_datos++] = rec_data;
        }
      
        /* Se verifica que la cantidad de bytes este presente y sea un numero valido*/
        if( (b_cmd_recibir_datos == 1) && (b_cmd_recibir_datos_ok == 1) 
            && (b_par_coma == 1) && (b_tam_paquete_recibido == 0))
        {
            buffer_tam_paquete_datos[indice_buffer_tam_paquete_datos] = '\0';
			tam_paquete_datos_tcp = atoi((char*)buffer_tam_paquete_datos);
	        b_tam_paquete_recibido = 1;
		}
        
        /*Se verifica si lo que se va a recibir son datos o u otro comando*/
        if((b_cmd_recibir_datos == 1) && (b_cmd_recibir_datos_ok == 1) && 
            (b_tam_paquete_recibido == 1) && (num_bytes_recibidos > pos_segunda_coma))
        {
            /*Se va leer datos del comando SOR*/
            if(cantidad_caracteres_paquete_datos_tcp < tam_paquete_datos_tcp)
            {
                /*Se guardan los datos TCP recibidos*/
                //circ_bbuf_push(&sCircBuff,rec_data);
                g_tcp_rx_buffer[cantidad_caracteres_paquete_datos_tcp] = rec_data; 
                cantidad_caracteres_paquete_datos_tcp++;
            }else
            {
                /*Ya se alcanzo la cantidad de bytes especificada*/
                if(rec_data == CMD_TERMINATOR)
                {
                    /*Se encontro el terminador(\n)*/
                    valor_retorno = 0; 
                }else
                {
                    /*No se encontro el caracter terminador,informar 
                    error de formato en el paquete*/
                    valor_retorno = -1;   
                }
                posicion_comienzo_datos = pos_segunda_coma + 1; 
                b_cmd_recibir_datos_ok = 0;
                b_par_coma = 0;
                b_tam_paquete_recibido = 0;
                cantidad_comas = 0;
                pos_primera_coma = 0;
                pos_segunda_coma = 0;
                indice_buffer_tam_paquete_datos = 0;
                memset(buffer_tam_paquete_datos,0,sizeof(buffer_tam_paquete_datos));
                cantidad_caracteres_paquete_datos_tcp = 0;
            } 
        }else
        {
            /*Se leen los datos como siempre*/
            if(rec_data == CMD_TERMINATOR)
            {
                /*Se encontro el terminador(\n)*/
                valor_retorno = 0;    
            }else
            {
                buffer_respuesta_comando[num_bytes_recibidos] = rec_data;
                num_bytes_recibidos++;
            } 
        }
    }else
    {
        if(timeout_uart_esp8266 > timeout)
        {
            /*Retornar error, indicando TIMEOUT. Ademas, preparar las variables
            para recibir la respuesta a otro comando.*/
            valor_retorno = -2;
            b_cmd_recibir_datos_ok = 0;
            b_par_coma = 0;
            b_tam_paquete_recibido = 0;
            cantidad_comas = 0;
            pos_primera_coma = 0;
            pos_segunda_coma = 0;
            indice_buffer_tam_paquete_datos = 0;
            memset(buffer_tam_paquete_datos,0,sizeof(buffer_tam_paquete_datos));
            cantidad_caracteres_paquete_datos_tcp = 0;
            LED_Write(0);
            debug_enviar("UART-ESP8266 => TIMEOUT!");
            debug_enviar("\n");
        }
    } 
retorno_interrupcion:
    #if DEBUG_SERIAL
    LED_Write(0u);
    #endif
    return valor_retorno;
}

void uart_enviar_datos(uint8_t *buf, uint16_t tam)
{
    g_cmd_estado = cmd_recibiendo;
    if(1==1){
        //UART_ESP_ClearTxBuffer();
        
        /*Funcion bloqueante, retorna cuando se pasaron cargaron todos los datos al 
        buffer de transmision, sin embargo, no signica que fueron transmitidos todos*/
        UART_ESP_PutArray(buf,tam);
        
        /*Espera que se transmitan todos los Bytes almacenados en el buffer. Obs: no
        se tiene en cuenta los 4 Bytes del FIFO Hardware.*/
        //while(UART_ESP_GetTxBufferSize > 0);
    }
     
}

uint8_t uart_leer_datos(uint8_t *buffer)
{
    uint16_t valor_retorno = 0;
    int8_t ret_uart;
    
    timeout_uart_esp8266 = 0;
    num_bytes_recibidos = 0;
    
    /*Bloqueaante, espera a recibir todo el paquete de respuesta del modulo.*/
    do 
    {
        ret_uart = uart_espera_paquete(buffer,TIMEOUT_RECEPCION_UART_ESP8266);
    }while( ret_uart == 1 );
    
    if(ret_uart == 0)
    {
        /*Se indica que el sistema esta procesando el paquete recibido.*/
        g_cmd_estado = cmd_procesando; 
       
        valor_retorno = num_bytes_recibidos;
        
        /*Se resetea el contador de Bytes de recepcion*/
        num_bytes_recibidos = 0;
    }else if(ret_uart == -1)
    {  
        buffer[0] = '\0';
        valor_retorno = 0;
    }else if(ret_uart == -2)
    {
        buffer[0] = '\0';
        valor_retorno = 0;
    }
    return valor_retorno;
}

void set_b_cmd_recibir_datos()
{
    b_cmd_recibir_datos = 1;
    return;
}

void rst_b_cmd_recibir_datos()
{
    b_cmd_recibir_datos = 0;
    return; 
}

uint8_t get_posicion_comienzo_datos()
{
    return posicion_comienzo_datos;
}

void rst_posicion_comienzo_datos()
{
    posicion_comienzo_datos = 0;
    return; 
}

uint16_t get_tam_paquete_datos_tcp()
{
    return tam_paquete_datos_tcp;
}

void rst_tam_paquete_datos_tcp()
{
    tam_paquete_datos_tcp = 0;
}
/* [] END OF FILE */