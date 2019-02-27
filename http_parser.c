/*
 * http_parser.c
 *
 *  Created on: 18/4/2016
 *      Author: Vicente González
 */
#include <string.h>
#include "http_parser.h"

/*
 * Un mensaje HTTP que recibe un servidor contiene:
 * - request line, formada por 3 elementos: method, request target y HTTP-version
 * - una o más header fields, formado por: field-name y field-value (separados por : y espacios)
 * Si hay un error en request line se retorna inmediatamente con un error.
 * También se retorna con un error si no hay datos, aunque todos los valores del
 * request line y de header fields tienen valores correctos.
 * Parámetros:
 * 	b -> puntero a la cadena que contiene el paquete HTTP
 * 	tam -> tamaño del paquete HTTP
 * 	pd -> puntero donde están los datos separados
 * 	maxFieldData -> cantidad máxima de field data disponibles en pd
 * Retorno:
 * 	0 si no hubo error detectado
 * 	Posibles errores (valor negativo)
 * 	-1 -> Espacio antes del Método
 * 	-2 -> No hay CRLF al final del Request
 * 	-3 -> Error de formato en el request line
 * 	-4 -> Demasiados Headers Field
 * 	-5 -> No hay datos
 */
int httpParser(unsigned char *b, int tam, HTTPparseData *pd){
	char *pUltimo, *pSig;
	int i;

	if (*b == ' ' || *b == '\t' || *b == '\n' || *b == '\r'){
		// hay un espacio al inicio
		return -1;	// error
	}
	// separamos la primera línea
	if (strtok((char *)b, "\r\n") == NULL){	// pone el NULL al final de la línea
		// no encontró CRLF
		return -2;
	}
	pd->rl.method = strtok((char *)b, " ");	// leemos el método
	pd->rl.reqTarget = strtok(NULL, " ");	// leemos el target
	pd->rl.HTTPVersion = strtok(NULL, " ");	// leemos la versión
	if (pd->rl.method == NULL || pd->rl.reqTarget == NULL || pd->rl.HTTPVersion ==NULL){
		// Error en request line
		return -3;
	}

	// separamos los header fields
	for (i = 0; i < pd->cantHF; i++){	// ponemos a cero el resto de la estructura
		pd->hf[i].fName = pd->hf[i].fValue = NULL;
	}
	pd->data = NULL;

	pUltimo = pd->rl.HTTPVersion + strlen(pd->rl.HTTPVersion) +1;
	if (*pUltimo == '\n'){	// Si al final hay un CRLF
		pUltimo ++;
	}
	for (i = 0; i < pd->cantHF; i++){
		if (pUltimo[0] == '\n' || pUltimo[0] == '\r'){
			// línea vacía, salimos
			break;
		}
		if (strtok(pUltimo, "\r\n") == NULL){	// pone el NULL al final de la línea
			// no encontró CRLF, esto indica fin de la cabecera HTML
			break;
		}
		pSig = pUltimo + strlen(pUltimo) +1;
		if (*pSig == '\n'){	// Si al final hay un CRLF
			pSig++;
		}
		pd->hf[i].fName = strtok(pUltimo, ":");
		if ((pd->hf[i].fValue = strtok(NULL, "\0")) == NULL){
			// No hay : en la línea del header
			return -6;
		}
		pUltimo = pSig;
	}

	if (i == pd->cantHF){
		// demasiados header fields
		return -4;
	}
//	for(; *pUltimo != '\0'; pUltimo++){
//		if (*pUltimo == ' ' || *pUltimo == '\n' || *pUltimo == '\r'){
//			continue;
//		} else {
//			break;
//		}
//	}
	// quitamos espacios antes de los datos
	for(; *pUltimo != '\0' && (*pUltimo == ' ' || *pUltimo == '\n' || *pUltimo == '\r'); pUltimo++);

	if (*pUltimo == '\0'){
		// no hay datos
		return -5;
	}

	pd->data = pUltimo;

	return 0;
}
