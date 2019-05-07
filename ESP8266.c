/**
* @file ESP8266.c
*
* @author Felipe Costa 
*
* @date 24 de abril de 2019
*
* @brief Libreria de funciones para utilizar el ESP8266
*
\mainpage Descripcion 
Esta es la libreria de funciones para interactuar con el modulo ESP8266 
con el firmware propietario. Se utiliza un esquema sincrono y bloqueante. 
Basicamente, se envia un comando, y se espera la respuesta a este comando 
o que se cumpla un determinado tiempo de espera, para luego continuar con 
la ejecucion del programa.


\section dependencia_sec Dependencia de hardware. 
Para utilizar esta liberia, se necesita un periférico UART, un contador multiplo de 
2ms y un puerto de salida GPIO.
\section platform_sec	Cambio de plataforma 
Los drivers utilizados en esta liberia estan hechos para funcionar en el PSoC 5LP. 
@warning En caso de querer utilizar un micro-controlador diferente, es necesario realizar 
cambios en las siguientes funciones dentro del archivo puerto_serial.c : #uart_espera_paquete, 
#CY_ISR, #incializar_esp8266 y #uart_enviar_datos.
*/




#include "project.h"
#include "globals.h"
#include "ESP8266.h"
#include "puerto_serial.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

uint8_t buffer_serial_tx[SERIAL_BUFFER_SIZE];	/**< Buffer utilizado para almacenar los Bytes recibidos por el puerto serial, utilizado por el ESP8266.*/
uint8_t buffer_serial_rx[SERIAL_BUFFER_SIZE];	/**< Buffer utilizado para almacenar los Bytes a ser enviados por el puerto serial, utilizado por el ESP8266.*/

struct elementos_punto_acceso puntos_acceso[16];

