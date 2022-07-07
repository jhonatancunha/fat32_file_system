/**
 * Descrição:
 *  Especificação da Função que implementa o comando info.
 *
 * Autores:
 *  Gustavo Sengling Favaro,
 *  Jessé Pires Barbato Rocha,
 *  Jhonatan Guilherme de Oliveira Cunha
 *
 *  Criado em: 30/05/2022
 *
 *  Última atualização: 29/06/2022
 */

#pragma once
#include <stdio.h>
#include <stdlib.h>

#include "../../ListDirEntry/listDirEntry.h"
#include "../../utils/utils.h"

/**
 * @brief Implementação do comando do comando info.
 * @param[in] file Ponteiro para a imagem.
 * @param[in] bootSector Estrutura do bootSector.
 * @param[in] fat Ponteiro para a fat.
 * @param[in] fsInfo Ponteiro para a estrutura FSInfo.
 */
void infoCommand(FILE *file, struct bootSectorStruct bootSector,
                 FAT32_Clusters *fat, struct FSInfoStruct *fsInfo);
