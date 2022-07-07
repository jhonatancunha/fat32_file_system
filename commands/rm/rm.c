#include "rm.h"

// Função responsável por marcar os cluster ocupados pelo arquivo como livres
static inline void freeFATClusters(FAT32_Clusters* fat, uint32_t cluster,
                                   struct FSInfoStruct* fsInfo) {
  uint32_t previousCluster = cluster;
  uint32_t nextCluster = cluster;

  // Marcando clusters como livres
  while (1) {
    if (fat[nextCluster] == END_OF_CHAIN || fat[nextCluster] >= END_OF_CHAIN_2)
      break;

    previousCluster = nextCluster;
    nextCluster = fat[nextCluster];
    fat[previousCluster] = FREE_CLUSTER;
    fsInfo->FSI_Free_Count++;
  }

  fat[nextCluster] = FREE_CLUSTER;
  fsInfo->FSI_Free_Count++;
}

// Função responsável por realizar o comando rm para shortEntries
// retorna 1 se a DirEntry removida é a última de todos os cluster utilizados
// por este diretório
static inline int rmShortEntry(
    NodeDirEntry* nodeDirEntry, FILE* file, FAT32_Clusters* fat,
    struct bootSectorStruct bootSector, struct FSInfoStruct* fsInfo,
    int positionOfDirEntryInList, int finalPositionOfList,
    int positionOfDirEntryOnFile, uint32_t previousCluster) {
  uint32_t cluster =
      JOIN_LOW_HIGH_CLUSTER(nodeDirEntry->entry->shortEntry.DIR_FstClusLO,
                            nodeDirEntry->entry->shortEntry.DIR_FstClusHI);

  freeFATClusters(fat, cluster, fsInfo);
  int removedLastPosition = 0;

  // Verifico se o arquivo movido estava no fim ou meio da lista
  if (positionOfDirEntryInList == finalPositionOfList) {
    nodeDirEntry->entry->shortEntry.DIR_Name[0] = DIR_ENTRY_LAST_AND_UNUSED;
    removedLastPosition = 1;

    // Caso a dirEntry for a última do cluster (e o cluster for o último),
    // liberar o cluster. Não liberar caso for o diretório root
    if (positionOfDirEntryOnFile == 0 &&
        nodeDirEntry->cluster != getClusterOfRootDir(bootSector) &&
        fat[nodeDirEntry->cluster] == END_OF_CHAIN) {
      fat[nodeDirEntry->cluster] = FREE_CLUSTER;

      if (nodeDirEntry->previous != NULL)
        fat[nodeDirEntry->previous->cluster] = END_OF_CHAIN;

      fsInfo->FSI_Free_Count++;
    }
  } else {
    nodeDirEntry->entry->shortEntry.DIR_Name[0] = DIR_ENTRY_UNUSED;
  }

  // Atualizando Informações na FAT e no fsinfo
  saveFAT(bootSector, file, fat);
  saveFSInfo(fsInfo, bootSector, file);

  return removedLastPosition;
}

void rmCommand(ListDirEntry* listDirEntry, char* entryName, FILE* file,
               FAT32_Clusters* fat, struct bootSectorStruct bootSector,
               struct FSInfoStruct* fsInfo) {
  // Verificando se o nome do arquivo é válido
  if (strlen(entryName) > 11) {
    printf("Nome proibido! Número máximo de caracteres para arquivo é 11.\n");
    return;
  }

  // Obtendo a DirEntry e a posição da mesma
  int positionOfDirEntryInList = 0;
  NodeDirEntry* nodeDirEntry = getNodeDirEntryByFilename(
      listDirEntry, entryName, &positionOfDirEntryInList);

  // Verificando se existe um arquivo com o nome informado
  if (nodeDirEntry == NULL) {
    printf("Não há nenhum arquivo com o nome informado: %s\n", entryName);
    return;
  }

  // Definindo posição da DirEntry no cluster
  int maxDirInOneCluster = MAX_DIR_IN_CLUSTER(bootSector);
  int positionOfFile = positionOfDirEntryInList % maxDirInOneCluster;

  // Verificando se a entrada realmente é um arquivo
  if (nodeDirEntry->entry->shortEntry.DIR_Attr != ARCHIVE) {
    printf("Impossível remover! A entrada não é do tipo arquivo.\n");
    return;
  }

  int removedLastPosition =
      rmShortEntry(nodeDirEntry, file, fat, bootSector, fsInfo,
                   positionOfDirEntryInList, listDirEntry->qtdDirEntries - 1,
                   positionOfFile, listDirEntry->tail->previous->cluster);

  // Salvando alterações
  saveDirEntryOnPositionOfFile(bootSector, file, nodeDirEntry->entry,
                               nodeDirEntry->cluster, positionOfFile);

  // Caso o arquivo removido esteja na última posição, remover última Dir
  // Entry
  if (removedLastPosition) removeLastDirEntry(listDirEntry);
}
