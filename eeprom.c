/*
 * eeprom.c
 *
 *  Created on: 31 dic. 2017
 *      Author: alumno-lp1
 */

#include <stdint.h>

#include "project.h"
#include "crc16.h"

/*
 * Variable que contiene el CRC calculado para las funciones de este archivo
 */
uint16_t crc;

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
int8_t escribeConfigEEPROM (void *config, uint16_t tamConfig){
	uint8_t i;
	uint8_t *punt = (uint8_t *) config;
	uint16_t nBloques = (tamConfig/CYDEV_EEPROM_ROW_SIZE) +
			            ((tamConfig - ((tamConfig/CYDEV_EEPROM_ROW_SIZE) * CYDEV_EEPROM_ROW_SIZE)) ? 1 : 0);

	if (tamConfig > CYDEV_EE_SIZE){
		return -2;	// No hay espacio en la EEPROM
	}

	EEPROM_CONFIGURACION_UpdateTemperature();

	// calculamos CRC
	crc = crc16((uint8_t *) config, tamConfig);
	// escribimos la configuración
	for (i = 0; i < nBloques; i++){
//		 escribimos en bloques de 16 bytes
		if (EEPROM_CONFIGURACION_Write(punt + (i * CYDEV_EEPROM_ROW_SIZE), i) != CYRET_SUCCESS){
			return -1;
		}
	}
//	 escribimos el CRC
	if (EEPROM_CONFIGURACION_ByteWritePos((uint8_t)(crc & 0xFF), i, 0) != CYRET_SUCCESS){			// primera posición de la siguiente fila de la EEPROM
		return -1;
	}
	if (EEPROM_CONFIGURACION_ByteWritePos((uint8_t)((crc >> 8) & 0xFF), i, 1)!= CYRET_SUCCESS){	// segunda posición de la siguiente fila de la EEPROM
		return -1;
	}

	return 0;
}


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
int8_t leeConfigEEPROM (void *config, uint16_t tamConfig, uint16_t *crcDest){
	uint32_t i;
	uint8_t *punt = (uint8_t *)config;
	uint16_t nBloques = (tamConfig/CYDEV_EEPROM_ROW_SIZE) + ((tamConfig - ((tamConfig/CYDEV_EEPROM_ROW_SIZE) * CYDEV_EEPROM_ROW_SIZE)) ? 1 : 0);

	// leemos la configuración
	for (i = 0; i < tamConfig; i++, punt++){
		// leemos por bytes
		*punt = EEPROM_CONFIGURACION_ReadByte(i);
	}

	// leemos el CRC de la EEPROM
	*crcDest = EEPROM_CONFIGURACION_ReadByte(nBloques * CYDEV_EEPROM_ROW_SIZE) |
			   EEPROM_CONFIGURACION_ReadByte((nBloques * CYDEV_EEPROM_ROW_SIZE) + 1) << 8;

	// calculamos CRC
	crc = crc16((uint8_t *)config, tamConfig);

	if (crc == *crcDest) {	// Comparamos el CRC calculado con el leído
		return 0;
	} else {
		return -1;
	}
}
