/*******************************************************************************
 *                                  LIZARD BYTE
 * ******************************************************************************
 * Nome do Arquivo:     main.c
 * Processador:       	PIC18LF27J13
 * Opção de Clock:	HS 20MHz (externo) - CPU: 48MHz
 * Compilador:        	C18 v3.47 ou superior
 * Autor:		Arnaldo de Matos Gomes
 * Versão:              01
 *******************************************************************************/

/** I N C L U D E S **********************************************************/

#include <delays.h>
#include <stdio.h>
#include <adc.h>
#include <usart.h>
#include <rtcc.h>
#include <string.h>
#include "EspecialCONf.h"       // CONfiguração especial do PIC18L27J13
#include "Leituras.h"
#include "ConfHDW.h"
#include "FSIO.h"
#include "SDCard.h"



/** D E F I N E s  ***********************************************************/
#define DATA_SIZE           15  // Define o número de caracteres dos comandos via bluetooth

/** V A R I A V E I S   G L O B A I S ****************************************/
char *dadoRecebido;
char comandoRecebido = OFF;
char SDCardMenorValor[8];
char SDCardMedia[8];
char SDCardMaiorValor[8];
char SDCardTemp[4];
char toggleBluetooth = OFF;
char toggleBluetoothINT2 = OFF;
int timeoutBluetooth = 0;
char togglePowerOn = OFF;
int timeoutPowerOn = 0;
char PowerOffBluetooth = OFF;                 
int OffsetTemperatura = 0;                 
char calibracao = OFF;
char loopAquisicao = OFF;


/**  P R O T O T I P O S   P R I V A D O S ***********************************/
void Tratamento_Low_Interrupt(void);
void PiscaLed(char led);
void atraso500ms(void);
void TensaoDeReferencia(void);
void ConvFloatStringTemperatura(float dado);
void ConvFloatStringMedidas(float dado);
void EnviaComandoBluetooth(char rom *comando);
void SDCardConvFloatStrMenorValor(float dado);
void SDCardConvFloatStrMaiorValor(float dado);
void SDCardConvFloatStrMedia(float dado);
void SDCardConvFloatStrTemp(float dado);
void RTCC_Configura(char hora, char min, char seg, char dia, char mes, char ano, char diaSem);
int ConvStringInt(char *comando, char parcela);
char ConvDecimalHex(char valor);
char ConverteBCDparaDecimal(char valor);
void LeUSART(char *buffer, unsigned char len);

//---Global structures used in deep sleep library---
rtccTimeDate RtccTimeDate ,Rtcc_read_TimeDate;

/** F U N C O E S ************************************************************/

/******************************************************************************
 * Funcao:          void main(void)
 * Entrada:         Nenhuma (void)
 * Saída:           Nenhuma (void)
 * Descrição:       Função principal do programa.
 *****************************************************************************/

