/*
 * servidor_web.c
 *
 *  Created on: 3 ago. 2018
 *      Author: lp1-2018
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "paginas.h"
#include "project.h"
#include "servidor_web_banderas.h"
#include "interfaz_debug.h"
#include "globals.h"
#include "servidor_web.h"
#include "http_parser.h"
#include "form_parser.h"
#include "ESP8266.h"
#include "esp8266_cliente.h"
#include "ip.h"
#include "eeprom.h"
#include "sockets.h"
#include "favicon_catolica.h" 

#define CADENA_VACIA	""

#define METODO_GET  "GET"
#define METODO_POST	"POST"

#define TAM_BUF_WEB_ENVIO 2048
#define TAM_BUF_WEB_RECIBIR 1024
static char buffer_web_recibido[TAM_BUF_WEB_RECIBIR];
static char buffer_web_envio[TAM_BUF_WEB_ENVIO];
static char buffer_temporal[1460];

/*
 * Donde se guarda los datos HTTP parseados
 */
#define MAX_HTTP_HEADER 20
static HTTPheaderFields headerHTTP[MAX_HTTP_HEADER];
static HTTPparseData datosHTTP = {{NULL, NULL, NULL}, headerHTTP, MAX_HTTP_HEADER};

/*
 * Donde se guardan los datos del form parseados
 */
#define MAX_FORM_DATA 10
formData fd[MAX_FORM_DATA];

/*
 * Identificadores socket
 */
static int16_t idSocketCliente;	// Identificador del socket del cliente que se conecta a este servidor
static int16_t idSocketServ;		// Identificador del servidor

/*
 * Variable de bandera. Comunicación entre rutina de servicio y estas rutinas
 */
static volatile uint32_t timeoutWeb;

/*
 * Variable de bandera. Indica que un cliente ya hizo el login
 */
static uint8_t login;

/*
 * estructura de datos de configuración de este TIM
 */
static struct configuracion configTIM;

/*
 * Timeouts. Múltiplo de 2 ms.
 */
#define TIMEOUT_ACTIVIDAD_WEB 30000	// timeout de inactividad web = 1 minuto
static float periodo_timer_web_seg; 


bool b_datos_tim_recibido = 0;
typedef enum  {ingreso, configuracion, cambiar_usuario, salida, datos_guardados} paginas_web;
static paginas_web pagina_web_actual; 
static char timeout_web_cadena[64]; 

/*
 ************************************************************************
 * Prototipo de funciones de manejo de las páginas web de este servidor
 ************************************************************************
 */
void indexFun (void);
void configFormFun (void);
void cambiarFormFun (void);
void salirFun (void);
void configFun (char *c);
void cambiarFun (char *c);
void ingresoFun (char *c);
void faviconFun (void); 
/*
 ************************************************************************
 * Fin Prototipo de funciones de manejo de las páginas web de este servidor
 ************************************************************************
 */

/*
 * Páginas web que podemos recibir con método GET
 */
recursoGET paginasGET[] = {
		{"/", indexFun},
		{"/index.html", indexFun},
		{"/index.htm", indexFun},
		{"/config_form.html", configFormFun},
		{"/cambiar_form.html", cambiarFormFun},
		{"/salir.html", salirFun},
        {"/favicon.ico", faviconFun}
};

/*
 * Páginas web que podemos recibir con método POST
 */
recursoPOST paginasPOST[] = {
		{"/config.html", configFun},
		{"/cambiar.html", cambiarFun},
		{"/ingreso.html", ingresoFun}
};

/*
 *********************************************************************
 * Funciones que responden a solicitudes de páginas determinadas.
 ********************************************************************
 */
/*
 * Funciones GET
 */
void faviconFun(){
    debug_enviar("SERVIDOR => Respuesta al REQUEST de favicon.ico"); 
    strcpy (buffer_web_envio, cabeceraOK);
//	strcat (buffer_web_envio, "Content-Type: image/x-icon\r\n");
    strcat (buffer_web_envio, "Accept-Ranges: bytes\r\n");
    sprintf (&buffer_web_envio[strlen(buffer_web_envio)], cabContentLength, sizeof(data_favicon_ico));
    strcat (buffer_web_envio, "Content-Type: image/vnd.microsoft.icon\r\n");
    strcat (buffer_web_envio, finCabecera);
    
    /*Se envia el header en primer lugar.*/
    esp8266_enviar_datos_tcp(idSocketCliente, strlen(buffer_web_envio), buffer_web_envio);
    
    /*Luego se envia los datos en forma binaria.*/
    esp8266_enviar_datos_tcp(idSocketCliente, sizeof(data_favicon_ico), data_favicon_ico);
}

