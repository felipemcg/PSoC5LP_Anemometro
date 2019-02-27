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
#ifndef ESP8266_H 
#define ESP8266_H
    
    #include "project.h"
    
    /*Definicion de los comandos aceptados por el modulo*/
    #define CMD_VERIFICAR_ENCENDIDO "MIS\n"
    #define CMD_REINICIAR_MODULO "MRS\n"
    #define CMD_CONFIGURAR_UART "MUC,%lu\n"
    
    #define CMD_ESCANEAR_AP "WFS\n"
    #define CMD_WIFI_MODO "WFM,%d\n"
    #define CMD_CONFIGURAR_ESTACION "WCF,%s,%s,%s,%s\n"
    #define CMD_CONECTAR_AP "WFC,%s,%s\n"
    #define CMD_CONFIGURAR_SOFTAP "WAC,%s,%s,%s\n"
    #define CMD_CREAR_SOFTAP "WFA,%s,%s,%d,%d,%d\n"

    #define CMD_CONECTAR_SERVIDOR_TCP "CCS,%s,%d\n"
    #define CMD_CREAR_SERVIDOR_TCP "SLC,%d,%d\n"
    #define CMD_ACEPTAR_CLIENTES_TCP "SAC,%d\n"
    #define CMD_ENVIAR_DATOS_TCP "SOW,%d,%d,"
    #define CMD_RECIBIR_DATOS_TCP "SOR,%d\n"
    #define CMD_CERRAR_SOCKET_TCP "SOC,%d\n"
    
    /*Estructura de datos utilizada para la funcion escanear_estaciones*/
    struct elementos_punto_acceso{
        char ssid[16];
        int16_t rssi; 
    };
    
    /*Funciones para el manejo del dispositivo*/
    int8_t esp8266_verificar_encedido();
    int8_t esp8266_reiniciar_modulo();
    int8_t esp8266_configurar_uart(uint32_t baud_rate);

    /*Funciones configuracion WiFi*/
    int8_t esp8266_escanear_estaciones(struct elementos_punto_acceso aps);
    int8_t esp8266_wifi_modo(uint8_t modo_wifi);
    int8_t esp8266_configurar_estacion(char *ip, char *dns, char* gateway, char* subnet);
    int8_t esp8266_conectar_estacion(char *ssid, char *pass);
    int8_t esp8266_configurar_softAP(char *ip, char* gateway, char* subnet);
    int8_t esp8266_crear_softAP(char *ssid, char* pass, uint8_t canal, uint8_t oculto, uint8_t cant_dispositivos);

    /*Funciones protocolo TCP*/
    int8_t esp8266_conectar_servidor_tcp(char *IP, uint16_t puerto);
    int8_t esp8266_enviar_datos_tcp(uint8_t socket, uint16_t cant_bytes, char *data);
    int16_t esp8266_leer_datos_tcp(uint8_t socket, uint8_t *buffer);
    int8_t esp8266_cierra_socket_tcp(uint8_t socket);
    int8_t esp8266_crear_servidor_tcp(uint16_t puerto, uint8_t cant_clientes);
    int8_t esp8266_aceptar_clientes_tcp(uint8_t socket);
#endif
/* [] END OF FILE */