void main(void)
{

    
    char contEventos = 0;                           // Informa os eventos de leituras
    static char envia_titulo[] = "Temperatura\n";   // Envia String para o S2 Terminal Bluetooth
//    char envia_menor[] = "Menor Valor\n";         // Pode ser utilizada para o envio da menor leitura
//    char envia_maior[] = "Maior Valor\n";         // Pode ser utilizada para o envio da maior leitura
    static char envia_media[] = "Force\n";          // Envia String para o S2 Terminal Bluetooth
#if !DEBUG
    char ligarBluetooth = ON;                       // Utilizada para realizar o modo debug do bluetooth
#endif
    
        
    char i = 0;    
    DWORD ultimaLeitura;                        // Informa a última leitura registrada com sucesso no SD Card
    DWORD contLeitura;                          // Registra a quantidade de aquisições do momento
    BYTE SDCardNoSlot = FALSE;                  // SD Card presente no slot
    BYTE SDCardInicializado = FALSE;            // SD Card foi inicializado corretamente
    BYTE retorno;                               // Variável para debug do SD Card
    BYTE falhaSDCard = 0;                       // Status de falha do SD Card

    BYTE dia;
    BYTE mes;
    WORD ano;
    BYTE hora;
    BYTE min;
    BYTE seg;

    // Configura o hardware
    ConfHDW();

    // Configura RTCC com valores default
    RTCC_Configura(13,00,00,01,02,15,0);
    /*--- RTCC e suas particularidades de ajuste dos campos ---
     * Ajustando a hora
     * 0x00 - 12 AM
     * 0x01 - 1 AM
     *
     * 0x35 - 11PM
     * A mesma lógica vale para os minutos, segundos, mês e o ano*/

    // Condição Inicial do Hardware ao conectar a alimentação ou no power reset
    LED_STATUS = DESLIGADO;
    LED_POWER = DESLIGADO;           
    POWER_BLUETOOTH = DESLIGADO;
    POWER_SDCARD = DESLIGADO;

    // Habilita vetores de Alta e Baixa Prioridade
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;

    while(1)
    {

        SDCardInicializado = FALSE;     // Cada Power On inicia o SD Card se estiver no Slot
        SDCardNoSlot = FALSE;
        Sleep();                        // Rotina que coloca o processador no modo Sleep
  
        while (togglePowerOn)
        {
            
    /*-------------------- Máquina de estado POWER BLUETOOTH ---------------------*/

            // # if 1 - Aplicativo ~ # if 0 Debug - Ver arquivo ConfHDW.h
#if DEBUG
            if (toggleBluetoothINT2 && !toggleBluetooth || calibracao)
            {

                AbreUSART();                // Configura a USART

//                POWER_BLUETOOTH = LIGADO;   // Utilizado apenas no modo Debug se for de interesse

                toggleBluetooth = ON;


                    while (!PW_BLUETOOTH);                  // Mantem enquanto o PW_BLUTEOOTH for acionado
                    Delay10KTCYx(240);                      // Debounce 200ms


            }


            if (!toggleBluetoothINT2 && toggleBluetooth)
                {
                    Close1USART();              // Fecha a USART para evitar receber dados no power off!
                    Delay10KTCYx(240);          // Espera 200ms antes de prosseguir no algoritmo

//                    POWER_BLUETOOTH = DESLIGADO;          // Utilizado apenas no modo Debug se for de interesse

                    toggleBluetooth = OFF;

                    timeoutBluetooth = 0;

                        while (!PW_BLUETOOTH);                  // Mantem enquanto o PW_BLUTEOOTH for acionado
                        Delay10KTCYx(240);                      // Debounce 200ms
                }
#else

            if (ligarBluetooth)
            {

                AbreUSART();                // Configura a USART

                POWER_BLUETOOTH = LIGADO;

                toggleBluetooth = ON;

                ligarBluetooth = OFF;

                        
            }
# endif



     /*----------------------- Máquina de estado SD Card --------------------------*/

            if (!SDCardNoSlot && !SD_CD && !SDCardInicializado)     // Verifica a presença do cartão no slot
            {
                INTCONbits.GIEH = 0;
                INTCONbits.GIEL = 0;
                
                SDCardNoSlot = ON;
                Delay10KTCYx(120);                                  // Aguarda 100ms para alimentar o SD Card
                POWER_SDCARD = LIGADO;


                if (SDCardPresente())
                {

                    // Procedimento para leitura da data e hora
                    RtccReadTimeDate(&Rtcc_read_TimeDate);

                    dia = Rtcc_read_TimeDate.f.mday;
                    dia = ConverteBCDparaDecimal(dia);
                    mes = Rtcc_read_TimeDate.f.mon;
                    mes = ConverteBCDparaDecimal(mes);
                    ano = Rtcc_read_TimeDate.f.year;
                    ano = ano + 2000;
                    hora = Rtcc_read_TimeDate.f.hour;
                    hora = ConverteBCDparaDecimal(hora);
                    min = Rtcc_read_TimeDate.f.min;
                    min = ConverteBCDparaDecimal(min);
                    seg = Rtcc_read_TimeDate.f.sec;
                    seg = ConverteBCDparaDecimal(seg);

                    // Rotina pertecente ao SD Card
                    SetClockVars (ano, mes, dia, hora, min, seg);

                    ultimaLeitura = SDCardLeArquivo();
                    contLeitura = ultimaLeitura;                        // Atualiza as leituras de acordo LISTA.TXT

                        if (!ultimaLeitura)                             // Cria o diretório e o arquivo 0_LISTA e LISTA.TXT
                        {
                            retorno = SDCardSistemaDeArquivo();

                            if(!retorno){falhaSDCard = 2;}               // Falha no sistema de arquivos
                            else{contLeitura = 1;}                       // Inicializa o contador de aquisições
                        }

                        SDCardInicializado = TRUE;

                }
                else                                                     // Falha na inicialização
                {

                    falhaSDCard = 1;
                    PiscaLed(0);
                    PiscaLed(0);
                    PiscaLed(0);
                }

            INTCONbits.GIEH = 1;
            INTCONbits.GIEL = 1;

            }


                if (SDCardNoSlot && SD_CD)
                {
                    INTCONbits.GIEH = 0;
                    INTCONbits.GIEL = 0;

                    SDCardNoSlot = OFF;
                    SDCardInicializado = FALSE;
                    POWER_SDCARD = DESLIGADO;
                    falhaSDCard = 0;                                   // Reset das falhas
                    PiscaLed(0);
                    PiscaLed(0);
                    PiscaLed(0);

                    INTCONbits.GIEH = 1;
                    INTCONbits.GIEL = 1;
    
                }

     /*--- Fim Parcial SD Card ---*/

    /*--------------------------- A T E N Ç Ã O -----------------------------------
     Por questão de organização do fluxo de dados o restante do algoritmo responsável
     por gravar os dados aquisitados estão implementados na máquina do Bluetooth*/

    /*------------------------ Máquina de estado AQUISIÇÕES-----------------------*/

            if (StatusLeituras())
            {
                float *resultado, menorValor, media, maiorValor, temperatura;
                timeoutPowerOn = 0;             

                INTCONbits.GIEH = 0;
                INTCONbits.GIEL = 0;

                resultado = LeituraAmostras();
                menorValor = resultado[0];
                media = resultado[1];
                maiorValor = resultado[2];
                temperatura = resultado[3];
               

                if(toggleBluetooth)                 // Bluetooth ligado
                {
                    timeoutBluetooth = 0;           // TODO - Aguarda a aplicação do Android para resetar se houver troca de dados

                    // Pode ser utilizado se houver interesse do envio da menor leitura e da maior leitura
                    // Transmite via Bluetooth os resultados!
//                    puts1USART(envia_menor);
//                    Busy1USART();
//                    ConvFloatStringMedidas(menorValor);
//                    Busy1USART();
//
//                    puts1USART(envia_maior);
//                    Busy1USART();
//                    ConvFloatStringMedidas(maiorValor);
//                    Busy1USART();

                    puts1USART(envia_media);
                    Busy1USART();
                    ConvFloatStringMedidas(media);
                    Busy1USART();

                    puts1USART(envia_titulo);
                    Busy1USART();
                    ConvFloatStringTemperatura(temperatura);
                }

    /*------------------------ Grava dados no SD Card -----------------------------*/

                if (SDCardInicializado)
                {
                    // Pode ser utilizado se houver interesse do envio da menor leitura e da maior leitura
//                    SDCardConvFloatStrMenorValor(menorValor);
//
//                    SDCardConvFloatStrMaiorValor(maiorValor);

                    SDCardConvFloatStrMedia(media);

                    SDCardConvFloatStrTemp(temperatura);

                    // Procedimento para leitura da data e hora
                    RtccReadTimeDate(&Rtcc_read_TimeDate);

                    dia = Rtcc_read_TimeDate.f.mday;
                    dia = ConverteBCDparaDecimal(dia);
                    mes = Rtcc_read_TimeDate.f.mon;
                    mes = ConverteBCDparaDecimal(mes);
                    ano = Rtcc_read_TimeDate.f.year;
                    ano = ano + 2000;
                    hora = Rtcc_read_TimeDate.f.hour;
                    hora = ConverteBCDparaDecimal(hora);
                    min = Rtcc_read_TimeDate.f.min;
                    min = ConverteBCDparaDecimal(min);
                    seg = Rtcc_read_TimeDate.f.sec;
                    seg = ConverteBCDparaDecimal(seg);
          
                    // Rotina pertecente ao SD Card
                    SetClockVars (ano, mes, dia, hora, min, seg);
                    
                    // Pode ser utilizado se houver interesse do envio da menor leitura e da maior leitura
//                    retorno = SDCardGravaDados(contLeitura,SDCardMenorValor,SDCardMaiorValor,SDCardMedia,SDCardTemp);

                    retorno = SDCardGravaDados(contLeitura,SDCardMedia,SDCardTemp);

                    contLeitura++;                  // Atualiza a quantidade de aquisições até o momento
                    
                    if (!retorno){falhaSDCard = 3;}                   // Falha ao gravar dados

                    SDCardGravaLista(contLeitura);

                        if (!retorno){falhaSDCard = 4;}               // Falha ao gravar a lista

                            if (falhaSDCard == 3 || falhaSDCard == 4 && contLeitura > 0)
                            {
                                SDCardPresente();                    // Reinicia o SD Card se houver alguma falha
                                contLeitura--;
                            }


                                if (falhaSDCard)                     // Houve falha no SDCard - Renicialze o cartão e/ou verfique o seu sistema de arquivos
                                {
                                    PiscaLed(0);
                                    PiscaLed(0);
                                    PiscaLed(0);
                                    falhaSDCard = 0;
                                }
                    }

                                if(!toggleBluetooth && !SDCardInicializado)     // Não um modo válido para salvar os dados
                                {
                                    PiscaLed(0);
                                    PiscaLed(0);
                                    PiscaLed(0);
                                }
            /* O hardware não dedecta se o bluetooth não está emparelhado, o que pode implicar a não sinalização de um
             modo válido, porém o usuário deve ficar atento no recebimento dos dados no aplicativo do Android */

            LED_STATUS = DESLIGADO;         // Fim do ciclo de leitura dos dados

            contEventos++;

            INTCONbits.GIEH = 1;
            INTCONbits.GIEL = 1;

        }

                                    if (contEventos >= 9u)               // Confere o Vbat a cada 10 aquisções realizadas
                                    {
                                        TensaoDeReferencia();
                                        LED_POWER = LIGADO;
                                        contEventos = 0;
                                    }
    /*--- Fim Aquisições ---*/


     /*------------------ Máquina de estado comandos ANDROID ---------------------*/

            if (loopAquisicao)
            {
                delay500ms();
                delay500ms();
                delay500ms();
                delay500ms();              // Realiza uma aquisição a cada 2s

                INTCON3bits.INT1IF = 1;    // Mantem o sistema em loop de aquisição até que este seja desligado
            }

            if (comandoRecebido == ON)
            {

                int hora,min,seg,mes,dia,ano,diaSem;


                comandoRecebido = OFF;      

                switch (dadoRecebido[0])                        // Verifica o ID do comando
                {

                    case '1':                                   // Comando - Start aquisições

                        INTCON3bits.INT1IF = 1;

                        break;

                    case '2':                                   // Comando - Troca a senha do bluetooth

                        PIE1bits.RC1IE = 0;                     // Desabilita a interrupção da USART - Aguarda novo power ON
                        KEY = 1;                                // Habilita a recepção do comando AT - HC05

                        Delay10KTCYx(100);
                        EnviaComandoBluetooth("AT+PSWD=");      // HC05

                        for (i = 2;i < 6;i++)                   // Retira os quatros digitos da senha - xxxx
                        {
                            while(Busy1USART());
                            Write1USART(dadoRecebido[i]);
                        }

                        EnviaComandoBluetooth("\r");            // HC05
                        EnviaComandoBluetooth("\n");               
                      
                        KEY = 0;                                // Finaliza a recepção do comando AT

                        // Desliga Bluethoot
                        Close1USART();                          // Fecha a USART para evitar receber dados no power off!
                        Delay10KTCYx(240);                      // Espera 200ms antes de prosseguir no algoritmo

                        POWER_BLUETOOTH = DESLIGADO;

                        toggleBluetooth = OFF;

                        toggleBluetoothINT2 = OFF;

                        timeoutBluetooth = 0;

                        break;

                    case '3':                                   // Comando - Troca do nome bluetooth

                        PIE1bits.RC1IE = 0;                     // Desabilita a interrupção da USART - Aguarda novo power ON
                        KEY = 1;                                // Habilita a recepção do comando AT

                        Delay10KTCYx(100);
                        EnviaComandoBluetooth("AT+NAME=");

                        for (i = 2;i < 7;i++)                   // Retira os cinco digitos do nome - xxxxx
                        {
                            while(Busy1USART());
                            Write1USART(dadoRecebido[i]);
                        }

                        EnviaComandoBluetooth("\r");
                        EnviaComandoBluetooth("\n");

                        KEY = 0;                                // Finaliza a recepção do comando AT

                        // Desliga Bluethoot
                        Close1USART();                          // Fecha a USART para evitar receber dados no power off!
                        Delay10KTCYx(240);                      // Espera 200ms antes de prosseguir no algoritmo

                        POWER_BLUETOOTH = DESLIGADO;

                        toggleBluetooth = OFF;

                        toggleBluetoothINT2 = OFF;

                        timeoutBluetooth = 0;

                        break;

                    case '4':                                   // Comando - Ajuste data e hora
                        
                        hora    = ConvStringInt(dadoRecebido,2);
                        hora    = ConvDecimalHex(hora);
                        min     = ConvStringInt(dadoRecebido,4);
                        min     = ConvDecimalHex(min);
                        seg     = ConvStringInt(dadoRecebido,6);
                        seg     = ConvDecimalHex(seg);
                        dia     = ConvStringInt(dadoRecebido,8);
                        dia     = ConvDecimalHex(dia);
                        mes     = ConvStringInt(dadoRecebido,10);
                        ano     = ConvStringInt(dadoRecebido,12);
                        diaSem  = ConvStringInt(dadoRecebido,14);

                        RTCC_Configura(hora,min,seg,dia,mes,ano,diaSem);
                        break;

                    case '5':                                   // Offset da temperatura

                        if (dadoRecebido[2] ==  '+')
                        {
                            OffsetTemperatura = ConvStringInt(dadoRecebido,3);
                        }
                        if (dadoRecebido[2] ==  '-')
                        {
                            OffsetTemperatura = ConvStringInt(dadoRecebido,3);
                            OffsetTemperatura = 0 - OffsetTemperatura;
                        }

                        break;
                        
                    case '6':                                   // Retorna os parametros da calibração

                        ParametrosCalibracao();

                        break;

                    case '7':                                   // Comando - Start aquisições continuamente a cada 2s

                        loopAquisicao = ON;
                        INTCON3bits.INT1IF = 1;

                        break;

                    case '8':                                   // Comando - Power Off

                        PowerOffBluetooth = ON;
                        INTCONbits.INT0IF = ON;

                        break;
                        
                    default:
                        Nop();
                }

            }/*--- Fim Comandos Android*/

        }/*--- Fim togglePowerOn ----*/

    }/*--- Fim do while(1) ---*/

 }/*--- Fim do void main(void) --- */