void salirFun(){
    debug_enviar("SERVIDOR => Respuesta 404"); 
	// Enviamos la redicción a index.html
	strcpy (buffer_web_envio, redirect);
    sprintf (&buffer_web_envio[strlen(buffer_web_envio)], cabContentLength, strlen (hyperlink_index));
    strcat (buffer_web_envio,cabCacheControl);
	strcat (buffer_web_envio, finCabecera);
    
    strcat (buffer_web_envio, hyperlink_index);
    
	esp8266_enviar_datos_tcp (idSocketCliente, strlen(buffer_web_envio), buffer_web_envio);
}

void indexFun (){
    debug_enviar("SERVIDOR => Se enviar el index.html");
	// Enviamos el index.html
	strcpy (buffer_web_envio, cabeceraOK);
	strcat (buffer_web_envio, cabContentType);
	sprintf (&buffer_web_envio[strlen(buffer_web_envio)], cabContentLength, strlen (index_p));
    strcat (buffer_web_envio, "Connection: keep-alive\r\n");
    strcat (buffer_web_envio,cabCacheControl);
	strcat (buffer_web_envio, finCabecera);

	strcat (buffer_web_envio, index_p);

	esp8266_enviar_datos_tcp (idSocketCliente, strlen(buffer_web_envio), buffer_web_envio);

	login = 0;	// indicamos que aún no se hizo el logín
    pagina_web_actual = ingreso;
}

void configFormFun (){
    debug_enviar("SERVIDOR => Se enviar el config.html");
	uint16_t crc;
    uint16_t cant_bytes_bufWebEnvio = 0;
    char dirIP[20]; 
    bool b_tim_wifi_conectado; 
    bool b_tim_ncap_conectado;
    b_tim_wifi_conectado = get_estado_wifi_tim(); 
    b_tim_ncap_conectado = get_estado_ncap_tim();
	if (login == 1){
        memset(buffer_temporal,0,sizeof(buffer_temporal));
        //leemos los valores guardados en la EEPROM
		if (leeConfigEEPROM (&configTIM, sizeof(configTIM), &crc) == 0)
        {
			// correcto
            IP2ASCII(configTIM.dirIPNCAP,dirIP);
            if(b_tim_wifi_conectado == true && b_tim_ncap_conectado == true ){
                sprintf (buffer_temporal, config_form, configTIM.SSID_AP, dirIP, 
                    configTIM.puertoNCAP,"green","Conectado","green","Conectado");
            }
            if(b_tim_wifi_conectado == true && b_tim_ncap_conectado == false ){
                sprintf (buffer_temporal, config_form, configTIM.SSID_AP, dirIP, 
                    configTIM.puertoNCAP,"green","Conectado","red","Desconectado");
            }
            if(b_tim_wifi_conectado == false){
                sprintf (buffer_temporal, config_form, configTIM.SSID_AP, dirIP, 
                    configTIM.puertoNCAP,"red","Desconectado","red","Desconectado");
            }
		} 
        else 
        {
            if(b_tim_wifi_conectado == true && b_tim_ncap_conectado == true ){
                sprintf (buffer_temporal, config_form, CADENA_VACIA, CADENA_VACIA, 
                    CADENA_VACIA,"green","Conectado","green","Conectado");
            }
            if(b_tim_wifi_conectado == true && b_tim_ncap_conectado == false ){
                sprintf (buffer_temporal, config_form, CADENA_VACIA, CADENA_VACIA, 
                    CADENA_VACIA,"green","Conectado","red","Desconectado");
            }
            if(b_tim_wifi_conectado == false){
                sprintf (buffer_temporal, config_form, CADENA_VACIA, CADENA_VACIA, 
                    CADENA_VACIA,"red","Desconectado","red","Desconectado");
            }
		}      
        //Armamos el header
        memset(buffer_web_envio,0,sizeof(buffer_web_envio));
		strcpy (buffer_web_envio, cabeceraOK);
		strcat (buffer_web_envio, cabContentType);    
		sprintf (&buffer_web_envio[strlen(buffer_web_envio)], cabContentLength, 
        strlen (buffer_temporal));
        strcat (buffer_web_envio,cabCacheControl);
		strcat (buffer_web_envio, finCabecera);
        
        //Adjuntamos la pagina config_form con los parametros cargados.
        strcat(buffer_web_envio,buffer_temporal);
	
        // Enviamos el config_form.html
		esp8266_enviar_datos_tcp (idSocketCliente, strlen(buffer_web_envio), buffer_web_envio);
        pagina_web_actual = configuracion; 
        timeoutWeb = 0;
	} else {
		salirFun ();
	}
}

