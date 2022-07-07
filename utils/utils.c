#include "utils.h"

uint64_t getFirstDataSector(struct bootSectorStruct bootSector) {
  return bootSector.BPB_RsvdSecCnt +
         (bootSector.BPB_NumFATs * bootSector.BPB_FATSz32) + rootDirSectors;
}

uint64_t getFat1Address(struct bootSectorStruct bootSector) {
  return bootSector.BPB_RsvdSecCnt * bootSector.BPB_BytsPerSec;
}

uint64_t getFat2Address(struct bootSectorStruct bootSector) {
  return (bootSector.BPB_FATSz32 + bootSector.BPB_RsvdSecCnt) *
         bootSector.BPB_BytsPerSec;
}

uint64_t getFirstSectorOfCluster(struct bootSectorStruct bootSector,
                                 uint32_t N) {
  if (N < 2) return 0;

  return (((N - 2) * bootSector.BPB_SecPerClus) +
          getFirstDataSector(bootSector)) *
         bootSector.BPB_BytsPerSec;
}

uint64_t getClusterOfRootDir(struct bootSectorStruct bootSector) {
  return bootSector.BPB_RootClus;
}

uint64_t getTotalNumberOfDataSectors(struct bootSectorStruct bootSector) {
  uint64_t dataSec =
      bootSector.BPB_TotSec32 -
      (bootSector.BPB_RsvdSecCnt +
       (bootSector.BPB_NumFATs * bootSector.BPB_FATSz32) + rootDirSectors);

  return dataSec / bootSector.BPB_SecPerClus;
}

uint32_t getFreeClusterCount(struct FSInfoStruct *fsInfo) {
  return fsInfo->FSI_Free_Count;
}

uint64_t getSizeOfImage(FILE *file) {
  fseek(file, 0, SEEK_END);
  uint64_t sizeOfFile = ftell(file);
  fseek(file, 0, SEEK_SET);

  return sizeOfFile;
}

uint64_t getDataSize(uint64_t sizeOfImage, struct bootSectorStruct bootSector) {
  uint64_t totalSec32 = bootSector.BPB_TotSec32;
  uint64_t totalDataCluster = getTotalNumberOfDataSectors(bootSector);
  uint64_t dataSizeInBytes =
      (totalSec32 - totalDataCluster) * bootSector.BPB_BytsPerSec;

  return sizeOfImage - dataSizeInBytes;
}

static inline int convertBinaryToNumber(int *binary, int start, int end) {
  int aux = 1;
  int result = 0;

  for (int i = start; i <= end; i++) {
    if (binary[i] == 1) {
      result += aux;
    }
    aux <<= 1;
  }

  return result;
}

void convertNumberToBinary(uint64_t number, int *binary) {
  int i = 0;
  while (number > 0) {
    binary[i] = number % 2;
    number = number / 2;
    i++;
  }
}

void getDateFromFile(uint16_t date) {
  int binary[16] = {0};
  uint64_t dateNumber = date;

  convertNumberToBinary(dateNumber, binary);

  int day = convertBinaryToNumber(binary, 0, 4);
  int mounth = convertBinaryToNumber(binary, 5, 8);
  int countOfYears = convertBinaryToNumber(binary, 9, 15);
  int year = 1980 + countOfYears;

  printf("%02d/%02d/%04d", day, mounth, year);
}

void getTimestampFromFile(uint16_t time) {
  int binary[16] = {0};
  uint64_t timeStamp = time;

  // Convert number to binary
  int i = 0;
  while (timeStamp > 0) {
    binary[i] = timeStamp % 2;
    timeStamp = timeStamp / 2;
    i++;
  }

  int seconds = convertBinaryToNumber(binary, 0, 4) * 2;
  int minutes = convertBinaryToNumber(binary, 5, 10);
  int hours = convertBinaryToNumber(binary, 11, 15);

  printf("%02d:%02d:%02d", hours, minutes, seconds);
}

