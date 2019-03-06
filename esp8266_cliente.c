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
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "globals.h"
#include "ESP8266.h"
#include "ip.h"
#include "servidor_web.h"
#include "servidor_web_banderas.h"
#include "eeprom.h"
#include "interfaz_debug.h"
#include "TII.h"

/*Bandera para indicar si el TIM se encuentra conectada a la red WiFi*/
static bool b_estado_wifi_tim = false;

/*Bandera para indicar si el TIM se encuentra conectado al servidor.*/
static bool b_estado_ncap_tim = false; 

static bool b_cambio_config_ap = false; 
static bool b_cambio_config_ncap = false;
static bool b_cambio_config = false; 

static bool b_app_enviar_datos = false; 
static bool b_app_recibir_datos = false; 
static bool b_app_cerrar_socket = false; 

static uint16_t tam_paquete_recibido_dato_tcp = 0;

/*
 * Timeouts. Múltiplo de 2 ms.
 */
#define TIMEOUT_RECONEXION_WIFI 150000  // Tiempo de espera para recon = 5 minutos.
#define TIMEOUT_RECONEXION_SERVIDOR 90000 // Tiempo de espera para recon = 3 minutos.
static volatile uint32_t contador_espera_reconexion = 0;

/*Declaraciones de los estados*/
void estado_encendido(void);
void estado_leer_datos_tim(void);
void estado_leer_datos_tim_servidor(void);
void estado_espera_datos_portal_web(void);
void estado_conectar_wifi(void);
void estado_espera_cambios_datos_ap(void);
void estado_espera_reconexion_wifi(void);
void estado_conectar_servidor_tcp(void);
void estado_espera_cambios_servidor(void);
void estado_espera_reconexion_servidor(void);
void estado_espera_instruccion_aplicacion(void);
void estado_escribir_datos_tcp(void);
void estado_leer_datos_tcp(void);
void estado_cerrar_socket_tcp(void);

/*Declaracion del puntero a funcion*/
void (*estado_modulo_esp8266)(void);

static int8_t respuesta_esp;
static struct configuracion configuracion_TIM;
static char buffer_debug[128];
static char dirIP[20];
volatile static int8_t sock; 

void estado_encendido(void){
    debug_enviar("ME-ESP8266 => Estado: Verificar encendido.");
    debug_enviar("\n");
    gb_socket_listo = 0;
    respuesta_esp = esp8266_verificar_encedido();
    if(respuesta_esp == 0){
        estado_modulo_esp8266 = estado_leer_datos_tim; 
    }else{
        estado_modulo_esp8266 = estado_encendido;
    }    
}

void estado_leer_datos_tim(void){
    uint16_t crc;
    debug_enviar("ME-ESP8266 => Estado: Leer datos TIM.");
    debug_enviar("\n");
    if (leeConfigEEPROM (&configuracion_TIM, sizeof(configuracion_TIM), &crc) == 0)
    {
        IP2ASCII(configuracion_TIM.dirIPNCAP,dirIP);
        sprintf(buffer_debug,"Datos leidos EEPROM => SSID:%s\tPSW:%s\tIP:%s\tPUERTO:%d",
            configuracion_TIM.SSID_AP,configuracion_TIM.pwdAP,dirIP,
            configuracion_TIM.puertoNCAP);
        debug_enviar(buffer_debug);
        debug_enviar("\n");
		// correcto
//        if(b_cambio_config == 1){
//            estado_modulo_esp8266 = estado_leer_datos_tim; 
//        }else{
//            estado_modulo_esp8266 = estado_conectar_wifi;
//        }
        estado_modulo_esp8266 = estado_conectar_wifi;
	} 
    else 
    {
        debug_enviar("ME-ESP8266 => Estado: Leer datos TIM, error al leer de la EEPROM,");
        debug_enviar("\n");
        estado_modulo_esp8266 = estado_espera_datos_portal_web;
	}
    b_cambio_config = 0;
}

void estado_espera_datos_portal_web(){
    debug_enviar("ME-ESP8266 => Estado: Espera datos portal WEB.");
    debug_enviar("\n");
    if(b_cambio_config == 1){
        estado_modulo_esp8266 = estado_leer_datos_tim;
    }else{
        estado_modulo_esp8266 = estado_espera_datos_portal_web;
    } 
}