void cambiarFormFun (){
	if (login == 1){
		// Enviamos el cambiar_form.html
		strcpy (buffer_web_envio, cabeceraOK);
		strcat (buffer_web_envio, cabContentType);
		sprintf (&buffer_web_envio[strlen(buffer_web_envio)], cabContentLength, strlen (cambiar_form));
        strcat (buffer_web_envio,cabCacheControl);
		strcat (buffer_web_envio, finCabecera);

		strcat (buffer_web_envio, cambiar_form);

		esp8266_enviar_datos_tcp (idSocketCliente, strlen(buffer_web_envio), buffer_web_envio);
        pagina_web_actual = cambiar_usuario; 
	} else {
		salirFun ();
	}
}

void notFoundFun (){
	strcpy (buffer_web_envio, cabeceraNotFound);
	strcat (buffer_web_envio, cabContentType);
	sprintf (&buffer_web_envio[strlen(buffer_web_envio)], cabContentLength, strlen (notFound));
    strcat (buffer_web_envio,cabCacheControl);
	strcat (buffer_web_envio, finCabecera);

	strcat (buffer_web_envio, notFound);

	esp8266_enviar_datos_tcp (idSocketCliente, strlen(buffer_web_envio), buffer_web_envio);
}
/*
 * Funciones POST
 */

/*
 * Función que envía el mensaje de error en la configuración del TIM.
 * Parámetros:
 * 	Ninguno
 * Retorno:
 * 	Ninguno
 */
void respConfigError(void){
	strcpy (buffer_web_envio, cabeceraOK);
	strcat (buffer_web_envio, cabContentType);
	sprintf (&buffer_web_envio[strlen(buffer_web_envio)], cabContentLength, strlen (errorConfig));
    strcat (buffer_web_envio,cabCacheControl);
	strcat (buffer_web_envio, finCabecera);

	strcat (buffer_web_envio, errorConfig);

	esp8266_enviar_datos_tcp (idSocketCliente, strlen(buffer_web_envio), buffer_web_envio);
}

/*
 * Función que envía el mensaje de error en el cambio de contraseña de acceso a la configuración del TIM.
 * Parámetros:
 * 	Ninguno
 * Retorno:
 * 	Ninguno
 */
void respCambiarError(void){
	strcpy (buffer_web_envio, cabeceraOK);
	strcat (buffer_web_envio, cabContentType);
	sprintf (&buffer_web_envio[strlen(buffer_web_envio)], cabContentLength, strlen (errorCambiar));
    strcat (buffer_web_envio,cabCacheControl);
	strcat (buffer_web_envio, finCabecera);

	strcat (buffer_web_envio, errorCambiar);

	esp8266_enviar_datos_tcp (idSocketCliente, strlen(buffer_web_envio), buffer_web_envio);
}

/*
 * Función que verifica los datos recibidos del POST de configuración del TIM (config.html)
 * y si son correctos se guardan los valores nuevos en la EEPROM. Solamente se verifica si
 * el número IP tiene el formato correcto y el número de puerto es un número positivo y menor
 * que 65536.
 * Parámetros:
 * 	c -> puntero a la cadena de datos del POST
 * Retorno:
 * 	Ninguno
 */
