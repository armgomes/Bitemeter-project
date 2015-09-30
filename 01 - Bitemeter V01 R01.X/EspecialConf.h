/*******************************************************************************
 *                                  LIZARD BYTE
 *******************************************************************************
 * Nome do Arquivo:     EspecialConf.h
 *******************************************************************************/

#ifndef ESPECIALCONF_H
#define	ESPECIALCONF_H

/** B I T S   D E   C O N F I G U R A Ç Ã O   D O   P I C 1 8 F 2 7 J 1 3 ******/

//Watchdog Timer:
#pragma config WDTEN        = OFF
//96MHz PLL Prescaler Selection (PLLSEL=0):
#pragma config PLLDIV       = 5          // Divide by 5 (20 MHz oscillator input)
//PLL Enable Configuration Bit:
#pragma config CFGPLLEN     = ON      
//Stack Overflow/Underflow Reset:
#pragma config STVREN       = OFF
//Extended Instruction Set:
#pragma config XINST        = OFF
//Code Protect:
#pragma config CP0          = OFF
//Oscillator:
#pragma config OSC          = HSPLL         // HS + PLL
//T1OSC/SOSC Power Selection Bits:
#pragma config SOSCSEL      = DIG       
//EC Clock Out Enable Bit:
#pragma config CLKOEC       = OFF
//Fail-Safe Clock Monitor:
#pragma config FCMEN        = OFF
//Internal External Oscillator Switch Over Mode:
#pragma config IESO         = ON
//Watchdog Postscaler:
#pragma config WDTPS        = 4         //1:4 Espera cerca de 16ms para realizar o reset
//DSWDT Clock Select:
#pragma config DSWDTOSC     = INTOSCREF
//RTCC Clock Select:
#pragma config RTCOSC       = INTOSCREF
//Deep Sleep BOR:
#pragma config DSBOREN      = OFF
//Deep Sleep Watchdog Timer:
#pragma config DSWDTEN      = OFF       
//Deep Sleep Watchdog Postscaler:
#pragma config DSWDTPS      = 2        
//IOLOCK One-Way Set Enable bit:
#pragma config IOL1WAY      = ON
//ADC 10 or 12 Bit Select:
#pragma config ADCSEL       = BIT12
//PLL Selection Bit:
#pragma config PLLSEL       = PLL96
//MSSP address masking:
#pragma config MSSP7B_EN    = MSK5

//Write/Erase Protect Page Start/End Location:
//#pragma Mode default

//Write/Erase Protect Configuration Region :
//// Função ignorada devido WPDIS == 0

//Write Protect Disable bit:
#pragma config WPDIS        = ON

//Write/Erase Protect Region Select bit (valid when WPDIS = 0):
// Função ignorada devido WPDIS == 0

#endif	/* ESPECIALCONF_H */

