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
#include "globals.h"
#include "ESP8266.h"
#include "puerto_serial.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

uint8_t buffer_serial_tx[SERIAL_BUFFER_SIZE]; 
uint8_t buffer_serial_rx[SERIAL_BUFFER_SIZE];

struct elementos_punto_acceso puntos_acceso[16];
/*
Funcion que verifica que el modulo ESP8266 esta encendido y funcionando. 
Parametros:
    Ninguno. 
Retorno: 
    0   -> No hay error, el modulo esta funcionando.
    Posibles errores(unicamente valores negativos). 
    -1  -> No se recibe respuesta del modulo.
*/
int8_t esp8266_verificar_encedido(){
    int8_t valor_retorno = -1; 
    uint8_t cmd_respuesta; 
    uint16_t cant_bytes_recibidos = 0;
    
    sprintf((char *)buffer_serial_tx,CMD_VERIFICAR_ENCENDIDO); 
    
    uart_enviar_datos((char *)buffer_serial_tx,strlen((char *)buffer_serial_tx));
    
    cant_bytes_recibidos = uart_leer_datos(&buffer_serial_rx[0]); 
    
    if(cant_bytes_recibidos > 0){
        cmd_respuesta = buffer_serial_rx[0];
        if(cmd_respuesta == CMD_RESP_OK){
            valor_retorno = 0; 
        }
    }
    return valor_retorno;
}

/*
Funcion que reinicia el modulo ESP8266. Cuando esto ocurre, se envian datos 
no revelantes por el UART, deben ser ignorados hasta recibir el caracter R,
lo cual indica que el modulo esta listo para recibir comandos. 
Parametros:
    Ninguno. 
Retorno: 
    0   -> El modulo fue reiniciado correctamente y esta listo
    para recibir comandos.
    Posibles errores(unicamente valores negativos). 
    -1  -> No se recibe respuesta del modulo.
*/
int8_t esp8266_reiniciar_modulo()
{
    int8_t valor_retorno = -1; 
    uint8_t cmd_respuesta; 
    uint16_t cant_bytes_recibidos = 0;
    
    sprintf((char *)buffer_serial_tx,CMD_REINICIAR_MODULO); 
    
    uart_enviar_datos((char *)buffer_serial_tx,strlen((char *)buffer_serial_tx));
    
    cant_bytes_recibidos = uart_leer_datos(&buffer_serial_rx[0]); 
    
    if(cant_bytes_recibidos > 0)
    {
        cmd_respuesta = buffer_serial_rx[cant_bytes_recibidos];
        if(cmd_respuesta == CMD_START_OK)
        {
            valor_retorno = 0; 
        }
        else
        {
        	valor_retorno = -1;
        } 
    }
    return valor_retorno;
}

/*
Funcion que cambia la tasa de transferencia del periferico UART del modulo. 
Parametros:
    baud_rate -> Nueva tasa de transferecia a ser utilizada para la 
    comunicacion a traves del UART. 
Retorno: 
    0   -> La tasa de transferencia fue modificada con exito. Luego de 
    recibir esta respuesta, se debe esperar al menos 5 milisegundos 
    antes de enviar otro comando.
    Posibles errores(unicamente valores negativos). 
    -1  -> Tasa de transferencia fuera de rango.
*/
int8_t esp8266_configurar_uart(uint32_t baud_rate)
{
    int8_t valor_retorno = -1; 
    uint8_t cmd_respuesta; 
    uint16_t cant_bytes_recibidos = 0;
    
    sprintf((char *)buffer_serial_tx,CMD_CONFIGURAR_UART,baud_rate); 
    
    uart_enviar_datos((char *)buffer_serial_tx,strlen((char *)buffer_serial_tx));
    
    cant_bytes_recibidos = uart_leer_datos(&buffer_serial_rx[0]); 
    
    if(cant_bytes_recibidos > 0)
    {
        cmd_respuesta = buffer_serial_rx[0];
        if(cmd_respuesta == CMD_RESP_OK)
        {
            valor_retorno = 0; 
        }
        else
        {
        	valor_retorno = -1;
        } 
    }
    return valor_retorno;
}

            /*========================================
              Funciones referente a la conexion WiFi
            *========================================*/

