
/*******************************************************************************
 *                                  LIZARD BYTE
 *******************************************************************************
 * Nome do Arquivo:     Leituras.c
 *******************************************************************************/

/** I N C L U D E S **********************************************************/
#include <usart.h>
#include <timers.h>
#include "Leituras.h"
#include "ConfHDW.h"
#include "FSIO.h"
#include <flash.h>

/** V A R I A V E I S   G L O B A I S ****************************************/
unsigned float leitura [63];            // Quantidade de amostras
float temperatura = 0;                  // Temperatura lida no momento da amostragem
char flagLeitura = OFF;                 // Inicia a máquina de estado de cálculo
extern char toggleBluetooth;
extern char toggleBluetoothINT2;
extern int timeoutBluetooth;
extern char togglePowerOn;
extern int timeoutPowerOn;
extern int OffsetTemperatura;
extern char PowerOffBluetooth;          // Desliga o hardware via comando do bluetooth
extern char loopAquisicao;              // Loop de aquisição contínuo


extern char calibracao;         
char passo = 0;                 
extern char calibracaoOk;               // Realizou o procedimento de
unsigned float constante_1 = 1;           
unsigned float constante_2 = 1;           
unsigned float constante_3 = 1;           
unsigned float constante_4 = 1;           
unsigned float constante_5 = 1;           
rom unsigned int constanteReferencia = 0;// Constante da referência 17.4k e 10k
extern float alfa;
extern float b;
static float ALFA=0;
static float B=0;
extern int setPoint;                    // Setpoint da temperatura de calibração
static int auxSetPoint = 0;


/** F U N C O E S ************************************************************/

/******************************************************************************
 * Função:          Indica que as leituras foram requisitadas
 * Entrada:         Nenhuma (void)
 * Saída:           Nenhuma (char)
 * Descrição:       Retorna a permissão para apresentação das medidas
 *****************************************************************************/
char StatusLeituras()
{
    return flagLeitura;
}

/******************************************************************************
 * Funcao:          Máquina de estado responsável pela o cálculo da média da
 * força em Newtons
 * Entrada:         Nenhuma (void)
 * Saída:           ptResultado(float)
 * Descrição:       Retorna o valor da média da força aquisitada
 *****************************************************************************/
