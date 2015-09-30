/*******************************************************************************
 *                                  LIZARD BYTE
 *******************************************************************************
 * Nome do Arquivo:     SDCard.c
 *******************************************************************************/

/** I N C L U D E S **********************************************************/
#include <delays.h>
#include <stdio.h>
#include <stdlib.h>
#include <rtcc.h>
#include "SDCard.h"
#include "MDD_File_System/Compiler.h"
#include "MDD_File_System/FSconfig.h"
#include "MDD_File_System/FSDefs.h"
#include "MDD_File_System/FSIO.h"
#include "MDD_File_System/GenericTypeDefs.h"
#include "MDD_File_System/salloc.h"
#include "MDD_File_System/SD-SPI.h"


/** V A R I A V E I S   L O C A I S ****************************************/

/** F U N C Õ E S ************************************************************/

/******************************************************************************
 * Funcao:          char SDCardPresente(void)
 * Entrada:         Nenhuma (void)
 * Saída:           Retorna TRUE ou FALSE
 * Descrição:       Verifica se o cartão está presente no leitor verificando o
 * switch e realiza o protocolo de inicialização do SD Card.
 *****************************************************************************/

char SDCardPresente (void)
{
    BYTE timeout = 0;

    if (MDD_MediaDetect())
    {
        delay500ms();                   // Aguarda 500ms antes de incializar o SD card

        do
        {
            /* Esta rotina em conjunto com a rotina FSInit() realiza o procedimento
             de inicialização do SD Card em até 5 vezes. Se o cartão foi inicializado
             corretamente a rotina retorna TRUE caso contrário retorna FALSE*/

            timeout++;

        }while(!FSInit() && timeout < 5u);

            if (timeout < 5u){return TRUE;}
            else{return FALSE;}
    }
}

/******************************************************************************
 * Funcao:          char SDCardLeArquivo(void)
 * Entrada:         Nenhuma (void)
 * Saída:           Retorna a última leitura realizada
 * Descrição:       Verifica a existência do arquivo LISTA.TXT e retorna a
 * última aquisição realizada. O arquivo LISTA.TXT contém a última aquisição
 * gravada no SD Card.
 *****************************************************************************/

char SDCardLeArquivo(void)
{

    char leArquivo[6];                      // Lê dado gravado no arquivo - Valor máximo de gravações 32767 2^15
    char dirname1[9] = "0_LISTA";           // Diretório que contém o arquivo LISTA.TXT
    FSFILE * pointer;
    BYTE passo = 0;                         // Registra os passos necessários para realizar o processo
    BYTE timeout = 0;
    DWORD resultado = 0;                    // Recebe o valor lido do arquivo LISTA.TXT

    // Mudança de diretório
    if (!FSchdir (dirname1))
    {
        do
        {
            // Abre o arquivo no modo de leitura
            pointer = FSfopenpgm ("LISTA.TXT", "r");
            if (pointer != NULL)                                // 1º passo
                    passo++;

            // Lê arquivo de 1 a 5 byte
            if (FSfread (leArquivo, 6, 1, pointer) != 1)        // 2º passo
                    passo++;

            // Verifica se já chegou no fim do arquivo
            if (FSfeof (pointer))                               // 3º passo
                    passo++;

            // Fecha o arquivo
            if (!FSfclose (pointer))                            // 4º passo
                    passo++;

            if (passo >=4u)
            {
                    timeout = 2;                    // Realiza na 2 tentativas de leitura do arquivo!
                resultado = atol(leArquivo);        // Converte a string lida para inteiro - Valor máximo de 32767
                return resultado;
            }
            else
            {
                passo = 0;
                timeout++;
            }

        }while(timeout < 2u);

            if (timeout > 1)return FALSE;

    }
    else                                            // Não existe o diretório 0_LISTA
    {
        return FALSE;
    }
}

/******************************************************************************
 * Funcao:          char SDCardSistemaDeArquivo(void)
 * Entrada:         Nenhuma (void)
 * Saída:           Retorna TRUE ou FALSE
 * Descrição:       A função gera o arquvio arquivo LISTA.TXT com o valor de
 * inicialização.
*****************************************************************************/

