#include "touch.h"

static inline void touchShortDirEntry(FAT32_Clusters *fat,
                                      struct FSInfoStruct *fsInfo,
                                      struct bootSectorStruct bootSector,
                                      FILE *file, ListDirEntry *listDirEntry,
                                      char *fatShortDirEntryName) {
  uint64_t totalFatEntries = getTotalNumberOfDataSectors(bootSector) + 1;

  uint32_t freeCluster = getOneFreeClusterInFat(fat, fsInfo, totalFatEntries);

  if (freeCluster == -1) {
    printf("Sem espaço de armazenamento livre.");
    return;
  }

  uint16_t currentTime = getCurrentTime();
  uint16_t currentDate = getCurrentDate();
  uint16_t lowCluster = GET_LOW_CLUSTER(freeCluster);
  uint16_t highCluster = GET_HIGH_CLUSTER(freeCluster);

  NodeDirEntry *nodeListDirEntry =
      (NodeDirEntry *)calloc(1, sizeof(NodeDirEntry));

  DirEntry *dirEntry = (DirEntry *)calloc(1, sizeof(DirEntry));

  strcpy(dirEntry->shortEntry.DIR_Name, fatShortDirEntryName);
  dirEntry->shortEntry.DIR_CrtTime = currentTime;
  dirEntry->shortEntry.DIR_WrtTime = currentTime;
  dirEntry->shortEntry.DIR_LstAccDate = currentDate;
  dirEntry->shortEntry.DIR_CrtDate = currentDate;
  dirEntry->shortEntry.DIR_WrtDate = currentDate;
  dirEntry->shortEntry.DIR_Attr = ARCHIVE;
  dirEntry->shortEntry.DIR_FstClusLO = lowCluster;
  dirEntry->shortEntry.DIR_FstClusHI = highCluster;

  nodeListDirEntry->entry = dirEntry;
  nodeListDirEntry->next = NULL;

  fat[freeCluster] = END_OF_CHAIN;
  fsInfo->FSI_Nxt_Free = freeCluster;
  fsInfo->FSI_Free_Count--;

  saveDirEntryOnFile(fat, fsInfo, bootSector, file, listDirEntry,
                     nodeListDirEntry, dirEntry);
}

void touchCommand(FAT32_Clusters *fat, struct FSInfoStruct *fsInfo,
                  struct bootSectorStruct bootSector, FILE *file,
                  ListDirEntry *listDirEntry, char *name) {
  // verificação básica (a ser mudada)
  if (strlen(name) > 11) {
    printf("Nome proibido. Número máximo de caracteres para arquivo é 11.\n");
    return;
  }

  // Verificando se já existe algum outro arquivo com o novo nome que deseja-se
  // utilizar
  if (getNodeDirEntryByFilename(listDirEntry, name, NULL) != NULL) {
    printf("Nome: %s já utilizado por outro arquivo.\n", name);
    return;
  }

  char *fatDirEntryName = fatWayShortDirName(name);

  if (!validateDirName(fatDirEntryName)) {
    printf("Nome inválido, não utilize caracteres especiais.\n");
    return;
  }

  touchShortDirEntry(fat, fsInfo, bootSector, file, listDirEntry,
                     fatDirEntryName);

  free(fatDirEntryName);
}