float *LeituraAmostras()
{
    char i;
    float mediaSimples = 0;                             // Recebe a média das amostras
    int auxMediaSimples = 0;
    float resultado[4];                                 // Apresenta o resultado
    float *ptResultado;                                 // Ponteiro do resultado
//    float ALFA=0;
//    float B=0;
//    int ajusteFator = 0;                              // Ajuste do fator Temperatura
    float n = 1;                                        // Percentual no fator de correção
//    int setPoint = 1;                                 // Setpoint da temperatura de calibração
    int deltaTemperatura = 0;                           // Avalia o deta da temperatura em relação ao Setpoint
    static float resultadoAnterior =0;                  // Registra o último resultado
    static float resultadoSemAjuste = 0;                // Regitra o último resultado sem ajuste
    static int temperaturaAnterior = SETPOINTDEFAULT;   // Registra o último resultado
    int correcao = 0;                                   // Verifica se a força foi corrigida
    int temp;                                           // Transforma a medida da temperatura em inteiro




    for (i = 0;i < 4;i++)           // Limpa o vetor - resultado
    {
        resultado[i] = 0;
    }

        mediaSimples = 0;
        for (i = 0; i < AMOSTRAS; i++)
        {
            mediaSimples = leitura[i] + mediaSimples;
        }

        mediaSimples /= AMOSTRAS;
        auxMediaSimples = mediaSimples;                             // Registra o valor inteiro da operação
        mediaSimples = auxMediaSimples;
        mediaSimples = mediaSimples - constanteReferencia;          // Constante da referência 17.4k e 10k
        resultado[1] = mediaSimples;
        /* Utilizar as três linhas abaixo somente se houver a necessidade de verificar a força sem correção*/
//        AbreUSART();
//        mediaSimples = (mediaSimples - B)/ALFA;         
//        ConvFloatStringMedidas(mediaSimples);           // Utilizada para Debug - valor sem ajuste dado a variação da temperatura

        if (!calibracaoOk)                      // Sistema nunca foi calibrado
        {
            ALFA = ALFA_DEFAULT;
            B = B_DEFAULT;
            setPoint = SETPOINTDEFAULT;
        }
        else                                    // Sensor de carga calibrado
        {
            ALFA = alfa;
            B = b;
        }

        temp =  temperatura;
        deltaTemperatura = temperaturaAnterior - temp;
        if (deltaTemperatura < 0)deltaTemperatura = deltaTemperatura*-1;


        // Delta de temperatura em relação ao Setpoint >= 15°C para melhorar a correção
        if (deltaTemperatura >= 15u)n = 1.15;                         // 15% de correção no fator

/*A predição da temperatura do sensor segue a seguinte Tabela Verdade:
 *     VT PS|S
 *      0  0|0
 *      0  1|1
 *      1  0|0
 *      1  0|1
 *  Onde VT, VS e S significa respectivamente: Variação da Temperatura da PCB;
 *  Predição da variação da temperatura do sensor verificado se houve alteração
 *  da força e por último a Saída lógica.
 *   Entrada -> 0 Não há variação
 *           -> 1 Há variação
 *   Saída   -> 0 Não corrige a força
 *           -> 1 Corrige a força
 */

        if (temp < setPoint)
        {

            resultado[1] = (resultado[1] - B)/ALFA;             // Cálculo da força

            // Apresenta o resultado com duas casas após a vírgula
            resultado[1] = Apresenta2casas(resultado[1]);

            // Tentativa da predição da variação da temperatura do sensor de força
            if ((temperaturaAnterior == temp) && (resultadoSemAjuste == resultado[1]) && !correcao)      // 1ª linha da tabela verdade
            {
                resultado[1] = resultadoAnterior;
                correcao = ON;
            }

            // Tentativa da predição da variação da temperatura do sensor de força
            if ((temperaturaAnterior == temp) && (resultadoSemAjuste != resultado[1]) && !correcao)      // 2ª linha da tabela verdade
            {
                resultadoSemAjuste =  resultado[1];
                resultado[1] = resultado[1] + (setPoint - temp)*TAXA_VARIACAO*n;
                resultado[1] = Apresenta2casas(resultado[1]);
                resultadoAnterior = resultado[1];
                correcao = ON;
            }

            // Tentativa da predição da variação da temperatura do sensor de força
            if ((temperaturaAnterior != temp) && (resultadoSemAjuste == resultado[1]) && !correcao)      // 3ª linha da tabela verdade
            {
                resultado[1] = resultadoAnterior;
                correcao = ON;
            }

            // Tentativa da predição da variação da temperatura do sensor de força
            if ((temperaturaAnterior != temp) && (resultadoSemAjuste != resultado[1]) && !correcao)      // 4ª linha da tabela verdade
            {
                resultadoSemAjuste =  resultado[1];
                resultado[1] = resultado[1] + (setPoint - temp)*TAXA_VARIACAO*n;
                resultado[1] = Apresenta2casas(resultado[1]);
                resultadoAnterior = resultado[1];
                correcao = ON;
            }

            temperaturaAnterior = temp;

        }

        if (temp > setPoint)
        {

            resultado[1] = (resultado[1] - B)/ALFA;             // Cálculo da força

            // Apresenta o resultado com duas casas após a vírgula
            resultado[1] = Apresenta2casas(resultado[1]);

            // Tentativa da predição da variação da temperatura do sensor de força
            if ((temperaturaAnterior == temp) && (resultadoSemAjuste == resultado[1]) && !correcao)      // 1ª linha da tabela verdade
            {
                resultado[1] = resultadoAnterior;
                correcao = ON;
            }

            // Tentativa da predição da variação da temperatura do sensor de força
            if ((temperaturaAnterior == temp) && (resultadoSemAjuste != resultado[1]) && !correcao)      // 2ª linha da tabela verdade)
            {
                resultadoSemAjuste =  resultado[1];
                resultado[1] = resultado[1] - (temp - setPoint)*TAXA_VARIACAO*n;
                resultado[1] = Apresenta2casas(resultado[1]);
                resultadoAnterior = resultado[1];
                correcao = ON;
            }

            // Tentativa da predição da variação da temperatura do sensor de força
            if ((temperaturaAnterior != temp) && (resultadoSemAjuste == resultado[1]) && !correcao)      // 3ª linha da tabela verdade)
            {
                resultado[1] = resultadoAnterior;
                correcao = ON;
            }

            // Tentativa da predição da variação da temperatura do sensor de força
            if ((temperaturaAnterior != temp) && (resultadoSemAjuste != resultado[1]) && !correcao)      // 4ª linha da tabela verdade)
            {
                resultadoSemAjuste =  resultado[1];
                resultado[1] = resultado[1] - (temp - setPoint)*TAXA_VARIACAO*n;
                resultado[1] = Apresenta2casas(resultado[1]);
                resultadoAnterior = resultado[1];
                correcao = ON;
            }

            temperaturaAnterior = temp;

        }

        if (temp == setPoint)
        {
            resultado[1] = (resultado[1] - B)/ALFA;                     // Cálculo da força
            resultado[1] = Apresenta2casas(resultado[1]);
            resultadoAnterior = resultado[1];
            temperaturaAnterior = temp;
        }
        

        if (resultado[1] < 0)resultado[1] = 0;                          // Situação em há desistência da força no momento da aquisição!

        resultado[3] = temperatura;
        ptResultado = resultado;
        flagLeitura = OFF;

        return ptResultado;

}