/******************************************************************************
 * Funcao:		void Tratamento_Low_Interrupt(void)
 * Entrada:		Nenhuma (void)
 * Saída:		Nenhuma (void)
 * Descrição:	Função de tratamento das interrupções de BAIXA prioridade
 *****************************************************************************/
// ATENÇÃO NA SINTAXE de declaração com #pragma interruptlow = Baixa prioridade

#pragma interrupt Tratamento_Low_Interrupt
void Tratamento_Low_Interrupt(void)
{

    if (DataRdy1USART())
    {
        static char i;

        // Configura Watchdog

        WDTCONbits.SWDTEN = 1;      //  SWDTEN: Software Enable or Disable the Watchdog Timer bit(1)
                                    // 1 = WDT is turned on

        gets1USART(dadoRecebido,DATA_SIZE);

        WDTCONbits.SWDTEN = 0;      // Se ocorrer a recepção de todos os bits não há o reset da CPU - Time out ignorado!!!

        timeoutBluetooth = 0;       // Significa que algum dado chegou na usart

        for (i=0;i<DATA_SIZE;i++)
        {
            comandoRecebido = ON;
        }

        PIR1bits.RC1IF = 0;
    }

#if DEBUG
    if (INTCON3bits.INT2IF && INTCON3bits.INT2IE)
    {
        POWER_BLUETOOTH =! POWER_BLUETOOTH;
        if (POWER_BLUETOOTH == DESLIGADO)Close1USART(); // Fecha a USART para evitar receber dados no power off!
        toggleBluetoothINT2 =! toggleBluetoothINT2;
        while (!PW_BLUETOOTH);                          // Mantem enquanto o PW_BLUTEOOTH for acionado
        Delay10KTCYx(240);                              // Debounce 200ms
        INTCON3bits.INT2IF = 0;
    }
#endif
}// end Tratamento_Low_Interrupt