#define CANTIDAD_FORM_CONFIG	4
void configFun (char *c){
	int ret, i, cant, iSSID = -1, iContras = -1, iIP = -1, iPuerto = -1;
	uint32_t ip;
    uint32_t puerto;
    uint8_t cantidad_cambios_config = 0;
	if (login == 1){
		// Verificamos los datos recibidos y respondemos acorde
		ret = formParser (c, fd, MAX_FORM_DATA);
		if (ret == CANTIDAD_FORM_CONFIG){
			// se recibió el formulario correctamente
			// verificamos los valores
			for (i = 0, cant = 0; i < ret; i++)
            {
				if (strcmp (fd[i].nombre, "SSID") == 0)
                {
					if (strlen (fd[i].contenido) >= MAX_TEXTO_CONFIG)
                    {
						// demasiado grande
						respConfigError ();
						return;
					} 
                    else 
                    {
						iSSID = i;
						cant++;
					}
				} else if (strcmp (fd[i].nombre, "contras") == 0){
					if (strlen (fd[i].contenido) >= MAX_TEXTO_CONFIG){
						// demasiado grande
						respConfigError ();
						return;
					} else {
						iContras = i;
						cant++;
					}
				} else if (strcmp (fd[i].nombre, "IP_NCAP") == 0){
					if (strlen (fd[i].contenido) >= MAX_TEXTO_CONFIG){
						// demasiado grande
						respConfigError ();
						return;
					} else {
						iIP = i;
						cant++;
					}
				} else if (strcmp (fd[i].nombre, "P_NCAP") == 0){
					if (strlen (fd[i].contenido) >= MAX_TEXTO_CONFIG){
						// demasiado grande
						respConfigError ();
						return;
					} else {
						iPuerto = i;
						cant++;
					}
				}
			}
			// respondemos en consecuencia
			if (cant == ret && (iSSID != -1 && iPuerto != -1 && iIP != -1 && iContras != -1))
            {
                /*Verificar si los datos ingresados de WiFi y NCAP son distintos a 
                los guardados en la memoria EEPROM.*/
                if( (strcmp(configTIM.SSID_AP, fd[iSSID].contenido) != 0) ||
                (strcmp(configTIM.pwdAP, fd[iContras].contenido) != 0) )
                {
                    cantidad_cambios_config++;
                    //set_b_cambio_config_ap(); 
                }
                
                ip = ASCII2uint32((uint8_t *) fd[iIP].contenido);
                puerto = atoi(fd[iPuerto].contenido);
                
                if( (configTIM.dirIPNCAP != ip) || (configTIM.puertoNCAP != puerto) )
                {
                    cantidad_cambios_config++;
                   //set_b_cambio_config_ncap(); 
                }
                
                if(cantidad_cambios_config > 0){
                    set_b_cambio_config();
                    debug_enviar("SERVIDOR => Fueron modificados los datos del TIM.");
                    debug_enviar("\n");
                }
                else
                {
                    debug_enviar("SERVIDOR => No hubo cambios en los datos del TIM.");
                    debug_enviar("\n");
                } 
                    
				// correcto, cargamos los valores en la estructura
				if (ip == 0) {
					// error de formato ip
					respConfigError();
					return;
				} else {
					configTIM.dirIPNCAP = ip;
				}
				if ((ip = atoi(fd[iPuerto].contenido)) > UINT16_MAX || ip < 0) {
					// error de formato ip
					respConfigError();
					return;
				} else {
					configTIM.puertoNCAP = ip;
				}
				strcpy (configTIM.SSID_AP, fd[iSSID].contenido);	// ponemos la configuración
				strcpy (configTIM.pwdAP, fd[iContras].contenido);	// ponemos la configuración
                
				// configurar la EEPROM
				if (escribeConfigEEPROM (&configTIM, sizeof (configTIM)) == 0)
                {
                    //Los datos fueron guardados correctamente
                    //Intentar conectar a la estacion y al servidor en este punto? 
                    b_datos_tim_recibido = 1;  
					strcpy (buffer_web_envio, cabeceraOK);
					strcat (buffer_web_envio, cabContentType);
					sprintf (&buffer_web_envio[strlen(buffer_web_envio)], cabContentLength, strlen (configCorrecta));
                    strcat (buffer_web_envio,cabCacheControl);
					strcat (buffer_web_envio, finCabecera);
					strcat (buffer_web_envio, configCorrecta);

					esp8266_enviar_datos_tcp (idSocketCliente, strlen(buffer_web_envio), buffer_web_envio);
                    pagina_web_actual = datos_guardados;
				} 
                else 
                {
					// error al escribir EEPROM
					strcpy (buffer_web_envio, cabeceraOK);
					strcat (buffer_web_envio, cabContentType);
					sprintf (&buffer_web_envio[strlen(buffer_web_envio)], cabContentLength, strlen (errorEEPROM));
                    strcat (buffer_web_envio,cabCacheControl);
					strcat (buffer_web_envio, finCabecera);

					strcat (buffer_web_envio, errorEEPROM);

					esp8266_enviar_datos_tcp (idSocketCliente, strlen(buffer_web_envio), buffer_web_envio);
				}
			} else {
				// no se recibieron todos los datos
				respConfigError ();
			}
		} else {
			// el formulario no tiene la cantidad correcta, respondemos error
			respConfigError ();
		}
	} else {
		salirFun ();
	}
}

/*
 * Función que verifica los datos recibidos del post de cambio de usuario y contraseña (cambiar.html)
 * y si son correctos se guardan los valores nuevos en la EEPROM. Solamente se verifica si
 * la contraseña y el usuario anterior coinciden con los almacenados.
 * Parámetros:
 * 	c -> puntero a la cadena de datos del POST
 * Retorno:
 * 	Ninguno
 */