/******************************************************************************
 * Funcao:		void Tratamento_High_Interrupt(void)
 * Entrada:		Nenhuma (void)
 * Saída:		Nenhuma (void)
 * Descrição:	Função de tratamento das interrupções de ALTA prioridade
 *****************************************************************************/

#pragma interrupt Tratamento_High_Interrupt
void Tratamento_High_Interrupt(void)
{
    static char auxiliarSleepMode = OFF;

    // Switch SW1 - Liga/Desliga
    if (INTCONbits.INT0IF)
    {
        if (!togglePowerOn && !POWER)                         // Habilita o sistema a entrar no modo de operação
        {

            INTCONbits.TMR0IE = 1;      // Habilita a interrupção do timer 0 
            INTCONbits.TMR0IF = 0;      // Sinaliza a intrrupção do timer 0
            INTCON3bits.INT1IE = 1;     // Habilita a INT1
            INTCON3bits.INT1IF = 0;     // Limpa a sinalização da  interrupcão
#if DEBUG
            INTCON3bits.INT2IE = 1;     // Habilita a INT2
#else
            INTCON3bits.INT2IE = 0;     // Dasabilita a INT2 para debug
#endif
            INTCON3bits.INT2IF = 0;     // Limpa a sinalização da  interrupcão
              
            togglePowerOn = ON;

            toggleBluetooth  = OFF;

            toggleBluetoothINT2 = OFF;

            LED_POWER = LIGADO;

            // Configura como entrada Alta Impedância para diminuir o consumo ao retirar o SD Card
            SD_CS_TRIS = 0;
            SPICLOCK = 0;
            SPIOUT = 0;

            auxiliarSleepMode = OFF;

            TensaoDeReferencia();                   // Verifica a tensão de referência

                while (!POWER)                     // Aguarda a liberação do switch
                {
#if DEBUG
                    if (!PW_BLUETOOTH)
                    {
                        LED_STATUS = LIGADO;
                        calibracao = ON;
                    }
#endif
                }
                Delay10KTCYx(240);                  // Debounce 200ms
//                if (!calibracao)calibracao = ON;                  // Utilizado no debug se desejar avaliar a rotina da calibração
                               
        }

            if (togglePowerOn && !POWER || auxiliarSleepMode || PowerOffBluetooth)            // Desabilita o sistema e coloca no modo Sleep
            {
                Close1USART();              // Fecha a USART para evitar receber dados no power off!
                Delay10KTCYx(240);          // Espera 200ms antes de prosseguir no algoritmo

                INTCONbits.TMR0IE = 0;      // Dasabilita a interrupção do timer 0
                INTCONbits.TMR0IF = 0;      // Limpa a sinalização da interrupção
                INTCON3bits.INT1IE = 0;     // Dasabilita a interrupção do INT1
                INTCON3bits.INT2IE = 0;     // Dasabilita a interrupção do INT2


                togglePowerOn = OFF;
                toggleBluetooth = OFF;
                toggleBluetoothINT2 = OFF;

                timeoutBluetooth = 0;
                timeoutPowerOn = 0;

                calibracao = OFF;
                passo  = 0;
          
                POWER_BLUETOOTH = DESLIGADO;

                MDD_SDSPI_ShutdownMedia();

                // Configura a entrada em Alta Impedância para diminuir o consumo ao retirar o SD Card
                SD_CS_TRIS = 1;
                SPICLOCK = 1;
                SPIOUT = 1;

                POWER_SDCARD = DESLIGADO;
            
                LED_STATUS = DESLIGADO;

                LED_POWER = DESLIGADO;

                PowerOffBluetooth = OFF;

                loopAquisicao = OFF;

                    while (!POWER);                 // Aguarda a liberação do switch
                    Delay10KTCYx(240);              // Debounce 200ms
            }

    INTCONbits.INT0IF = 0;
    }

    if (INTCON3bits.INT1IF && INTCON3bits.INT1IE)
    {
        char i,j;
        char timeout = 0;
        float mediaSimples = 0;
        float maiorLeitura = 0;                             // Recebe a maior amostra lida
        int auxMediaSimples = 0;
        char statusStart = OFF;
        char envia_constante_1[] = "Constante 1\n";         //Apresenta a primeira constante
        char envia_constante_2[] = "Constante 2\n";         //Apresenta a segunda constante
        char envia_constante_3[] = "Constante 3\n";         //Apresenta a terceira constante
        char envia_constante_4[] = "Constante 4\n";         //Apresenta a quarta constante
        char envia_constante_5[] = "Constante 5\n";         //Apresenta a quinta constante
        char startAquisicao = OFF;
        char startCalibracao = OFF;
        int flagCalibracao;

        unsigned float Sx,Sy,Sx2,Sxy;                       // Somatórios
        float force_F2,force_F3,force_F4,force_F5;

        timeoutPowerOn = 0;                                 // Reset do timeout - Sistema em atividade

        while(timeout < 60u)
        {

            // Máquina de estado da leitura do sensor de mordida
            for(i = 0;i < AMOSTRAS;i++)                     // Limpa o vetor - leitura
            {
                leitura[i] = 0;
            }

            SetChanADC(ADC_CH0);                            // Seta o canal ADC do sensor FlexiForce

                for (i = 0;i < AMOSTRAS;i++)                // Laço que permite intervalo de tempo entre as amostragens
                {
                    for(j = 0;j < AMOSTRAS;j++)
                    {
                        
                        ConvertADC();
                        while(BusyADC());
                        leitura[i] += ReadADC();
                        
                    }
                leitura[i] /= AMOSTRAS;

                }
 
                    mediaSimples = 0;
                    for (i = 0; i < AMOSTRAS; i++)
                    {
                        mediaSimples = leitura[i] + mediaSimples;
                    }

                        mediaSimples /= AMOSTRAS;
                        auxMediaSimples = mediaSimples;             // Registra o valor inteiro da operação
                        mediaSimples = auxMediaSimples;
                        mediaSimples = mediaSimples - constanteReferencia;       

                        if (mediaSimples > 1257)                    // Dispara a leitura para um valor próximo 0.98N
                        {
                            if (!calibracao)startAquisicao = ON;
                            else startCalibracao = ON;
                            timeout = 60;
                        }

                            if (!startAquisicao)PiscaLedAguardando();

                                timeout++;

                                if (START && !statusStart)statusStart = ON;         // Monitora o estado do botão START

                                    while (!START && statusStart)                   // Cancela a aquisição - START OFF
                                    {
                                        timeout = 60;
                                        LED_STATUS = DESLIGADO;
                                        INTCON3bits.INT1IF = 0;
                                        while (!START);                             // Aguarda a liberação do switch
                                        Delay10KTCYx(240);                          // Debounce 200ms
                                    }

                                        if (!POWER)                                 // Cancela a aquisição - Power OFF
                                        {
                                            auxiliarSleepMode = ON;
                                            INTCONbits.INT0IF = 1;
                                            timeout = 60;
                                        }

        }
        
        if (startAquisicao)                     // Início do laço que trata a aquisição da medida do sensor após identitificado o valor mínimo
        {

            LED_STATUS = LIGADO;
            atraso1seg();                       // Aguarda o tempo do Drift do sensor
//            atraso1seg();
//            atraso1seg();

            // Máquina de estado da leitura do sensor de mordida
            for(i = 0;i < AMOSTRAS;i++)         // Limpa o vetor - leitura
            {
                leitura[i] = 0;
            }

                for (i = 0;i < AMOSTRAS;i++)    // Amostra as leituras
                {
                    for(j = 0;j < AMOSTRAS;j++)
                    {
                        ConvertADC();
                        while(BusyADC());
                        leitura[i] += ReadADC();
                    }

                leitura[i] /= AMOSTRAS;

                }

                    for (i = 0;i < AMOSTRAS;i++)   // Ordena o vetor da leitura
                        {
                            for (j = 0;j < AMOSTRAS;j++)
                            {
                                if (leitura[i] < leitura[j])
                                {
                                    maiorLeitura = leitura[i];
                                    leitura[i] = leitura[j];
                                    leitura[j] = maiorLeitura;
                                }
                            }
                        }

                         // Máquina de estado da temperatura no momento da leitura
                        SetChanADC(ADC_CH1);                // Seta o canal ADC do sensor de temperatura
                        temperatura = 0;                    // Início da nova amostragem - Reset da variável
                        for (i = 0;i <16;i++)               // Realiza a média com  medidas
                        {
                            ConvertADC();
                            while(BusyADC());
                            temperatura += ReadADC();
                        }

                        temperatura /=16;
                        temperatura = temperatura*RESOLUCAO;
                        temperatura = (temperatura*100) - 50;
                        temperatura = temperatura + OffsetTemperatura;


        flagLeitura = ON;
        startAquisicao = OFF;
        }

     // Rotina do processo da calibração
     if (startCalibracao)
     {

        LED_STATUS = LIGADO;
        atraso1seg();                               // Aguarda o tempo do Drift do sensor


        // Máquina de estado da leitura do sensor de mordida
        for(i = 0;i < AMOSTRAS;i++)                 // Limpa o vetor - leitura
        {
            leitura[i] = 0;
        }

            for (i = 0;i < AMOSTRAS;i++)            // Amostra as leituras
            {
                for(j = 0;j < AMOSTRAS;j++)
                {
                    ConvertADC();
                    while(BusyADC());
                    leitura[i] += ReadADC();
                }

            leitura[i] /= AMOSTRAS;

            }

                mediaSimples = 0;

                for (i = 0; i < AMOSTRAS; i++)
                {
                    mediaSimples = leitura[i] + mediaSimples;
                }

                mediaSimples /= AMOSTRAS;
                auxMediaSimples = mediaSimples;                             // Registra o valor inteiro da operação
                mediaSimples = auxMediaSimples;
                mediaSimples = mediaSimples - constanteReferencia;          // Constante da referência 17.4k e 10k 

                if (passo == 0)puts1USART(envia_constante_1), constante_1 = mediaSimples;
                if (passo == 1)puts1USART(envia_constante_2), constante_2 = mediaSimples;
                if (passo == 2)puts1USART(envia_constante_3), constante_3 = mediaSimples;
                if (passo == 3)puts1USART(envia_constante_4), constante_4 = mediaSimples;
                if (passo == 4)puts1USART(envia_constante_5), constante_5 = mediaSimples;

                ConvFloatStringMedidas(mediaSimples);

                passo++;

                    if (passo > 4)                                          // Fim da rotina da calibração
                    {
                        passo = 0;
                        calibracao = OFF;

                        // Calculando o "alfa" e o "b" utilizando o método dos minímos quadrados
                        /*---------------------------------------------------------------------
                         * alfa = (n*Sxy - Sx*Sy)/(n*Sx^2 - Sx*Sx)
                         * b = (Sy*Sx^2 - Sx*Sxy)/(n*Sx^2- Sx*Sx)
                         * onde S é o símbolo do somatório
                         */

                        Sx = F1 + F2 + F3 + F4 + F5;                                                    // Somatório em Newtons das forças referência
                        Sy = constante_1 + constante_2 + constante_3 + constante_4 + constante_5;       // Somatório das Tensões referências
                        Sx2 = F1*F1 + F2*F2 + F3*F3 + F4*F4 + F5*F5;
                        Sxy = F1*constante_1 + F2*constante_2 + F3*constante_3 + F4*constante_4 + F5*constante_5;

                        // alfa
                        alfa = (N*Sxy - Sx*Sy)/(N*Sx2 - Sx*Sx);
                        //b
                        b = (Sy*Sx2 - Sx*Sxy)/(N*Sx2 - Sx*Sx);

                        // Validação da calibração +/- 5% de tolerância

                        // F1 - Não entra na validação devido a sua baixa exatidão e precisão (repetibilidade)

                        // F2
                        force_F2 = (constante_2 - b)/alfa;

                        // F3
                        force_F3 = (constante_3 - b)/alfa;

                        // F4
                        force_F4 = (constante_4 - b)/alfa;

                        // F5
                        force_F5 = (constante_5 - b)/alfa;

                        if (force_F2 <= F2_SUP)passo++;
                        if (force_F2 >= F2_INF)passo++;

                        if (force_F3 <= F3_SUP)passo++;
                        if (force_F3 >= F3_INF)passo++;

                        if (force_F4 <= F4_SUP)passo++;
                        if (force_F4 >= F4_INF)passo++;

                        if (force_F5 <= F5_SUP)passo++;
                        if (force_F5 >= F5_INF)passo++;

                        atraso1seg();

                        if (passo < 8)
                        {
                            LED_STATUS = LIGADO;
                            LED_POWER = DESLIGADO;
                        }
                        else
                        {
                            calibracaoOk = ON;

                            // Faz a leitura da temperatura da calibração

                            SetChanADC(ADC_CH1);                // Seta o canal ADC do sensor de temperatura
                            temperatura = 0;                    // Início da nova amostragem - Reset da variável
                            for (i = 0;i <16;i++)               // Realiza a média com  medidas
                            {
                                ConvertADC();
                                while(BusyADC());
                                temperatura += ReadADC();
                            }

                            temperatura /=16;
                            temperatura = temperatura*RESOLUCAO;
                            temperatura = (temperatura*100) - 50;
                            temperatura = temperatura + OffsetTemperatura;
                            setPoint = temperatura;

                            /******* Gravação da flash - alfa, b e setPoint********
                            * Endereço 0x1FBE0 até 0x1FBE3 - alfa
                            * Endereço 0x1FBE4 até 0x1FBE7 - b
                            * Endereço 0x1FBE8 até 0x1FBE9 - setPoint
                            * Endereço 0x1FBEA - Flag de verificação da calibração
                            *******************************************************/
                            ALFA = alfa;                        // Cria cópia do ponteiro
                            B = b;                              // Cria cópia do ponteiro
                            auxSetPoint = setPoint;             // Cria cópia do ponteiro
                            WriteBytesFlash(0x1FBE0,4,(char)&ALFA);
                            WriteBytesFlash(0x1FBE4,4,(char)&B);
                            WriteBytesFlash(0x1FBE8,2,(char)&auxSetPoint);
                            ReadFlash(0x1FBEA,2,&flagCalibracao);

                            if (flagCalibracao == 0xFFFF)               // Se foi calibrado pelo menos uma vez não realiza nova atualização na posição da flash
                            {
                                flagCalibracao = 1;
                                WriteBytesFlash(0x1FBEA,2,(char)&flagCalibracao);
                            }
                       
                            passo = 0;
                            // Comentar as linhas abaixo para avaliar a rotina da calibração
                            auxiliarSleepMode = ON;
                            INTCONbits.INT0IF = 1;
                        }
                    }

                        if (!POWER)          // Cancela a calibracação
                        {
                            auxiliarSleepMode = ON;
                            INTCONbits.INT0IF = 1;
                        }
     }
        
        // Fim das máquinas de estado
        INTCON3bits.INT1IF = 0;
    }

    if (INTCONbits.TMR0IF && INTCONbits.TMR0IE)
    {
        WriteTimer0(18661);                                         // Ajusta o valor do timer para 1000ms

        timeoutPowerOn++;

        if (timeoutPowerOn > 120u && !calibracao &&!loopAquisicao)  // 120s sem atividade desliga o sistema
        {

            timeoutPowerOn = 0;

            timeoutBluetooth = 0;

            auxiliarSleepMode = ON;

            INTCONbits.INT0IF = 1;
        }

            // Timeout Bluetooth
            if (toggleBluetooth)timeoutBluetooth++;

                if (timeoutBluetooth > 60u && !calibracao && !loopAquisicao) // 60s sem atividade no bluetooth será desligado
                {

                    Close1USART();                                          // Fecha a USART para evitar receber dados no power off!
                    Delay10KTCYx(240);                                      // Espera 200ms antes de prosseguir no algoritmo

                    POWER_BLUETOOTH = DESLIGADO;

                    toggleBluetooth = OFF;

                    toggleBluetoothINT2 = OFF;

                    timeoutBluetooth = 0;

                }
        INTCONbits.TMR0IF = 0;
    }

}//end Tratamento_High_Interrupt