/******************************************************************************
 * Funcao:          PiscaLed
 * Entrada:         char
 * Saída:           Nenhuma (void)
 * Descrição:       Sinalização GERAL - LED STATUS e LED POWER
 *****************************************************************************/
void PiscaLed(char led)
{
    if(led)LED_POWER = LIGADO;
    else LED_STATUS = LIGADO;
    Delay10KTCYx(120);                          // 50ms ligado

    if(led)LED_POWER = DESLIGADO;
    else LED_STATUS = DESLIGADO;                // 50ms desligado
    Delay10KTCYx(120);

    if(led)LED_POWER = LIGADO;
    else LED_STATUS = LIGADO;
    Delay10KTCYx(120);

    if(led)LED_POWER = DESLIGADO;
    else LED_STATUS = DESLIGADO;
    Delay10KTCYx(120);

    if(led)LED_POWER = LIGADO;
    else LED_STATUS = LIGADO;
    Delay10KTCYx(120);

    if(led)LED_POWER = DESLIGADO;
    else LED_STATUS = DESLIGADO;
    Delay10KTCYx(240);                       // 100ms desligado
    Delay10KTCYx(240);
}


/******************************************************************************
 * Funcao:          atraso1seg
 * Entrada:         Nenhuma (void)
 * Saída:           Nenhuma (void)
 * Descrição:       Promove um delay de 1 segundo
 *****************************************************************************/

