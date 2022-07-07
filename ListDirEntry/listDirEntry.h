#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../FAT/fat.h"

#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20
#define ATTR_LONG_NAME \
  ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID
#define ATTR_LONG_NAME_MASK                                     \
  ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID | \
      ATTR_DIRECTORY | ATTR_ARCHIVE

#define GET_TYPE_OF_ENTRY(entry) \
  (entry->shortEntry.DIR_Attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID))

#define VALIDATE_TYPE_ENTRY(entry)                                           \
  (((entry->longEntry.LDIR_Attr & ATTR_LONG_NAME_MASK) != ATTR_LONG_NAME) && \
   (entry->longEntry.LDIR_Ord != 0xE5))

typedef struct NodeDirEntry {
  struct NodeDirEntry* next;
  struct NodeDirEntry* previous;
  DirEntry* entry;
  uint32_t cluster;
} NodeDirEntry;

typedef struct ListDirEntry {
  int qtdDirEntries;
  int amountOfCluster;
  struct NodeDirEntry* head;
  struct NodeDirEntry* tail;
} ListDirEntry;

/**
 * @brief Cria uma instância da lista de DirEntry.
 * @return O ponteiro para a lista criada.
 */
ListDirEntry* createListDirEntry();

/**
 * @brief Insere uma DirEntry na lista.
 * @param[in] list Lista onde será inserida a DirEntry.
 * @param[in] entry Instância da DirEntry que será inserida.
 * @param[in] position Posição da lista onde a DirEntry será inserida (NULL caso
 * queira inserir no fim da lista).
 * @return Ponteiro da DirEntry inserida na lista.
 */
DirEntry* insertDirEntry(ListDirEntry* list, NodeDirEntry* entry,
                         int* position);

/**
 * @brief Destroi a lista de DirEntry por completo.
 * @param[in] list Lista que será deletada.
 */
void destroyListDirEntry(ListDirEntry* list);

/**
 * @brief Remove a ultima DirEntry da lista.
 * @param[in] list Lista de onde o nó será removido.
 */
void removeLastDirEntry(ListDirEntry* list);

/**
 * @brief Lê o cluster inteiro e salva as DirEntry do mesmo na lista de
 * DirEntry.
 * @param[in] file Ponteiro FILE para o arquivo.
 * @param[in] list Lista onde será inserido as DirEntry.
 * @param[in] cluster Cluster onde se encontra as DirEntry.
 */
void readEntireClusterAndSaveInList(FILE* file, ListDirEntry* list,
                                    uint32_t cluster);
