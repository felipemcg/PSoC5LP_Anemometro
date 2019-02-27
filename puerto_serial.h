/* ========================================
 *
 * Copyright FELIPE COSTA, 2018
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF FELIPE COSTA.
 *
 * ========================================
*/

#ifndef serial_port_H
#define serial_port_H
    #include "project.h"

    CY_ISR_PROTO(ESP_Recv);
    
    void initUART();
    void uart_espera_paquete();
    void uart_enviar_datos(void *buffer, uint16_t tam);
    uint8_t uart_leer_datos(uint8_t *buffer);
    void set_b_cmd_recibir_datos();
    void rst_b_cmd_recibir_datos();
    uint8_t get_posicion_comienzo_datos();
    uint16_t get_tam_paquete_datos_tcp();
    void rst_tam_paquete_datos_tcp();
    void incializar_esp8266();

#endif
/* [] END OF FILE */