/*            
Funcion que escanea los puntos de acceso al alcance del modulo.  
Parametros:
    TODO: Pasar como parametro una estructura o un array de manera tal
    a que la aplicacion disponga de los nombres y la potencia de los 
    puntos de acceso. 
Retorno: 
    0   -> No hay error, el modulo esta funcionando. 
    Posibles errores(unicamente valores negativos).
    -1  -> No se recibe respuesta del modulo.
*/
int8_t esp8266_escanear_estaciones(struct elementos_punto_acceso aps){
    int8_t valor_retorno = -1;
    uint8_t cmd_respuesta; 
    uint16_t cant_bytes_recibidos = 0;
    uint8_t cant_puntos_acceso_encontrados; 
    
    sprintf((char *)buffer_serial_tx,CMD_ESCANEAR_AP); 
    
    uart_enviar_datos((char *)buffer_serial_tx,strlen((char *)buffer_serial_tx));
    
    cant_bytes_recibidos = uart_leer_datos(&buffer_serial_rx[0]);
    
    if (cant_bytes_recibidos > 0){
        cmd_respuesta = buffer_serial_rx[0];
        switch(cmd_respuesta){
            case CMD_RESP_OK:
                //cant_puntos_acceso_encontrados = buffer_serial_rx[2] 
                valor_retorno = 0; 
                break;
            case CMD_ERROR_1:
                valor_retorno = -1;
            default: 
                break;
        }
    }
    return valor_retorno;
}

/*            
Funcion que cambia el modo WiFi del modulo.  
Parametros:
    modo_wifi -> Valor postivio del 0 al 3. 
        0 -> WiFi Apagado. 
        1 -> WiFi modo estacion. 
        2 -> WiFi modo Punto de acceso. 
        3 -> WiFi modo estacion + punto de acceso.                   
Retorno: 
    0   -> El modo fue establecido correctamente.  
    Posibles errores(unicamente valores negativos).
    -1  -> Parametro modo_wifi fuera de rango.
    -2  -> No se pudo establecer la configuracion deseada.
*/
int8_t esp8266_wifi_modo(uint8_t modo_wifi){
    int8_t valor_retorno = -1;
    uint8_t cmd_respuesta; 
    uint16_t cant_bytes_recibidos = 0;
    
    sprintf((char *)buffer_serial_tx,CMD_WIFI_MODO,modo_wifi); 
    
    uart_enviar_datos((char *)buffer_serial_tx,strlen((char *)buffer_serial_tx));
    
    cant_bytes_recibidos = uart_leer_datos(&buffer_serial_rx[0]);
    
    if (cant_bytes_recibidos > 0){
        cmd_respuesta = buffer_serial_rx[0];
        switch(cmd_respuesta){
            case CMD_RESP_OK:
                valor_retorno = 0; 
                break;
            case CMD_ERROR_1:
                valor_retorno = -1;
            case CMD_ERROR_2:
                valor_retorno = -2;
            default: 
                break;
        }
    }
    return valor_retorno;
}

/*
Funcion que configura la interfaz de estacion antes de conectarse a un punto de acceso.  
Parametros:
    ip      -> Direccion IP del modulo ESP8266. 
    dns     -> Direccion del servidor DNS para el modulo ESP8266. 
    gateway -> Direccion de la puerta de enlace para el modulo ESP8266. 
    subnet  -> Direccion de mascara de red para el modulo ESP8266.
Retorno: 
    0   -> No hay error, el modulo esta funcionando. 
    Posibles errores(unicamente valores negativos).
    -1  -> No se recibe respuesta del modulo.
    -2  -> Se supero el tiempo de espera para la conexion.
    -3  -> Contraseña Incorrecta.
    -4  -> No se encuentra/no esta disponible el SSID
    -5  -> Escaneo completo pero no se conecto.
*/
int8_t esp8266_configurar_estacion(char *ip, char *dns, char* gateway, char* subnet)
{
    int8_t valor_retorno = -1;
    uint8_t cmd_respuesta; 
    uint16_t cant_bytes_recibidos = 0;
    
    sprintf((char *)buffer_serial_tx,CMD_CONFIGURAR_ESTACION,ip,dns,gateway,subnet); 
    
    uart_enviar_datos((char *)buffer_serial_tx,strlen((char *)buffer_serial_tx));
    
    cant_bytes_recibidos = uart_leer_datos(&buffer_serial_rx[0]);
    
    if (cant_bytes_recibidos > 0)
    {
        cmd_respuesta = buffer_serial_rx[0];
        switch(cmd_respuesta)
        {
            case CMD_RESP_OK:
                valor_retorno = 0; 
                break;
            case CMD_ERROR_1:
                valor_retorno = -1;
                break;
            case CMD_ERROR_2:
                valor_retorno = -2;
                break;
            case CMD_ERROR_3: 
                valor_retorno = -3;
                break;
            case CMD_ERROR_4: 
                valor_retorno = -4;
                break;
            case CMD_ERROR_5: 
                valor_retorno = -5;
                break;
            default: 
                break;
        }
    }
    return valor_retorno;
}

