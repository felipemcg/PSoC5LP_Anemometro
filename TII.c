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
#include "project.h"
#include "Datatype.h"
#include "ESP8266.h"
#include "globals.h"
#include "puerto_serial.h"
#include "esp8266_cliente.h"
#include "interfaz_debug.h"

bool b_me_esp8266_datos_recibidos = false; 
bool b_me_esp8266_datos_enviados = false;  

cmd_in TIM_GetPacket(uint8_t *buffer){
    debug_enviar("TII => Obtener paquete");
    debug_enviar("\n");
    cmd_in packet_in; 
    packet_in.transd = (uint16)((buffer[0]<<8)+(buffer[1]<<0));
    packet_in.cmd_class = buffer[2];
    packet_in.cmd_func = buffer[3];
    packet_in.length = (uint16)((buffer[4]<<8)+(buffer[5]<<0));
    
    
//    for(i=0;i<packet1451->length;i++){
//        packet1451->payload[i] = buffer[6+i];
//    }
	
//    for(i=0;i<48;i++){
//        buffer[i] = 0;
//    }
    
    return packet_in;
}

uint8 TII_ReceivePacket(uint8 *buffer, uint8 length){
    uint16_t tam_paquete_tcp = 0;
    // Falta verificar que hace para quedarse escuchando los paquetes recibdos.
//    if( //espera instruccion){
//        set_b_app_recibir_datos();
//    }
//    if( //datos recibidos){ 
//        
//    }
    
//    if(tam_paquete_tcp > 0){
//        memcpy((uint8_t*)buffer,g_tcp_rx_buffer,tam_paquete_tcp);
//        memset(g_tcp_rx_buffer,0,sizeof(g_tcp_rx_buffer));
//        rst_tam_paquete_datos_tcp();
//        return 1; 
//    }
    set_b_app_recibir_datos(); 
    tam_paquete_tcp = get_tam_paquete_recibido_dato_tcp();
    if(tam_paquete_tcp > 0){
        memcpy((uint8_t*)buffer,g_tcp_rx_buffer,tam_paquete_tcp);
        memset(g_tcp_rx_buffer,0,sizeof(g_tcp_rx_buffer));
        rst_tam_paquete_recibido_dato_tcp();
        debug_enviar("TII => Recibido: ");
        debug_enviar((char*)buffer);
        debug_enviar("\n");
        return 1; 
    } 
    return 0;    
}

uint8 TII_SendPacket(uint8 *packet, uint8 packet_len){
    set_b_app_recibir_datos();
    memcpy(g_tcp_tx_buffer,packet,packet_len);
    g_tam_tcp_tx = packet_len;		
    debug_enviar("TII => Enviar: ");
    debug_enviar((char*)packet);
    debug_enviar("\n");
    return 1;
}
// Funcion que envia la senal digitalizada
//void TII_SendSignal(void){
//    
//    uint16 j;
//    uint16 packet_len_signal = 2048;
//    uint8 buffer[2048];
//    
//    
//
//    for(j=0;j<800;j++){
//               buffer[j] = vector_calibracion_Canales[j];
//   }
//       W5100_1_TcpSend(tcpSocket, (uint8 *)buffer,packet_len_signal );
//
//    
//    return;
//}


