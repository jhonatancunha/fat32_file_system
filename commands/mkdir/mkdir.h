/**
 * Descrição:
 *  Especificação da Função que implementa o comando mkdir.
 *
 * Autores:
 *  Gustavo Sengling Favaro
 *  Jessé Pires Barbato Rocha,
 *  Jhonatan Guilherme de Oliveira Cunha,
 *
 *  Criado em: 15/06/2022
 *
 *  Última atualização: 29/06/2022
 */

#pragma once
#include <stdio.h>

#include "../../StackDirectory/stackDirectory.h"
#include "../cd/cd.h"

/**
 * @brief Implementação do comando mkdir.
 * @param[in] fat Ponteiro para a fat.
 * @param[in] fsInfo Ponteiro para a estrutura FSInfo.
 * @param[in] bootSector Estrutura do BootSector.
 * @param[in] file Ponteiro para a imagem.
 * @param[in] listDirEntry Ponteiro para a lista de DirEntries.
 * @param[in] stack Ponteiro para a stack de diretórios.
 * @param[in] name Nome que será dado para o novo diretório.
 */
void mkdirCommand(FAT32_Clusters* fat, struct FSInfoStruct* fsInfo,
                  struct bootSectorStruct bootSector, FILE* file,
                  ListDirEntry* listDirEntry, StackDirectory* stack,
                  char* name);
