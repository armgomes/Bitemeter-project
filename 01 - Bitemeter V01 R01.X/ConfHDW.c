/*******************************************************************************
 *                                  LIZARD BYTE
 *******************************************************************************
 * Nome do Arquivo:     ConfHDW.c
 *******************************************************************************/

/** I N C L U D E S **********************************************************/
#include <adc.h>
#include <timers.h>
#include <usart.h>
#include <pps.h>
#include "ConfHDW.h"
#include "SDCard.h"
#include <flash.h>

/** V A R I A V E I S   G L O B A I S ****************************************/
float alfa, b;
int setPoint;
char calibracaoOk;

/** F U N C O E S ************************************************************/

/******************************************************************************
 * Funcao:          void ConfHDW (void)
 * Entrada:         Nenhuma (void)
 * Saída:           Nenhuma (void)
 * Descrição:       Configura hardware, periféricos e interrupções
 *****************************************************************************/
void ConfHDW (void)
{
    int flagCalibracao;
/************************** Remapeando o hardware *****************************/

    PPSUnLock();

    iPPSInput (IN_FN_PPS_INT1,IN_PIN_PPS_RP4);              // Configura INT1 no pino 22
#if DEBUG
    iPPSInput (IN_FN_PPS_INT2,IN_PIN_PPS_RP9);              // Configura INT2 no pino 27
#endif
    iPPSInput (IN_FN_PPS_SDI2,IN_PIN_PPS_RP8);              // Configura SDI2 no pino 26

    iPPSOutput(OUT_PIN_PPS_RP7,OUT_FN_PPS_SDO2);            // Configura SDO2 no pino 25
    iPPSOutput(OUT_PIN_PPS_RP6,OUT_FN_PPS_SCK2);            // Configura SCK2 no pino 24

    PPSLock();


/************************** Configuração I/O **********************************/

    // Configura entradas digitais
    TRISBbits.TRISB0 = 1;           // RB0: entrada digital - POWER
    TRISBbits.TRISB1 = 1;           // RB1: entrada digital - START
    TRISBbits.TRISB6 = 1;           // RB6: entrada digital - POWER BLUETOOTH
    TRISCbits.TRISC3 = 1;           // RC3: entrada digital - Switch SD Card (Pode ser retirada se não o utilizar o bootloader)
    

    // Configura saídas digitais
    TRISCbits.TRISC0 = 0;           // RC0: saída digital - LED_POWER
    TRISCbits.TRISC1 = 0;           // RC1: saída digital - LED_STATUS
    TRISCbits.TRISC5 = 0;           // RC3: saída digital - KEY
    TRISCbits.TRISC4 = 0;           // RC4: saída digital - POWER BLUETOOTH
    TRISCbits.TRISC2 = 0;           // RC2: saída digital - POWER SD CARD

    INTCON2bits.RBPU = 0;           //Habilita Pull-up


/************************Configuração Periféricos******************************/

// Configura o Modo ADC

    OpenADC(
            // config
            ADC_FOSC_64 &           // Frequência de operação do dispositivo Fad=Fosc/64 = 750kHz
            ADC_RIGHT_JUST &        // Bits justificados a direita
            ADC_8_TAD,              // Tempo de amostragem - 8 vezes (evitando o aliasing do sinal)
            // config 2
            ADC_CH0 &               // Configura o canal 0
            ADC_INT_OFF,            // Habilita a interrupção
            // config 3
            ADC_REF_VDD_VSS,
            // portconfig
            ADC_VBG_OFF);            // VBG output of Band Gap módulo está desabilitado

    //ANCON0: A/D PORT CONFIGURATION REGISTER 1 (BANKED F49h)
    //PCFG7(1) PCFG6(1) PCFG5(1) PCFG4 PCFG3 PCFG2 PCFG1 PCFG0
    //  x         x         x      0     1     1     0     0
    ANCON0 = 0x0C;                  // Configura AN0, AN1 e AN4 e restante como entrada digital

    //ANCON1: A/D PORT CONFIGURATION REGISTER 1 (BANKED F49h)
    //VBGEN r ? PCFG12 PCFG11 PCFG10 PCFG9 PCFG8
    //  0   x -    1      1      1     1      1
    ANCON1 = 0x1F;                  // Configura como entrada digital

    // Configura Timer 0
    // Configura o valor inicial do timer para obter 43.7ms
    OpenTimer0(
            // Config
            TIMER_INT_ON &              // Configura a interrupção do TIMER 0
            T0_16BIT &                  // Configura o TIMER 1 com 16 bits
            T0_SOURCE_INT &             // Clock interno
            T0_PS_1_256);               // Prescaler 256

    WriteTimer0(18661);                 // Ajusta o valor do tempo para 1000ms

// Configura Watchdog
    WDTCONbits.SWDTEN = 0;      //  SWDTEN: Software Enable or Disable the Watchdog Timer bit(1)
                                    // 1 = WDT is turned on
/***********************Configuração Interrupção*******************************/

    RCONbits.IPEN = 1;      // Habilita prioridades de interrupcao
    INTCONbits.GIEH = 0;     // Desabilita vetor de alta prioridade
    INTCONbits.GIEL = 0;    // Desabilita vetor de baixa prioridade

    //TIMER0
    INTCONbits.TMR0IE = 1;      // Habilita a interrupção do timer 0
    INTCONbits.TMR0IF = 0;      // Desliga o flag do timer 0 para ele nao interromper a CPU
    // Tratamento de interrupcao no vetor de alta prioridade

    //INT0
    INTCON2bits.INTEDG0 = 0;    // Borda de descida
    // INT0 por definição do hardware do uC é uma interrupcao no vetor de AlTA prioridade
    INTCONbits.INT0IE= 1;       // Habilita interrupcao INT0
    INTCONbits.INT0IF = 0;      // Limpa a sinalização da  interrupcao,

    //INT1
    INTCON2bits.INTEDG1 = 0;    // Borda de descida
    INTCON3bits.INT1IP = 1;     // Tratamento de interrupcao no vetor de AlTA prioridade
    INTCON3bits.INT1IE = 0;     // Desabilita a INT1
    INTCON3bits.INT1IF = 0;     // Limpa a sinalização da  interrupcao,

     //INT2
    INTCON2bits.INTEDG2 = 0;    // Borda de descida
    INTCON3bits.INT2IP = 0;     // Tratamento de interrupcao no vetor de BAIXA prioridade
    INTCON3bits.INT2IE = 0;     // Desabilita a INT2
    INTCON3bits.INT2IF = 0;     // Limpa a sinalização da  interrupcao,



    IPR1bits.RCIP = 0;          // Tratamento de interrupcao no vetor de BAIXA prioridade
    PIR1bits.RCIF = 0;          // Limpa a sinalização da  interrupcao

    /******************* Leitura da flash - alfa, b e setPoint*****************
     *
     * Endereço 0x1FBE0 até 0x1FBE3 - alfa
     * Endereço 0x1FBE4 até 0x1FBE7 - b
     * Endereço 0x1FBE8 até 0x1FBE9 - setPoint
     * Endereço 0x1FBEA - Flag de verificação da calibração
     * ************************************************************************/
    ReadFlash(0x1FBEA,2,&flagCalibracao);
    if (flagCalibracao != 0xFFFF)                   // Avalia se o sistema foi calibrado pelo menos uma vez
    {
        ReadFlash(0x1FBE0,4,&alfa);                 // Atualiza alfa com o valor armazenado na flash
        ReadFlash(0x1FBE4,4,&b);                    // Atualiza b com o valor armazenado na flash
        ReadFlash(0x1FBE8,2,&setPoint);             // Atualiza setPoint com o valor armazenado na flash
    }
    else calibracaoOk = OFF;                        // Sistema nunca foi calibrado assumindo os valores defaults de alfa, b e setPoint

}

