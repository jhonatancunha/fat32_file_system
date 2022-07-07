/**
 * Descrição:
 *  Especificação da Função do comando cp.
 *
 * Autores:
 *  Gustavo Sengling Favaro
 *  Jessé Pires Barbato Rocha,
 *  Jhonatan Guilherme de Oliveira Cunha,
 *
 *  Criado em: 27/06/2022
 *
 *  Última atualização: 29/06/2022
 */

#pragma once
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "../../StackDirectory/stackDirectory.h"
#include "../../utils/utils.h"
#include "../cd/cd.h"
#include "../pwd/pwd.h"
#include "../rm/rm.h"
#include "../touch/touch.h"

/**
 * @brief Implementação do comando cp
 * @param[in] stackDirectory Pilha de diretórios de um dado diretório.
 * @param[in] file Ponteiro FILE para o arquivo.
 * @param[in] fat Ponteiro FAT32_Clusters da tabela FAT.
 * @param[in] bootSector Instância da struct bootSectorStruct.
 * @param[in] fsInfo Ponteiro da estrutura FSInfoStruct.
 * @param[in] source Caminho para arquivo de origem.
 * @param[in] destination Caminho para arquivo de destino.
 */
void cpCommand(StackDirectory* stackDirectory, FILE* file, FAT32_Clusters* fat,
               struct bootSectorStruct bootSector, struct FSInfoStruct* fsInfo,
               char* source, char* destination);
