/**
 * Descrição:
 *  Especificação da Função que implementa o comando ls.
 *
 * Autores:
 *  Gustavo Sengling Favaro
 *  Jessé Pires Barbato Rocha,
 *  Jhonatan Guilherme de Oliveira Cunha,
 *
 *  Criado em: 25/05/2022
 *
 *  Última atualização: 29/06/2022
 */

#pragma once
#include <stdio.h>

#include "../../ListDirEntry/listDirEntry.h"
#include "../../utils/utils.h"

/**
 * @brief Implementação do comando do comando ls.
 * @param[in] listDirEntry Ponteiro a lista de DirEntries.
 */
void lsCommand(ListDirEntry *listDirEntry);
