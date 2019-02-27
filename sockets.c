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
#include "ESP8266.h"
#include "project.h"
#include "globals.h"


int listen(int puerto,int backlog){
    int8_t sock;
    int8_t ret_val;
    sock = esp8266_crear_servidor_tcp(puerto,backlog);
    if(sock < 0){
        ret_val = -1;
    }else{
        ret_val = sock;
    }
    return ret_val;  
}

int accept (int sockfd){
    int8_t sock;
    int8_t ret_val;
    sock = esp8266_aceptar_clientes_tcp(sockfd);
    if(sock < 0){
        ret_val = -1;
    }else{
        ret_val = sock;
    }
    return sock;
}

int recv(int sockfd, void *buf, int len, unsigned int flags){
    int8_t ret_val = -1;
    uint16_t cant_bytes_recibidos_tcp = 0; 
    uint8_t temp_buffer_rx_tcp[TCP_RX_BUFFER_SIZE]; 
    
    cant_bytes_recibidos_tcp = esp8266_leer_datos_tcp(sockfd,temp_buffer_rx_tcp);
    if(cant_bytes_recibidos_tcp > 0){
        if(cant_bytes_recibidos_tcp > len){
            memcpy(buf,temp_buffer_rx_tcp,len);
        }else{
            memcpy(buf,temp_buffer_rx_tcp,cant_bytes_recibidos_tcp);
        }
        ret_val = 0;
    }else{
        //Error, valores negativos
        ret_val = -1;
    }
    return ret_val;
}

int send(int sockfd, void *msg, int len, int flags){
    int8_t ret_val = -1;
    int8_t valor_retorno_esp8266 = -1;
    uint16_t cant_bytes_recibidos_tcp = 0; 
    uint8_t temp_buffer_rx_tcp[TCP_RX_BUFFER_SIZE]; 
    
    valor_retorno_esp8266 = esp8266_enviar_datos_tcp (sockfd, len, msg);
    if(valor_retorno_esp8266 == 0){
        ret_val = len;
    }else{
        ret_val = -1;
    }
    return ret_val;
}
/* [] END OF FILE */
