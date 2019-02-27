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

/* [] END OF FILE */
#include <stdlib.h>
#include <stdbool.h>
#include "project.h"
#include "ring_buffer.h"
#include "globals.h"
#include "interfaz_debug.h"

#define DEBUG_SERIAL 0

static volatile bool b_paquete_serial_recibido = 0;
static volatile bool b_cmd_recibir_datos = 0;
static volatile uint16_t num_bytes_recibidos = 0;
static volatile uint8_t posicion_comienzo_datos = 0;
static volatile uint16_t tam_paquete_datos_tcp = 0;
static volatile uint16_t timeout_uart_esp8266; 

circ_bbuf_t sCircBuff = {
    .buffer = serialCircBuffer,  
    .head = 0,                        
    .tail = 0,                        
    .maxlen = SERIAL_BUFFER_SIZE  
    };

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
    return;
}

void uart_espera_paquete(){
    #if DEBUG_SERIAL
    DB1_Write(1u);
    #endif
    
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
    uint16_t rx_buffer_size = UART_ESP_GetRxBufferSize();
    
    if(rx_buffer_size > 0){ 
        /*Leemos el registro de estado del UART*/
        //rec_status = UART_ESP_RXSTATUS_REG;
        /*Verificamos que hayan datos dentro del buffer del UART*/
        //if(rec_status & UART_ESP_RX_STS_FIFO_NOTEMPTY){
        if(1==1){
            /*Se extrae el byte recibido del HW UART*/
            rec_data = UART_ESP_GetByte();
            
            /*Si se trata de recepcion de datos, se verfica que no hubo error*/
            if(b_cmd_recibir_datos == 1 && num_bytes_recibidos == 0 && rec_data == CMD_RESP_OK){
                b_cmd_recibir_datos_ok = 1;
            }
            
            /*Se lleva la cuenta de las comas y se guarda la posicion de la primera y segunda, despues 
            de eso ya no entra aqui, solo cuando se termino de procesar el paquete.*/
            if( (b_cmd_recibir_datos == 1) && (b_cmd_recibir_datos_ok == 1) && (rec_data == ',') && (b_par_coma == 0) ){
            	if(cantidad_comas == 0){
            		pos_primera_coma = num_bytes_recibidos;
            	}
            	if(cantidad_comas == 1){
            		pos_segunda_coma = num_bytes_recibidos;
                    /*Indica que ya se encontraron el par de comas*/
            		b_par_coma = 1;
            	}
        	    cantidad_comas++;
            }
            
            /*Si se trata de recepcion de datos, se almacena la cantidad de bytes a recibir*/
            /*Agregar verificacion de si lo que se esta guardando es efectivamente un numero*/
            if( (b_cmd_recibir_datos == 1) && (b_cmd_recibir_datos_ok == 1) && (rec_data != ',') && (cantidad_comas  > 0) && ( cantidad_comas < 2)){
                /*Falta agregar verificacion de que el caracter que se va almacenar es efectivamente un numero.*/
                buffer_tam_paquete_datos[indice_buffer_tam_paquete_datos++] = rec_data;
            }
          
            /* Se verifica que la cantidad de bytes este presente y sea un numero valido*/
            if( (b_cmd_recibir_datos == 1) && (b_cmd_recibir_datos_ok == 1) && (b_par_coma == 1) && (b_tam_paquete_recibido == 0)){
                buffer_tam_paquete_datos[indice_buffer_tam_paquete_datos] = '\0';
    			tam_paquete_datos_tcp = atoi((char*)buffer_tam_paquete_datos);
    	        b_tam_paquete_recibido = 1;
    		}
            
            /*Se verifica si lo que se va a recibir son datos o u otro comando*/
            if((b_cmd_recibir_datos == 1) && (b_cmd_recibir_datos_ok == 1) && 
            (b_tam_paquete_recibido == 1) && (num_bytes_recibidos > pos_segunda_coma) )
            {
                /*Se va leer datos del comando SOR*/
                if(cantidad_caracteres_paquete_datos_tcp < tam_paquete_datos_tcp)
                {
                    /*Se guardan los datos TCP recibidos*/
                    //circ_bbuf_push(&sCircBuff,rec_data);
                    g_tcp_rx_buffer[cantidad_caracteres_paquete_datos_tcp] = rec_data; 
                    cantidad_caracteres_paquete_datos_tcp++;
                }else{
                    /*Ya se alcanzo la cantidad de bytes especificada*/
                    if(rec_data == CMD_TERMINATOR){
                        /*Se encontro el terminador(\n)*/
                        b_paquete_serial_recibido  = 1; 
                    }else{
                        /*No se encontro el caracter terminador,informar 
                        error de formato en el paquete*/
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
            }else{
                /*Se leen los datos como siempre*/
                if(rec_data == CMD_TERMINATOR){
                    /*Se encontro el terminador(\n)*/
                    b_paquete_serial_recibido  = 1;    
                }else{
                    circ_bbuf_push(&sCircBuff,rec_data);
                    num_bytes_recibidos++;
                } 
            }
        }
    }
retorno_interrupcion:
    #if DEBUG_SERIAL
    DB1_Write(0u);
    #endif
    return;
}

void uart_enviar_datos(void *buffer, uint16_t tam){
    num_bytes_recibidos = 0;
    g_cmd_estado = cmd_recibiendo;
    if(1==1){
        /*Funcion bloqueante, solo vuelve cuando se enviaron todos los datos*/
        UART_ESP_PutArray(buffer,tam);
        //UART_ESP_PutString(string);
    }
     
}

uint8_t uart_leer_datos(uint8_t *buffer){
    uint8_t data;
    uint16_t i=0;
    
    /*Bloqueaante, se espera a recibir todo el paquete de respuesta del modulo,
    la rutina ISR del UART se encarga de setear la variable.*/
    while(b_paquete_serial_recibido != 1){
        uart_espera_paquete();
    }
    
    /*Se indica que el sistema esta procesando el paquete recibido.*/
    g_cmd_estado = cmd_procesando; 
    
    /*Se limpia el buffer para recibir nuevos datos por el canal serial.*/
    i = sizeof(buffer); 
    for(i = 0; i < sizeof(buffer); i++ ){
        buffer[i] = (uint8)0; 
    }
     
    
//    while(circ_bbuf_pop(&sCircBuff,&data) != -1){
//        buffer[i++] = data; 
//    }
    
    /*Se saca byte por byte del buffer circular*/
    for(i = 0; i < num_bytes_recibidos; i++ ) {
        /*Se verifica que existan datos*/
        if(circ_bbuf_pop(&sCircBuff,&data) == -1){
            /*Esta condicion nunca se debe cumplir, ya que deberia estar todo lo que se recibio*/
            LED_Write(1u);
            return 0;
        }
        buffer[i] = data; 
    }
    buffer[i] = '\0';
    
    /*Se resetean las banderas necesarias para recibir el nuevo paquete*/
    b_paquete_serial_recibido  = 0;
    /*Se resetea el contadore de bytes de recepcion*/
    num_bytes_recibidos = 0;
   
    return i;   
}

void initUART(){
    UART_ESP_Start();
}

void set_b_cmd_recibir_datos(){
    b_cmd_recibir_datos = 1;
    return;
}

void rst_b_cmd_recibir_datos(){
    b_cmd_recibir_datos = 0;
    return; 
}

uint8_t get_posicion_comienzo_datos(){
    return posicion_comienzo_datos;
}

void rst_posicion_comienzo_datos(){
    posicion_comienzo_datos = 0;
    return; 
}

uint16_t get_tam_paquete_datos_tcp(){
    return tam_paquete_datos_tcp;
}

void rst_tam_paquete_datos_tcp(){
    tam_paquete_datos_tcp = 0;
}