/*
Funcion que conecta el ESP8266 a un punto de acceso.  
Parametros:
    ssid -> Nombre del punto de acceso al cual se quiere conectar. 
    pass -> Contraseña del punto de acceso al cual se quiere conectar. 
Retorno: 
    0   -> No hay error, el modulo esta funcionando. 
    Posibles errores(unicamente valores negativos).
    -1  -> No se recibe respuesta del modulo.
    -2  -> Se supero el tiempo de espera para la conexion.
    -3  -> Contraseña incorrecta.
    -4  -> No se encuentra/no esta disponible el SSID
    -5  -> Escaneo completo pero no se conecto.
    -6  -> Se perdio la conexion.
    -7  -> La radio se encuentra en reposo.
*/
int8_t esp8266_conectar_estacion(char *ssid, char *pass){
    int8_t valor_retorno = -1;
    uint8_t cmd_respuesta; 
    uint16_t cant_bytes_recibidos = 0;
    
    sprintf((char *)buffer_serial_tx,CMD_CONECTAR_AP,ssid,pass); 
    
    uart_enviar_datos((char *)buffer_serial_tx,strlen((char *)buffer_serial_tx));
    
    cant_bytes_recibidos = uart_leer_datos(&buffer_serial_rx[0]);
    
    if (cant_bytes_recibidos > 0){
        cmd_respuesta = buffer_serial_rx[0];
        switch(cmd_respuesta){
            case CMD_RESP_OK:
                valor_retorno = 0; 
                break;
            case CMD_ERROR_1:
                valor_retorno = -1;
                break;
            case CMD_ERROR_2:
                valor_retorno = -2;
                break;
            case CMD_ERROR_3: 
                valor_retorno = -3;
                break;
            case CMD_ERROR_4: 
                valor_retorno = -4;
                break;
            case CMD_ERROR_5: 
                valor_retorno = -5;
                break;
            case CMD_ERROR_6: 
                valor_retorno = -6;
                break;
            case CMD_ERROR_7: 
                valor_retorno = -7;
                break;
            default: 
                break;
        }
    }
    return valor_retorno;
}

/*
Funcion que conecta el ESP8266 a un servidor remoto TCP.  
Parametros:
    ip  -> Direccion IP del servidor al cual se quiere conectar. 
    port-> Puerto en el cual se encuentra escuchando el servidor. 
Retorno: 
    [0..4] -> La conexion se realizo con exito, se retorna el socket asignado,
    puede ser del 0 al 4.
    Posibles errores(unicamente valores negativos).
    -1  -> El numero de puerto esta fuera de rango.
    -2  -> WiFi desconectado.
    -3  -> No hay socket disponible para crear la conexion.
    -4  -> Error al conectar al servidor.
*/
int8_t esp8266_conectar_servidor_tcp(char *IP, uint16_t puerto){
    int8_t valor_retorno = -5;
    uint8_t cmd_respuesta; 
    uint16_t cant_bytes_recibidos = 0;
    
    sprintf((char *)buffer_serial_tx,CMD_CONECTAR_SERVIDOR_TCP,IP,puerto);
    
    uart_enviar_datos((char *)buffer_serial_tx,strlen((char *)buffer_serial_tx));
    
    cant_bytes_recibidos = uart_leer_datos(&buffer_serial_rx[0]);
    
    if (cant_bytes_recibidos > 0){
        cmd_respuesta = buffer_serial_rx[0];
        switch(cmd_respuesta){
            case CMD_RESP_OK: 
                valor_retorno = buffer_serial_rx[2] - '0';
                break;
            case CMD_ERROR_1:
                valor_retorno = -1;
                break;
            case CMD_ERROR_2:
                valor_retorno = -2;
                break;
            case CMD_ERROR_3: 
                valor_retorno = -3;
                break;
            case CMD_ERROR_4: 
                valor_retorno = -4;
                break;
            default: 
                break;
        }        
    }
    return valor_retorno;
}