char *prettyShortDirName(char *name, uint8_t attrOfEntry) {
  if (strlen(name) == 0) return NULL;

  char *prettyName = (char *)calloc(12, sizeof(char));

  int i, j;
  for (i = 0, j = 0; i < 8; i++) {
    if (name[i] == ' ') continue;

    prettyName[j++] = tolower(name[i]);
  }

  int hasExtension = name[8] != 0x20 || name[9] != 0x20 || name[10] != 0x20;

  if (hasExtension && attrOfEntry == ARCHIVE) {
    prettyName[j++] = '.';
  }

  for (; i < 11; i++) {
    if (name[i] == ' ') continue;

    prettyName[j++] = tolower(name[i]);
  }

  return prettyName;
}

char *fatWayShortDirName(char *name) {
  if (strlen(name) == 0) return NULL;

  int lastPosition = strlen(name) - 1;
  int stopConditionFor = strlen(name);
  char *fatWayName = (char *)calloc(11, sizeof(char));
  memset(fatWayName, 0x20, 11);

  char *firstName;

  // Putting the extension in string
  if (strchr(name, '.') != NULL) {
    firstName = (char *)calloc(8, sizeof(char));
    memcpy(firstName, name, strlen(name) - 4);

    fatWayName[10] = toupper(name[lastPosition]);
    fatWayName[9] = toupper(name[lastPosition - 1]);
    fatWayName[8] = toupper(name[lastPosition - 2]);
    stopConditionFor = strlen(firstName);
  } else {
    firstName = (char *)calloc(11, sizeof(char));
    memcpy(firstName, name, strlen(name));
  }

  for (int i = 0; i < stopConditionFor; i++) {
    // if (firstName[i] == '.') firstName[i] = ' ';
    fatWayName[i] = toupper(firstName[i]);
  };

  free(firstName);
  return fatWayName;
}

NodeDirEntry *getNodeDirEntryByFilename(ListDirEntry *listDirEntry,
                                        char *filename, int *positionOfFile) {
  if (listDirEntry == NULL) return NULL;
  if (strlen(filename) == 0) return NULL;

  NodeDirEntry *aux = listDirEntry->head;
  char dirEntryHasBeingFound = '0';

  if (positionOfFile != NULL) *positionOfFile = 0;

  while (1) {
    char *prettyName = NULL;
    if (aux->entry->shortEntry.DIR_Name[0] != DIR_ENTRY_LAST_AND_UNUSED) {
      prettyName = prettyShortDirName(aux->entry->shortEntry.DIR_Name,
                                      aux->entry->shortEntry.DIR_Attr);

      if (prettyName[0] != DIR_ENTRY_UNUSED) {
        if (strcmp(prettyName, filename) == 0) {
          dirEntryHasBeingFound = '1';
        }
      }
    }

    if (prettyName != NULL) free(prettyName);

    if (dirEntryHasBeingFound == '1') break;

    if (aux->next == NULL) break;
    aux = aux->next;
    if (positionOfFile != NULL) *positionOfFile += 1;
  }

  if (dirEntryHasBeingFound == '0') return NULL;

  return aux;
}

void saveDataClusterInFile(struct bootSectorStruct bootSector, FILE *file,
                           uint32_t cluster, uint8_t *buffer,
                           uint32_t sizeOfBuffer) {
  uint64_t clusterPosition = getFirstSectorOfCluster(bootSector, cluster);

  // Posicionando ponteiro no dirEntry informado
  fseek(file, clusterPosition, SEEK_SET);
  fwrite(buffer, sizeOfBuffer, 1, file);
}

void saveDirEntryOnPositionOfFile(struct bootSectorStruct bootSector,
                                  FILE *file, DirEntry *entry, uint32_t cluster,
                                  int dirEntryPosition) {
  uint64_t clusterPosition = getFirstSectorOfCluster(bootSector, cluster);

  int dirEntryOffset = sizeof(*entry) * dirEntryPosition;

  fseek(file, clusterPosition + dirEntryOffset, SEEK_SET);
  fwrite(entry, sizeof(*entry), 1, file);
}

