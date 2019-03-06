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
#ifndef TII_H_
#define TII_H_
    
    #include "project.h"
    #include "Datatype.h"

    uint8 TII_ReceivePacket(uint8_t *buffer, uint8 length);
    //void TIM_GetPacket(char *buffer,cmd_in *packet1451);
    cmd_in TIM_GetPacket(uint8_t *buffer);
    void TIM_ProcessPacket(cmd_in *packet_in);
    
    void rst_b_paquete_tim_recibido(void);
    void rst_b_paquete_tim_separado(void);
    void rst_b_paquete_tim_procesado(void);
    
    void TIM_Process(void);
    
#endif

