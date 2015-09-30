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

/** F U N C � E S ************************************************************/

/******************************************************************************
 * Funcao:          char SDCardPresente(void)
 * Entrada:         Nenhuma (void)
 * Sa�da:           Retorna TRUE ou FALSE
 * Descri��o:       Verifica se o cart�o est� presente no leitor verificando o
 * switch e realiza o protocolo de inicializa��o do SD Card.
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
             de inicializa��o do SD Card em at� 5 vezes. Se o cart�o foi inicializado
             corretamente a rotina retorna TRUE caso contr�rio retorna FALSE*/

            timeout++;

        }while(!FSInit() && timeout < 5u);

            if (timeout < 5u){return TRUE;}
            else{return FALSE;}
    }
}

/******************************************************************************
 * Funcao:          char SDCardLeArquivo(void)
 * Entrada:         Nenhuma (void)
 * Sa�da:           Retorna a �ltima leitura realizada
 * Descri��o:       Verifica a exist�ncia do arquivo LISTA.TXT e retorna a
 * �ltima aquisi��o realizada. O arquivo LISTA.TXT cont�m a �ltima aquisi��o
 * gravada no SD Card.
 *****************************************************************************/

char SDCardLeArquivo(void)
{

    char leArquivo[6];                      // L� dado gravado no arquivo - Valor m�ximo de grava��es 32767 2^15
    char dirname1[9] = "0_LISTA";           // Diret�rio que cont�m o arquivo LISTA.TXT
    FSFILE * pointer;
    BYTE passo = 0;                         // Registra os passos necess�rios para realizar o processo
    BYTE timeout = 0;
    DWORD resultado = 0;                    // Recebe o valor lido do arquivo LISTA.TXT

    // Mudan�a de diret�rio
    if (!FSchdir (dirname1))
    {
        do
        {
            // Abre o arquivo no modo de leitura
            pointer = FSfopenpgm ("LISTA.TXT", "r");
            if (pointer != NULL)                                // 1� passo
                    passo++;

            // L� arquivo de 1 a 5 byte
            if (FSfread (leArquivo, 6, 1, pointer) != 1)        // 2� passo
                    passo++;

            // Verifica se j� chegou no fim do arquivo
            if (FSfeof (pointer))                               // 3� passo
                    passo++;

            // Fecha o arquivo
            if (!FSfclose (pointer))                            // 4� passo
                    passo++;

            if (passo >=4u)
            {
                    timeout = 2;                    // Realiza na 2 tentativas de leitura do arquivo!
                resultado = atol(leArquivo);        // Converte a string lida para inteiro - Valor m�ximo de 32767
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
    else                                            // N�o existe o diret�rio 0_LISTA
    {
        return FALSE;
    }
}

/******************************************************************************
 * Funcao:          char SDCardSistemaDeArquivo(void)
 * Entrada:         Nenhuma (void)
 * Sa�da:           Retorna TRUE ou FALSE
 * Descri��o:       A fun��o gera o arquvio arquivo LISTA.TXT com o valor de
 * inicializa��o.
*****************************************************************************/

char SDCardSistemaDeArquivo(void)
{
    char dirname1[9] = "0_LISTA";               // Diret�rio 0_LISTA
    char listaInicial[2] = "1";                 // Identifica a aquisi��o inicial
    FSFILE * pointer;
    BYTE passo = 0;                             // Registra os passos necess�rios para realizar o processo
    BYTE timeout = 0;

    do
    {
        // Seta a data e hora inicial
        // Isso vai determinar o tempo de cria��o do arquivo que estamos prestes a fazer
        // 2014 setembro, 29, 16:00:00 - Data e hora no sistema americano
//        if (!SetClockVars (2014, 9, 29, 16, 00, 00))
//                passo++;

        // Cria o diret�rio 0_LISTA
        if (!FSmkdir (dirname1))                                        // 1� passo
                passo++;

        // Mudan�a de diret�rio
        if (!FSchdir (dirname1))                                        // 2� passo
                passo++;

        // Cria o arquivo LISTA.TXT
        pointer = FSfopenpgm ("LISTA.TXT", "w");                        // 3� passo
        if (pointer != NULL)
                passo++;

        // Escreve o n�mero "1" indicando que nehuma aquisi��o foi realizada.
        if (!FSfwrite ((void *)listaInicial, 1, 1, pointer) != 1)      // 4� passo
                passo++;

        // Fecha o arquivo
        if (!FSfclose (pointer))                                        // 5� passo
                passo++;

        if (passo >= 5u)
        {
            timeout = 2;                       // Realiza at� 2 tentativas na grava��o dos arquivos!
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
 * Sa�da:           Retorna TRUE ou FALSE
 * Descri��o:       A fun��o cria o arquivo o diret�rio da aquisi��o corrente,
 * seguindo o padr�o xxxxx_LB, onde xxxxx � o n�mero recebido de LISTA.TXT gerado
 * previamente. Em seguida a fun��o registra o valor lido no arquivo LB.TXT que
 * fica registrado no diret�rio xxxxx_LB.
 *****************************************************************************/

//char SDCardGravaDados(DWORD lista, char menorValor[8], char maiorValor[8], char media[8], char temperatura[4])
char SDCardGravaDados(DWORD lista, char media[8], char temperatura[4])
{

    char dirname1[2] = "\\";                // Diret�rio raiz
    char proximoDiretorio[8];               // L� o �ndice do pr�ximo diret�rio a ser criado
    FSFILE * pointer;
    BYTE assinatura[3] = "LB";
    BYTE passo = 0;                         // Registra os passos necess�rios para realizar o processo
    BYTE timeout = 0;
    WORD listaInt = lista;                 // Se for poss�vel esta fun��o vai trabalhar com DWORD no futuro
    BYTE separador[2] = ";";


    sprintf(proximoDiretorio,"%i", listaInt);  // Converte inteiro em string
    sprintf(proximoDiretorio, "%s_%s", proximoDiretorio, assinatura);     // Concatena LB xxxx ->xxxx representa quantidade de leituras


    do
    {
        // Mudan�a de diret�rio
        if (!FSchdir (dirname1))                                    // 1� passo
                passo++;

        // Seta a data e hora inicial
        // Isso vai determinar o tempo de cria��o do arquivo que estamos prestes a fazer
        // 2014 setembro, 29, 16:00:00 - Data e hora no sistema americano
//        if (!SetClockVars (2014, 9, 29, 16, 00, 00))
//                passo++;

        // Cria o diret�rio xxx_BT
        if (!FSmkdir (proximoDiretorio))                            // 2� passo
		passo++;

        // Mudan�a de diret�rio
	if (!FSchdir (proximoDiretorio))                            // 3� passo
		passo++;

        // Cria o arquivo xxxxx_LB.TXT
        pointer = FSfopenpgm ("LB.TXT","w");                        // 4� passo
        if (pointer != NULL)
                passo++;
// Pode ser utilizado se houver interesse em gravar a menor leitura e a maior leitura
        // Escreve no arquivo xx_BT os valores da aquisi��o
//        if(!FSfwrite (menorValor, 1, strlen(menorValor), pointer) != strlen(menorValor))    // 5� passo
//                passo++;
//
//        if(!FSfwrite (separador, 1, strlen(separador), pointer) != strlen(separador))       // 6� passo
//                passo++;

//        if(!FSfwrite (maiorValor, 1, strlen(maiorValor), pointer) != strlen(maiorValor))    // 7� passo
//                passo++;
//
//        if(!FSfwrite (separador, 1, strlen(separador), pointer) != strlen(separador))       // 8� passo
//                passo++;

        if(!FSfwrite (media, 1, strlen(media), pointer) != strlen(media))                   // 9� passo
                passo++;

        if(!FSfwrite (separador, 1, strlen(separador), pointer) != strlen(separador))       // 10� passo
                passo++;

        if(!FSfwrite (temperatura, 1, strlen(temperatura), pointer) != strlen(temperatura)) // 11� passo
                passo++;

        // Fecha o arquivo
        if (!FSfclose (pointer))                                                            //12� passo
                passo++;

//        if (passo >= 12u)
        if (passo >= 8u)
        {
            timeout = 2;                       // Realiza at� 2 tentativas de grava��o do arquivo!
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
 * Sa�da:           Retorna TRUE ou FALSE
 * Descri��o:       A fun��o atualiza o valor do arquivo LISTA.TXT
 *****************************************************************************/

char SDCardGravaLista(DWORD lista)
{

    char dirname1[9] = "\\0_LISTA";         // Diret�rio LISTA
    char dirname2[2] = "\\";                // Diret�rio raiz
    FSFILE * pointer;
    BYTE passo = 0;                         // Marca os  passos necess�rios para realizar todo o processo
    BYTE timeout = 0;
    WORD listaInt = lista;                  // Se for poss�vel esta fun��o vai trabalhar com DWORD no futuro
    BYTE LISTA[6];                          // O valor da LISTA convertida em string

    sprintf(LISTA,"%i", listaInt);         // Converte inteiro em string

    do
    {
        // Mudan�a de diret�rio
        if (!FSchdir (dirname1))                                            // 1� passo
                passo++;

        // Seta a data e hora inicial
        // Isso vai determinar o tempo de cria��o do arquivo que estamos prestes a fazer
        // 2014 setembro, 29, 16:00:00 - Data e hora no sistema americano
//        if (!SetClockVars (2014, 9, 29, 16, 00, 00))
//                passo++;

        // Escreve no arquivo LISTA.TXT
        pointer = FSfopenpgm ("LISTA.TXT","w");                             // 2� passo
        if (pointer != NULL)
                passo++;

        // Atualiza a lista.
	if (!FSfwrite ((void *)LISTA, 1, strlen(LISTA), pointer) != 1)      // 3� passo
                passo++;

        // Fecha o arquivo
        if (!FSfclose (pointer))                                            // 4� passo
                passo++;

        // Retorna para o diret�rio raiz
	if (!FSchdir (dirname2))                                            // 5� passo
                passo++;

        if (passo >= 5u)
        {
            timeout = 2;                       // Realiza at� 2 tentativas de grava��o do arquivo!
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
 * Sa�da:           Nenhuma (void)
 * Descri��o:       Promove um delay de 500ms
 *****************************************************************************/

void delay500ms(void)
{
    Delay10KTCYx(240);
    Delay10KTCYx(240);
    Delay10KTCYx(120);
}