/**
Funcion que verifica que el modulo ESP8266 esta encendido y funcionando. 
    @retval  0  No hay error, el modulo esta funcionando.
    @retval -1  Error, no se recibe respuesta del modulo.
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

/**
Funcion que reinicia el modulo ESP8266. Cuando esto ocurre, se envian datos 
no revelantes por el UART, deben ser ignorados hasta recibir el caracter R,
lo cual indica que el modulo esta listo para recibir comandos. 
    @retval 0   El modulo fue reiniciado correctamente y esta listo para 
    recibir comandos.
    @retval -1  Error, no se recibe respuesta del modulo.
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

/**
Funcion que cambia la velocidad de transmisión del periférico UART, utilizado por el 
módulo para comunicarse con el micro-controlador externo.
    @param baud_rate    Velocidad de transmisión deseada. El rango permitido para este
	parámetro va desde 9600 a 921600. 
    @retval 0       El cambio de velocidad se realizo con éxito. Es necesario esperar al
	menos 5 ms para enviar el siguiente comando utilizando la nueva velocidad.
    @retval -1      Error, el parámetro baud_rate se encuentra fuera de rango.
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

/**            
    Funcion que escanea los puntos de acceso al alcance del modulo.  
    TODO: Pasar como parametro una estructura o un array de manera tal
    a que la aplicacion disponga de los nombres y la potencia de los 
    puntos de acceso. 
    @param[out] aps
    @retval 0   No hay error, el modulo esta funcionando. 
    @retval -1  No se recibe respuesta del modulo.
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

/**            
Funcion que cambia el modo WiFi del modulo.  
    @param modo_wifi
    0	-> 	WiFi apagado  \n
    1	-> 	Modo estacion \n
    2 	->	Modo punto de acceso \n 
    3 	-> 	Modo estacion + punto de acceso \n                
    @retval  0  El modo fue establecido correctamente.  
    @retval -1  Parametro modo_wifi fuera de rango.
    @retval -2  No se pudo establecer la configuracion deseada.
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

/**
	Funcion que configura de forma manual los parámetros de la interfaz de red de la estación, 
	desactivando la asignación por DHCP.  
    @param ip      Dirección IP a ser asignada al modulo. 
    @param dns     Dirección del servidor DNS. 
    @param gateway Dirección de la puerta de enlace. 
    @param subnet  Dirección de la mascara de la red.
    @retval 0   Configuración exitosa. 
    @retval -1  Error, dirección IP invalida.
    @retval -2  Error, dirección DNS invalida.
    @retval -3 	Error, dirección Gateway invalida.
    @retval -4  Error, dirección Subnet invalida.
    @retval -5  Error, no se pudo establecer la configuración deseada. 
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

/**
	Comando utilizado para conectar el módulo a un punto de acceso (AP, por sus
	siglas en ingles).  
    @param ssid Nombre del punto de acceso al cual se desea conectar el módulo. 
    @param pass Contraseña del punto de acceso al cual se desea conectar el módulo.
    @retval  0  Conexión exitosa. 
    @retval -1  Error, no se pudo establecer la conexión al punto de acceso.
    @retval -2  Error, se alcanzo el tiempo de espera máximo (20 segundos) sin poder
	establecer la conexión.
    @retval -3  Error, contraseña incorrecta.
    @retval -4  Error, no se encuentra el punto de acceso.
    @retval -5  Escaneo completo pero no se conecto.
    @retval -6  Se perdio la conexion.
    @retval -7  La radio se encuentra en reposo.
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

/**
Funcion que conecta el ESP8266 a un servidor TCP remoto.  
    @param IP       Dirección IP del servidor al cual se quiere establecer la conexión,
	como también puede ser un nombre de host. 
    @param puerto     Puerto del servidor. Puede tener un valor máximo de 65535.  
    @retval [0..3]  La conexion se realizo con exito, se retorna el 
    socket asignado, puede ser del 0 al 3.
    @retval -1      Error, el parámetro puerto esta fuera de rango.
    @retval -2      Error, no hay una conexión WiFi activa.
    @retval -3      Error, no hay recursos disponibles para establecer la conexión.
    @retval -4      Error, no se pudo establecer la conexión al servidor.
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

/**
    Comando utilizado para enviar datos a través de una conexión TCP. 
    Para utilizar este comando, es necesario primero utilizar el comando CCS, 
    para establecer la conexión a un servidor, y/o el comando SAC, que acepta 
    un cliente que intenta conectarse a un servidor en el modulo. 
	@note En caso de que el parametro #cant_bytes supere el valor de 1460, esta 
	funcion se encarga de separar el contenido de #data en paquetes de 1460 y los 
	envia por partes.
    @param socket   Parámetro utilizado para identificar las conexiones. 
	Los valores permitidos para este parámetro van de 0 a 3. 
    @param cant_bytes  Cantidad de Bytes a ser enviados.
    @param data     Puntero al array donde se encuentran almacenada la cadena de 
    datos a ser enviados. La longitud de esta cadena debe ser igual al del 
    parámetro cant_Bytes, en caso de que no sean iguales, los datos no serán enviados
    @retval  0      Los datos fueron enviados correctamente.
    @retval -1      Error, los datos no fueron enviados.
    @retval -2      Error, el socket no tiene una conexión activa.
    @retval -3      Error, el parámetro cant_Bytes se encuentra fuera de rango.
    @retval -4      Error, el parámetro socket se encuentra fuera de rango.
    @retval -5      Error, no hay una conexión WiFi activa.
    @retval -6      Error, el socket no utiliza el protocolo TCP.
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
    }
    return valor_retorno;
}

/**
Funcion para recibir datos a través de una conexión TCP. Para utilizar 
este comando, es necesario primero utilizar el comando CCS, para establecer la 
conexión a un servidor, y/o el comando SAC, que acepta un cliente que 
intenta conectarse a un servidor en el modulo
    @param[in] socket   Parámetro utilizado para identificar las conexiones. 
    Los valores permitidos para este parámetro van de 0 a 3. 
    @param[out] buffer  Puntero al array donde se almacenaran los datos recibidos. 
    @retval cantidad_bytes   Cantidad de bytes recibidos en el socket. 
    @retval -1      Error, el parámetro socket se encuentra fuera de rango.
    @retval -2      Error, no hay una conexión WiFi activa.
    @retval -3      Error, el parámetro socket no tiene una conexión activa.
    @retval -4 		Error, el parámetro socket no utiliza el protocolo TCP.
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
            case CMD_ERROR_4: 
                valor_retorno = -4;
                break;
            default: 
                break;
        }        
    }
    return valor_retorno;
}

/**
	Comando utilizado para cerrar las conexiones activas.
    @param socket   Parámetro utilizado para identificar las conexiones. 
    Los valores permitidos para este parámetro van de 0 a 3. 
    @retval  0      La conexión fue cerrada con éxito. 
    @retval -1      Error, el parámetro socket se encuentra fuera de rango
    @retval -2      Error, no hay una conexión WiFi activa.
    @retval -3     	Error, el socket no tiene una conexión activa.
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
            case CMD_ERROR_3:
                valor_retorno = -3;
                break;
            default: 
                break;
            }
    }
    return valor_retorno;
}

/**
Funcion que se encarga de crear un servidor TCP en el modulo. Pueden trabajar
en simultaneo como máximo 4 servidores.
    @param puerto           Puerto a ser utilizado por el servidor. 
    Puede tener un valor máximo de 65535.
    @param cant_clientes    Especifica la cantidad de conexiones simultaneas 
    que puede aceptar el servidor. Los valores permitidos para ese parámetro va desde 1
	hasta 4
    @retval [0..3] El servidor fue creado exitosamente. Se retorna un numero de socket_pasivo
	que sera utilizado para identificar al servidor. El único comando que
	utiliza este valor como parámetro es el comando SAC. Los valores
	permitidos para este numero van de 0 a 3.
    @retval -1    	Error, el parámetro puerto se encuentra fuera de rango.
    @retval -2      Error, el parámetro cantidad_clientes se encuentra fuera de rango.
    @retval -3      Error, no hay una conexión WiFi activa.
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
            default: 
                break;
        }
    }
    return valor_retorno;
}

/**
Funcion para aceptar clientes que desean conectarse a un servidor
TCP del modulo. Para utilizar esta funcion, en primer lugar se debe llamar a 
esp8266_crear_servidor_tcp, ya que esta funcion retorna un valor que se utiliza 
para poder aceptar a los clientes.
    @param socket  Parámetro utilizado para identificar de cual servidor se 
    deben aceptar los clientes. Para obtener este parámetro, se debe almacenar el
	valor de retorno de la funcion esp8266_crear_servidor_tcp. Los valores permitidos 
	para este parámetro van de 0 a 3.
    @retval [0..3] El cliente fue aceptado con éxito al servidor. Se retorna un numero
	socket de manera tal a identificar al cliente y poder intercambiar
	datos. Los valores permitidos para este numero van de 0 a 3.
    @retval -1      Error, el parámetro socket se encuentra fuera de rango.
    @retval -2      Error, no hay recursos disponibles para aceptar el cliente, se rechaza
	la conexión.
    @retval -3      Ya se alcanzo el numero máximo de conexiones simultaneas permitidas 
    para este servidor. Se rechaza el cliente.
    @retval -4      El servidor no tiene clientes que quieran conectarse.
    @retval -5      El servidor no se encuentra activo.
    @retval -6      Error, no hay una conexión WiFi activa.
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

/**
    Comando utilizado para configurar de forma manual los parámetros de la 
    interfaz de red del punto de acceso (softAP).
    @param ip       Dirección IP a ser asignada al modulo.
    @param gateway  Dirección de la puerta de enlace. 
    @param subnet   Dirección de la mascara de la red. 
    @retval  0      Configuración exitosa.
    @retval -1      Error, dirección IP invalida.
    @retval -2      Error, dirección Gateway invalida.
    @retval -3      Error, dirección Subnet invalida.
    @retval -4      Error, no se pudo aplicar la configuracion.
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

/**
Funcion para crear en el modulo un punto de acceso (AP, por sus
siglas en ingles). El modo de autenticación es WPA2-PSK.
(La IP por defecto del ESP8266 es el 192.168.4.1)
    @param ssid    Puntero al nombre del punto de acceso. (63 caracteres como maximo)
    @param pass    Contraseña del punto de acceso, longitud minima de 8 caracteres.
    @param canal   Numero del canal WiFi que utilizara el punto de acceso. Valores
	permitidos del 1 al 13
    @param oculto  0 para habilitar el broadcast del SSID, 1 para ocultarlo.
    @param cant_dispositivos  Numero máximo de conexiones simultaneas que permite atender el
	punto de acceso. Valores permitidos del 1 al 4.
    @retval 0    El punto de acceso fue creado correctamente.
    @retval -1   Error, el numero de canal esta fuera de rango.
    @retval -2   Error, el parámetro ssid_oculto esta fuera de rango
    @retval -3   Error, el parámetro max_con esta fuera de rango.
    @retval -4   Error, no se pudo crear el punto de acceso.
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