/*
Funcion que envia datos a traves de un socket TCP.  
Parametros:
    socket     -> Numero que indica el socket por donde se enviaran los datos. 
    cant_bytes -> Cantidad de bytes a ser enviados.
    datos      -> Puntero al array donde se encuentran almacenado los datos.
Retorno: 
    0 -> Los datos fueron enviados correctamente.
    Posibles errores(unicamente valores negativos).
    -1  -> No se pudo enviar los datos a traves del socket.
    -2  -> El socket no esta conectado.
    -3  -> Cantidad de bytes para escribir fuera de rango*.
    -4  -> Numero de socket fuera de rango.
*/
int8_t esp8266_enviar_datos_tcp(uint8_t socket, uint16_t cant_bytes, char *data){
    int8_t valor_retorno = -1; 
    uint8_t cmd_respuesta; 
    uint16_t cant_bytes_recibidos = 0;
    uint16_t indice_bytes = 0;
    uint8_t pos_comienzo_datos = 0;
    uint16_t num_paquetes = 0;
    uint8_t cantidad_envios_entero = 0;
    int16_t cantidad_bytes_resto = 0;
    uint8 cantidad_envios_correctos = 0;
    
    cantidad_envios_entero = (int)floor(cant_bytes/TCP_TX_BUFFER_SIZE);
    cantidad_bytes_resto = (cant_bytes % TCP_TX_BUFFER_SIZE);
    
    if(cantidad_envios_entero > 0){
        for(num_paquetes = 0; num_paquetes < cantidad_envios_entero; 
            num_paquetes++)
        {
            sprintf((char*)buffer_serial_tx,CMD_ENVIAR_DATOS_TCP,socket,
            TCP_TX_BUFFER_SIZE);
            pos_comienzo_datos = strlen((char *)buffer_serial_tx);
            for(indice_bytes = 0; (indice_bytes < TCP_TX_BUFFER_SIZE); indice_bytes++)
            {
                buffer_serial_tx[pos_comienzo_datos + indice_bytes] = 
                data[indice_bytes + num_paquetes*TCP_TX_BUFFER_SIZE];  
            }
            buffer_serial_tx[pos_comienzo_datos + indice_bytes] = '\n';
            
            uart_enviar_datos((char *)buffer_serial_tx,pos_comienzo_datos + indice_bytes+1);
            
            cant_bytes_recibidos = uart_leer_datos(&buffer_serial_rx[0]);
            
            if (cant_bytes_recibidos > 0)
            {
                cmd_respuesta = buffer_serial_rx[0];
                switch(cmd_respuesta)
                {
                    case CMD_RESP_OK: 
                        cantidad_envios_correctos++;
                        valor_retorno = 0;
                        break;
                    case CMD_ERROR_1:
                        valor_retorno = -1;
                        break;
                    case CMD_ERROR_2:
                        valor_retorno = -2;
                        break;
                    case CMD_ERROR_3: 
                        valor_retorno = -3;
                        break;
                    case CMD_ERROR_4: 
                        valor_retorno = -4;
                        break;
                    default: 
                        break;
                }
            }
        }
    }
    
    if(cantidad_envios_correctos == cantidad_envios_entero)
    {
        valor_retorno = 0;
    }
    else
    {
        
    }
    
    if(cantidad_bytes_resto > 0)
    {
        sprintf((char*)buffer_serial_tx,CMD_ENVIAR_DATOS_TCP,socket,
        (int)(cantidad_bytes_resto));
        pos_comienzo_datos = strlen((char *)buffer_serial_tx);
        for(indice_bytes = 0; (indice_bytes < cantidad_bytes_resto); indice_bytes++)
        {
            buffer_serial_tx[pos_comienzo_datos + indice_bytes] = 
            data[indice_bytes + num_paquetes*TCP_TX_BUFFER_SIZE];  
        }
        buffer_serial_tx[pos_comienzo_datos + indice_bytes] = '\n';
        
        uart_enviar_datos((char *)buffer_serial_tx,pos_comienzo_datos + indice_bytes+1);
        
        cant_bytes_recibidos = uart_leer_datos(&buffer_serial_rx[0]);
        
        if (cant_bytes_recibidos > 0)
        {
            cmd_respuesta = buffer_serial_rx[0];
            switch(cmd_respuesta)
            {
                case CMD_RESP_OK: 
                    cantidad_envios_correctos++;
                    valor_retorno = 0;
                    break;
                case CMD_ERROR_1:
                    valor_retorno = -1;
                    break;
                case CMD_ERROR_2:
                    valor_retorno = -2;
                    break;
                case CMD_ERROR_3: 
                    valor_retorno = -3;
                    break;
                case CMD_ERROR_4: 
                    valor_retorno = -4;
                    break;
                default: 
                    break;
            }
        }
    }
    return valor_retorno;
}

