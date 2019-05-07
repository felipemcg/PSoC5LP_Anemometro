/**
* @file puerto_serial.c
*
* @author Felipe Costa 
*
* @date 5 de mayo de 2019
*
* @brief Conjunto de funciones para controlar el puerto serial.
*
*/
#include <stdlib.h>
#include <stdbool.h>
#include "project.h"
#include "globals.h"
#include "interfaz_debug.h"

#define DEBUG_SERIAL 0

/*
 * Timeouts. Múltiplo de 2 ms.
 */
#define TIMEOUT_RECEPCION_UART_ESP8266 20000 /**< Tiempo de espera para la respuesta a un comando, multiplo de 2ms */   

static bool b_cmd_recibir_datos = 0;        /**< Bandera utilizada por la funcion #uart_espera_paquete para indicar si se esperan datos.*/
static bool b_cmd_recibir_datos_ok = 0;     /**< Bandera utilizada por la funcion #uart_espera_paquete para notificar que el comando de recepcion de 
datos (SOR) fue ejecutado correctamente.*/
static bool b_par_coma = 0;                 /**< Bandera utilizada por la funcion #uart_espera_paquete para notificar cuando se encontro el primer 
par de comas en la respuesta recibida al ejectuar el comando de recepcion de datos (SOR).*/
static bool b_tam_paquete_recibido = 0;     /**< Bandera utilizada por la funcion #uart_espera_paquete para notificar cuando se recibio el tamaño 
del paquete de datos.*/
static uint16_t num_bytes_recibidos = 0;    /**< Variable para contar la cantidad de bytes en total se recibieron por el puerto serial.*/
static uint8_t posicion_comienzo_datos = 0; /**< Variable utilizada para guardar la posicion en donde empiezan los datos recibidos.*/
static uint16_t tam_paquete_datos_tcp = 0;  /**< Variable utilizada para guardar el tamaño en Bytes del paquete de datos recibido.*/
static uint16_t timeout_uart_esp8266 = 0;   /**< Variable utilizada como contador, se actualiza en la rutina de interrupcion #CY_ISR.*/
static uint8_t cantidad_comas = 0;          /**< Variable utilizada por la funcion #uart_espera_paquete para contar cuantos caracteres de coma 
se recibieron en la respuesta.*/
static uint8_t pos_primera_coma = 0;        /**< Variable utilizada por la funcion #uart_espera_paquete para guardar la posicion de la primera coma recibida.*/
static uint8_t pos_segunda_coma = 0;        /**< Variable utilizada por la funcion #uart_espera_paquete para guardar la posicion de la segunda coma recibida.*/
static uint8_t indice_buffer_tam_paquete_datos = 0; /**< Variable utilizada como indice para el array #buffer_tam_paquete_datos.*/
static uint8_t buffer_tam_paquete_datos[4] = {0};   /**< Variable utilizada por la funcion #uart_espera_paquete para guardar en ASCII el tamaño del paquete de datos recibido.*/
static uint16_t cantidad_caracteres_paquete_datos_tcp = 0; /**< Variable utilizada por la funcion #uart_espera_paquete como contador de Bytes para los 
caracteres que se van recibiendo del paquete de datos.*/

/**
    Estados del sistema del puerto serial.
*/
typedef enum  
{
    cmd_procesando, /**< El sistema se encuentra procensado un comando.*/
    cmd_recibiendo  /**< El sistema puede recibir la respuesta de un comando.*/
} cmd_status;
s
static cmd_status cmd_estado = cmd_procesando; /**< Variable que indica si el sistema se encuentra en un estado de recepcion de respuesta de un comando 
o procesando el comando.*/


/**
Rutina de servicio de interrupcion del contador. Se ingresa en esta funcion 
cada 2ms para incrementar en una unidad la variable #timeout_uart_esp8266.
    @warning Esta funcion es dependiente de la plataforma, por lo tanto, al 
    realizarse un cambio de plataforma, es necesario realizar cambios.
*/
CY_ISR(contador_timeout_esp8266){
    Timer_esp8266_ReadStatusRegister();
    timeout_uart_esp8266++; 
    isr_timer_esp8266_ClearPending();
    return; 
}

