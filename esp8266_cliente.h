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
#ifndef ESP8266_CLIENTE_H 
#define ESP8266_CLIENTE_H
    
    void maquina_estados_esp8266_modo_cliente();
    void iniciar_me_esp8266_modo_cliente();
    void (*estado_modulo_esp8266)(void);
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


    void initmeESPCliente();
    
    bool get_estado_wifi_tim();
    bool get_estado_ncap_tim();
    
    bool get_b_cambio_config_ap(void);
    void set_b_cambio_config_ap(void);
    void rst_b_cambio_config_ap(void);
    
    bool get_b_cambio_config_ncap(void);
    void set_b_cambio_config_ncap(void);
    void rst_b_cambio_config_ncap(void);
    
    bool get_b_cambio_config(void);
    void set_b_cambio_config(void);
    void rst_b_cambio_config(void);
    
    bool get_b_app_enviar_datos(void);
    void set_b_app_enviar_datos(void);
    void rst_b_app_enviar_datos(void);
    
    bool get_b_app_recibir_datos(void);
    void set_b_app_recibir_datos(void);
    void rst_b_app_recibir_datos(void);
    
    bool get_b_app_cerrar_socket(void);
    void set_b_app_cerrar_sockets(void);
    void rst_b_app_cerrar_socket(void);    
    
    uint16_t get_tam_paquete_recibido_dato_tcp();
    void rst_tam_paquete_recibido_dato_tcp();
    
#endif

/* [] END OF FILE */
