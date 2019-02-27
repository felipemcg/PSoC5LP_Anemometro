/*
 * comunicacion-ncap.c
 *
 *  Created on: 16 ago. 2018
 *      Author: lp1-2018
 */
#include "project.h"

// tiempos de espera
#define	SEGUNDO	50	// un segundo si la interrupción del timer es cada 20 ms
#define ESPERA_INICIAL			(3 * SEGUNDO)
#define ESPERA_CONNECION_AP 	(20 * SEGUNDO)

// los posibles estados de esta máquina
enum estConNCAP {
	esperaInicial,		// esperamos a que se inicie el ESP8266
	esperaConexionAP,	// esperamos a que se conecte al AP
	esperaConexionNCAP,	// espera a que se conecte al NCAP
	conectado			// estamos conectados al NCAP
};

struct error {
	uint8_t esperaConexionAP;
	uint8_t esperaConexionNCAP;
	uint8_t estadoDefault;
} error;

// variable de estado de esta máquina
enum estConNCAP estadoConeccionNCAP;

// variable que contiene la cuenta para el timeout
uint32_t timeout;

// Bandera que indica que estamos conectados al NCAP
uint8_t bandConectado;

// configuración del AP
struct datosAP {
	//const char ssid[] = "ProfesoresCYT";
	//const char psw[] = "profeCYT2018";
} datosAP;

// datos del NCAP
struct datosNCAP {
	//const char dirIP[] = "10.10.15.3";
	//const uint16_t puerto = 2568;
} datosNCAP;

// Buffer para poner los datos recibidos del ESP8266
char datos[512];

/*
 * Esta función inicializa la máquina de estados de conexión con el NCAP.
 * Parámetros:
 * 	Ninguno.
 * Retorno:
 * 	Ninguno.
 */
void inicComNCAP (){
	// inicializamos el temporizador que servirá de timeout

	// inicializamos la máquina de estados
	estadoConeccionNCAP = esperaInicial;

	// limpiar bandera de errores
	error.esperaConexionAP = 0;
	error.esperaConexionNCAP = 0;
	error.estadoDefault = 0;

	// Bandera de conexión limpia
	bandConectado = 0;
}

/*
 * Esta función intenta conectarse con el NCAP, si es que no estamos conectados aun.
 * Este TIM se conecta al NCAP y se queda esperando un comando del NCAP. Los posibles
 * comandos son: realizar una medición y programar una medición cada una cierta
 * cantidad de tiempo.
 * Parámetros:
 * 	Ninguno.
 * Retorno:
 * 	Ninguno.
 */
void conectarNCAP () {
	switch (estadoConeccionNCAP){
	case esperaInicial:
		bandConectado = 0;
		if (timeout > ESPERA_INICIAL){
			//ESP8266WiFiConnect (datosAP.ssid, datosAP.psw);
			estadoConeccionNCAP = esperaConexionAP;
			timeout = 0;
		}
		break;
	case esperaConexionAP:
		bandConectado = 0;
//		if (ESP8266HayDatos ()){
//			if(ESP8266okRecibido ()){
//				ESP8266ClientConServer (datosNCAP.dirIP, datosNCAP.puerto);
//				estadoConeccionNCAP = esperaConexionNCAP;
//				timeout = 0;
//			}
//		}
		if (timeout > ESPERA_CONNECION_AP){
			// error en el ESP8266
			error.esperaConexionAP++;
			timeout = 0;
		}
		break;
	case esperaConexionNCAP:
		bandConectado = 0;
//		if (ESP8266HayDatos ()){
//			if(ESP8266okRecibido ()){
//				estadoConeccionNCAP = conectado;
//				bandConectado = 1;
//				timeout = 0;
//			}
//		}
		if (timeout > ESPERA_CONNECION_AP){
			// error en el ESP8266
			error.esperaConexionNCAP++;
			timeout = 0;
		}
		break;
	case conectado:
		bandConectado = 1;
		break;
	default:
		bandConectado = 0;
		error.estadoDefault++;
		break;
	}
}

/*
 * Esta función busca un comando del NCAP y lo ejecuta. Si el sistema está realizando
 * medidas cada cierto tiempo, en esta función se verifica si hay información que
 * enviar al NCAP. El sistema solo puede estar en uno un otro modo de operación.
 * Parámetros:
 * 	Ninguno.
 * Retorno:
 * 	Ninguno.
 */
void comandosNCAP () {
	if (bandConectado){
//		if (ESP8266RecibeDatos (datos) == 0){
//			// se recibieron datos del NCAP - procesamos
//
//		} else {
//			// tratamos los errores en la comunicación con el NCAP
//		}
	}
}