#define CANTIDAD_FORM_CAMBIAR	4
void cambiarFun(char *c){
	// vemos si el usuario y la contraseña anterior coinciden
	// si coinciden guardamos los cambios y informamos el resultado
	int ret, i, cant, iUnuevo = -1, iCnueva = -1, iUactual = -1, iCactual = -1;

	if (login == 1){
		// Verificamos los datos recibidos y respondemos acorde
		ret = formParser (c, fd, MAX_FORM_DATA);
		if (ret == CANTIDAD_FORM_CAMBIAR){
			// se recibió el formulario correctamente
			// verificamos los valores
			for (i = 0, cant = 0; i < ret; i++){
				if (strcmp (fd[i].nombre, "uActual") == 0){
					if (strlen (fd[i].contenido) >= MAX_TEXTO_CONFIG){
						// demasiado grande
						respCambiarError ();
						return;
					} else {
						iUactual = i;
						cant++;
					}
				} else if (strcmp (fd[i].nombre, "cActual") == 0){
					if (strlen (fd[i].contenido) >= MAX_TEXTO_CONFIG){
						// demasiado grande
						respCambiarError ();
						return;
					} else {
						iCactual = i;
						cant++;
					}
				} else if (strcmp (fd[i].nombre, "uNuevo") == 0){
					if (strlen (fd[i].contenido) >= MAX_TEXTO_CONFIG){
						// demasiado grande
						respCambiarError ();
						return;
					} else {
						iUnuevo = i;	// guardamos el índice del nuevo usuario
						cant++;
					}
				} else if (strcmp (fd[i].nombre, "cNueva") == 0){
					if (strlen (fd[i].contenido) >= MAX_TEXTO_CONFIG){
						// demasiado grande
						respCambiarError ();
						return;
					} else {
						iCnueva = i;	// guardamos el índice de la nueva contraseña
						cant++;
					}
				}
			}
			// respondemos en consecuencia
			if (cant == ret && (iUnuevo != -1 && iCnueva != -1 && iUactual != -1 && iCactual != -1)){
				// Verificamos contraseña y usuario
				if (strcmp(fd[iUactual].contenido, configTIM.usuario) != 0 || strcmp(fd[iCactual].contenido, configTIM.pwdUsuario) != 0) {
					respCambiarError();
					return;
				}
				// correcto, copiamos contenidos
				strcpy (configTIM.usuario, fd[iUnuevo].contenido);
				strcpy (configTIM.pwdUsuario, fd[iCnueva].contenido);

				// configurar la EEPROM
				if (escribeConfigEEPROM (&configTIM, sizeof (configTIM)) == 0){
					strcpy (buffer_web_envio, cabeceraOK);
					strcat (buffer_web_envio, cabContentType);
					sprintf (&buffer_web_envio[strlen(buffer_web_envio)], cabContentLength, strlen (cambiarCorrecta));
                    strcat (buffer_web_envio,cabCacheControl);
					strcat (buffer_web_envio, finCabecera);

					strcat (buffer_web_envio, cambiarCorrecta);

					esp8266_enviar_datos_tcp (idSocketCliente, strlen(buffer_web_envio), buffer_web_envio);
				} else {
					// error al escribir EEPROM
					strcpy (buffer_web_envio, cabeceraOK);
					strcat (buffer_web_envio, cabContentType);
					sprintf (&buffer_web_envio[strlen(buffer_web_envio)], cabContentLength, strlen (errorEEPROM));
                    strcat (buffer_web_envio,cabCacheControl);
					strcat (buffer_web_envio, finCabecera);

					strcat (buffer_web_envio, errorEEPROM);

					esp8266_enviar_datos_tcp (idSocketCliente, strlen(buffer_web_envio), buffer_web_envio);
				}

			} else {
				// no se recibieron todos los datos
				respCambiarError ();
			}
		} else {
			// el formulario no tiene la cantidad correcta, respondemos error
			respCambiarError ();
		}
	} else {
		salirFun ();
	}
}

/*
 * Función que envía el mensaje de error en el cambio de contraseña de acceso a la configuración del TIM.
 * Parámetros:
 * 	Ninguno
 * Retorno:
 * 	Ninguno
 */
void respIngresoError(void){
	strcpy (buffer_web_envio, cabeceraOK);
	strcat (buffer_web_envio, cabContentType);
	sprintf (&buffer_web_envio[strlen(buffer_web_envio)], cabContentLength, strlen (errorCambiar));
    strcat (buffer_web_envio,cabCacheControl);
	strcat (buffer_web_envio, finCabecera);

	strcat (buffer_web_envio, errorCambiar);

	esp8266_enviar_datos_tcp (idSocketCliente, strlen(buffer_web_envio), buffer_web_envio);
}

