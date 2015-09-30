/*******************************************************************************
 *                                  LIZARD BYTE
 *******************************************************************************
 * Nome do Arquivo:     Leituras.h
 *******************************************************************************/

#ifndef LEITURAS_H
#define	LEITURAS_H

/** I N C L U D E S **********************************************************/

#include <delays.h>
#include <adc.h>


/** D E F I N E s  ***********************************************************/

#define AMOSTRAS            63          // Quantidade da amostragem do sensor de for�a
// Anterando os #defines anteriores � necess�rio alterar a vari�vel global Leituras.c
/** V A R I A V E I S   G L O B A I S ****************************************/
//unsigned float leitura [amostras];            // Quantidade de amostras
// "leitura[AMOSTRAS]"

// Pode ser utilizado se houver interesse do envio da menor leitura e da maior leitura
//#define MENOR           1           // Menor valor lido da amostragem = (AMOSTRA - (AMOSTRAS - 1))
//#define MAIOR           61          // Maior valor lido da amostragem = (AMOSTRAS - 2)
//#define DIVISOR         61          // Divisor da m�dia = (AMOSTRAS - 2)

// Defines da calibra��o
#define N                   5                   // Quantidade das for�as de refer�ncias

#define F1                  1.0094              // F1=0.103 * 9.8       massa 103g
#define F2                  5.0470              // F2=0.515 * 9.8       massa 515g
#define F3                  10.1136             // F3=1.032 * 9.8       massa 1032g
#define F4                  14.4060             // F4=1.470 * 9.8       massa 1470g
#define F5                  20.3056             // F1=2.072 * 9.8       massa 2072g

#define F2_SUP              5.2994              // Limite superior +5%
#define F3_SUP              10.6193             // Limite superior +5%
#define F4_SUP              15.1263             // Limite superior +5%
#define F5_SUP              21.3209             // Limite superior +5%

#define F2_INF              4.7947              // Limite superior -5%
#define F3_INF              9.6079              // Limite superior -5%
#define F4_INF              13.6857             // Limite superior -5%
#define F5_INF              19.2903             // Limite superior -5%

#define TAXA_VARIACAO       -0.04003416         // Taxa de varia��o do sensor de for�a em fun��o da temperatura N/�C - Sensor A201-25
#define SETPOINTDEFAULT     27                  // Setpoint default de 30�C
#define ALFA_DEFAULT        9.4457              // Alfa default
#define B_DEFAULT           1260.0764           // Alfa default

#define RESOLUCAO (3.3/4095.0)                  // Resolu��o da refer�ncia do ADC com 12 bits
#define VBATREF         2.37                    // Tens�o de refer�ncia que leva o sistema alertar o usu�rio  Vbat = 6.5V


/**  P R O T O T I P O S   *****************************************************/

float *LeituraAmostras(void);
char StatusLeituras();
void PiscaLedAguardando(void);
void Tratamento_High_Interrupt(void);
float Apresenta2casas(float valor);
void ParametrosCalibracao(void);

#endif	/* LEITURAS_H */