void atraso1seg(void)
{
    Delay10KTCYx(240);
    Delay10KTCYx(240);
    Delay10KTCYx(240);
    Delay10KTCYx(240);
    Delay10KTCYx(240);
}

/******************************************************************************
 * Funcao:          atraso1seg
 * Entrada:         Nenhuma (void)
 * Saída:           Nenhuma (void)
 * Descrição:       Promove um delay de 500ms
 *****************************************************************************/

void atraso500ms(void)
{
    Delay10KTCYx(240);
    Delay10KTCYx(240);
    Delay10KTCYx(120);
}


/******************************************************************************
 * Funcao:          TensaoDeReferencia
 * Entrada:         Nenhuma (void)
 * Saída:           Nenhuma (void)
 * Descrição:       Verifica a condição de Vbat
 *****************************************************************************/
void TensaoDeReferencia(void)
{
    char i;
    float Vbat = 0, Vbateria = 0;
    SetChanADC(ADC_CH4);                    // Seta o canal ADC do sensor de temperatura
    for (i = 0;i <5;i++)                    // Realiza a média com 5 medidas
    {
        ConvertADC();
        while(BusyADC());
        Vbat += ReadADC();
    }
    Vbat /= 5;
    Vbat *= RESOLUCAO;
    Vbateria = Vbat * 2.74;                 // Informa o valor estimado da tensão da bateria
    
    if (!toggleBluetooth)
    {
    AbreUSART();                            // Configura a USART
    ConvFloatStringMedidas(Vbateria);       // Envia o valor lido para o terminal Bluetooth
    Close1USART();                          // Fecha a USART para evitar receber dados no power off!
    }else ConvFloatStringMedidas(Vbateria);     

    if (Vbat < VBATREF)
    {
        for(i = 0;i < 2;i++)                // Sinaliza que a tensão de entrada está abaixo do nível adequado ao funcionamento
        {
            PiscaLed(1);
        }
        LED_POWER = LIGADO;
    }
}

