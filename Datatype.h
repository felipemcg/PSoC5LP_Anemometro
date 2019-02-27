/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#ifndef DATATYPE_H_
#define DATATYPE_H_

/* ----------------------- Inclusion de archivos de cabecera ----------------------------------*/
#include <device.h>
#include "stdio.h"


/* -------------------------------- Constantes ------------------------------------------------*/
/* Estados del TIM */
#define TIM_INIT    0x00
#define TIM_ACTIVE  0x01
#define TIM_SLEEP   0x02

/* Constantes para CMD_CLASS */
//Definidos por el IEEE 1451
#define COMMON_CMD      0x01
#define XDCR_IDLE       0x02
#define XDCR_OPERATE    0x03
#define XDCR_EITHER     0x04
#define TIM_SLEEP_      0x05
#define TIM_ACTIVE_     0x06
#define ANY_STATE       0x07
//Definidos por el usuario
#define RECONFIG_CMD    0x81


/* Commons Commands */
#define QUERY_TEDS          0x01
#define READ_TEDS_SEGMENT   0x02
#define WRITE_TEDS_SEGMENT  0x03
#define UPDATE_TEDS         0x04

/* Either Idle or Operate state commands */
#define TRANSDUCER_NA        0x00
#define TRANSDUCER_OPERATE          0x01
#define TRANSDUCER_IDLE             0x02
#define TRANSDUCER_DIGITAL_SIGNAL   0x18
#define TRANSDUCER_CALIBRATION_SIGNAL   0x19
    
/* Operate state commands */
#define READ_TRANSDUCER_CHANNEL 0x01
#define WRITE_TRANSDUCER_CHANNEL 0x02


/* Reconfiguration commands */
#define ADD_SENSOR              0x01
#define REMOVE_SENSOR           0x02
#define CREATE_SENSOR           0x03
#define CHANGE_SAMPLING_TIME    0x08

/* Otros */
#define TRUE    1
#define FALSE   0

/* Definiciones de version */
#define TIM_VERSION         1
#define IEEE1451_VERSION    1

/* Definiciones de cantidad de canales y direcciones BroadCast*/
#define N_CHANNELS  3
#define ADDR_GLOBAL 0xFFFF
#define ADDR_GROUP  0x8000
#define ADDR_TIM    0x0000

/* Definicion de tamaños */
#define PAYLOAD_LEN 20
#define TEDS_LEN    42

/* Filas y columnas de los TEDS */
#define nROWS       3
#define nCOLUMNS    16  

/* TEDS access codes */
#define MetaTEDS        0x01
#define PhyTEDS         0x13
#define TxdcrChTEDS     0x03
#define TxdcrChSCHTEDS  0x80

/*Numero maximo de transductores en este TIM*/
#define NTRANSDUCERS_MAX    5

#define SEND_FAIL       1
#define SEND_SUCCESS    2
    
  
/* --------------------------------- Estructuras ----------------------------------------------*/
//Paquete DATOS del TIM                                                                         .
typedef struct packed{
    uint16  transd;
    uint8   cmd_class;
    uint8   cmd_func;
    uint16  length;
    uint8   payload[TEDS_LEN];
}cmd_in;

//Paquete GENERADO por el TIM
typedef struct packed_out{
    uint16  transd;
    uint8   cmd_class;
    uint8   cmd_func;
    uint16  length;
    uint8   payload[PAYLOAD_LEN];
}cmd_out;

//Paquete de RESPUESTA
typedef struct packet_reply{
    uint8  sf; 
    uint16 length;
    uint8 payload[TEDS_LEN];
}cmd_reply;

/* ------------------------------ Variables Globales -------------------------------------------*/
extern uint8 WakeUpCmd_CM;                                  //Indica que el mote desperto al PSoC
extern uint8 WakeUpCmd_RTC;                                 //Indica que el RTC desperto al PSoC
extern uint8 UART_flag;                                     //
extern uint8 UART_ISR_flag;                                 //
extern uint8 UART_timeout;                                  //
extern uint8 ACKmote_arrival;                               //
extern uint8 packet_arrival;                                //Indica si un paquete llego exitosamente
extern uint8 RxBuffer[48];                                  //Contiene los datos recibidos por UART
extern uint16 RTC_SamplingTime;                                   //Tiempo de muestreo del nodo
extern uint8 sendSUCCESS;

//extern uint8 vector_calibracion_Canales[800];             //Contiene la senal digitalizada

uint8 TIM_State;                                            //Almacena el estado del TIM
uint8 reconfig_state;
//float (*ptrReadTxdcrCh[NTRANSDUCERS_MAX])() = {NULL};                     //Funciones que leen los canales
//uint8 TxdcrCH_State[NTRANSDUCERS_MAX] = {TRANSDUCER_NA};



/* ------------------------------------ TEDS ----------------------------------------------------*/
//uint8 MetaTEDS_1[nROWS][nCOLUMNS] = {{0x01,0x02,0x00,0x00,0x00,0x28,0x01,0x02,0x03,0x04,0x05,0x06,0x01,0x01,0x01,0x01},
//                                     {0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01},
//                                     {0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x81,0x82,0x00,0x00,0x00,0x00,0x00,0x00}
//                                    };
//uint8 PhyTEDS_1[nROWS][nCOLUMNS] = {{0x01,0x02,0x00,0x00,0x00,0x13,0x03,0x04,0x05,0x13,0x01,0x01,0x0A,0x01,0x02,0x0B},
//                                     {0x04,0x00,0x00,0x00,0xFF,0x0C,0x02,0x00,0xFF,0x0D,0x02,0x00,0xFF,0x0E,0x02,0x00},
//                                     {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x60,0x00,0x00,0x00,0x00,0x00,0x00}
//                                    };



/* ------------------------------Prototipos de funciones----------------------------------------*/
#endif
//[] END OF FILE