void estado_conectar_wifi(void){
    debug_enviar("ME-ESP8266 => Estado: Conectar WiFi.");
    debug_enviar("\n");
    b_estado_wifi_tim = false;
    gb_socket_listo = 0;
    respuesta_esp = esp8266_conectar_estacion(configuracion_TIM.SSID_AP,
        configuracion_TIM.pwdAP);
    if(b_cambio_config == 1)
    {
        estado_modulo_esp8266 = estado_leer_datos_tim; 
    }
    else
    {
        if(respuesta_esp == 0)
        {
            b_estado_wifi_tim = true;
            estado_modulo_esp8266 = estado_conectar_servidor_tcp; 
        } 
        if(respuesta_esp == -2)
        {
            estado_modulo_esp8266 = estado_espera_reconexion_wifi;
            contador_espera_reconexion = 0; 
        }
        if( (respuesta_esp == -3) || (respuesta_esp == -4) ){
            estado_modulo_esp8266 = estado_espera_datos_portal_web;
        }
    }   
}

void estado_espera_reconexion_wifi(){
    debug_enviar("ME-ESP8266 => Estado: Espera reconexion WiFi.");
    debug_enviar("\n");
    if(b_cambio_config == true)
    {
        estado_modulo_esp8266 = estado_leer_datos_tim;
    }
    else
    {
        if(contador_espera_reconexion > TIMEOUT_RECONEXION_WIFI)
        {
            estado_modulo_esp8266 = estado_conectar_wifi;
        }else
        {
            estado_modulo_esp8266 = estado_espera_reconexion_wifi;
        } 
    }
} 

void estado_conectar_servidor_tcp(void){
    debug_enviar("ME-ESP8266 => Estado: Conectar Servidor.");
    debug_enviar("\n");
    b_estado_ncap_tim = false;
    IP2ASCII(configuracion_TIM.dirIPNCAP,dirIP);
    sock = esp8266_conectar_servidor_tcp(dirIP,configuracion_TIM.puertoNCAP);
    if(b_cambio_config == true){
        estado_modulo_esp8266 = estado_leer_datos_tim;
    }
    else
    {
        if(sock >= 0)
        {
            b_estado_ncap_tim = true;
            gb_socket_listo = 1;
            estado_modulo_esp8266 = estado_espera_instruccion_aplicacion;
        } 
        if(sock == -1)
        {
            estado_modulo_esp8266 = estado_conectar_servidor_tcp;
        }
        if(sock == -2)
        {
            estado_modulo_esp8266 = estado_conectar_wifi;
        }
        if(sock == -4)
        {
            estado_modulo_esp8266= estado_espera_reconexion_servidor;
            contador_espera_reconexion = 0;
        }
    } 
}

void estado_espera_reconexion_servidor(){
    debug_enviar("ME-ESP8266 => Estado: Espera reconexion Servidor.");
    debug_enviar("\n");
    if(b_cambio_config == true)
    {
        estado_modulo_esp8266 = estado_leer_datos_tim;
    }
    else
    {
        if(contador_espera_reconexion > TIMEOUT_RECONEXION_SERVIDOR)
        {
            estado_modulo_esp8266 = estado_conectar_servidor_tcp;
        }else
        {
            estado_modulo_esp8266 = estado_espera_reconexion_servidor;
        }
    }
}

void estado_espera_instruccion_aplicacion(void){
    debug_enviar("ME-ESP8266 => Estado: Espera instruccion aplicacion.");
    debug_enviar("\n");
    if(b_cambio_config == 1)
    {
        estado_modulo_esp8266 = estado_leer_datos_tim;
    }
    else
    {
        estado_modulo_esp8266 = estado_espera_instruccion_aplicacion;
        if(b_app_enviar_datos == 1)
        {
            estado_modulo_esp8266 = estado_escribir_datos_tcp;
        }
        if(b_app_recibir_datos == 1)
        {
            estado_modulo_esp8266 = estado_leer_datos_tcp;
        }
        if(b_app_cerrar_socket == 1)
        {
            estado_modulo_esp8266 = estado_cerrar_socket_tcp;
        }
    }
}

void estado_escribir_datos_tcp(void){
    debug_enviar("ME-ESP8266 => Estado: Escribir datos TCP.");
    debug_enviar("\n");
    /*Se verifica que la aplicacion haya solicitado enviar datos*/
    respuesta_esp = esp8266_enviar_datos_tcp(sock,g_tam_tcp_tx,(char*)g_tcp_tx_buffer);
    b_app_enviar_datos = 0;
    
    rst_b_paquete_tim_recibido();
    rst_b_paquete_tim_separado();
    rst_b_paquete_tim_procesado();
    
    if(b_cambio_config == 0)
    {
        if(respuesta_esp == -2)
        {
            estado_modulo_esp8266 = estado_conectar_servidor_tcp; 
        }
        if(respuesta_esp == 0)
        {
            estado_modulo_esp8266 = estado_espera_instruccion_aplicacion; 
        }
    }else{
        estado_modulo_esp8266 = estado_leer_datos_tim;
    } 
}

