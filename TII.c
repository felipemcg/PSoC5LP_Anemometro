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
#include "Datatype.h"
#include "ESP8266.h"
#include "globals.h"
#include "puerto_serial.h"
#include "esp8266_cliente.h"
#include "interfaz_debug.h"

bool b_me_esp8266_datos_recibidos = false; 
bool b_me_esp8266_datos_enviados = false;  

//Bandera que indica que se recibio un paquete TIM a traves del socket TCP. 
bool b_paquete_tim_recibido = false;

//Bandera que indica que el paquete TIM fue separado en sus campos correspondientes.
bool b_paquete_tim_separado = false; 

//Bandera que indica que el paquete TIM fue procesado.
bool b_paquete_tim_procesado = false;

/*
Funcion que se encarga de recibir un paquete proveniente del NCAP a traves del socket
TCP que proporciona el ESP8266 a traves de la maquina de estados en el archivo 
esp8266_cliente.c.  
Parametros:
    *buffer -> Puntero al array donde se almacena el paquete recibido.  
Retorno: 
    0   -> L.
*/
uint8 TII_ReceivePacket(uint8 *buffer, uint8 length){
    uint16_t tam_paquete_tcp = 0;
    //Se verifica que no se este procesando un paquete, antes de recibir otro. 
    if(b_paquete_tim_recibido  == false)
    {
        set_b_app_recibir_datos(); 
        tam_paquete_tcp = get_tam_paquete_recibido_dato_tcp();
        
        //Se verifica si hay datos disponibles en el socket. 
        if(tam_paquete_tcp > 0)
        {
            rst_b_app_recibir_datos();
            
            memcpy((uint8_t*)buffer,g_tcp_rx_buffer,tam_paquete_tcp);
            memset(g_tcp_rx_buffer,0,sizeof(g_tcp_rx_buffer));
            rst_tam_paquete_recibido_dato_tcp();
            
            debug_enviar("TII => Recibido: ");
            debug_enviar((char*)buffer);
            debug_enviar("\n");
            
            b_paquete_tim_recibido = true;
            return 1; 
        }
    }
    return 0;    
}

/*
Funcion que separa el paquete crudo recibido de manera tal a construir el paquete de
mensaje de comando 1451 segun como se define en la tabla 12 de la seccion 6.2 del 
documento 1451.0.pdf. 
Parametros:
    *buffer -> Puntero al array donde se encuentra almacenado el paquete recibido.
    Obs: La funcion TII_ReceivePacket() se encarga de cargar el array que debe recibir
    esta funcion.
Retorno: 
    packet_in   -> Se retorna el paquete 1451.  
*/
cmd_in TIM_GetPacket(uint8_t *buffer){
    cmd_in packet_in; 
   
    debug_enviar("TII => Obtener paquete");
    debug_enviar("\n");
    
    packet_in.transd = (uint16)((buffer[0]<<8)+(buffer[1]<<0));
    packet_in.cmd_class = buffer[2];
    packet_in.cmd_func = buffer[3];
    packet_in.length = (uint16)((buffer[4]<<8)+(buffer[5]<<0));
    for(int i=0;i<packet_in.length;i++){
        packet_in.payload[i] = buffer[6+i];
    }
    
    return packet_in; 
}