/*
 * Función que verifica los datos recibidos del post de ingreso (ingreso.html)
 * y si son correctos se activa la bandera de login.
 * Parámetros:
 * 	c -> puntero a la cadena de datos del POST
 * Retorno:
 * 	Ninguno
 */
#define CANTIDAD_FORM_INGRESO	2
void ingresoFun (char *c){
	int ret, i, cant = 0, iUsuario = -1, iContras = -1;
    uint16_t crc;
    char buf_debug1[30];
    char buf_debug2[128];
	// Verificamos los datos recibidos y respondemos acorde
	ret = formParser (c, fd, MAX_FORM_DATA);
	if (ret == CANTIDAD_FORM_INGRESO){
		// se recibió el formulario correctamente
		// verificamos los valores
		for (i = 0, cant = 0; i < ret; i++){
            memset(buf_debug1,0,sizeof(buf_debug1)); 
            memset(buf_debug2,0,sizeof(buf_debug2)); 
            strcpy(buf_debug1,fd[i].nombre);
            strcpy(buf_debug2,fd[i].contenido);
			if (strcmp (fd[i].nombre, "userid") == 0){
				if (strlen (fd[i].contenido) >= MAX_TEXTO_CONFIG){
					// demasiado grande
					salirFun ();
					return;
				} else {
					iUsuario = i;
					cant++;
				}
			} else if (strcmp (fd[i].nombre, "psw") == 0){
				if (strlen (fd[i].contenido) >= MAX_TEXTO_CONFIG){
					// demasiado grande
					salirFun ();
					return;
				} else {
					iContras = i;
					cant++;
				}
			}
		}
		// respondemos en consecuencia
		if (cant == ret && (iUsuario != -1 && iContras != -1)){
            //Leemos el usuario y contraseña de la memoria EEPROM 
            if (leeConfigEEPROM (&configTIM, sizeof(configTIM), &crc) == 0)
            {
			    // correcto
                // Verificamos contraseña y usuario
    			if (strcmp(fd[iUsuario].contenido, configTIM.usuario) != 0 || strcmp(fd[iContras].contenido, configTIM.pwdUsuario) != 0) {
    				salirFun();	// volvemos al ingreso
    				return;
    			}

    			// indicamos que se ingresó correctamente
    			login = 1;

    			// enviamos la página de configuración
    			strcpy (buffer_web_envio, redirectConfig);
                strcat (buffer_web_envio, "Connection: keep-alive\r\n");
                sprintf (&buffer_web_envio[strlen(buffer_web_envio)], cabContentLength, strlen (hyperlink_config_form));
    			strcat (buffer_web_envio, finCabecera);
                strcat (buffer_web_envio,hyperlink_config_form);

    			esp8266_enviar_datos_tcp (idSocketCliente, strlen(buffer_web_envio), buffer_web_envio);
		    } 
            else 
            {
                salirFun();
		    }     
		} else {
			// no se recibieron todos los datos
			salirFun();	// volvemos al ingreso
		}
	} else {
		// el formulario no tiene la cantidad correcta, respondemos error
		salirFun();	// volvemos al ingreso
	}
}

/*
 *********************************************************************
 * FIN Funciones que responden a solicitudes de páginas determinadas.
 *********************************************************************
 */

CY_ISR(contador_web_ISR){
    Timer_web_ReadStatusRegister();
    timeoutWeb++;
    isr_timer_web_ClearPending();
    return; 
}

/*
 * Función que inicializa lo que sea necesario para el servidor web de este
 * dispositivo.
 * En particular se inicializa_
 * - El ESP8266 para crear un AP y esperar la conexión de un cliente en una dirección IP
 *   determinada (192.168.4.1) puerto 80. Solo puede atender a un cliente.
 * - Configuramos también un temporizador para gestionar el timeout de las páginas.
 * - Inicializa la EEPROM donde se guardarán los datos de configuración de este TIM.
 * Parámetros:
 * 	Ninguno
 * Retorno:
 * 	Ninguno.
 */
