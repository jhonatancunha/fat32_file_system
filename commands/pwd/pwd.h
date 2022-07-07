/**
 * Descrição:
 *  Especificação da Função do comando pwd.
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
#include <string.h>

#include "../../StackDirectory/stackDirectory.h"

/**
 * @brief Retorna uma string com o diretório absoluto em relação ao diretório atual.
 * @param[in] stackDirectory Pilha de diretórios de um dado diretório.
 * @return String contendo o caminho absoluto do diretório atual.
 */
char* pwdCommand(StackDirectory* stackDirectory);