/*
 * eeprom.h
 *
 *  Created on: 31 dic. 2017
 *      Author: alumno-lp1
 */

#ifndef EEPROM_H_
#define EEPROM_H_

#include <stdint.h>

/*
 * Función que escribe datos en la EEPROM, se escribe por filas de 16 bytes cada una.
 * En total se escriben un número entero de filas, que puede ser mayor o igual a las necesarias.
 * El CRC se escribe en la fila siguiente, en los dos primeros bytes.
 * Esta función calcula el CRC antes de escribir la EEPROM.
 * Parámetros:
 * 	config ->  puntero a la posición donde se encuentran los datos a salvar.
 * 	tamConfig -> tamaño en bytes de los datos a salvar.
 * Retorno:
 * 	 0 si no hubo error
 * 	-1 si ocurrió algún error en escritura;
 * 	-2 si la estructura no cabe en la EEPROM disponible
 */
int8_t escribeConfigEEPROM (void *config, uint16_t tamConfig);

/*
 * Función que lee datos de la EEPROM, lo mismo que el CRC guardado.
 * Esta función verifica el CRC.
 * Parámetros:
 * 	config ->  puntero a la posición donde se guardarán los datos leídos.
 * 	tamConfig -> tamaño en bytes de los datos a ser leídos.
 * 	crcDest -> puntero a la posición donde se guardará el CRC leído
 * Retorno:
 * 	 0 si el CRC es correcto
 * 	-1 si el CRC no es correcto
 */
int8_t leeConfigEEPROM (void *config, uint16_t tamConfig, uint16_t *crcDest);

#endif /* EEPROM_H_ */