uint8 TII_SendPacket(uint8 *packet, uint8 packet_len){
    if(get_b_app_cerrar_socket() || get_b_app_recibir_datos()){
       return 0; 
    }else{
        set_b_app_enviar_datos();
        memcpy(g_tcp_tx_buffer,packet,packet_len);
        g_tam_tcp_tx = packet_len;		
        debug_enviar("TII => Enviar: ");
        debug_enviar((char*)packet);
        debug_enviar("\n"); 
        return 1;
    }
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


void TIM_ReadTransducerChannel(cmd_in *pack_1451, cmd_reply * pack_reply_ptr){

    uint8 j=0;
    uint8 temp_packet[8] = {0};
    float txdcr_measurement = 0,txdcr_measurement1 = 0;

		pack_reply_ptr->length = (uint16)0x04; //El tamaño del payload es de 4 bytes.
        /* Se lee el canal */
		if(pack_1451->transd == 0x01){//Se lee la temperatura del aire
            txdcr_measurement = 0;
        }
		else if(pack_1451->transd == 0x02){//Se lee la humedad del aire
                txdcr_measurement = 0;
	        }
		else if(pack_1451->transd == 0x03){//Velocidad
				txdcr_measurement = 0.05;
				//txdcr_measurement1 = 0.1;
				/*Principal(viento);
				txdcr_measurement = viento[0];
				txdcr_measurement1 = viento[1];*/
				pack_reply_ptr->length = (uint16)0x04; 
	        }	
		else if(pack_1451->transd == 0x04){//Angulo, actual nada
	        	/*Principal(viento);
				txdcr_measurement = viento[1];*/
                txdcr_measurement = 86.1;
                pack_reply_ptr->length = (uint16)0x04; 
	        }
		else if(pack_1451->transd == 0x05){//Bateria
                txdcr_measurement =0;
	        }
        else if(pack_1451->transd == 0x06){//Calibracion actual tof1 y tof2
				/*CalibracionCero();
				txdcr_measurement = viento[0];//velocidad(actual velocidad1)
				txdcr_measurement1 = viento[1];//angulo(actual velocidad2)*/
				pack_reply_ptr->length = (uint16)0x08; //El tamaño del payload es de 8 bytes.
	        }	

	temp_packet[j] = *((uint8 *)(&txdcr_measurement)+3);
    temp_packet[j+1] = *((uint8 *)(&txdcr_measurement)+2);
    temp_packet[j+2] = *((uint8 *)(&txdcr_measurement)+1);
    temp_packet[j+3] = *((uint8 *)(&txdcr_measurement)+0);
	
	temp_packet[j+4] = *((uint8 *)(&txdcr_measurement1)+3);
    temp_packet[j+5] = *((uint8 *)(&txdcr_measurement1)+2);
    temp_packet[j+6] = *((uint8 *)(&txdcr_measurement1)+1);
    temp_packet[j+7] = *((uint8 *)(&txdcr_measurement1)+0);
    	
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

void TII_PackReply(cmd_reply * pack_reply_ptr, uint8  *buffer){
    uint8 i;   
    buffer[0] = pack_reply_ptr->sf;
    buffer[1] = (uint8)(pack_reply_ptr->length>>8) & 0xFF;
    buffer[2] = (uint8)(pack_reply_ptr->length>>0) & 0xFF;
    
    for(i=0;i<(pack_reply_ptr->length);i++){
        buffer[i+3] = (uint8)(pack_reply_ptr->payload[i]);
    }
    return;
}

/*
Funcion que procesa el comando 1451 recibido, y realiza las acciones correspondientes. 
Parametros:
    *buffer -> Puntero al array donde se encuentra almacenado el paquete recibido.
    Obs: La funcion TII_ReceivePacket() se encarga de cargar el array que debe recibir
    esta funcion.
Retorno: 
    packet_in   -> Se retorna el paquete 1451.  
*/
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

void TIM_Process(void){
    uint8_t paquete_TIM_recibido[20];
    cmd_in packet1451_in;
    
    
    if(b_paquete_tim_recibido == false){
        memset(paquete_TIM_recibido,0,sizeof(paquete_TIM_recibido));
        //Se recibe un paquete del NCAP. 
        TII_ReceivePacket(&paquete_TIM_recibido[0],0);
    }
    if(b_paquete_tim_recibido == true && b_paquete_tim_separado == false)
    {
        //Se separa el paquete crudo para construir el paquete 1451. 
        packet1451_in = TIM_GetPacket(&paquete_TIM_recibido[0]);
        b_paquete_tim_separado = true;
    }
    if(b_paquete_tim_recibido == true && b_paquete_tim_separado == true 
        && b_paquete_tim_procesado == false)
    {
        /*Se procesa el paquete 1451*/
        TIM_ProcessPacket(&packet1451_in);
        b_paquete_tim_procesado = true;
    }
    
    return; 
} 
    

void rst_b_paquete_tim_recibido(void){
    b_paquete_tim_recibido = false;
}

void rst_b_paquete_tim_separado(void){
    b_paquete_tim_separado = false;
}

void rst_b_paquete_tim_procesado(void){
    b_paquete_tim_procesado = false;
}
/* [] END OF FILE */