int validateDirName(char *dirname) {
  int size = strlen(dirname);

  // Verificando primeiro caracter
  if (dirname[0] == 0x20) {
    printf("Nome inválido!\n");
    return 0;
  }

  for (int i = 1; i < size; i++) {
    switch (dirname[i]) {
      case 0x22:
      case 0x2A:
      case 0x2B:
      case 0x2C:
      case 0x2E:
      case 0x2F:
      case 0x3A:
      case 0x3B:
      case 0x3C:
      case 0x3D:
      case 0x3E:
      case 0x3F:
      case 0x5B:
      case 0x5C:
      case 0x5D:
      case 0x7C:
        return 0;
        break;
    }
  }

  return 1;
}

uint16_t getCurrentDate() {
  time_t dateNow;
  struct tm *dateStruct;
  time(&dateNow);
  dateStruct = localtime(&dateNow);

  unsigned long int teste = dateNow * 1000;

  int day = dateStruct->tm_mday;
  int mounth = dateStruct->tm_mon + 1;
  int year = dateStruct->tm_year + 1900;
  int fatYeat = year - 1980;

  int binDay[5] = {0};
  int binMounth[4] = {0};
  int binYear[7] = {0};

  convertNumberToBinary(day, binDay);
  convertNumberToBinary(mounth, binMounth);
  convertNumberToBinary(fatYeat, binYear);

  int dateBinary[16] = {0};
  int i = 0;
  for (int j = 0; j < 5; i++, j++) dateBinary[i] = binDay[j];
  for (int j = 0; j < 4; i++, j++) dateBinary[i] = binMounth[j];
  for (int j = 0; j < 7; i++, j++) dateBinary[i] = binYear[j];

  uint16_t currentData = convertBinaryToNumber(dateBinary, 0, 16);

  return currentData;
}

uint16_t getCurrentTime() {
  time_t timeNow;
  struct tm *timeStruct;
  time(&timeNow);
  timeStruct = localtime(&timeNow);

  int seconds = timeStruct->tm_sec / 2;
  int minutes = timeStruct->tm_min;
  int hours = timeStruct->tm_hour;

  int binSeconds[5] = {0};
  int binMinutes[6] = {0};
  int binHours[5] = {0};

  convertNumberToBinary(seconds, binSeconds);
  convertNumberToBinary(minutes, binMinutes);
  convertNumberToBinary(hours, binHours);

  int timeBinary[16] = {0};
  int i = 0;
  for (int j = 0; j < 5; i++, j++) timeBinary[i] = binSeconds[j];
  for (int j = 0; j < 6; i++, j++) timeBinary[i] = binMinutes[j];
  for (int j = 0; j < 5; i++, j++) timeBinary[i] = binHours[j];

  uint16_t currentTime = convertBinaryToNumber(timeBinary, 0, 16);

  return currentTime;
}

uint32_t getOneFreeClusterInFat(FAT32_Clusters *fat,
                                struct FSInfoStruct *fsInfo,
                                uint64_t totalFatEntries) {
  uint32_t freeCluster = -1;
  uint32_t lastFreeCluster =
      fsInfo->FSI_Nxt_Free == FSI_NXT_FREE_UNKNOWN ? 2 : fsInfo->FSI_Nxt_Free;

  for (int i = lastFreeCluster; i < totalFatEntries; i++) {
    if (fat[i] == FREE_CLUSTER) {
      freeCluster = i;
      break;
    }
  }

  // Caso não encontrar nenhum cluster acima
  // Verificamos se existe algum cluster antes do lastFreeCluster
  if (freeCluster == -1) {
    for (int i = 2; i <= lastFreeCluster; i++) {
      if (fat[i] == FREE_CLUSTER) {
        freeCluster = i;
        break;
      }
    }
  }

  return freeCluster;
}

