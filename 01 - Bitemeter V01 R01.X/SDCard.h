/*******************************************************************************
 *                                  LIZARD BYTE
 *******************************************************************************
 * Nome do Arquivo:     SDCard.h
 *******************************************************************************/

#ifndef SDCARD_H
#define	SDCARD_H

#include "MDD_File_System/GenericTypeDefs.h"



/** I N C L U D E S **********************************************************/


/** D E F I N E S ************************************************************/

/** P R O T O T I P O S *******************************************************/
void delay500ms(void);
char SDCardPresente (void);
char SDCardLeArquivo(void);
char SDCardSistemaDeArquivo(void);
// Pode ser utilizado se houver interesse em gravar a menor leitura e maior leitura
//char SDCardGravaDados(DWORD indice, char menorValor[8], char maiorValor[8], char media[8], char temperatura[4]);
char SDCardGravaDados(DWORD indice, char media[8], char temperatura[4]);
char SDCardGravaLista(DWORD indice);


#endif	/* SDCARD_H */