void AbreUSART()
{

    // Configura o modo USART 1

    baud1USART(
               // baudconfig
               BAUD_IDLE_RX_PIN_STATE_HIGH &    // Sinal de RX ocioso em nivel lógico alto
               BAUD_IDLE_TX_PIN_STATE_HIGH &    // Sinal de TX ocioso em nivel lógico alto
               BAUD_8_BIT_RATE &                // Gerador de baudrate de 8 bits
               BAUD_WAKEUP_ON &                 // Habilita wake-up de modo sleep pelo RX da EUSART
               BAUD_AUTO_OFF );	// Desabilita auto-detecção de baudrate

    Open1USART(
               // Config
               USART_TX_INT_OFF &               // Interrupção desabilitada no TX
               USART_RX_INT_ON &                // Interrupção habilitada no RX
               USART_ASYNCH_MODE &              // Modo assícrono
               USART_EIGHT_BIT &                // Modo de comunicação em 8 bits
               USART_CONT_RX &                  // Modo de recepção: contínua
               USART_ADDEN_OFF &
               USART_BRGH_LOW,                  // Modo do gerador de baudrate: baud_low
               77	// spbrg: valor carregado no gerador de baudrate
                                        // para comunicação com 9600bps e Fosc=48MHz.
                                        // fórmula no modo baud_low:  spbrg = (Fosc / (baudrate*64) )-1
                                        // fórmula no modo baud_high: spbrg = (Fosc / (baudrate*16) )-1
              );
    BAUDCON1bits.ABDOVF = 1;        // 0 = Auto-baud timer did not overflow
    // TXSTAx: TRANSMIT STATUS AND CONTROL REGISTER
    // CSRC TX9 TXEN SYNC SENDB BRGH TRMT TX9D
    //   -   0   1    0     0    0    -     -
    TXSTA1 = 0x20;
    BAUDCON1bits.WUE = 0;           // 0 = Receiver is operating normally

    // Configurações do módulo EUSART (serial)
    // Pinos usados pela EUSART devem ser configurados como entrada digital,
    // conforme datasheet pg345
    TRISCbits.TRISC6 = 0;	//RC6 (TX - transmissão)
    TRISCbits.TRISC7 = 1;	//RC7 (RX - recepção)

    KEY = 0;                    // Desabilita a recepção do comando AT

}