/*
Funcion que leer los datos recibidos a traves de un socket TCP.  
Parametros:
    socket  -> Numero que indica el socket desde donde se leeran los datos. 
    buffer  -> Puntero al array donde se almacenaran los datos recibidos.
Retorno: 
    Numero positivo o cero -> Cantidad de bytes recibidos en el socket. 
    Posibles errores(unicamente valores negativos).
    -1  -> Numero de socket fuera de rango.
    -2  -> WiFi desconectado.
    -3  -> El socket se encuentra desconectado.
*/
int16_t esp8266_leer_datos_tcp(uint8_t socket, uint8_t *buffer){
    int16_t valor_retorno = -1;
    uint8_t cmd_respuesta; 
    uint16_t cant_bytes_recibidos = 0;
   
    uint16_t i = 0;
    uint16_t tam_paquete_tcp = 0;
    uint8_t pos_comienzo_datos = 0;
    
    sprintf((char *)buffer_serial_tx,CMD_RECIBIR_DATOS_TCP,socket);
     
    uart_enviar_datos((char *)buffer_serial_tx,strlen((char *)buffer_serial_tx));
    
    /*Para indicar a la funcion uart_espera_paquete que se recibiran datos.*/
    set_b_cmd_recibir_datos();
    
    cant_bytes_recibidos = uart_leer_datos(&buffer_serial_rx[0]);
    
    /*Para poner a la funcion uart_espera_paquete de vuelta en un estado normal.*/
    rst_b_cmd_recibir_datos();
    
    /*Se verifica si se recibieron datos del modulo*/
    if (cant_bytes_recibidos > 0){
        /*Se verifca que no hayan errores*/
        cmd_respuesta = buffer_serial_rx[0];
        switch(cmd_respuesta){
            case CMD_RESP_OK:
                /*Se extrae el paquete TCP recibido*/
                tam_paquete_tcp = get_tam_paquete_datos_tcp();
    //            pos_comienzo_datos = get_posicion_comienzo_datos();
    //            while(tam_paquete_tcp--){
    //                buffer[i] =  buffer_serial_rx[pos_comienzo_datos + i];
    //                i++;
    //            }
                //
                memcpy(buffer,g_tcp_rx_buffer,tam_paquete_tcp);
                rst_tam_paquete_datos_tcp();
                valor_retorno = tam_paquete_tcp;
                break;
            case CMD_ERROR_1:
                valor_retorno = -1;
                break;
            case CMD_ERROR_2:
                valor_retorno = -2;
                break;
            case CMD_ERROR_3: 
                valor_retorno = -3;
                break;
            default: 
                break;
        }        
    }
    return valor_retorno;
}

/*
Funcion que cierra un socket TCP.  
Parametros:
    socket  -> Numero que indica el socket a ser cerrado.
Retorno: 
    0   -> El socket se cerro correctamente. 
    Posibles errores(unicamente valores negativos).
    -1  -> Numero de socket fuera de rango.
    -2  -> WiFi desconectado.
*/
int8_t esp8266_cierra_socket_tcp(uint8_t socket){
    int8_t valor_retorno = -1;
    uint8_t cmd_respuesta; 
    uint16_t cant_bytes_recibidos = 0;
    
    sprintf((char *)buffer_serial_tx,CMD_CERRAR_SOCKET_TCP,socket);
    
    uart_enviar_datos((char *)buffer_serial_tx,strlen((char *)buffer_serial_tx));
    
    cant_bytes_recibidos = uart_leer_datos(&buffer_serial_rx[0]); 
    
    if (cant_bytes_recibidos > 0){
        cmd_respuesta = buffer_serial_rx[0];
        switch(cmd_respuesta){
            case CMD_RESP_OK: 
                valor_retorno = 0;
                break;
            case CMD_ERROR_1:
                valor_retorno = -1;
                break;
            case CMD_ERROR_2:
                valor_retorno = -2;
                break;
            default: 
                break;
            }
    }
    return valor_retorno;
}