char SDCardSistemaDeArquivo(void)
{
    char dirname1[9] = "0_LISTA";               // Diretório 0_LISTA
    char listaInicial[2] = "1";                 // Identifica a aquisição inicial
    FSFILE * pointer;
    BYTE passo = 0;                             // Registra os passos necessários para realizar o processo
    BYTE timeout = 0;

    do
    {
        // Seta a data e hora inicial
        // Isso vai determinar o tempo de criação do arquivo que estamos prestes a fazer
        // 2014 setembro, 29, 16:00:00 - Data e hora no sistema americano
//        if (!SetClockVars (2014, 9, 29, 16, 00, 00))
//                passo++;

        // Cria o diretório 0_LISTA
        if (!FSmkdir (dirname1))                                        // 1º passo
                passo++;

        // Mudança de diretório
        if (!FSchdir (dirname1))                                        // 2º passo
                passo++;

        // Cria o arquivo LISTA.TXT
        pointer = FSfopenpgm ("LISTA.TXT", "w");                        // 3º passo
        if (pointer != NULL)
                passo++;

        // Escreve o número "1" indicando que nehuma aquisição foi realizada.
        if (!FSfwrite ((void *)listaInicial, 1, 1, pointer) != 1)      // 4º passo
                passo++;

        // Fecha o arquivo
        if (!FSfclose (pointer))                                        // 5º passo
                passo++;

        if (passo >= 5u)
        {
            timeout = 2;                       // Realiza até 2 tentativas na gravação dos arquivos!
            return TRUE;
        }
        else
        {
            passo = 0;
            timeout++;
        }
        if (timeout > 1u)return FALSE;

    }while(timeout < 2u);

}

/******************************************************************************
 * Funcao:          char SDCardGravaDados(void)
 * Entrada:         leitura
 * Saída:           Retorna TRUE ou FALSE
 * Descrição:       A função cria o arquivo o diretório da aquisição corrente,
 * seguindo o padrão xxxxx_LB, onde xxxxx é o número recebido de LISTA.TXT gerado
 * previamente. Em seguida a função registra o valor lido no arquivo LB.TXT que
 * fica registrado no diretório xxxxx_LB.
 *****************************************************************************/

//char SDCardGravaDados(DWORD lista, char menorValor[8], char maiorValor[8], char media[8], char temperatura[4])
char SDCardGravaDados(DWORD lista, char media[8], char temperatura[4])
{

    char dirname1[2] = "\\";                // Diretório raiz
    char proximoDiretorio[8];               // Lê o índice do próximo diretório a ser criado
    FSFILE * pointer;
    BYTE assinatura[3] = "LB";
    BYTE passo = 0;                         // Registra os passos necessários para realizar o processo
    BYTE timeout = 0;
    WORD listaInt = lista;                 // Se for possível esta função vai trabalhar com DWORD no futuro
    BYTE separador[2] = ";";


    sprintf(proximoDiretorio,"%i", listaInt);  // Converte inteiro em string
    sprintf(proximoDiretorio, "%s_%s", proximoDiretorio, assinatura);     // Concatena LB xxxx ->xxxx representa quantidade de leituras


    do
    {
        // Mudança de diretório
        if (!FSchdir (dirname1))                                    // 1º passo
                passo++;

        // Seta a data e hora inicial
        // Isso vai determinar o tempo de criação do arquivo que estamos prestes a fazer
        // 2014 setembro, 29, 16:00:00 - Data e hora no sistema americano
//        if (!SetClockVars (2014, 9, 29, 16, 00, 00))
//                passo++;

        // Cria o diretório xxx_BT
        if (!FSmkdir (proximoDiretorio))                            // 2º passo
		passo++;

        // Mudança de diretório
	if (!FSchdir (proximoDiretorio))                            // 3º passo
		passo++;

        // Cria o arquivo xxxxx_LB.TXT
        pointer = FSfopenpgm ("LB.TXT","w");                        // 4º passo
        if (pointer != NULL)
                passo++;
// Pode ser utilizado se houver interesse em gravar a menor leitura e a maior leitura
        // Escreve no arquivo xx_BT os valores da aquisição
//        if(!FSfwrite (menorValor, 1, strlen(menorValor), pointer) != strlen(menorValor))    // 5º passo
//                passo++;
//
//        if(!FSfwrite (separador, 1, strlen(separador), pointer) != strlen(separador))       // 6º passo
//                passo++;

//        if(!FSfwrite (maiorValor, 1, strlen(maiorValor), pointer) != strlen(maiorValor))    // 7º passo
//                passo++;
//
//        if(!FSfwrite (separador, 1, strlen(separador), pointer) != strlen(separador))       // 8º passo
//                passo++;

        if(!FSfwrite (media, 1, strlen(media), pointer) != strlen(media))                   // 9º passo
                passo++;

        if(!FSfwrite (separador, 1, strlen(separador), pointer) != strlen(separador))       // 10º passo
                passo++;

        if(!FSfwrite (temperatura, 1, strlen(temperatura), pointer) != strlen(temperatura)) // 11º passo
                passo++;

        // Fecha o arquivo
        if (!FSfclose (pointer))                                                            //12º passo
                passo++;

//        if (passo >= 12u)
        if (passo >= 8u)
        {
            timeout = 2;                       // Realiza até 2 tentativas de gravação do arquivo!
            return TRUE;
        }
        else
        {
            passo = 0;
            timeout++;
        }

        if (timeout > 1u)return FALSE;

    }while(timeout < 2u);


}