/******************************************************************************
 * Funcao:          ConvFloatStringTemperatura
 * Entrada:         Nenhuma (float)
 * Saída:           Nenhuma (void)
 * Descrição:       Converte o valor float para string - Temperatura
 *****************************************************************************/
 void ConvFloatStringTemperatura(float dado)
 {
     int inteiro;
     char array[4];                         // Determina o número de casas a serem convertidas (Ex: 100\NUL)
     inteiro = dado;                        // Recebe a parte inteira do float
     sprintf(array,"%2d",inteiro);          // Realiza a conversão para string
     puts1USART(array);
     Busy1USART();
     Write1USART('\n');
 }

/******************************************************************************
 * Funcao:          ConvFloatStringMedidas
 * Entrada:         Nenhuma (float)
 * Saída:           Nenhuma (void)
 * Descrição:       Converte o valor float para string - Menor valor, maior valor
 * e a média das medidas
 *****************************************************************************/
 void ConvFloatStringMedidas(float dado)
 {
     int inteiro,decimal;
     char array[8];                     // Determina o número de casas a serem convertidas (Ex: 1234.56\NULL)
     inteiro = dado;                    // Recebe a parte inteira do float
     decimal = (dado - inteiro)*100;    // Recebe a parte decimal do float (x*100 - Representa duas casas após a vírgula)
     if (decimal < 10)                  // Verifica se a parte decimal necessita de ser preenchida com 0. Resposta do tipo = 120.03
     {
         sprintf(array,"%3d.0%d",inteiro,decimal); //Convert to string
     }
     else
     {
         sprintf(array,"%3d.%d",inteiro,decimal); //Convert to string
     }
     puts1USART(array);
     Busy1USART();
     Write1USART('\n');
 }

 /******************************************************************************
 * Funcao:          ConvFloatStringMedidas
 * Entrada:         Nenhuma (float)
 * Saída:           Nenhuma (void)
 * Descrição:       Converte o valor float para string - Menor valor, maior valor
 * e a média das medidas
 *****************************************************************************/

 void EnviaComandoBluetooth(char rom *comando)
{

    KEY = 1;                        // Coloca em modo de comando HC05

    while (*comando)
    {
        while(Busy1USART());        //Espera desocupar USART
        Write1USART(*comando);
        comando++;
    }
}