/******************************************************************************
 * Funcao:          PiscaLed
 * Entrada:         char
 * Saída:           Nenhuma (void)
 * Descrição:       Sinalização GERAL - LED STATUS e LED POWER
 *****************************************************************************/
void PiscaLedAguardando(void)
{

    LED_STATUS = LIGADO;                    // 50ms ligado
    Delay10KTCYx(60);

    LED_STATUS = DESLIGADO;                 // 50ms desligado
    Delay10KTCYx(60);
    
}

/******************************************************************************
 * Funcao:          Apresenta2casas
 * Entrada:         float
 * Saída:           int
 * Descrição:       A função de apresenta o resultado  com duas casas ápos a
 * vírgula, mas sem o critério de arredondamento da 3º casa.
 *****************************************************************************/

float Apresenta2casas(float valor)
{
    int parteInteira, parteFacionaria;
    float aux1, aux2,resultado;


    parteInteira = valor;
    aux1 = (valor - parteInteira)*100;

    parteFacionaria = aux1;
    aux2 = parteFacionaria*0.01;

    resultado = parteInteira+aux2;

    return resultado;

}

/******************************************************************************
 * Funcao:          ConstantesCalibracao
 * Entrada:         void
 * Saída:           void
 * Descrição:       Retorna os parâmetros do processo da calibração
 *****************************************************************************/

