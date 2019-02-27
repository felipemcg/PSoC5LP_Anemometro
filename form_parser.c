/*
 * form_parser.c
 *
 *  Created on: 27 jul. 2018
 *      Author: lp1-2018
 */
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include "form_parser.h"

/*
 * Función que quita los espacios en blanco (según isspace) del final
 * de una cadena.
 * Parámetros:
 *  c -> dirección de la cadena
 * Retorno:
 * 	Ninguno
 */
void quitaEspaciosFinal (char *c){
	char *temp = &c[strlen(c) -1];

	while (isspace (*temp))
		*temp-- = '\0';

}

/*
 * Función que quita los espacios en blanco (según isspace) de delante
 * de una cadena.
 * Parámetros:
 *  c -> dirección de la cadena
 * Retorno:
 * 	Ninguno
 */
void quitaEspaciosDelante (char *c){
	char *temp = c;

	while (isspace (*temp))
		temp++;

	if (temp != c)
		while ((*c++ = *temp++) != '\0');

}


/*
 * Función que separa los pares nombre/contenido de un
 * mensaje POST o GET con el tipo de contenido "content type"
 * application/x-www-form-urlencoded.
 * Parámetros:
 * 	c -> cadena que contiene los pares}
 * 	f -> espacio de memoria para almacenar los pares separados
 * 	maxDatos -> cantidad máxima de pares
 * Retorno:
 * 	 valor positivo -> no hubo error, es la cantidad de pares leídos
 * 	-1 -> en uno de los datos solo se leyó el nombre y no el contenido
 * 	-2 -> no hay suficiente memoria para almacenar todos los pares
 */
int formParser (char *c, formData *f, uint16_t maxDatos){
	int i, j;

	// inicializamos la estructura de datos
	for (i = 0; i < maxDatos; i++){
		f[i].nombre = f[i].contenido = NULL;
	}

	// cargamos la estructura de datos formData
	for (i = 0, j = 0; i < maxDatos; i++){
		// buscamos el primer caracter que no sea espacio en blanco
		while (isspace (c[j]))
			j++;
		if (c[j] == '\0' || c[j] == '\r' || c[j] == '\n'){
			if (c[j] != '\0'){
				c[j] = '\0';
			}
			return i+1;	// final de los datos
		}

		f[i].nombre = &c[j];	// guardamos el nombre

		// buscamos el '='
		while (c[j] != '=' && c[j] != '\0' && c[j] != '\r' && c[j] != '\n')
			j++;
		if (c[j] == '\0' || c[j] == '\r' || c[j] == '\n')
			return -1;	// Error, falta el contenido

		c[j] = '\0';	// cambiamos el = por NULL
		f[i].contenido = &c[++j];	// guardamos el contenido y saltamos el =

		// buscamos el '&'
		while (c[j] != '&' && c[j] != '\0' && c[j] != '\r' && c[j] != '\n')
			j++;
		if (c[j] == '\0' || c[j] == '\r' || c[j] == '\n'){
			if (c[j] != '\0'){
				c[j] = '\0';
			}
			return i+1;	// Terminó
		}
		c[j] = '\0';	// cambiamos el & por NULL

		j++;	// saltamos el '&'
	}

	if (i == maxDatos){
		return -2;	// error, no hay más lugar
	}

	return i;
}
