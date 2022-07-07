/**
 * Descrição:
 *  Especificação da Função do comando rmdir.
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
#include <stdlib.h>

#include "../../ListDirEntry/listDirEntry.h"
#include "../../StackDirectory/stackDirectory.h"
#include "../../utils/utils.h"
#include "../cd/cd.h"
#include "../pwd/pwd.h"

/**
 * @brief Remove um diretório caso ele esteja vazio.
 * @param[in] stackDirectory Pilha de diretórios de um dado diretório.
 * @param[in] entryName Diretório a ser removido.
 * @param[in] file Ponteiro FILE para o arquivo.
 * @param[in] fat Ponteiro FAT32_Clusters da tabela FAT que será salva.
 * @param[in] bootSector Instância da struct bootSectorStruct.
 * @param[in] fsInfo Ponteiro da estrutura FSInfoStruct que será salva.
 */
void rmdirCommand(StackDirectory* stackDirectory, char* entryName, FILE* file,
                  FAT32_Clusters* fat, struct bootSectorStruct bootSector,
                  struct FSInfoStruct* fsInfo);