/**
Funcion para inicializar el modulo ESP8266. Inicializa el puerto serial, el 
contador de timeout, y se habilita la alimentacion del modulo a traves de un
puerto de salida. 
    @warning Esta funcion es dependiente de la plataforma, por lo tanto, al 
    realizarse un cambio de plataforma, es necesario realizar cambios.
*/
void incializar_esp8266()
{
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

/**            
Funcion que revisa el buffer de recepcion serial y lee Byte por Byte los datos, para 
determinar cuando se recibio un paquete completo. Existen 2 tipos de paquetes: Los
paquetes de respuestas a comandos, los cuales terminan en donde se encuentra el 
caracter \n. El otro paquete es el de los datos, en el cual se obtiene el tamaño de 
los datos a recibir, al recibir todos estos datos debe estar presente el caracter \n 
para que el paquete sea valido.
    @warning   Para re-utilizar esta funcion en una plataforma diferente, hay que implementar las
    funciones #UART_ESP_ReadRxData y #UART_ESP_GetRxBufferSize.
    @note   Cuando los parametros de Buffer Size dentro de la 
    configuracion del componente UART_ESP en el Top Design, tiene un valor mayor a 4 Bytes
    automaticamente de utiliza la interrupcion interna para pasar los Bytes del FIFO 
    Hardware al FIFO Software, para mas detalles, ver el datasheet y  el link: 
    https://community.cypress.com/thread/31534?start=0&tstart=0
    @param  *buffer_respuesta_comando   Puntero al array en donde se almacenara la respuesta 
    recibida. Obs: Aqui no se almacenan datos TCP ni UDP. 
    @param  timeout     Parametro para indicar cuanto tiempo se debe esperar a recibir un nuevo
    Byte. Para calcular el tiempo en segundos, multiplicar el valor de timeout por 2ms.
    @retval  0   Se recibio correctamente un paquete.  
    @retval -1   Se estaba recibiendo datos, al terminar la cantidad de Bytes, no se encontro
    el caracter \n.
    @retval -2   Ocurrio un TIMEOUT.
*/
int8_t uart_espera_paquete(uint8_t *buffer_respuesta_comando, uint32_t timeout){    
    int8_t valor_retorno = -127; 
    volatile char rec_data = 0;
    volatile uint16_t rx_buffer_size = 0;

    /*Se resetean(ponen a 0), todas las variables que actuan de bandera y 
    acumuladores, para recibir otra respuesta de comando.*/
    b_cmd_recibir_datos_ok = 0; 
    b_par_coma = 0;
    b_tam_paquete_recibido = 0;
    cantidad_comas = 0;
    pos_primera_coma = 0;
    pos_segunda_coma = 0;
    indice_buffer_tam_paquete_datos = 0;
    memset(buffer_tam_paquete_datos,0,sizeof(buffer_tam_paquete_datos));
    cantidad_caracteres_paquete_datos_tcp = 0;
    num_bytes_recibidos = 0;
    timeout_uart_esp8266 = 0;  

    #if DEBUG_SERIAL
    DEBUG_OUT_2_Write(1);
    #endif

    while(1)
    {
        /*Retorna la cantidad de Bytes disponibles en el FIFO Software, sin contar los 
        Bytes que se encuentran dentro del FIFO Hardware.*/
        rx_buffer_size = UART_ESP_GetRxBufferSize();
        
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
                && (rec_data != ',') && (cantidad_comas  > 0) 
                && ( cantidad_comas < 2))
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
            if((b_cmd_recibir_datos == 1) 
                && (b_cmd_recibir_datos_ok == 1) 
                && (b_tam_paquete_recibido == 1) 
                && (num_bytes_recibidos > pos_segunda_coma))
            {
                /*Se va leer datos del comando SOR*/
                if(cantidad_caracteres_paquete_datos_tcp < tam_paquete_datos_tcp)
                {
                    /*Se guardan los datos TCP recibidos*/
                    g_tcp_rx_buffer[cantidad_caracteres_paquete_datos_tcp] = rec_data; 
                    cantidad_caracteres_paquete_datos_tcp++;
                }else
                {
                    /*Ya se alcanzo la cantidad de bytes especificada*/
                    if(rec_data == CMD_TERMINATOR)
                    {
                        /*Se encontro el terminador(\n)*/
                        valor_retorno = 0;
                        break;
                    }else
                    {
                        /*No se encontro el caracter terminador,informar 
                        error de formato en el paquete*/
                        valor_retorno = -1;
                        break;    
                    }
                } 
            }else
            {
                /*Se leen los datos como siempre*/
                if(rec_data == CMD_TERMINATOR)
                {
                    /*Se encontro el terminador(\n)*/
                    DEBUG_OUT_4_Write(1);
                    valor_retorno = 0;
                    break;
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
                break;
                debug_enviar("UART-ESP8266 => TIMEOUT!");
                debug_enviar("\n");
            }
        }
    }
    #if DEBUG_SERIAL
    DEBUG_OUT_2_Write(0);
    #endif

    return valor_retorno;
}

