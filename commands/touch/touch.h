/**
 * Descrição:
 *  Especificação da Função do comando touch.
 *
 * Autores:
 *  Gustavo Sengling Favaro
 *  Jessé Pires Barbato Rocha,
 *  Jhonatan Guilherme de Oliveira Cunha,
 *
 *  Criado em: 16/06/2022
 *
 *  Última atualização: 29/06/2022
 */

#pragma once
#include <stdio.h>

#include "../../ListDirEntry/listDirEntry.h"
#include "../../utils/utils.h"

/**
 * @brief Implementação do comando touch.
 * @param[in] fat Ponteiro FAT32_Clusters.
 * @param[in] fsInfo Ponteiro da estrutura FSInfoStruct.
 * @param[in] bootSector Instância da struct bootSectorStruct.
 * @param[in] file Ponteiro FILE para o arquivo.
 * @param[in] listDirEntry Lista de DirEntry.
 * @param[in] name Nome do arquivo a ser criado.
 */
void touchCommand(FAT32_Clusters *fat, struct FSInfoStruct *fsInfo,
                  struct bootSectorStruct bootSector, FILE *file,
                  ListDirEntry *listDirEntry, char *name);