void configServidor () {
    isr_timer_web_StartEx(contador_web_ISR);
    Timer_web_Start();
    uint32_t periodo_timer_web = Timer_web_ReadPeriod();
    periodo_timer_web_seg = (float)(1/periodo_timer_web);
    
	// inicializamos la EEPROM
	// Verificamos que contenga datos válidos. Si no contiene hay que inicializar la EEPROM con datos por defecto
	// Enviamos los comandos para inicializar el ESP8266
    int8_t valor_retorno_esp8266 = -1;  
    
    //Ponemos el ESP8266 en modo SoftAP (Access Point) para que una PC o telefono pueda conectarse a el.
    debug_enviar("SERVIDOR: Configurando ESP8266 en modo Estacion+Punto de acceso.");
    debug_enviar("\n");
    valor_retorno_esp8266 = esp8266_wifi_modo(3);
    while(valor_retorno_esp8266 != 0){
        valor_retorno_esp8266 = esp8266_wifi_modo(3);
    }
    debug_enviar("SERVIDOR: Configuracion exitosa.");
    debug_enviar("\n");
    debug_enviar("SERVIDOR: Creando el punto de acceso.");
    debug_enviar("\n");
    valor_retorno_esp8266 = esp8266_crear_softAP("WTIM-01","12345678",5,0,2);
    while(valor_retorno_esp8266 != 0){
        valor_retorno_esp8266 = esp8266_crear_softAP("WTIM-01","12345678",5,0,1);
    }
    debug_enviar("SERVIDOR: Configuracion exitosa.");
    debug_enviar("\n");
    // Creamos un socket servidor
    debug_enviar("SERVIDOR: Creando servidor en el ESP8266.");
    debug_enviar("\n");
	idSocketServ = esp8266_crear_servidor_tcp(80,1);
    while( idSocketServ != 0){
        idSocketServ = esp8266_crear_servidor_tcp(80,1);
    }
    debug_enviar("SERVIDOR: Configuracion exitosa.");
    debug_enviar("\n");
	
	// seteamos las variables globales con valores iniciales
	login = 0;
    idSocketCliente = -1;
    // Configuramos el temporizador para timeout del servidor web
	timeoutWeb = 0;
}

/*
 * Función que cierra el socket cliente.
 * Parámetros:
 * 	id -> identificador del socket
 * Retorno:
 * 	Ninguno, las variables globales afectadas se actualizan.
 */
void cierraSocketCliente (uint16_t id){
	if (idSocketCliente >= 0){
	    esp8266_cierra_socket_tcp(idSocketCliente);
    }
	idSocketCliente = -1;	// indicamos que no hay cliente conectado
}


/*
 * Función que procesa los pedidos con el método GET del HTTP.
 * Parámetros:
 * 	d -> puntero a la estructura de datos rellenada por el parser HTTP.
 * 	retParser -> retorno del parser HTTP
 * Retorno:
 * 	Ninguno, las variables globales afectadas se actualizan.
 */
void respuestaMetodoGet (HTTPparseData *d, int retParser){
	int i;

	// buscamos la página para responder apartir del for anterior
	for (i = 0; i < sizeof (paginasGET)/sizeof(recursoGET); i++){
		if (strcmp (d->rl.reqTarget, paginasGET[i].rec) == 0){
			paginasGET[i].recFun();
			break;	// si encontramos la página, atendemos y salimos del for
		}
	}
	if (i == sizeof (paginasGET)/sizeof(recursoGET)){
		// no se encontró el recurso
		notFoundFun();
		login = 0;	// salimos
	}
}

/*
 * Función que procesa los pedidos con el método POST del HTTP.
 * Parámetros:
 * 	d -> puntero a la estructura de datos rellenada por el parser HTTP.
 * 	retParser -> retorno del parser HTTP
 * Retorno:
 * 	Ninguno, las variables globales afectadas se actualizan.
 */
void respuestaMetodoPost (HTTPparseData *d, int retParser){
	int i;

	// buscamos la página para responder
	for (i = 0; i < sizeof (paginasPOST)/sizeof(recursoPOST); i++){
		if (strcmp (d->rl.reqTarget, paginasPOST[i].rec) == 0){
			if (retParser == -5) {
				// no hay datos en el post, eso es un error
				notFoundFun();
				login = 0;	// salimos
			} else {
				paginasPOST[i].recFun(d->data);
			}
			break;	// si encontramos la página, atendemos y salimos del for
		}
	}
	if (i == sizeof (paginasPOST)/sizeof(recursoPOST)){
		// no se encontró el recurso
		notFoundFun();
		login = 0;	// salimos
	}
}
/*
 * Función que espera una solicitud de página web y envía la respuesta. Además recibe los
 * datos de configuración y modifica la estructura de datos pertinente y guarda esos nuevos
 * datos en la EEPROM.
 * Esta función utiliza variables globales para almacenar el estado en que se encuentra.
 * Parámetros:
 * 	Ninguno
 * Retorno:
 * 	Ninguno.
 */
