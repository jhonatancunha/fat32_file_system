/**
 * Descrição:
 *  Especificação das funções de manipulação da StackDiretctory bem como das
 * estruturas da mesma.
 *
 * Autores:
 *  Gustavo Sengling Favaro,
 *  Jessé Pires Barbato Rocha,
 *  Jhonatan Guilherme de Oliveira Cunha
 *
 *  Criado em: 22/05/2022
 *
 *  Última atualização: 29/06/2022
 */

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../ListDirEntry/listDirEntry.h"
#include "../utils/utils.h"

typedef struct NodeStackDirectory {
  struct NodeStackDirectory* next;
  struct NodeStackDirectory* previous;
  struct ListDirEntry* listDirEntry;
  char name[20];
} NodeStackDirectory;

typedef struct StackDirectory {
  int qtdDirectory;
  struct NodeStackDirectory* currentDirectory;
  struct NodeStackDirectory* rootDirectory;
} StackDirectory;

/**
 * @brief Cria uma instância da pilha de diretórios.
 * @return O ponteiro para a pilha criada.
 */
StackDirectory* createStackDirectory();

/**
 * @brief Insere um nó na pilha de diretórios.
 * @param[in] stack Pilha onde o diretório (nó) será inserido.
 * @param[in] node Instância do diretório que será inserido.
 * @param[in] name Nome do diretório que será empilhado.
 */
void push(StackDirectory* stack, NodeStackDirectory* node, char* name);

/**
 * @brief Remove o diretório que está no topo da pilha.
 * @param[in] stack Pilha de onde o diretório será removido.
 */
void pop(StackDirectory* stack);

/**
 * @brief Destroi a pilha por completo.
 * @param[in] stack Pilha que será deletada.
 */
void destroyStack(StackDirectory* stack);

/**
 * @brief Lê o diretório por completo e salva o mesmo na pilha de diretórios.
 * @param[in] fat Ponteiro para a tabela FAT 32.
 * @param[in] bootSector Instância da struct bootSectorStruct.
 * @param[in] file Ponteiro FILE para o arquivo.
 * @param[in] stack Pilha onde será inserido o diretório.
 * @param[in] cluster Cluster onde o diretório se encontra.
 * @param[in] name Nome do diretório que será lido.
 */
void readAllClusterOfDirectoryAndPushIntoStack(
    FAT32_Clusters* fat, struct bootSectorStruct bootSector, FILE* file,
    StackDirectory* stack, uint32_t cluster, char* name);