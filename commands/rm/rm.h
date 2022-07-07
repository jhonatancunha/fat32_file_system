/**
 * Descrição:
 *  Especificação da Função que implementa o comando rm.
 *
 * Autores:
 *  Gustavo Sengling Favaro
 *  Jessé Pires Barbato Rocha,
 *  Jhonatan Guilherme de Oliveira Cunha,
 *
 *  Criado em: 25/06/2022
 *
 *  Última atualização: 29/06/2022
 */

#pragma once
#include <stdio.h>
#include <stdlib.h>

#include "../../ListDirEntry/listDirEntry.h"
#include "../../utils/utils.h"

/**
 * @brief Implementação do comando rm.
 * @param[in] listDirEntry Ponteiro para a lista de DirEntries.
 * @param[in] entryName Nome do arquivo a ser removido.
 * @param[in] file Ponteiro para a imagem.
 * @param[in] fat Ponteiro para a FAT.
 * @param[in] bootSector Estrutura do BootSector.
 * @param[in] fsInfo Ponteiro para a estrutura FSInfo.
 */
void rmCommand(ListDirEntry* listDirEntry, char* entryName, FILE* file,
               FAT32_Clusters* fat, struct bootSectorStruct bootSector,
               struct FSInfoStruct* fsInfo);