void servidorWeb (){
	int retHTTPparser;
    int16_t cant_bytes_recibidos_tcp = 0;
    char buffer_itoa[20];
	// Si timeout se cierra el socket del cliente
	if ((timeoutWeb > TIMEOUT_ACTIVIDAD_WEB) && (pagina_web_actual == configuracion))
    {
		// cerramos el socket cliente
        debug_enviar("SERVIDOR => TIMEOUT! Se cierra el socket.");
        debug_enviar("\n");
		cierraSocketCliente (idSocketCliente);
		timeoutWeb = 0;	// inicializamos contador
		login = 0;		// indicamos que no hay ningún cliente logeado
        pagina_web_actual = ingreso;
	}

	// Ver si hay alguien que solicita conexión y aceptar, crear socket del cliente
	if (idSocketCliente < 0)
    {
        debug_enviar("SERVIDOR => Esperando cliente.");
        debug_enviar("\n");
		// si no hay cliente vemos si se conecto alguno
		idSocketCliente = accept (idSocketServ);
        if (idSocketCliente >= 0)
        {
            debug_enviar("SERVIDOR => Cliente conectado.");
            debug_enviar("\n");
        }
	} 
    else 
    {
		// Ver si hay datos para leer en el socket del cliente
        cant_bytes_recibidos_tcp = esp8266_leer_datos_tcp(idSocketCliente,(unsigned char *)buffer_web_recibido);
		if (cant_bytes_recibidos_tcp > 0)
        {
			// No hubo error
			// ejecutamos el parser HTTP
            sprintf(buffer_temporal,"SERVIDOR => Cantidad de bytes recibidos: %d\n", cant_bytes_recibidos_tcp);
            int dump = 0;
			if ((retHTTPparser = 
                httpParser((unsigned char *)buffer_web_recibido, strlen(buffer_web_recibido), &datosHTTP)) == 0 || retHTTPparser == -5)
            {
//                debug_enviar("SERVIDOR => HEADER HTTP Recibido: \r\n");
//                debug_enviar("Metodo: ");
//                debug_enviar(datosHTTP.rl.method);
//                debug_enviar("\r\n");
//                debug_enviar("Objetivo:  ");
//                debug_enviar(datosHTTP.rl.reqTarget);
//                debug_enviar("\r\n");
//                debug_enviar("Version:  ");
//                debug_enviar(datosHTTP.rl.HTTPVersion);
//                debug_enviar("\r\n");
//                debug_enviar("Header fields: ");
//                debug_enviar(datosHTTP.hf->fName);
//                debug_enviar(datosHTTP.hf->fValue);
//                debug_enviar("\r\n");
//                debug_enviar("Cantidad Header fields:");
//                itoa(datosHTTP.cantHF,buffer_itoa,10);
//                debug_enviar(buffer_itoa);
//                debug_enviar("\r\n");
//                debug_enviar("Datos: "); 
//                debug_enviar(datosHTTP.data);
//                debug_enviar("\r\n");
				// esperamos solamente GET y POST
				if (strcmp(datosHTTP.rl.method, METODO_GET) == 0)
                {
//                    debug_enviar("SERVIDOR => Recibido metodo GET");
//                    debug_enviar(datosHTTP.rl.method);
//                    debug_enviar(datosHTTP.data);
//                    debug_enviar("Parser:" );
//                    itoa(retHTTPparser,buffer_itoa,10);
//                    debug_enviar(buffer_itoa);
					respuestaMetodoGet (&datosHTTP, retHTTPparser);
				} 
                else if (strcmp(datosHTTP.rl.method, METODO_POST) == 0)
                {
					respuestaMetodoPost (&datosHTTP, retHTTPparser);
				} 
                else 
                {
					//enviarRespuestaErrorMetodo ();	// informamos los métodos que aceptamos
					esp8266_cierra_socket_tcp(idSocketCliente);
				}
			} 
            else
            {
				// Error en el parser HTTP
				esp8266_cierra_socket_tcp(idSocketCliente);
			}
		}
        else 
        {
            if(cant_bytes_recibidos_tcp == -3)
            {
                idSocketCliente = -1;
            }
			// ERROR al recibir paquete del ESP8266, procesar
			//esp8266_cierra_socket_tcp(idSocketCliente);
            
		}
	}
    if(pagina_web_actual == configuracion)
    {
        sprintf(timeout_web_cadena,"SERVIDOR => Contador de Timeout: %.3f seg.",0.002*timeoutWeb);
        debug_enviar(timeout_web_cadena);
        debug_enviar("\n");
    } 
}


struct configuracion obtener_configTIM(){
    return configTIM; 
} 