void TIM_ReadTransducerChannel(cmd_in *pack_1451,cmd_reply * pack_reply_ptr){

    uint8 j=0;
    uint8 temp_packet[8] = {0};
    float txdcr_measurement = 0,txdcr_measurement1 = 0;

		pack_reply_ptr->length = (uint16)0x04; //El tamaño del payload es de 4 bytes.
        /* Se lee el canal */
		if(pack_1451->transd == 0x01){//Se lee la temperatura del aire
		    //WakeComponents();
            //INICIALIZACION2();
			//RTC_EnableInt();
		   	//Encender_5V_Analog();
			//Apagar_5V_Analog();
			//SleepComponents();
			//WakeUp_ISR_Disable();
			//txdcr_measurement = SHT25_Temperatura();
            txdcr_measurement = 0;
			//CyPins_SetPin(LED1_0);
			//WakeUp_ISR_Enable();
        }
		else if(pack_1451->transd == 0x02){//Se lee la humedad del aire
				//WakeUp_ISR_Disable();
				//txdcr_measurement = SHT25_Humedad();
                txdcr_measurement = 0;
				//WakeUp_ISR_Enable();
	        }
		else if(pack_1451->transd == 0x03){//Velocidad
	        	//CyPins_SetPin(LED1_0);
				//WakeUp_ISR_Disable();
				//Principal(viento);
				txdcr_measurement = 0.02;
				txdcr_measurement1 = 0.1;
				//CyPins_SetPin(LED_0);
				//txdcr_measurement = 50;
				pack_reply_ptr->length = (uint16)0x08; //El tamaño del payload es de 8 bytes.
				//WakeUp_ISR_Enable();
	        }	
		else if(pack_1451->transd == 0x04){//Angulo, actual nada
	        	//txdcr_measurement = MEDIR_BATERIA();
				//CyPins_SetPin(LED_0);
				//WakeUp_ISR_Disable();
                //Principal(viento);
				//txdcr_measurement = viento[1];
				//WakeUp_ISR_Enable();
	        }
		else if(pack_1451->transd == 0x05){//Bateria
	        	//txdcr_measurement = MEDIR_BATERIA();
				//CyPins_SetPin(LED_0);
				//WakeUp_ISR_Disable();
				//txdcr_measurement = MEDIR_BATERIA();
                txdcr_measurement =0;
				//WakeUp_ISR_Enable();
	        }
        else if(pack_1451->transd == 0x06){//Calibracion actual tof1 y tof2
	        	//CyPins_SetPin(LED1_0);
				//WakeUp_ISR_Disable();
				//CalibracionCero();
				//txdcr_measurement = viento[0];//velocidad(actual velocidad1)
				//txdcr_measurement1 = viento[1];//angulo(actual velocidad2)
				//CyPins_SetPin(LED_0);
				//txdcr_measurement = 50;
				pack_reply_ptr->length = (uint16)0x08; //El tamaño del payload es de 8 bytes.
				//WakeUp_ISR_Enable();
	        }	

		temp_packet[j] = *((uint8 *)(&txdcr_measurement)+3);
        temp_packet[j+1] = *((uint8 *)(&txdcr_measurement)+2);
        temp_packet[j+2] = *((uint8 *)(&txdcr_measurement)+1);
        temp_packet[j+3] = *((uint8 *)(&txdcr_measurement)+0);
		
		temp_packet[j+4] = *((uint8 *)(&txdcr_measurement1)+3);
        temp_packet[j+5] = *((uint8 *)(&txdcr_measurement1)+2);
        temp_packet[j+6] = *((uint8 *)(&txdcr_measurement1)+1);
        temp_packet[j+7] = *((uint8 *)(&txdcr_measurement1)+0);
    
//		sprintf(bufprueba, "Valor:%3.3f Byte1:%x Byte2:%x Byte3:%x Byte4:%x ", txdcr_measurement,temp_packet[0],temp_packet[1],temp_packet[2],temp_packet[3]);
//		UART_PutString(bufprueba);
//		UART_PutChar(ENTER);
//		UART_PutChar(RETRO);
//		CyDelay(500);
		
    /* Se construye el paquete de respuesta*/
    //Encabezado
    pack_reply_ptr->sf = 0x01;
	
	//La cantidad de bytes se asigna dependiendo del sensor
    
    // Payload
    pack_reply_ptr->payload[0] = temp_packet[0];
    pack_reply_ptr->payload[1] = temp_packet[1];
    pack_reply_ptr->payload[2] = temp_packet[2];
    pack_reply_ptr->payload[3] = temp_packet[3];
	
	pack_reply_ptr->payload[4] = temp_packet[4];
    pack_reply_ptr->payload[5] = temp_packet[5];
    pack_reply_ptr->payload[6] = temp_packet[6];
    pack_reply_ptr->payload[7] = temp_packet[7];

    return;
}

void TII_PackReply(cmd_reply * pack_reply_ptr, uint8  buffer[48]){
    uint8 i;   
    buffer[0] = pack_reply_ptr->sf;
    buffer[1] = (uint8)(pack_reply_ptr->length>>8) & 0xFF;
    buffer[2] = (uint8)(pack_reply_ptr->length>>0) & 0xFF;
    
    for(i=0;i<(pack_reply_ptr->length);i++){
        buffer[i+3] = (uint8)(pack_reply_ptr->payload[i]);
    }
    return;
}

