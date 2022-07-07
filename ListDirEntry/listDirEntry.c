#include "listDirEntry.h"

ListDirEntry* createListDirEntry() {
  ListDirEntry* list = (ListDirEntry*)malloc(sizeof(ListDirEntry));
  list->head = NULL;
  list->tail = NULL;
  list->qtdDirEntries = 0;
  list->amountOfCluster = 0;

  return list;
}

static DirEntry* insertInTheLastPosition(ListDirEntry* list,
                                         NodeDirEntry* entry) {
  struct NodeDirEntry* aux = list->head;

  // Procura pelo fim da lista de DirEntry
  while (aux->next != NULL) {
    aux = aux->next;
  }

  // Insere novo nó
  entry->previous = aux;
  aux->next = entry;

  return entry->entry;
}

void removeLastDirEntry(ListDirEntry* list) {
  if (list == NULL) return;

  NodeDirEntry* aux = list->tail;

  // Caso já estiver no último, apenas decrementa a quantidade de clusters.
  if (aux->previous != NULL) {
    // Caso não seja a última posição, mas seja o último elemento da lista,
    // decrementa a quantidade de clusters.
    if (aux->cluster != aux->previous->cluster) {
      list->amountOfCluster--;
    }
  } else {
    list->amountOfCluster--;
  }

  // Atualiza a fila
  list->tail = aux->previous;
  if (aux->previous != NULL) aux->previous->next = NULL;

  free(aux->entry);
  free(aux);

  list->qtdDirEntries--;

  if (list->qtdDirEntries == 0) {
    list->tail = NULL;
    list->head = NULL;
  };
}

static DirEntry* insertInPosition(ListDirEntry* list, NodeDirEntry* entry,
                                  int position) {
  struct NodeDirEntry* aux = list->head;

  int idx = 0;

  // Percorre a lista enquanto não chegar na posição ou enquanto a próxima
  // DirEntry não dor nula
  while (idx != position && aux->next != NULL) {
    aux = aux->next;
    idx++;
  }

  // Atualiza as informações da lista
  free(aux->entry);
  aux->cluster = entry->cluster;
  aux->entry = entry->entry;
  free(entry);

  return aux->entry;
}

DirEntry* insertDirEntry(ListDirEntry* list, NodeDirEntry* entry,
                         int* position) {
  if (list == NULL) return NULL;
  if (entry == NULL) return NULL;

  DirEntry* returnedEntry = NULL;

  // Verifica se a DirEntry será inserida no meio ou no fim da lista
  int insertInTheMiddleOfList =
      (position != NULL) && (*position < list->qtdDirEntries);

  if (list->qtdDirEntries == 0) {
    list->head = entry;
    list->tail = entry;
    list->qtdDirEntries++;
    returnedEntry = entry->entry;
  } else if (insertInTheMiddleOfList) {
    returnedEntry = insertInPosition(list, entry, *position);
  } else {
    returnedEntry = insertInTheLastPosition(list, entry);
    list->tail = entry;
    list->qtdDirEntries++;
  }

  return returnedEntry;
}

void destroyListDirEntry(ListDirEntry* list) {
  if (list == NULL) return;

  struct NodeDirEntry* entry = list->head;

  if (entry != NULL) {
    // Percorre a lista de DirEntry e libera o espaço das entradas
    for (int i = 0; i < list->qtdDirEntries; i++) {
      struct NodeDirEntry* nextEntry = entry->next;

      free(entry->entry);
      free(entry);
      entry = nextEntry;
    }

    free(entry);
  }

  free(list);
}

void readEntireClusterAndSaveInList(FILE* file, ListDirEntry* list,
                                    uint32_t cluster) {
  DirEntry* dirEntry;
  struct NodeDirEntry* nodeListDirEntry;
  int sizeInBytes = 0;

  while (sizeInBytes < 512) {
    // Cria DirEntry e adiciona as informações ao mesmo
    dirEntry = (DirEntry*)malloc(sizeof(DirEntry));
    nodeListDirEntry =
        (struct NodeDirEntry*)malloc(sizeof(struct NodeDirEntry));
    nodeListDirEntry->cluster = cluster;

    fread(dirEntry, sizeof(*dirEntry), 1, file);

    // Verificando se a DirEntry é a última e não está sendo usada. Se sim,
    // libera as mesmas
    if (dirEntry->shortEntry.DIR_Name[0] == 0x00) {
      free(dirEntry);
      free(nodeListDirEntry);
      break;
    }

    nodeListDirEntry->entry = dirEntry;
    nodeListDirEntry->next = NULL;
    insertDirEntry(list, nodeListDirEntry, NULL);
    sizeInBytes += 32;
  }
}
