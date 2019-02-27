/*
 * html_parser.h
 *
 *  Created on: 20/4/2016
 *      Author: guest
 */

#ifndef HTTP_PARSER_H_
#define HTTP_PARSER_H_

/*
 * Estructura de datos que contiene apuntadores a los
 * distintos elementos del Request Header.
 * Se supone que usaremos la función strtok, la cual
 * agrega '\0' al final de cada token, por lo que
 * podemos tratar a cada elemento como una cadena
 * C estándar.
 */
typedef struct {
	char *method;
	char *reqTarget;
	char *HTTPVersion;
} requestLine;

/*
 * Estructura de datos que contiene apuntadores a los
 * dos elementos del Header Field.
 * Se supone que usaremos la función strtok, la cual
 * agrega '\0' al final de cada token, por lo que
 * podemos tratar a cada elemento como una cadena
 * C estándar.
 */
typedef struct{
	char *fName;
	char *fValue;
}HTTPheaderFields;

/*
 * Estructura de datos que contiene apuntadores a los
 * campos de la cabecera HTTP.
 * Se supone que usaremos la función strtok, la cual
 * agrega '\0' al final de cada token, por lo que
 * podemos tratar a cada elemento como una cadena
 * C estándar.
 *********************************************************
 * CUIDADO: Hay que crear espacio para los Header Fields
 * e inicializar el campo hf adecuadamente.
 *********************************************************
 */
//#define MAX_HEADER_FIELDS	20
typedef struct HTTPparseData{
	requestLine rl;
	HTTPheaderFields *hf;
	int cantHF;
	char *data;
}HTTPparseData;


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
int httpParser(unsigned char *b, int tam, HTTPparseData *pd);

#endif /* HTTP_PARSER_H_ */
