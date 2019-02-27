/*
 * ip.h
 *
 *  Created on: 7 ago. 2018
 *      Author: lp1-2018
 */

#ifndef IP_H_
#define IP_H_

/*
 * Convierte una dirección IP (número de 32 bits sin signo) a ASCII en el formato
 * estandar de IPv4 (Por ejemplo: 168.192.5.1).
 * Parámetros:
 * 	dirIP -> dirección IP como número de 32 bits sin signo
 *  dir -> dirección de la posición de memoria donde se pondrá la dirección IP en ASCII.
 *         Debe tener, al menos, 16 posiciones de memoria.
 * Retorno:
 * 	El valor de dir, donde se encuentra el valor en ASCII
 * 	en el formato AAA.BBB.CCC.DDD
 */
char *IP2ASCII (uint32_t dirIP, char *dir);

/*
 * Convierte una cadena a un número de 32 bits sin signo. Verifica el formato correcto.
 * Si hay error retorna 0, si no hay error retorna la dirección IP.
 * Parámetros:
 * 	buf -> dirección de la cadena que contiene la dirección en el formato AAA.BBB.CCC.DDD
 * Retorno:
 * 	Dirección IP como número de 32 bits sin signo
 */
uint32_t ASCII2uint32 (uint8_t *buf);

#endif /* IP_H_ */
