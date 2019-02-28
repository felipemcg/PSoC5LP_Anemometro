/*
 * paginas.h
 *
 *  Created on: 3 ago. 2018
 *      Author: lp1-2018
 */
#ifndef PAGINAS_H_
#define PAGINAS_H_

const char *redirect =	"HTTP/1.1 303 See Other\r\n"
		                "Location: /index.html\r\n";

const char *redirectConfig = 	"HTTP/1.1 303 See Other\r\n"
        						"Location: /config_form.html\r\n";
    
const char *hyperlink_config_form = "<!DOCTYPE html>\r\n"
                        "<html lang=\"es\">\r\n"
                        "<body>\r\n"
                        "<a href=\"/config_form.html\"> </a>\r\n"
                        "</body>\r\n"
                        "</html>\r\n";
    
const char *hyperlink_index = "<!DOCTYPE html>\r\n"
                        "<html lang=\"es\">\r\n"
                        "<body>\r\n"
                        "<a href=\"/index.html\"> </a>\r\n"
                        "</body>\r\n"
                        "</html>\r\n";

const char *cabeceraOK = 		"HTTP/1.1 200 OK\r\n";
const char *cabContentType = 	"Content-Type: text/html; charset=utf-8\r\n";
const char *cabContentLength = 	"Content-Length: %d\r\n";
const char *cabCacheControl =   "Cache-control: no-cache\r\n";
const char *finCabecera = 		"\r\n";
    
const char *index_p = 	"<!DOCTYPE html>\r\n"
						"<html lang=\"es\">\r\n"
                        "<head>\r\n"
                        "<title>Portal Anem&oacute;metro</title>\r\n"
                        "</head>\r\n"
						"<body>\r\n"
						"<h1>Introduzca sus credenciales</h1>\r\n"
						"<form action=\"/ingreso.html\" method=\"POST\">\r\n"
						"Usuario:<br>\r\n"
						"<input type=\"text\" name=\"userid\">\r\n"
						"<br>\r\n"
						"Contrase&ntilde;a:<br>\r\n"
						"<input type=\"password\" name=\"psw\"><br>\r\n"
						"<br>\r\n"
						"<input type=\"submit\" value=\"Ingresar\">\r\n"
						"</form>\r\n"
						"</body>\r\n"
						"</html>\r\n";

const char *config_form = 	"<!DOCTYPE html>\r\n"
							"<html lang=\"es\">\r\n"
                            "<head>\r\n"
                            "<title>Portal Anem&oacute;metro</title>\r\n"
                            "</head>\r\n"
							"<body>\r\n"
							"<h2>Configuraci&oacute;n de este TIM</h2>\r\n"
							"<p>Datos del Access Point al que se va a conectar este TIM</p>\r\n"
							"<form action=\"/config.html\" method=\"POST\">\r\n"
							"SSID del AP:<br>\r\n"
							"<input type=\"text\" name=\"SSID\" value=\"%s\"><br>\r\n"
							"Contrase&ntilde;a:<br>\r\n"
							"<input type=\"text\" name=\"contras\">\r\n"
							"<br>\r\n"
							"<p>Datos del NCAP al que se va a conectar este TIM</p>\r\n"
							"Direcci&oacute;n IP del NCAP:<br>\r\n"
							"<input type=\"text\" name=\"IP_NCAP\" value=\"%s\">\r\n"
							"<br>\r\n"
							"Puerto del NCAP:<br>\r\n"
							"<input type=\"text\" name=\"P_NCAP\" value=\"%d\">\r\n"
							"<br><br>\r\n"
							"<input type=\"submit\" value=\"Enviar\">\r\n"
							"</form>\r\n"
                            "<h2>Estado actual de la conexi&oacute;n del TIM</h2>\r\n"
                            "<ul style=\"list-style-type:circle;\">\r\n"
                            "<li>Conexi&oacute;n al punto de acceso:<span style=\"color:%s\">%s</span></li>\r\n"
                            "<li>Conexi&oacute;n al servidor NCAP:<span style=\"color:%s\">%s</span></li>\r\n"
                            "</ul>\r\n"
							"<br>\r\n"
							"Para cambiar la contrase&ntilde;a y usuario ingrese <a href=\"/cambiar_form.html\">aqu&iacute;</a>.\r\n"
							"<br><br>\r\n"
							"<a href=\"/salir.html\"><strong>Salir de la configuraci&oacute;n</strong></a>\r\n"
							"</body>\r\n"
							"</html>\r\n";

