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
bool gb_socket_listo = 0;
bool gb_enviar_datos_tcp = 0;
bool gb_leer_datos_tcp = 0;
bool gb_cerrar_socket_tcp = 0;
uint16_t g_tam_tcp_tx = 0;
    /*--------*/

uint8_t bufferPaqueteRecibido[20];
cmd_in packet1451_in;

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    
    /*Inicio de perifericos.*/
    incializar_esp8266();
    UART_DEBUG_Start();
    EEPROM_CONFIGURACION_Start();
    /*----------------------*/

    debug_enviar("Inicio de sistema.");
    uint8_t retval = 0;
    
    debug_enviar("Configuracion Servidor");
    configServidor();
    iniciar_me_esp8266_modo_cliente();
    for(;;)
    {
        /*Se encarga de gestionar la conexion al servidor y a la red WiFi, tambien 
        realiza las funcioens de escritura, lectura y cierre del socket.*/
       estado_modulo_esp8266();
    
        /*Verifica primero que se haya establecido una conexion al servidor y que el socket esta 
        listo para recibir datos*/
        retval = TII_ReceivePacket(&bufferPaqueteRecibido[0],0);
        /*Verifica si se recibierion datos a traves del socket*/
        if(retval == 1){
            packet1451_in = TIM_GetPacket(&bufferPaqueteRecibido[0]);
            /*Llamar a procces packet*/
            TIM_ProcessPacket(&packet1451_in);
            retval = 0;
        }else{
            /*No se recibio ningun paquete*/
        }  
        servidorWeb();
        debug_procesador_comandos();

        CyDelay(500);
    }
}

/* [] END OF FILE */