void estado_leer_datos_tcp(void){
    int16_t cant_bytes_tcp_recibidos;
    debug_enviar("ME-ESP8266 => Estado: Leer datos TCP.");
    debug_enviar("\n");
    cant_bytes_tcp_recibidos  = esp8266_leer_datos_tcp(sock,&g_tcp_rx_buffer[0]);
    sprintf(buffer_debug,"%d",cant_bytes_tcp_recibidos);
    debug_enviar("ME-ESP8266 => Estado: Leer datos TCP => Tam paquete: ");
    debug_enviar(buffer_debug);
    debug_enviar("\n");
    b_app_recibir_datos = false;
    if(b_cambio_config == 0)
    {
        if(cant_bytes_tcp_recibidos >= 0)
        {
            tam_paquete_recibido_dato_tcp = cant_bytes_tcp_recibidos;
            estado_modulo_esp8266 = estado_espera_instruccion_aplicacion; 
        }
        if(cant_bytes_tcp_recibidos == -3)
        {
            estado_modulo_esp8266 = estado_conectar_servidor_tcp; 
        }
    }else{
        estado_modulo_esp8266 = estado_leer_datos_tim;
    } 
}

void estado_cerrar_socket_tcp(void){
    debug_enviar("ME-ESP8266 => Estado: Cerrar coneccion servidor.");
    debug_enviar("\n");
    respuesta_esp = esp8266_cierra_socket_tcp(sock);
    b_app_cerrar_socket = 0;
    if(b_cambio_config == 0)
    {
        if(respuesta_esp == 0)
        {
            estado_modulo_esp8266 = estado_espera_instruccion_aplicacion; 
        }
    }else{
        estado_modulo_esp8266 = estado_leer_datos_tim;
    } 
}

CY_ISR(contador_espera_reconexion_ISR){
    Timer_espera_reconexion_ReadStatusRegister();
    contador_espera_reconexion++;
    isr_timer_reconexion_ClearPending();
    return; 
}

void iniciar_me_esp8266_modo_cliente(){
    estado_modulo_esp8266 = &estado_encendido;
    isr_timer_reconexion_StartEx(contador_espera_reconexion_ISR);
    Timer_espera_reconexion_Start();
    contador_espera_reconexion = 0;
}

bool get_estado_wifi_tim(){
    return b_estado_wifi_tim;
} 

bool get_estado_ncap_tim(){
    return b_estado_ncap_tim;
} 

    /*Funciones que manejan la bandera b_cambio_config_ap*/
bool get_b_cambio_config_ap(void){
    return b_cambio_config_ap;
}

void set_b_cambio_config_ap(void){
    b_cambio_config_ap = true;
}

void rst_b_cambio_config_ap(void){
    b_cambio_config_ap = false;
}

    /*Funciones que manejan la bandera b_cambio_config_ncap*/
bool get_b_cambio_config_ncap(void){
    return b_cambio_config_ncap;
}

void set_b_cambio_config_ncap(void){
    b_cambio_config_ncap = true;
}

void rst_b_cambio_config_ncap(void){
    b_cambio_config_ncap = false;
}

    /*Funciones que manejan la bandera b_cambio_config*/
bool get_b_cambio_config(void){
    return b_cambio_config;
}

void set_b_cambio_config(void){
    b_cambio_config = true;
}

void rst_b_cambio_config(void){
    b_cambio_config = false;
}

    /*Funciones que manejan la bandera b_app_enviar_datos*/
bool get_b_app_enviar_datos(void){
    return b_app_enviar_datos;
}

void set_b_app_enviar_datos(void){
    b_app_enviar_datos = true;
}

void rst_b_app_enviar_datos(void){
    b_app_enviar_datos = false;
}

    /*Funciones que manejan la bandera b_app_recibir_datos*/
bool get_b_app_recibir_datos(void){
    return b_app_recibir_datos;
}

void set_b_app_recibir_datos(void){
    b_app_recibir_datos = true;
}

void rst_b_app_recibir_datos(void){
    b_app_recibir_datos = false;
}

    /*Funciones que manejan la bandera b_app_cerrar_socket*/
bool get_b_app_cerrar_socket(void){
    return b_app_cerrar_socket;
}

void set_b_app_cerrar_sockets(void){
    b_app_cerrar_socket = true;
}

void rst_b_app_cerrar_socket(void){
    b_app_cerrar_socket = false;
}

uint16_t get_tam_paquete_recibido_dato_tcp(){
    return tam_paquete_recibido_dato_tcp;
}

void rst_tam_paquete_recibido_dato_tcp(){
    tam_paquete_recibido_dato_tcp = 0;
}
/* [] END OF FILE */
