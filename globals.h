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
#ifndef globals_H
#define globals_H
    
    #include "project.h"
    #include <stdbool.h>

    #define DELIMITER ","
    #define CMD_TERMINATOR '\n'
    #define CMD_RESP_OK '0'
    #define CMD_START_OK 'R'
    #define CMD_ERROR_1 '1'
    #define CMD_ERROR_2 '2'
    #define CMD_ERROR_3 '3'
    #define CMD_ERROR_4 '4'
    #define CMD_ERROR_5 '5'
    #define CMD_ERROR_6 '6'
    #define CMD_ERROR_7 '7'

    #define SERIAL_BUFFER_SIZE 1500
    #define TCP_TX_BUFFER_SIZE 1460
    #define TCP_RX_BUFFER_SIZE 1024


    /*Variables utilizadas por el ISR del UART*/
    volatile uint8_t serialCircBuffer[SERIAL_BUFFER_SIZE]; 
    /*----------*/

    
    typedef enum  {cmd_procesando, cmd_recibiendo} cmd_status;
    
    /*Declaracion de variables globales*/
    //gb_ : global bool variable 
    extern volatile cmd_status g_cmd_estado;
    extern uint16_t g_tam_tcp_tx;
    extern uint8_t g_tcp_tx_buffer[TCP_TX_BUFFER_SIZE]; 
    extern uint8_t g_tcp_rx_buffer[TCP_RX_BUFFER_SIZE];
    
    extern bool gb_uart_debug_comando_recibido; 
    extern bool gb_uart_debug_activado; 
    /*----------------------------------------*/


#endif

