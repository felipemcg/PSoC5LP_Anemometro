/*
 ============================================================================
 Name        : servidorWeb.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "form_parser.h"
#include "servidor_web.h"

#include "eeprom.h"

/*
 * cantidad máxima de pares nombre/contenido de los forms.
 */
#define MAX_DATOS 10

formData fd[MAX_DATOS];

int main(void) {
	// configuramos el socket servidor
	configServidor ();

//	indexFun ();
//	configFormFun();
//	cambiarFormFun();
//	salirFun();
	// ciclo infinito de espera
	while (1){
		// si hay datos de configuración del NCAP y no estamos conectados aún, nos conectamos
		// Aquí habría que ver si hay cambios en la configuración del NCAP para cerrar el socket actual y abrir otro
		// De esta función no se sale hasta que se recibe respuesta de los comandos seriales ejecutados
		//conectarNCAP ();

		// si se recibe un pedido de página web se envía la página, aquí hay que cuidar lo del password y el timeout
		// De esta función no se sale hasta que se recibe respuesta de los comandos seriales ejecutados
		servidorWeb ();

		// Vemos si hay un comando del NCAP para hacer la medida. Si se está en modo de medida cada un cierto tiempo y
		// si hay datos que transmitir y estamos conectados al NCAP se envían. No debería haber datos que transmitir si
		// no estamos conectados.
		//comandosNCAP ();
	}
	return EXIT_SUCCESS;
}

//int main(void) {
//	char cad[100] = "say=Hi&to=Mom&  password = holaquetal & parte = todos ";
//	int i = formParser (cad, fd, MAX_DATOS);
//
//	printf ("El retorno es %d\n", i);
//
//	for (; i > 0; i--){
//		quitaEspaciosFinal (fd[i-1].nombre);
//		quitaEspaciosFinal (fd[i-1].contenido);
//
//		quitaEspaciosDelante (fd[i-1].nombre);
//		quitaEspaciosDelante (fd[i-1].contenido);
//
//		printf ("\"%s\" = \"%s\"\n", fd[i-1].nombre, fd[i-1].contenido);
//	}
//
//	return EXIT_SUCCESS;
//}