/******************************************************************************
 * Funcao:          SDCardConvFloatStrMenorValor
 * Entrada:         Nenhuma (float)
 * Saída:           Nenhuma (void)
 * Descrição:       Converte o valor float para string - Menor valor, maior valor
 * e a média das medidas
 *****************************************************************************/
 void SDCardConvFloatStrMenorValor(float dado)
 {
     int inteiro,decimal;
     inteiro = dado;                    // Recebe a parte inteira do float
     decimal = (dado - inteiro)*100;    // Recebe a parte decimal do float (x*100 - Representa duas casas após a vírgula)
     if (decimal < 10)                  // Verifica se a parte decimal necessita de ser preenchida com 0. Resposta do tipo = 120.03
     {
         sprintf(SDCardMenorValor,"%3d.0%d",inteiro,decimal); //Convert to string 
     }
     else
     {
         sprintf(SDCardMenorValor,"%3d.%d",inteiro,decimal); //Convert to string
     }
 }


 /******************************************************************************
 * Funcao:          SDCardConvFloatStrMaiorValor
 * Entrada:         Nenhuma (float)
 * Saída:           Nenhuma (void)
 * Descrição:       Converte o valor float para string - Menor valor, maior valor
 * e a média das medidas
 *****************************************************************************/
 void SDCardConvFloatStrMaiorValor(float dado)
 {
     int inteiro,decimal;
     inteiro = dado;                    // Recebe a parte inteira do float
     decimal = (dado - inteiro)*100;    // Recebe a parte decimal do float (x*100 - Representa duas casas após a vírgula)
     if (decimal < 10)                  // Verifica se a parte decimal necessita de ser preenchida com 0. Resposta do tipo = 120.03
     {
         sprintf(SDCardMaiorValor,"%3d.0%d",inteiro,decimal); //Convert to string
     }
     else
     {
         sprintf(SDCardMaiorValor,"%3d.%d",inteiro,decimal); //Convert to string
     }
 }


 /******************************************************************************
 * Funcao:          SDCardConvFloatStrMedia
 * Entrada:         Nenhuma (float)
 * Saída:           Nenhuma (void)
 * Descrição:       Converte o valor float para string - Menor valor, maior valor
 * e a média das medidas
 *****************************************************************************/
 void SDCardConvFloatStrMedia(float dado)
 {
     int inteiro,decimal;
     inteiro = dado;                    // Recebe a parte inteira do float
     decimal = (dado - inteiro)*100;    // Recebe a parte decimal do float (x*100 - Representa duas casas após a vírgula)
     if (decimal < 10)                  // Verifica se a parte decimal necessita de ser preenchida com 0. Resposta do tipo = 120.03
     {
         sprintf(SDCardMedia,"%3d.0%d",inteiro,decimal); //Convert to string
     }
     else
     {
         sprintf(SDCardMedia,"%3d.%d",inteiro,decimal); //Convert to string
     }
 }

/******************************************************************************
 * Funcao:          RTCC_configura(void)
 * Entrada:         Nenhuma
 * Saída:           Nenhuma
 * Descrição:       Rotina RTCC
 * Esta função atualiza data e hora do sistema via aplicativo Android
 *****************************************************************************/

