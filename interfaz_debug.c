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
#include "project.h"
#include <stdbool.h>
#include <string.h>

#define BUFFER_DEBUG_SIZE 64
static char buffer_uart_debug[BUFFER_DEBUG_SIZE];

bool gb_uart_debug_comando_recibido = true; 
bool gb_uart_debug_activado = true; 

void debug_enviar(char *mensaje){
    if(gb_uart_debug_activado == true){
        UART_DEBUG_PutString(mensaje);
        //UART_DEBUG_PutString("\r\n");
    } 
    return;
}

int8_t debug_recibir(char *buffer)
{
    static uint8_t indice_buffer = 0;
    char caracter_recibido;
    int8_t valor_retorno = -1; 
    if(UART_ESP_GetRxBufferSize() > 0)
    {
        caracter_recibido = UART_DEBUG_GetChar();
        if(caracter_recibido != '\n')
        {
            buffer[indice_buffer] = caracter_recibido;
            indice_buffer++;
        }
        else
        {
            buffer[indice_buffer] = '\0';
            indice_buffer = 0;
            valor_retorno = indice_buffer; 
        }
    }
    return valor_retorno;
}

void debug_procesador_comandos(void)
{
    int8_t cant_bytes_recibidos; 
    cant_bytes_recibidos = debug_recibir(buffer_uart_debug);
    if( cant_bytes_recibidos > 0 )
    {
        if(strcmp(buffer_uart_debug,"OFF") == 0)
        {
            gb_uart_debug_activado = false;
            LED_Write(0);
        }
        if(strcmp(buffer_uart_debug,"ON") == 0)
        {
            gb_uart_debug_activado = true;
            LED_Write(1);
        }
    }
    return;
} 
/* [] END OF FILE */
