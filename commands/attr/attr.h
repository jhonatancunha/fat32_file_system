/**
 * Descrição:
 *  Especificação da Função do comando attr.
 *
 * Autores:
 *  Gustavo Sengling Favaro
 *  Jessé Pires Barbato Rocha,
 *  Jhonatan Guilherme de Oliveira Cunha,
 *
 *  Criado em: 03/06/2022
 *
 *  Última atualização: 29/06/2022
 */

#pragma once
#include <stdio.h>
#include <string.h>

#include "../../ListDirEntry/listDirEntry.h"
#include "../../utils/utils.h"

/**
 * @brief Implementação do comando attr.
 * @param[in] listDirEntry Ponteiro para Lista de DirEntries.
 * @param[in] entryName Ponteiro para o nome.
 */
void attrCommand(ListDirEntry* listDirEntry, char* entryName);