void RTCC_Configura(char hora, char min, char seg, char dia, char mes, char ano, char diaSem)
{

   RtccInitClock();                                         //turn on clock source
   RtccWrOn();                                              //write enable the rtcc registers
   //Set Date and time using global structures defined in libraries
   RtccTimeDate.f.hour = hora;                              //Set Hour
   RtccTimeDate.f.min =  min;                               //Set minute Obs: coloque o valor em hex sem o 0x
   RtccTimeDate.f.sec =  seg;                               //Set second
   RtccTimeDate.f.mday = dia;                               //Set day
   RtccTimeDate.f.mon =  mes;                               //Se month
   RtccTimeDate.f.year = ano;                               //set year
   RtccTimeDate.f.wday = diaSem;                            //Set which day of the week for the corrsponding date

   RtccWriteTimeDate(&RtccTimeDate,1);                      //write into registers

   mRtccOn();                                               //enable the rtcc

}



/******************************************************************************
 * Funcao:          ConvStringInt
 * Entrada:         comando, parcela
 * Saída:           resultado
 * Descrição:       A função converte o comando data e hora do Android recebido
 * em string para inteiro. A entrada "parcela" indica a 1ª string a ser transformada.
 * Ex: dado o comando 4 2052001510143, onde
 * 4  - ID do comando
 * 20 - Hora
 * 52 - Mininutos
 * 00 - Segundos
 * 15 - Dia
 * 10 - Mês
 * 14 - Ano
 * 3  - Dia da semana 0-dom, 1-seg,2-ter,3-qua,4-qui,5-sex e 6-sab
 * Para retirar a hora ConvString(string,2)
 * Para retirar os minutos ConvString(string,4)
 *****************************************************************************/
int ConvStringInt(char *comando, char parcela)
{
    char i;
    char parte[3];
    int resultado;
    for (i = 0;i < 3;i++)
    {
        parte[i] = comando[i + parcela];
        if (i == 2) parte[i] = 0;                   // Indica o fim da string
    }

    resultado = atoi(parte);
    return resultado;
}
/******************************************************************************
 * Funcao:          ConvDecimalHex
 * Entrada:         valor
 * Saída:           conversao
 * Descrição:       Converte o valor em decimal para hex
 *****************************************************************************/

char ConvDecimalHex(char valor)
{
    if (valor <= 9)return valor;
    if (valor >= 10 && valor <= 19)
    {
        valor = valor + 6;
        return valor;
    }
    if (valor >= 20 && valor <= 29)
    {
        valor = valor + 12;
        return valor;
    }
    if (valor >= 30 && valor <= 39)
    {
        valor = valor + 18;
        return valor;
    }
    if (valor >= 40 && valor <= 49)
    {
        valor = valor + 24;
        return valor;
    }
    if (valor >= 50 && valor <= 59)
    {
        valor = valor + 30;
        return valor;
    }
}


/******************************************************************************
 * Funcao:          SDCardConvFloatStrTemp
 * Entrada:         Nenhuma (float)
 * Saída:           char
 * Descrição:       Converte o valor float para string - Temperatura
 *****************************************************************************/
 void SDCardConvFloatStrTemp(float dado)
 {
     int inteiro;
     static char array[4];                      // Determina o número de casas a serem convertidas (Ex: 100\NUL)
     inteiro = dado;                            // Recebe a parte inteira do float
     sprintf(SDCardTemp,"%2d",inteiro);         // Realiza a conversão para string
 }

/******************************************************************************
 * Funcao:          ConverteBCDparaDecimal
 * Entrada:         char
 * Saída:           valor
 * Descrição:       Converte BCD para Decimal
 *****************************************************************************/
char ConverteBCDparaDecimal(char valor)
{
    char aux;
    aux = valor;
    aux = (aux >> 4) * 10;
    valor = 0b00001111 & valor;
    valor = aux + valor;
    return valor;

}


// Alocação da função de tratamento das interrupções de baixa prioridade
// no endereço 0x0018 da memória de programa.
//
#pragma code REMAPPED_LOW_INTERRUPT_VECTOR = 0x0018
void _Low_ISR (void)
{
    _asm goto Tratamento_Low_Interrupt _endasm
}

#pragma code	// Diretiva que retorna a alocação dos endereços
                // da memória de programa para seus valores padrão

/** F I M  D A  S E Ç Ã O  D E   V E C T O R   R E M A P P I N G *************/

/** EOF main.c ***************************************************************/