/*
Funcion que se encarga de crear un servidor local en el ESP8266.
Parametros:
    puerto       -> Puerto por el que se escuchara a los clientes.
    cant_clientes-> Cantidad maxima de clientes a ser aceptados por el servidor.
Retorno: 
    [0..4] -> El servidor fue creado con exito, se retorna el socket asignado,
    puede ser del 0 al 4.
    Posibles errores(unicamente valores negativos).
    -1  -> El numero de puerto esta fuera de rango.
    -2  -> El numero de clientes esta fuera de rango.
    -3  -> Ya existe un servidor en el puerto especificado.
    -4  -> WiFi desconectado.
*/
int8_t esp8266_crear_servidor_tcp(uint16_t puerto, uint8_t cant_clientes){
    int8_t valor_retorno = -1;
    uint8_t cmd_respuesta; 
    uint16_t cant_bytes_recibidos = 0;
    
    sprintf((char *)buffer_serial_tx,CMD_CREAR_SERVIDOR_TCP,puerto,cant_clientes);
     
    uart_enviar_datos((char *)buffer_serial_tx,strlen((char *)buffer_serial_tx));
    
    cant_bytes_recibidos = uart_leer_datos(&buffer_serial_rx[0]); 
    
    if (cant_bytes_recibidos > 0){
        cmd_respuesta = buffer_serial_rx[0];
        switch(cmd_respuesta){
            case CMD_RESP_OK: 
                valor_retorno = buffer_serial_rx[2] - '0';
                break;
            case CMD_ERROR_1:
                valor_retorno = -1;
                break;
            case CMD_ERROR_2:
                valor_retorno = -2;
                break;
            case CMD_ERROR_3:
                valor_retorno = -3;
                break;
            case CMD_ERROR_4:
                valor_retorno = -4;
                break;
            default: 
                break;
        }
    }
    return valor_retorno;
}

/*
Funcion que se encarga de aceptar a los clientes que intentan conectarse al servidor.
Parametros:
    socket -> Socket del servidor por el cual se escucha a posibles clientes. Es el socket 
    retornado por la funcion  esp8266_crear_servidor_tcp.
Retorno: 
    [0..4] -> El cliente fue aceptado por el servidor, se retorna el socket asignado,
    puede ser del 0 al 4.
    Posibles errores(unicamente valores negativos).
    -1  -> El numero de socket esta fuera de rango.
    -2  -> No hay socket disponible para aceptar al cliente.
    -3  -> Se alcanzo el numero maximo de clientes permitidos para este servidor.
    -4  -> El servidor no tiene clientes pendientes.
    -5  -> El servidor se encuentra desactivado.
    -6  -> WiFi desconectado.
*/
int8_t esp8266_aceptar_clientes_tcp(uint8_t socket){
    int8_t valor_retorno = -1;
    uint8_t cmd_respuesta; 
    uint16_t cant_bytes_recibidos = 0;
    
    sprintf((char *)buffer_serial_tx,CMD_ACEPTAR_CLIENTES_TCP,socket);
    
    uart_enviar_datos((char *)buffer_serial_tx,strlen((char *)buffer_serial_tx));
    
    cant_bytes_recibidos = uart_leer_datos(&buffer_serial_rx[0]); 
    
    if (cant_bytes_recibidos > 0){
        cmd_respuesta = buffer_serial_rx[0];
        switch(cmd_respuesta){
            case CMD_RESP_OK: 
                valor_retorno = buffer_serial_rx[2] - '0';
                break;
            case CMD_ERROR_1:
                valor_retorno = -1;
                break;
            case CMD_ERROR_2:
                valor_retorno = -2;
                break;
            case CMD_ERROR_3:
                valor_retorno = -3;
                break;
            case CMD_ERROR_4:
                valor_retorno = -4;
                break;
            case CMD_ERROR_5:
                valor_retorno = -5;
                break;
            case CMD_ERROR_6:
                valor_retorno = -6;
                break;
            default: 
                break;
        }
    }
    return valor_retorno;
}

