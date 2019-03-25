/* ========================================
 * PFC - Felipe Costa
 * ========================================
*/
#include "project.h"
#include "TII.h"
#include "globals.h"
#include "puerto_serial.h"
#include "Datatype.h"
#include "servidor_web.h"
#include "interfaz_debug.h"
#include "esp8266_cliente.h"

/*Unica definicion de las variables globales*/
volatile cmd_status g_cmd_estado = cmd_procesando;
uint8_t g_tcp_tx_buffer[TCP_TX_BUFFER_SIZE] = {0}; 
uint8_t g_tcp_rx_buffer[TCP_RX_BUFFER_SIZE] = {0}; 
uint16_t g_tam_tcp_tx = 0;
    /*--------*/

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    
    /*Inicio de perifericos.*/
    incializar_esp8266();
    UART_DEBUG_Start();
    EEPROM_CONFIGURACION_Start();
    configServidor();
    iniciar_me_esp8266_modo_cliente();
    for(;;)
    {
        /*Se encarga de gestionar la conexion al servidor y a la red WiFi, tambien 
        realiza las funcioens de escritura, lectura y cierre del socket.*/
        estado_modulo_esp8266();
        TIM_Process();  
        servidorWeb();
        debug_procesador_comandos();
        //CyDelay(500);
    }
}

/* [] END OF FILE */