uint32_t getFreeClusterAndConcatenate(FAT32_Clusters *fat,
                                      struct FSInfoStruct *fsInfo,
                                      uint64_t totalFatEntries,
                                      uint32_t lastFatPos) {
  uint32_t freeCluster = getOneFreeClusterInFat(fat, fsInfo, totalFatEntries);
  fat[lastFatPos] = freeCluster;
  fat[freeCluster] = END_OF_CHAIN;

  return freeCluster;
}

void saveFAT(struct bootSectorStruct bootSector, FILE *file,
             FAT32_Clusters *fat) {
  uint64_t fatSize = getTotalNumberOfDataSectors(bootSector) + 1;

  // Salvando FAT1
  fseek(file, getFat1Address(bootSector), SEEK_SET);
  fwrite(fat, fatSize, sizeof(*fat), file);

  // Salvando FAT2
  fseek(file, getFat2Address(bootSector), SEEK_SET);
  fwrite(fat, fatSize, sizeof(*fat), file);
}

int checkCanSaveInTheLastCluster(struct bootSectorStruct bootSector,
                                 int amountOfDirEntries, int amountOfCluster) {
  uint64_t maxSize =
      (bootSector.BPB_BytsPerSec * bootSector.BPB_SecPerClus) * amountOfCluster;

  uint64_t actualSize = (amountOfDirEntries * 32);

  return maxSize <= actualSize ? 0 : 1;
}

void saveFSInfo(struct FSInfoStruct *fsInfo, struct bootSectorStruct bootSector,
                FILE *file) {
  int fs_struct_offset = bootSector.BPB_BytsPerSec * bootSector.BPB_FSInfo;

  fseek(file, fs_struct_offset, SEEK_SET);
  fwrite(fsInfo, sizeof(*fsInfo), 1, file);
}

int tokenizePath(char ***tokenizedPath, char *path) {
  int size = strlen(path);

  if (size < 1) return -1;

  char pathTokenized[size + 1];
  memset(pathTokenized, '\0', sizeof(pathTokenized));
  strcpy(pathTokenized, path);

  // Contando quantas strings devemos armazenar
  int amountOfStrings = 1;
  char *token = strchr(pathTokenized, '/');
  while (token != NULL) {
    amountOfStrings++;
    token++;
    token = strchr(token, '/');
  }

  // Armazenando string em matrix
  char **allPaths = (char **)calloc(amountOfStrings + 1, sizeof(char *));
  int i = 0;
  token = strtok(pathTokenized, "/");
  char *buffer;

  while (token != 0x0) {
    buffer = (char *)calloc(strlen(token) + 1, sizeof(char));
    strcpy(buffer, token);
    allPaths[i++] = buffer;
    token = strtok(NULL, "/");
  }

  *tokenizedPath = allPaths;
  return amountOfStrings;
}

int findFreePositionOnDirEntryList(FAT32_Clusters *fat,
                                   ListDirEntry *listDirEntry,
                                   uint32_t *clusterOfEntry) {
  if (listDirEntry->qtdDirEntries == 0) return 0;

  if (listDirEntry == NULL) return -1;

  NodeDirEntry *nodeDirEntry = listDirEntry->head;

  int idx = 0;

  while (1) {
    if (nodeDirEntry->entry->shortEntry.DIR_Name[0] == DIR_ENTRY_UNUSED ||
        nodeDirEntry->entry->shortEntry.DIR_Name[0] ==
            DIR_ENTRY_LAST_AND_UNUSED) {
      *clusterOfEntry = nodeDirEntry->cluster;

      return idx;
    }

    idx++;
    if (nodeDirEntry->next == NULL) return -1;
    nodeDirEntry = nodeDirEntry->next;
  }
}