/**
Funcion para enviar los comandos al ESP8266 a traves del puerto serial.
    @warning   Para re-utilizar esta funcion en una plataforma diferente, hay que implementar las
    funciones necesarias.
    @param buf      Puntero al array donde se encuentran los datos a ser enviados.
    @param tam      Cantidad de Bytes que se encuentran dentro de buf. 
*/
void uart_enviar_datos(uint8_t *buf, uint16_t tam)
{
    volatile uint8_t tx_status; 
    if(cmd_estado == cmd_procesando)
    {
        /*Funcion bloqueante, retorna cuando se pasaron cargaron todos los datos 
        al buffer de transmision software. Obs: al retornar no significa que 
        ya fueron transmitidos todos los Bytes.*/
        UART_ESP_PutArray(buf,tam);
        
        /*Espera que se transmitan todos los Bytes almacenados en el 
        FIFO Software,FIFO hardware y que se transmita correctamente el 
        ultimo byte.*/
        do{
            /*Observar el comportamiento al utilizar ReadTxStatus(), debido a 
            que al llamar esta funcion, se limpian algunos bits.*/
            tx_status = UART_ESP_ReadTxStatus();
        }while( (UART_ESP_GetTxBufferSize() > 0) 
                || !(tx_status & UART_TX_STS_FIFO_EMPTY)
                || !(tx_status & UART_TX_STS_COMPLETE)); 
        
        cmd_estado = cmd_recibiendo;
    } 
}

/**
Funcion para copia la respuesta a los comandos dentro de un buffer. Esta funcion es independiente  
a la platoforma y sirve mas bien como una funcion envolvente de #uart_espera_paquete. 
    @note           Los datos recibidos por los sockets no se almacenan en este buffer. 
    @param  buffer     Puntero al array donde se almacenara la respuesta a los comandos.
    @return Cantidad de bytes recibidos por el puerto serial.
*/
uint8_t uart_leer_datos(uint8_t *buffer)
{
    uint16_t valor_retorno = 0;
    int8_t ret_uart;
    
    if(cmd_estado == cmd_recibiendo)
    {
        memset(buffer,'\0',sizeof(*buffer));

        /*Bloqueaante, espera a recibir todo el paquete de respuesta del modulo.*/
        ret_uart = uart_espera_paquete(buffer,TIMEOUT_RECEPCION_UART_ESP8266);

        if(ret_uart == 0)
        {
            /*Se indica que el sistema esta procesando el paquete recibido.*/
            cmd_estado = cmd_procesando;
            valor_retorno = num_bytes_recibidos;
        }else
        {
            buffer[0] = '\0';
            valor_retorno = 0;
        }
    } 
    return valor_retorno;
}

/**
Funcion utilizada por la biblioteca de funciones del ESP8266,
que establece en verdadero el valor la bandera #b_cmd_recibir_datos.
*/
void set_b_cmd_recibir_datos()
{
    b_cmd_recibir_datos = 1;
    return;
}

/**
Funcion utilizada por la biblioteca de funciones del ESP8266,
que establece en falso el valor la bandera #b_cmd_recibir_datos.
*/
void rst_b_cmd_recibir_datos()
{
    b_cmd_recibir_datos = 0;
    return; 
}

/**
Funcion utilizada por la biblioteca de funciones del ESP8266,
para determinar la posicion desde donde se encuentran los datos recibidos.
*/
uint8_t get_posicion_comienzo_datos()
{
    return posicion_comienzo_datos;
}

/**
Funcion utilizada por la biblioteca de funciones del ESP8266,
para poner a cero el contador de los datos recibidos.
*/
void rst_posicion_comienzo_datos()
{
    posicion_comienzo_datos = 0;
    return; 
}

/**
Funcion utilizada por la biblioteca de funciones del ESP8266,
para obtener el tamaño del paquete de datos recibido.
*/
uint16_t get_tam_paquete_datos_tcp()
{
    return tam_paquete_datos_tcp;
}

/**
Funcion utilizada por la biblioteca de funciones del ESP8266,
para poner a cero el contador del paquete de datos recibido.
*/
void rst_tam_paquete_datos_tcp()
{
    tam_paquete_datos_tcp = 0;
}
/* [] END OF FILE */