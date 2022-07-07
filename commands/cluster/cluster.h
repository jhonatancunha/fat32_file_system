/**
 * Descrição:
 *  Especificação da Função que implementa o comando cluster.
 *
 * Autores:
 *  Gustavo Sengling Favaro
 *  Jessé Pires Barbato Rocha,
 *  Jhonatan Guilherme de Oliveira Cunha,
 *
 *  Criado em: 10/06/2022
 *
 *  Última atualização: 29/06/2022
 */

#pragma once
#include <stdio.h>

#include "../../utils/utils.h"

/**
 * @brief Implementação do comando cluster.
 * @param[in] bootSector Ponteiro o bootSector.
 * @param[in] file Ponteiro para a imagem.
 * @param[in] number Número do cluster a ser exibido.
 */
void cluster(struct bootSectorStruct bootSector, FILE* file, uint32_t number);