/******************************************************************************
 * Funcao:          char SDCardGravaLista(void)
 * Entrada:         leitura
 * Saída:           Retorna TRUE ou FALSE
 * Descrição:       A função atualiza o valor do arquivo LISTA.TXT
 *****************************************************************************/

char SDCardGravaLista(DWORD lista)
{

    char dirname1[9] = "\\0_LISTA";         // Diretório LISTA
    char dirname2[2] = "\\";                // Diretório raiz
    FSFILE * pointer;
    BYTE passo = 0;                         // Marca os  passos necessários para realizar todo o processo
    BYTE timeout = 0;
    WORD listaInt = lista;                  // Se for possível esta função vai trabalhar com DWORD no futuro
    BYTE LISTA[6];                          // O valor da LISTA convertida em string

    sprintf(LISTA,"%i", listaInt);         // Converte inteiro em string

    do
    {
        // Mudança de diretório
        if (!FSchdir (dirname1))                                            // 1º passo
                passo++;

        // Seta a data e hora inicial
        // Isso vai determinar o tempo de criação do arquivo que estamos prestes a fazer
        // 2014 setembro, 29, 16:00:00 - Data e hora no sistema americano
//        if (!SetClockVars (2014, 9, 29, 16, 00, 00))
//                passo++;

        // Escreve no arquivo LISTA.TXT
        pointer = FSfopenpgm ("LISTA.TXT","w");                             // 2º passo
        if (pointer != NULL)
                passo++;

        // Atualiza a lista.
	if (!FSfwrite ((void *)LISTA, 1, strlen(LISTA), pointer) != 1)      // 3º passo
                passo++;

        // Fecha o arquivo
        if (!FSfclose (pointer))                                            // 4º passo
                passo++;

        // Retorna para o diretório raiz
	if (!FSchdir (dirname2))                                            // 5º passo
                passo++;

        if (passo >= 5u)
        {
            timeout = 2;                       // Realiza até 2 tentativas de gravação do arquivo!
            return TRUE;
        }
        else
        {
            passo = 0;
            timeout++;
        }

        if (timeout > 1u)return FALSE;

    }while(timeout < 2u);


}


/******************************************************************************
 * Funcao:          delay500ms
 * Entrada:         Nenhuma (void)
 * Saída:           Nenhuma (void)
 * Descrição:       Promove um delay de 500ms
 *****************************************************************************/

void delay500ms(void)
{
    Delay10KTCYx(240);
    Delay10KTCYx(240);
    Delay10KTCYx(120);
}