void TIM_ProcessPacket(cmd_in *packet_in){
    debug_enviar("TII => Procesar Paquete");
    debug_enviar("\n");
    /* Estructura que contiene el paquete de respuesta a los pedidos del NCAP */
    cmd_reply pack_reply;
    /* Vector en donde se almacena el paquete de respuesta para enviar por UART */
    uint8 reply[48] = {0};        
    
    /* Se examina el tipo de comando recibido*/
    switch(packet_in->cmd_class){
    /*----------------------------------------------------------------------*/
    /* Comandos comunes al TIM y a los TransducerChannels */
    case COMMON_CMD:
		//CyPins_SetPin(LED1_0);
        switch(packet_in->cmd_func){
        /* Query TEDS */
        case QUERY_TEDS:
			//CyPins_SetPin(LED_0);
            //TEDS_QueryTEDS(&pack_reply,packet_in->payload[0]);
            //TII_PackReply(&pack_reply,reply);
            TII_SendPacket(reply,48);
            break;
        /* Query TEDS Segment */    
        case READ_TEDS_SEGMENT:
            //TEDS_ReadTEDSSEgment(&pack_reply,packet_in->payload[0],0u);
            //TII_PackReply(&pack_reply,reply);
            TII_SendPacket(reply,48);
            break;
        /* Write TEDS Segment */    
        case WRITE_TEDS_SEGMENT:
            //TEDS_WriteTEDSSegment(packet_in->payload[0],packet_in->transd,packet_in->payload);
            break;
        }
    break;
    /*----------------------------------------------------------------------*/    
    /* Comandos correspondientes al estado IDLE */
    case XDCR_IDLE:
        //AddressGroupDefinition();    
    break;   
    /*----------------------------------------------------------------------*/
    /* Comandos correspondientes al estado de funcionamiento */
    case XDCR_OPERATE:
		//CyPins_SetPin(LED_0);
        switch(packet_in->cmd_func){
        case READ_TRANSDUCER_CHANNEL:
            LED_Write(1);
			//CyPins_SetPin(LED1_0);
            TIM_ReadTransducerChannel(packet_in,&pack_reply);
            TII_PackReply(&pack_reply,reply);
//            memset(reply,0,sizeof(reply));
//            for(uint8_t i = 0; i < 48; i++){
//                reply[i] = i+1;
//            }
            TII_SendPacket(reply,48);                       
            break;
        case WRITE_TRANSDUCER_CHANNEL:
			//Prueba de los canales
			//TIM_ReadAllChannels();
            break;  
        case TRANSDUCER_CALIBRATION_SIGNAL:   // Envia la senal de calibracion de los sensores
           // TIM_ReadTransducerChannel(&pack_reply);
            //TII_PackReply(&pack_reply,reply);
            TII_SendPacket(reply,48);  
            break;      
        }
    break;   
    /*----------------------------------------------------------------------*/    
    /* Comandos correspondientes a los estados IDLE y de funcionamiento */
    case XDCR_EITHER:
        switch(packet_in->cmd_func){
        /* Transducer channel Operate */
        case TRANSDUCER_OPERATE:
            //TIM_TransducerChannelOperate();
            break;
        /* Transducer Channel Idle */    
        case TRANSDUCER_IDLE:
			CySoftwareReset();
            //TIM_TransducerChannelIdle();
            break;
        case TRANSDUCER_DIGITAL_SIGNAL:   // Envia la senal digitalizada
            //SenalDigitalizada();
           // TII_SendSignal();
             break;
       
            
            
            
        }
    break;   
    /*----------------------------------------------------------------------*/
    /* Comandos correspondientes al estado SLEEP del TIM */
    case TIM_SLEEP_:
        //WakeUp();
    break;
    /*----------------------------------------------------------------------*/
    /* Comandos correspondientes al estado ACTIVE del TIM */
    case TIM_ACTIVE_:
    
    break;
    /*----------------------------------------------------------------------*/
    /* Comandos correspondientes a cualquier estado del TIM */
    case ANY_STATE:

    /*----------------------------------------------------------------------*/
    /* Comandos para reconfigurar el HW analogico del PSoC */
    case RECONFIG_CMD:
        switch(packet_in->cmd_func){
            case CHANGE_SAMPLING_TIME:
                //TIM_ChangeSamplingTime((uint16)((packet_in->payload[1]<<8) + (packet_in->payload[0]<<0)));
				break;
        }    
    break; 
    }
    return;
}

