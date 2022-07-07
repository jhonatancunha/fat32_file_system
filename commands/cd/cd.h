/**
 * Descrição:
 *  Especificação da Função do comando cd.
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

#include "../../StackDirectory/stackDirectory.h"
#include "../../utils/utils.h"
#include "../pwd/pwd.h"

/**
 * @brief Muda o diretório para o diretório destino a partir do diretório atual.
 * @param[in] fat Ponteiro FAT32_Clusters da tabela FAT que será salva.
 * @param[in] bootSector Instância da struct bootSectorStruct.
 * @param[in] stackDirectory Pilha de diretórios de um dado diretório.
 * @param[in] file Ponteiro FILE para o arquivo.
 * @param[in] path Diretório destino a ser acessado.
 * @return Valor inteiro indicando sucesso de execução, 0 para não e 1 para sim.
 */
int cdCommand(FAT32_Clusters* fat, struct bootSectorStruct bootSector,
              StackDirectory* stackDirectory, FILE* file, char* path);