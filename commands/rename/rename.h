/**
 * Descrição:
 *  Especificação da Função que implementa o comando help.
 *
 * Autores:
 *  Gustavo Sengling Favaro
 *  Jessé Pires Barbato Rocha,
 *  Jhonatan Guilherme de Oliveira Cunha,
 *
 *  Criado em: 24/06/2022
 *
 *  Última atualização: 29/06/2022
 */
#pragma once
#include <stdio.h>

#include "../../ListDirEntry/listDirEntry.h"
#include "../../utils/utils.h"

/**
 * @brief Implementação do comando rename.
 * @param[in] bootSector Instância da struct bootSectorStruct.
 * @param[in] file Ponteiro FILE para o arquivo.
 * @param[in] listDirEntry Ponteiro para a ListDirEntry em que realizaremos a
 * busca.
 * @param[in] oldName Nome do arquivo que desejamos renomear.
 * @param[in] newName Novo nome do arquivo.
 *
 */
void renameCommand(struct bootSectorStruct bootSector, FILE *file,
                   ListDirEntry *listDirEntry, char *oldName, char *newName);
