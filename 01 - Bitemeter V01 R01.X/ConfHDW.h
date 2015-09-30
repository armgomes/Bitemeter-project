/*******************************************************************************
 *                                  LIZARD BYTE
 *******************************************************************************
 * Nome do Arquivo:     ConfHDW.h
 *******************************************************************************/

#ifndef CONFHDW_H
#define	CONFHDW_H

/** I N C L U D E S **********************************************************/
#include <p18cxxx.h>	// Biblioteca com as definições do PIC selecionado


/** D E F I N E S ************************************************************/
#define ON                  1
#define OFF                 0
#define LIGADO              0
#define DESLIGADO           1
#define START               PORTBbits.RB1
#define PW_BLUETOOTH        PORTBbits.RB6
#define POWER               PORTBbits.RB0

#define LED_POWER           LATCbits.LATC0
#define LED_STATUS          LATCbits.LATC1
#define KEY                 LATCbits.LATC5
#define POWER_BLUETOOTH     LATCbits.LATC4
#define POWER_SDCARD        LATCbits.LATC2

#define DEBUG               1
// Define criado com o objetivo de habilitar ou desabilitar o pino 27
// "1" funcionamento normal "0" modo debug

/** P R O T O T I P O S *******************************************************/
 extern void ConfHDW (void);
 void AbreUSART(void);

#endif