int saveDirEntryOnFile(FAT32_Clusters *fat, struct FSInfoStruct *fsInfo,
                       struct bootSectorStruct bootSector, FILE *file,
                       ListDirEntry *listDirEntry,
                       NodeDirEntry *nodeListDirEntry, DirEntry *dirEntry) {
  uint64_t totalFatEntries = getTotalNumberOfDataSectors(bootSector) + 1;

  uint32_t clusterToSave = nodeListDirEntry->cluster;
  int freePositionOnList = -1;
  int canUseTheLastCluster = -1;

  // Tentando encontrar alguma posição livre na lista de DirEntry
  freePositionOnList =
      findFreePositionOnDirEntryList(fat, listDirEntry, &clusterToSave);

  // Não encontrou espaço na lista salvará no fim da mesma
  int positionToInsert = freePositionOnList != -1 ? freePositionOnList
                                                  : listDirEntry->qtdDirEntries;

  // Caso não exista posição livre na lista, devemos inserir no fim da mesma.
  // Sendo assim, devemos verifica se é possivel utilizar o último cluster
  // alocado pelo arquivo.
  if (freePositionOnList == -1)
    canUseTheLastCluster = checkCanSaveInTheLastCluster(
        bootSector, listDirEntry->qtdDirEntries, listDirEntry->amountOfCluster);

  // Obtendo posição que a DirEntry será salvo baseado em seu cluster
  int maxDirInOneCluster = MAX_DIR_IN_CLUSTER(bootSector);
  int dirEntryPosition = positionToInsert % maxDirInOneCluster;

  // Caso a posição a ser inserido for a última da lista...
  // Necessitamos alocar mais um cluster caso a posição a ser inserido no
  // arquivo for a 0 em relação ao seu cluster e a mesma for o fim das
  // dirEntry (DIR_ENTRY_LAST_AND_UNUSED).
  if (positionToInsert == listDirEntry->qtdDirEntries - 1) {
    if (dirEntryPosition == 0 &&
        listDirEntry->tail->entry->shortEntry.DIR_Name[0] ==
            DIR_ENTRY_LAST_AND_UNUSED) {
      clusterToSave = getOneFreeClusterInFat(fat, fsInfo, totalFatEntries);
      fat[listDirEntry->tail->previous->cluster] = clusterToSave;
      fat[clusterToSave] = END_OF_CHAIN;
      fsInfo->FSI_Nxt_Free = clusterToSave;
      fsInfo->FSI_Free_Count--;
    }
  }

  // Caso possa utilizar o último cluster alocado iremos usa-los.
  // Caso contrario, deveremos buscar outro cluster e concatena-lo na FAT
  if (canUseTheLastCluster == 1) {
    clusterToSave = listDirEntry->tail->cluster;
  } else if (freePositionOnList == -1) {
    clusterToSave = getFreeClusterAndConcatenate(fat, fsInfo, totalFatEntries,
                                                 listDirEntry->tail->cluster);
    dirEntryPosition = 0;
    listDirEntry->amountOfCluster++;
    fsInfo->FSI_Nxt_Free = clusterToSave;
    fsInfo->FSI_Free_Count--;

    // Zerar novo cluster
    // Pois o mesmo pode conter lixo
    // Aloca todo o espaço do buffer
    uint64_t sizeOfCluster =
        bootSector.BPB_BytsPerSec * bootSector.BPB_SecPerClus;
    uint8_t buffer[sizeOfCluster + 1];
    memset(buffer, '\0', sizeOfCluster + 1);
    fseek(file, getFirstSectorOfCluster(bootSector, clusterToSave), SEEK_SET);
    fwrite(buffer, 1, sizeOfCluster, file);
  }

  // Salva tudo
  nodeListDirEntry->cluster = clusterToSave;
  saveFAT(bootSector, file, fat);
  saveFSInfo(fsInfo, bootSector, file);
  dirEntry = insertDirEntry(listDirEntry, nodeListDirEntry, &positionToInsert);
  saveDirEntryOnPositionOfFile(bootSector, file, dirEntry, clusterToSave,
                               dirEntryPosition);

  return dirEntryPosition;
}
