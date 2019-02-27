/*
 * ip.c
 *
 *  Created on: 7 ago. 2018
 *      Author: lp1-2018
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/*
 * Convierte una dirección IP (número de 32 bits sin signo) a ASCII en el formato
 * estandar de IPv4 (Por ejemplo: 168.192.5.1).
 * Parámetros:
 * 	dirIP -> dirección IP como número de 32 bits sin signo
 *  dir -> dirección de la posición de memoria donde se pondrá la dirección IP en ASCII.
 *         Debe tener, al menos, 16 posiciones de memoria.
 * Retorno:
 * 	El valor de dir, donde se encuentra el valor en ASCII
 * 	en el formato YYY.YYY.YYY.YYY
 */
char *IP2ASCII (uint32_t dirIP, char *dir){
//    static char dir[16];

    sprintf(dir, "%d.%d.%d.%d", (int)(dirIP >> 24) & 0xFF, (int)(dirIP >> 16) & 0xFF,
            (int)(dirIP >> 8) & 0xFF, (int)dirIP & 0xFF);

    return dir;
}

/*
 * *************> ESTA FUNCIÓN NO ES THREAD SAFE <***************
 * Convierte una cadena a un número de 32 bits sin signo. Verifica el formato correcto.
 * Si hay error retorna 0, si no hay error retorna la dirección IP.
 * Parámetros:
 * 	buf -> dirección de la cadena que contiene la dirección en el formato YYY.YYY.YYY.YYY
 * Retorno:
 * 	Dirección IP como número de 32 bits sin signo o 0 si ocurrió algún error
 */
uint32_t ASCII2uint32 (uint8_t *buf){
	char *temp;
	int valor;
	uint32_t ret;

	if ((temp = strtok((char *)buf, "\r\n .")) == NULL){
		return 0;	// Se encontró el fin de cadena
	}
	valor = atoi(temp);
	if (valor > 255 || valor < 0){
		return 0;	// error
	}
	ret = (uint32_t)valor << 24;

	if ((temp = strtok(NULL, "\r\n .")) == NULL){
		return 0;	// Se encontró el fin de cadena
	}
	valor = atoi(temp);
	if (valor > 255 || valor < 0){
		return 0;	// error
	}
	ret |= ((uint32_t)valor << 16);

	if ((temp = strtok(NULL, "\r\n .")) == NULL){
		return 0;	// Se encontró el fin de cadena
	}
	valor = atoi(temp);
	if (valor > 255 || valor < 0){
		return 0;	// error
	}
	ret |= ((uint32_t)valor << 8);

	if ((temp = strtok(NULL, "\r\n .")) == NULL){
		return 0;	// Se encontró el fin de cadena
	}
	valor = atoi(temp);
	if (valor > 255 || valor < 0){
		return 0;	// error
	}
	return ret |= ((uint32_t)valor);
}

/* [] END OF FILE */