/*
Funcion que se encarga de configurar los parametros de red de la 
interfaz de softAP del modulo ESP8266.
Parametros:
    ip      -> Direccion IP que se asignara a la interfaz softAP del modulo. 
    gateway -> Puerta de enlace a ser utilizada en la interfaz. 
    subnet  -> Mascara de subred.
Retorno: 
    0   -> El AP fue configurado con exito.
    Posibles errores(unicamente valores negativos).
    -1  -> Direccion IP no valida.
    -2  -> Puerta de enlace no valida.
    -3  -> Mascara de subred no valida.
    -4  -> No se pudo aplicar la configuracion.
*/
int8_t esp8266_configurar_softAP(char *ip, char* gateway, char* subnet)
{
    int8_t valor_retorno = -1;
    uint8_t cmd_respuesta; 
    uint16_t cant_bytes_recibidos = 0;
    
    sprintf((char *)buffer_serial_tx,CMD_CONFIGURAR_SOFTAP,ip,gateway,subnet);
        
    uart_enviar_datos(buffer_serial_tx,strlen((char*)buffer_serial_tx));
    
    cant_bytes_recibidos = uart_leer_datos(&buffer_serial_rx[0]); 
    
    if (cant_bytes_recibidos > 0)
    {
        cmd_respuesta = buffer_serial_rx[0];
        switch(cmd_respuesta)
        {
	        case CMD_RESP_OK: 
	            valor_retorno = 0;
	            break;
	        case CMD_ERROR_1:
	            valor_retorno = -1;
	            break;
	        case CMD_ERROR_2:
	            valor_retorno = -2;
	            break;
	        case CMD_ERROR_3:
	            valor_retorno = -3;
	            break;
	        case CMD_ERROR_4:
	            valor_retorno = -4;
	            break;
	        default: 
	            break;
        }
    }
    return valor_retorno;
} 

/*
Funcion que se encarga de poner el modulo ESP8266 en modo SoftAP.
(La IP por defecto del ESP8266 es el 192.168.4.1)
Parametros:
    ssid   -> Puntero al nombre del AP a ser creado en el ESP8266 (63 caracteres como maximo).
    pass   -> Contraseña del AP (para WPA2 utilizar minimo una longitud de 8 caracteres, para dejar abierto utilizar NULL).
    canal  -> Canal WiFi que se utilizara, de 1 a 13.
    oculto -> 0 para habilitar el broadcast del SSID, 1 para ocultarlo.
    max_conn -> Determina el numero maximo de dispositivos que se pueden conectar al AP (pueden conectarse hasta 4 dispositivos).
Retorno: 
    0   -> El AP fue configurado con exito.
    Posibles errores(unicamente valores negativos).
    -1  -> El numero de canal esta fuera de rango.
    -2  -> El numero oculto esta fuera de rango.
    -3  -> El numero cant_dispositivos esta fuera de rango.
    -4  -> No se pudo crear el AP.
*/
int8_t esp8266_crear_softAP(char *ssid, char* pass, uint8_t canal, uint8_t oculto, uint8_t cant_dispositivos)
{
    int8_t valor_retorno = -1;
    uint8_t cmd_respuesta; 
    uint16_t cant_bytes_recibidos = 0;
    
    sprintf((char *)buffer_serial_tx,CMD_CREAR_SOFTAP,ssid,pass,canal,oculto,cant_dispositivos);
        
    uart_enviar_datos(buffer_serial_tx,strlen((char*)buffer_serial_tx));
    
    cant_bytes_recibidos = uart_leer_datos(&buffer_serial_rx[0]); 
    
    if (cant_bytes_recibidos > 0)
    {
        cmd_respuesta = buffer_serial_rx[0];
        switch(cmd_respuesta)
        {
	        case CMD_RESP_OK: 
	            valor_retorno = 0;
	            break;
	        case CMD_ERROR_1:
	            valor_retorno = -1;
	            break;
	        case CMD_ERROR_2:
	            valor_retorno = -2;
	            break;
	        case CMD_ERROR_3:
	            valor_retorno = -3;
	            break;
	        case CMD_ERROR_4:
	            valor_retorno = -4;
	            break;
	        default: 
	            break;
        }
    }
    return valor_retorno;
} 
/* [] END OF FILE */
