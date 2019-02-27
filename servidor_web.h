/*
 * servidor_web.h
 *
 *  Created on: 3 ago. 2018
 *      Author: lp1-2018
 */

#ifndef SERVIDOR_WEB_H_
#define SERVIDOR_WEB_H_

/*
 * Definición de tipo de los recursos disponibles en este servidor y
 * las funciones que generan la página
 */
typedef struct recursoGET {
	char *rec;
	void (*recFun)(void);
}recursoGET;

typedef struct recursoPOST {
	char *rec;
	void (*recFun)(char *c);
}recursoPOST;

/*
 * Estructura de datos con información de configuración de este nodo.
 */
#define MAX_TEXTO_CONFIG	30
struct configuracion {
	uint32_t dirIPNCAP;					// Dirección IP del NCAP al que se conectará este nodo
	uint16_t puertoNCAP;				// Puerto de la IP del NCAP al que se conectará este nodo
	char SSID_AP[MAX_TEXTO_CONFIG];		// SSID del AP
	char pwdAP[MAX_TEXTO_CONFIG];		// Contraseña del AP
	char usuario[MAX_TEXTO_CONFIG];		// Nombre del usuario de este TIM, el que puede cambiar la configuración
	char pwdUsuario[MAX_TEXTO_CONFIG];	// Contraseña del usuario de este TIM
};

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
void configServidor (void);

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
void servidorWeb (void);

/*
 * ********************************************
 * TEMPORAL, SOLO PARA PUREBAS
 */
void indexFun (void);
void configFormFun(void);
void cambiarFormFun(void);
void salirFun(void);

struct configuracion obtener_configTIM();

#endif /* SERVIDOR_WEB_H_ */
