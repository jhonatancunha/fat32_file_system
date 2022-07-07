/**
 * Descrição:
 *  Especificação da Função que implementa o comando mv.
 *
 * Autores:
 *  Gustavo Sengling Favaro
 *  Jessé Pires Barbato Rocha,
 *  Jhonatan Guilherme de Oliveira Cunha,
 *
 *  Criado em: 26/06/2022
 *
 *  Última atualização: 29/06/2022
 */

#pragma once
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
 * @brief Implementação do comando mv.
 * @param[in] stackDirectory Pilha de diretórios de um dado diretório.
 * @param[in] file Ponteiro para a imagem.
 * @param[in] fat Ponteiro para a fat.
 * @param[in] bootSector Estrutura do BootSector.
 * @param[in] fsInfo Ponteiro para a estrutura FSInfo.
 * @param[in] source Caminho do arquivo de origem.
 * @param[in] destination Caminho do arquivo de destino.
 */
void mvCommand(StackDirectory* stackDirectory, FILE* file, FAT32_Clusters* fat,
               struct bootSectorStruct bootSector, struct FSInfoStruct* fsInfo,
               char* source, char* destination);
