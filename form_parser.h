/*
 * form_parser.h
 *
 *  Created on: 27 jul. 2018
 *      Author: lp1-2018
 */

#ifndef FORM_PARSER_H_
#define FORM_PARSER_H_

/*
 * Estructura de datos que almacena los pares nombre y contenido
 * de un form HTML.
 */
typedef struct formDataDef {
	char *nombre;
	char *contenido;
} formData;


/*
 * Función que quita los espacios en blanco (según isspace) del final
 * de una cadena.
 * Parámetros:
 *  c -> dirección de la cadena
 * Retorno:
 * 	Ninguno
 */
void quitaEspaciosFinal (char *c);

/*
 * Función que quita los espacios en blanco (según isspace) de delante
 * de una cadena.
 * Parámetros:
 *  c -> dirección de la cadena
 * Retorno:
 * 	Ninguno
 */
void quitaEspaciosDelante (char *c);

/*
 * Función que separa los pares nombre/contenido de un
 * mensaje POST o GET con el tipo de contenido "content type"
 * application/x-www-form-urlencoded.
 * Parámetros:
 * 	c -> cadena que contiene los pares}
 * 	f -> espacio de memoria para almacenar los pares separados
 * 	maxDatos -> cantidad máxima de pares
 * Retorno:
 * 	 0 -> no hubo error
 * 	-1 -> en uno de los datos solo se leyó el nombre y no el contenido
 * 	-2 -> no hay suficiente memoria para almacenar todos los pares
 */
int formParser (char *c, formData *f, uint16_t maxDatos);

#endif /* FORM_PARSER_H_ */