void ParametrosCalibracao (void)
{

    char envia_constante_1[] = "Constante 1\n";         //Apresenta a primeira constante
    char envia_constante_2[] = "Constante 2\n";         //Apresenta a segunda constante
    char envia_constante_3[] = "Constante 3\n";         //Apresenta a terceira constante
    char envia_constante_4[] = "Constante 4\n";         //Apresenta a quarta constante
    char envia_constante_5[] = "Constante 5\n";         //Apresenta a quinta constante
    char envia_constante_6[] = "alfa\n";                //Apresenta a quinta constante
    char envia_constante_7[] = "b\n";                   //Apresenta a quinta constante
    char envia_constante_8[] = "Setpoint\n";            //Apresenta a quinta constante
    float auxTemp = 0;


    puts1USART(envia_constante_1);
    ConvFloatStringMedidas(constante_1);
    puts1USART(envia_constante_2);
    ConvFloatStringMedidas(constante_2);
    puts1USART(envia_constante_3);
    ConvFloatStringMedidas(constante_3);
    puts1USART(envia_constante_4);
    ConvFloatStringMedidas(constante_4);
    puts1USART(envia_constante_5);
    ConvFloatStringMedidas(constante_5);

    puts1USART(envia_constante_6);
    ConvFloatStringMedidas(ALFA);
    puts1USART(envia_constante_7);
    ConvFloatStringMedidas(B);

    auxTemp = setPoint;
    puts1USART(envia_constante_8);
    ConvFloatStringMedidas(auxTemp);


}


// Alocação da função de tratamento das interrupções de ALTA prioridade
// no endereço 0x008 da memória de programa
#pragma code REMAPPED_HIGH_INTERRUPT_VECTOR = 0x008
void _high_ISR (void)
{
    _asm goto Tratamento_High_Interrupt _endasm
}

//#pragma code	// Diretiva que retorna a alocação dos endereços
				// da memória de programa para seus valores padrão

/** F I M   S E Ç Ã O  D E   V E C T O R E S   D E   I N T E R R U P Ç Ã O ***/

/** EOF main.c ***************************************************************/