const char *cambiar_form = 	"<!DOCTYPE html>\r\n"
							"<html lang=\"es\">\r\n"
                            "<head>\r\n"
                            "<title>Portal Anem&oacute;metro</title>\r\n"
                            "</head>\r\n"
							"<body>\r\n"
							"<h2>Cambiar usuario y contrase&ntilde;a</h2>\r\n"
							"<p>Solo se dispone de un usuario y su correspondiente contrase&ntilde;a en este TIM</p>\r\n"
							"<form action=\"/cambiar.html\" method=\"POST\">\r\n"
							"Usuario Actual:<br>\r\n"
							"<input type=\"text\" name=\"uActual\">\r\n"
							"<br>\r\n"
							"Contrase&ntilde;a Actual:<br>\r\n"
							"<input type=\"password\" name=\"cActual\">\r\n"
							"<br>\r\n"
							"Usuario nuevo:<br>\r\n"
							"<input type=\"text\" name=\"uNuevo\">\r\n"
							"<br>\r\n"
							"Contrase&ntilde;a nueva:<br>\r\n"
							"<input type=\"password\" name=\"cNueva\">\r\n"
							"<br><br>\r\n"
							"<input type=\"submit\" value=\"Enviar\">\r\n"
							"</form>\r\n"
							"<br><br>\r\n"
							"<a href=\"/salir.html\"><strong>Salir de la configuraci&oacute;n</strong></a>\r\n"
							"</body>\r\n"
							"</html>\r\n";

const char *salir =	"<!DOCTYPE html>\r\n"
					"<HTML lang=\"es\">\r\n"
					"<head>\r\n"
					"<meta http-equiv=\"refresh\" content=\"0;url=/index.html\">\r\n"
					"</head>\r\n"
					"</html>\r\n";

const char *cabeceraNotFound = "HTTP/1.1 404 Not Found\r\n";

const char *notFound = 	"<!DOCTYPE html>\r\n"
						"<html lang=\"es\">\r\n"
						"<head>\r\n"
						"<meta http-equiv=\"Content-type\" content=\"text/html;charset=UTF-8\">\r\n"
						"</head>\r\n"
						"<body>\r\n"
						"<center>\r\n"
						"<h1>404 - Archivo no encontrado</h1>\r\n"
						"<h3>Intente <a href=\"/index.html\">aqu&iacute;</a>.</h3>\r\n"
						"</center>\r\n"
						"</body>\r\n"
						"</html>\r\n";

const char *errorConfig =	"<!DOCTYPE html>\r\n"
							"<html lang=\"es\">\r\n"
                            "<head>\r\n"
                            "<title>Portal Anem&oacute;metro</title>\r\n"
                            "</head>\r\n"
							"<body>\r\n"
							"<h2>ERROR, LOS DATOS DEL TIM NO HAN SIDO CAMBIADOS</h2>\r\n"
							"<br>\r\n"
							"Verifique los valores de direcci&oacute;n IP y puerto introducidos.<br><br><br>\r\n"
							"<a href=\"/config.html\"><strong>Volver</strong></a><br>\r\n"
							"<a href=\"/salir.html\"><strong>Salir de la configuraci&oacute;n</strong></a>\r\n"
							"</body>\r\n"
							"</html>\r\n";

const char *configCorrecta =	"<!DOCTYPE html>\r\n"
								"<html lang=\"es\">\r\n"
                                "<head>\r\n"
                                "<title>Portal Anem&oacute;metro</title>\r\n"
                                "</head>\r\n"
								"<body>\r\n"
								"<h2>Configuraci&oacute;n realizada con &eacute;xito</h2>\r\n<br><br>\r\n"
								"<a href=\"/salir.html\"><strong>Salir de la configuraci&oacute;n</strong></a>\r\n"
								"</body>\r\n"
								"</html>\r\n";

const char *errorEEPROM =	"<!DOCTYPE html>\r\n"
							"<html lang=\"es\">\r\n"
                            "<head>\r\n"
                            "<title>Portal Anem&oacute;metro</title>\r\n"
                            "</head>\r\n"
							"<body>\r\n"
							"<h2>ERROR AL ESCRIBIR LA EEPROM</h2>\r\n<br><br>\r\n"
							"<a href=\"/salir.html\"><strong>Salir de la configuraci&oacute;n</strong></a>\r\n"
							"</body>\r\n"
							"</html>\r\n";

const char *errorCambiar =	"<!DOCTYPE html>\r\n"
							"<html lang=\"es\">\r\n"
                            "<head>\r\n"
                            "<title>Portal Anem&oacute;metro</title>\r\n"
                            "</head>\r\n"
							"<body>\r\n"
							"<h2>ERROR, LA CONTRASE&Ntilde;A Y/O EL USUARIO NO HA SIDO CAMBIADO</h2>\r\n"
							"<br>\r\n"
							"Verifique los valores de direcci&oacute;n IP y puerto introducidos.<br><br><br>\r\n"
							"<a href=\"/cambiar.html\"><strong>Volver</strong></a><br>\r\n"
							"<a href=\"/salir.html\"><strong>Salir de la configuraci&oacute;n</strong></a>\r\n"
							"</body>\r\n"
							"</html>\r\n";

const char *cambiarCorrecta =	"<!DOCTYPE html>\r\n"
								"<html lang=\"es\">\r\n"
                                "<head>\r\n"
                                "<title>Portal Anem&oacute;metro</title>\r\n"
                                "</head>\r\n"
								"<body>\r\n"
								"<h2>Cambio de usuario y/o contrase&ntilde;a exitoso</h2>\r\n<br><br>\r\n"
								"<a href=\"/salir.html\"><strong>Salir de la configuraci&oacute;n</strong></a>\r\n"
								"</body>\r\n"
								"</html>\r\n";

#endif /* PAGINAS_H_ */
