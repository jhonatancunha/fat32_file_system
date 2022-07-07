#include "cp.h"

// Obtêm o proximo cluster que está encadeado
static inline uint32_t getNextCluster(FAT32_Clusters* fat, uint32_t cluster) {
  if (fat[cluster] == END_OF_CHAIN || fat[cluster] >= END_OF_CHAIN_2) return -1;
  return fat[cluster];
}

// Realiza a cópia a área de dados do cluster para outro cluster livre
static inline int copyDataCluster(FAT32_Clusters* fat,
                                  struct FSInfoStruct* fsInfo,
                                  struct bootSectorStruct bootSector,
                                  FILE* file, DirEntry* dirEntry) {
  uint16_t lowCluster = dirEntry->shortEntry.DIR_FstClusLO;
  uint16_t highCluster = dirEntry->shortEntry.DIR_FstClusHI;
  uint32_t cluster = JOIN_LOW_HIGH_CLUSTER(lowCluster, highCluster);

  uint32_t sizeOfBytesInCluster =
      bootSector.BPB_SecPerClus * bootSector.BPB_BytsPerSec;
  uint32_t fileSize = dirEntry->shortEntry.DIR_FileSize;
  int amountOfCluster = ceil(fileSize / (float)sizeOfBytesInCluster);
  amountOfCluster = amountOfCluster == 0 ? 1 : amountOfCluster;

  // Caso não exista mais cluster livres, não deixar alocar
  if (getFreeClusterCount(fsInfo) < amountOfCluster) {
    printf("Sem espaço de armazenamento.\n");
    return 0;
  }

  // Verificando se existe clusters vazios para realizar a cópia
  uint64_t totalFatEntries = getTotalNumberOfDataSectors(bootSector) + 1;
  uint32_t backupNextFreeFSInfo = fsInfo->FSI_Nxt_Free;
  int clusterVect[amountOfCluster];
  for (int i = 0; i < amountOfCluster; i++) {
    uint32_t freeCluster = getOneFreeClusterInFat(fat, fsInfo, totalFatEntries);
    if (freeCluster == -1) {
      printf("Sem espaço de armazenamento livre.");
      fsInfo->FSI_Nxt_Free = backupNextFreeFSInfo;
      return 0;
    }

    clusterVect[i] = freeCluster;
    fsInfo->FSI_Nxt_Free = freeCluster + 1;
  }

  // Aloca todo o espaço do buffer
  uint8_t* buffer =
      (uint8_t*)calloc(amountOfCluster * sizeOfBytesInCluster, sizeof(uint8_t));
  memset(buffer, '\0', amountOfCluster * sizeOfBytesInCluster);

  // Alimenta o buffer por completo com os dados do cluster origem
  uint32_t auxCluster = cluster;
  int offset = 0;
  do {
    fseek(file, getFirstSectorOfCluster(bootSector, auxCluster), SEEK_SET);
    fread(buffer + offset, sizeof(uint8_t), sizeOfBytesInCluster, file);
    offset += sizeOfBytesInCluster;
    auxCluster = getNextCluster(fat, auxCluster);
  } while (auxCluster != -1);

  // Atualizando dados da FAT e FsStruct
  offset = 0;
  for (int i = 0; i < amountOfCluster; i++) {
    if (i != amountOfCluster - 1)
      fat[clusterVect[i]] = clusterVect[i + 1];
    else
      fat[clusterVect[i]] = END_OF_CHAIN;

    fsInfo->FSI_Nxt_Free = clusterVect[i];
    fsInfo->FSI_Free_Count--;

    saveDataClusterInFile(bootSector, file, clusterVect[i], buffer + offset,
                          sizeOfBytesInCluster);

    offset += sizeOfBytesInCluster;
  }

  // Atualizand cluster do arquivo e salva
  dirEntry->shortEntry.DIR_FstClusHI = GET_HIGH_CLUSTER(clusterVect[0]);
  dirEntry->shortEntry.DIR_FstClusLO = GET_LOW_CLUSTER(clusterVect[0]);
  saveFAT(bootSector, file, fat);
  saveFSInfo(fsInfo, bootSector, file);

  free(buffer);
  return 1;
}

// Função responsável por realizar o comando cp para shortEntries
static inline int cpShortDirEntry(ListDirEntry* listDirEntry, FILE* file,
                                  FAT32_Clusters* fat,
                                  struct bootSectorStruct bootSector,
                                  struct FSInfoStruct* fsInfo,
                                  NodeDirEntry* nodeDirEntry) {
  NodeDirEntry* nodeDirEntryCopy =
      (NodeDirEntry*)calloc(1, sizeof(NodeDirEntry));

  DirEntry* dirEntryCopy = (DirEntry*)calloc(1, sizeof(DirEntry));
  memcpy(dirEntryCopy, nodeDirEntry->entry, sizeof(*dirEntryCopy));

  nodeDirEntryCopy->entry = dirEntryCopy;

  uint16_t currentTime = getCurrentTime();
  uint16_t currentDate = getCurrentDate();
  dirEntryCopy->shortEntry.DIR_WrtTime = currentTime;
  dirEntryCopy->shortEntry.DIR_WrtDate = currentDate;
  dirEntryCopy->shortEntry.DIR_LstAccDate = currentDate;

  int success = copyDataCluster(fat, fsInfo, bootSector, file, dirEntryCopy);

  if (!success) {
    free(nodeDirEntryCopy);
    free(dirEntryCopy);
    return -1;
  }

  return saveDirEntryOnFile(fat, fsInfo, bootSector, file, listDirEntry,
                            nodeDirEntryCopy, dirEntryCopy);
}

// Função auxiliar para realizar tokenização do caminho de origem
static inline int formatFirstPath(char* source, char* destination,
                                  char* sourceName, char* pwd) {
  char** sourceFile;
  int amountOfTokenizedPath = tokenizePath(&sourceFile, source);

  if (amountOfTokenizedPath == -1) return 0;

  if (sourceFile == NULL) return 0;

  for (int i = 0; i < amountOfTokenizedPath - 1; i++) {
    strcat(destination, sourceFile[i]);
    if (i != amountOfTokenizedPath - 2) strcat(destination, "/");
  }

  if (amountOfTokenizedPath == 1 || strcmp(destination, ".") == 0)
    strcpy(destination, pwd);
  else if (amountOfTokenizedPath > 1 && strcmp(destination, "") == 0)
    strcpy(destination, "/");

  if (source[0] == '/')
    strcat(sourceName, sourceFile[amountOfTokenizedPath - 2]);
  else if (sourceFile[amountOfTokenizedPath - 1] != NULL)
    strcat(sourceName, sourceFile[amountOfTokenizedPath - 1]);
  else if (sourceFile[amountOfTokenizedPath - 2] != NULL)
    strcat(sourceName, sourceFile[amountOfTokenizedPath - 2]);
  else
    strcat(sourceName, sourceFile[amountOfTokenizedPath]);

  for (int i = 0; i < amountOfTokenizedPath; i++) free(sourceFile[i]);
  free(sourceFile);

  return 1;
}

// Função auxiliar para realizar tokenização do caminho de destino
static inline int formatSecondPath(char* destination, char* pwd, char* out) {
  if (strcmp(destination, ".") == 0) {
    strcpy(out, pwd);
    return 1;
  }

  char** destinationFile;
  int amountOfTokenizedPath = tokenizePath(&destinationFile, destination);

  if (amountOfTokenizedPath == -1) return 0;

  if (destinationFile == NULL) return 0;

  strcpy(out, destination);

  for (int i = 0; i < amountOfTokenizedPath; i++) free(destinationFile[i]);
  free(destinationFile);

  return 1;
}

// Função que realiza a cópia de arquivos dentro da imagem
static inline void cpImageToImage(
    StackDirectory* stackDirectory, FILE* file, FAT32_Clusters* fat,
    struct bootSectorStruct bootSector, struct FSInfoStruct* fsInfo,
    char* startPath, int successDestination, char* pathSource, char* sourceFile,
    char* source, char* destination, char* formattedDestination) {
  // Verifico se consigo alcançar diretório de origem do arquivo que desejo
  // mover, senão volto ao que estava e encerro
  // Realizo também a verificação se já estou no diretório origem
  int isStackInSourceDestination = strcmp(startPath, pathSource);
  if (!cdCommand(fat, bootSector, stackDirectory, file, pathSource) &&
      isStackInSourceDestination) {
    return;
  }

  ListDirEntry* listDirEntry = stackDirectory->currentDirectory->listDirEntry;

  int sourcePositionOfDirEntryInList = 0;
  NodeDirEntry* sourceNodeDirEntry = sourceNodeDirEntry =
      getNodeDirEntryByFilename(listDirEntry, sourceFile,
                                &sourcePositionOfDirEntryInList);

  // Verificando se existe um arquivo com o nome informado
  if (sourceNodeDirEntry == NULL) {
    printf("[cp] Não há nenhum arquivo com o nome informado: %s\n", source);
    return;
  }

  if (sourceNodeDirEntry->entry->shortEntry.DIR_Attr == DIRECTORY) {
    printf("[cp] Não é possivel realizar a cópia de um diretório: %s\n",
           source);
    return;
  }

  // Copiando para nao perder referencia
  NodeDirEntry nodeDirEntryCopy;
  DirEntry dirEntryCopy;
  memcpy(&dirEntryCopy, sourceNodeDirEntry->entry, sizeof(dirEntryCopy));
  nodeDirEntryCopy.entry = &dirEntryCopy;

  // Verifico se consigo alcançar diretório destino, senão volto ao que estava e
  // encerro
  cdCommand(fat, bootSector, stackDirectory, file, startPath);
  if (!cdCommand(fat, bootSector, stackDirectory, file, formattedDestination) ||
      !successDestination) {
    return;
  }

  listDirEntry = stackDirectory->currentDirectory->listDirEntry;

  // Verificando se existe um arquivo com o nome informado, caso exista volto
  // ao diretório origem e encerro
  if (getNodeDirEntryByFilename(listDirEntry, sourceFile, NULL) != NULL) {
    printf("[mv] Arquivo com nome (%s) já existente na pasta destino: %s\n",
           sourceFile, destination);

    return;
  }

  // Salvando arquivo em seu novo diretório
  int positionInserted = cpShortDirEntry(listDirEntry, file, fat, bootSector,
                                         fsInfo, &nodeDirEntryCopy);
}

// Função para realizar a cópia do arquivo da imagem para o computador
static inline void cpImageToComputer(StackDirectory* stackDirectory, FILE* file,
                                     FAT32_Clusters* fat,
                                     struct bootSectorStruct bootSector,
                                     struct FSInfoStruct* fsInfo,
                                     char* startPath, char* pathSource,
                                     char* sourceFile, char* source,
                                     char* destination) {
  // Verifico se consigo alcançar diretório de origem do arquivo que desejo
  // mover, senão volto ao que estava e encerro
  // Realizo também a verificação se já estou no diretório origem
  // Voltando para a root para usar caminho absoluto
  cdCommand(fat, bootSector, stackDirectory, file, "/");

  int isStackInSourceDestination = strcmp(startPath, pathSource);
  if (!cdCommand(fat, bootSector, stackDirectory, file, pathSource) &&
      isStackInSourceDestination) {
    return;
  }

  ListDirEntry* listDirEntry = stackDirectory->currentDirectory->listDirEntry;

  int sourcePositionOfDirEntryInList = 0;
  NodeDirEntry* sourceNodeDirEntry = sourceNodeDirEntry =
      getNodeDirEntryByFilename(listDirEntry, sourceFile,
                                &sourcePositionOfDirEntryInList);

  // Verificando se existe um arquivo com o nome informado
  if (sourceNodeDirEntry == NULL) {
    printf("[cp] Não há nenhum arquivo com o nome informado: %s\n", source);
    return;
  }

  if (sourceNodeDirEntry->entry->shortEntry.DIR_Attr == DIRECTORY) {
    printf("[cp] Não é possivel realizar a cópia de um diretório: %s\n",
           source);
    return;
  }

  // Realizo a tokenização do caminho destino
  int strDestination = strlen(destination) + strlen(sourceFile) + 3;
  char destinationFile[strDestination];
  memset(destinationFile, '\0', strDestination);
  strcat(destinationFile, destination);
  strcat(destinationFile, "/");
  strcat(destinationFile, sourceFile);

  FILE* fileComputer = fopen(destinationFile, "w");

  // Verifico se consigo alcançar diretório destino, senão encerro
  if (fileComputer == NULL) {
    printf("Arquivo não encontrado: %s\n", destinationFile);
    return;
  }

  // Copiando dados e guardando em buffer
  uint16_t lowCluster = sourceNodeDirEntry->entry->shortEntry.DIR_FstClusLO;
  uint16_t highCluster = sourceNodeDirEntry->entry->shortEntry.DIR_FstClusHI;
  uint32_t cluster = JOIN_LOW_HIGH_CLUSTER(lowCluster, highCluster);

  uint32_t sizeOfBytesInCluster =
      bootSector.BPB_SecPerClus * bootSector.BPB_BytsPerSec;
  uint32_t fileSize = sourceNodeDirEntry->entry->shortEntry.DIR_FileSize;
  int amountOfCluster = ceil(fileSize / (float)sizeOfBytesInCluster);
  amountOfCluster = amountOfCluster == 0 ? 1 : amountOfCluster;

  // Aloca todo o espaço do buffer
  uint8_t* buffer =
      (uint8_t*)calloc(amountOfCluster * sizeOfBytesInCluster, sizeof(uint8_t));

  memset(buffer, '\0', amountOfCluster * sizeOfBytesInCluster);

  // Alimenta o buffer por completo
  uint32_t auxCluster = cluster;
  int offset = 0;
  do {
    fseek(file, getFirstSectorOfCluster(bootSector, auxCluster), SEEK_SET);
    fread(buffer + offset, sizeof(uint8_t), sizeOfBytesInCluster, file);
    offset += sizeOfBytesInCluster;
    auxCluster = getNextCluster(fat, auxCluster);
  } while (auxCluster != -1);

  fwrite(buffer, 1, fileSize, fileComputer);

  fclose(fileComputer);
  free(buffer);
}

// Função para realizar a cópia do arquivo do computador para a imagem
static inline void cpComputerToImage(StackDirectory* stackDirectory, FILE* file,
                                     FAT32_Clusters* fat,
                                     struct bootSectorStruct bootSector,
                                     struct FSInfoStruct* fsInfo,
                                     char* startPath, int successDestination,
                                     char* source, char* destination,
                                     char* filename) {
  struct stat statbuf;
  stat(source, &statbuf);

  if ((statbuf.st_mode & S_IFMT) == S_IFDIR) {
    printf("Não é possivel realizar a cópia de um diretório: %s\n", source);
    return;
  }

  FILE* fileComputer = fopen(source, "r");

  // Verifico se consigo alcançar diretório origem, senão encerro
  if (fileComputer == NULL || !successDestination) {
    printf("Arquivo não encontrado: %s\n", source);
    return;
  }

  fseek(fileComputer, 0, SEEK_END);
  uint64_t sizeOfFile = ftell(fileComputer) + 1;
  fseek(fileComputer, 0, SEEK_SET);

  if (sizeOfFile == -1) {
    printf("Caminho de origem não é de um arquivo: %s\n", source);
    return;
  }

  uint8_t* buffer = (uint8_t*)calloc(sizeOfFile, sizeof(uint8_t));
  memset(buffer, '\0', sizeOfFile);
  fread(buffer, sizeOfFile, 1, fileComputer);
  buffer[sizeOfFile] = '\0';

  fclose(fileComputer);

  // Voltando para a root para usar caminho absoluto
  cdCommand(fat, bootSector, stackDirectory, file, "/");

  // Verifico se consigo chegar até o diretório destino
  int isStackInSourceDestination = strcmp(startPath, destination);
  if (!cdCommand(fat, bootSector, stackDirectory, file, destination) &&
      isStackInSourceDestination) {
    return;
  }

  ListDirEntry* listDirEntry = stackDirectory->currentDirectory->listDirEntry;

  int destinationPositionOfDirEntryInList = 0;
  NodeDirEntry* destinationNodeDirEntry = getNodeDirEntryByFilename(
      listDirEntry, filename, &destinationPositionOfDirEntryInList);

  // Verificando se existe um arquivo com o nome informado
  if (destinationNodeDirEntry != NULL) {
    printf("Já existe um arquivo com o nome informado: %s\n", filename);
    return;
  }

  touchCommand(fat, fsInfo, bootSector, file,
               stackDirectory->currentDirectory->listDirEntry, filename);

  destinationPositionOfDirEntryInList = 0;
  destinationNodeDirEntry = getNodeDirEntryByFilename(
      listDirEntry, filename, &destinationPositionOfDirEntryInList);

  // Salvando no arquivo
  uint16_t lowCluster =
      destinationNodeDirEntry->entry->shortEntry.DIR_FstClusLO;
  uint16_t highCluster =
      destinationNodeDirEntry->entry->shortEntry.DIR_FstClusHI;
  uint32_t cluster = JOIN_LOW_HIGH_CLUSTER(lowCluster, highCluster);

  uint32_t sizeOfBytesInCluster =
      bootSector.BPB_SecPerClus * bootSector.BPB_BytsPerSec;
  int amountOfCluster = ceil(sizeOfFile / (float)sizeOfBytesInCluster);
  amountOfCluster = amountOfCluster == 0 ? 1 : amountOfCluster;

  if (getFreeClusterCount(fsInfo) < amountOfCluster) {
    printf("Sem espaço de armazenamento.\n");
    free(buffer);
    rmCommand(listDirEntry, filename, file, fat, bootSector, fsInfo);
    return;
  }

  // Verificando se existe clusters vazios para realizar a cópia
  uint64_t totalFatEntries = getTotalNumberOfDataSectors(bootSector) + 1;
  uint32_t backupNextFreeFSInfo = fsInfo->FSI_Nxt_Free;
  int clusterVect[amountOfCluster];
  clusterVect[0] = cluster;
  for (int i = 1; i < amountOfCluster; i++) {
    uint32_t freeCluster = getOneFreeClusterInFat(fat, fsInfo, totalFatEntries);

    if (freeCluster == -1) {
      printf("Sem espaço de armazenamento livre.");
      fsInfo->FSI_Nxt_Free = backupNextFreeFSInfo;
      free(buffer);
      return;
    }

    clusterVect[i] = freeCluster;
    fsInfo->FSI_Nxt_Free = freeCluster + 1;
  }

  uint8_t* bufferToSave =
      (uint8_t*)calloc(amountOfCluster * sizeOfBytesInCluster, sizeof(uint8_t));
  memset(bufferToSave, '\0', amountOfCluster * sizeOfBytesInCluster);
  memcpy(bufferToSave, buffer, amountOfCluster * sizeOfBytesInCluster);
  free(buffer);

  // Atualizando dados da FAT e FsStruct
  int offset = 0;
  for (int i = 0; i < amountOfCluster; i++) {
    if (i != amountOfCluster - 1)
      fat[clusterVect[i]] = clusterVect[i + 1];
    else
      fat[clusterVect[i]] = END_OF_CHAIN;

    fsInfo->FSI_Nxt_Free = clusterVect[i];
    fsInfo->FSI_Free_Count--;

    saveDataClusterInFile(bootSector, file, clusterVect[i],
                          bufferToSave + offset, sizeOfBytesInCluster);

    offset += sizeOfBytesInCluster;
  }

  // Atualizand cluster do arquivo
  destinationNodeDirEntry->entry->shortEntry.DIR_FstClusHI =
      GET_HIGH_CLUSTER(clusterVect[0]);
  destinationNodeDirEntry->entry->shortEntry.DIR_FstClusLO =
      GET_LOW_CLUSTER(clusterVect[0]);
  destinationNodeDirEntry->entry->shortEntry.DIR_FileSize = sizeOfFile;

  saveFAT(bootSector, file, fat);
  saveFSInfo(fsInfo, bootSector, file);

  int maxDirInOneCluster = MAX_DIR_IN_CLUSTER(bootSector);
  int dirEntryPosition =
      destinationPositionOfDirEntryInList % maxDirInOneCluster;

  saveDirEntryOnPositionOfFile(bootSector, file, destinationNodeDirEntry->entry,
                               destinationNodeDirEntry->cluster,
                               dirEntryPosition);

  free(bufferToSave);
}

void cpCommand(StackDirectory* stackDirectory, FILE* file, FAT32_Clusters* fat,
               struct bootSectorStruct bootSector, struct FSInfoStruct* fsInfo,
               char* source, char* destination) {
  char sourceFlagCopyFromImage[5];
  char destinationFlagCopyFromImage[5];
  memset(sourceFlagCopyFromImage, '\0', sizeof(sourceFlagCopyFromImage));
  memset(destinationFlagCopyFromImage, '\0',
         sizeof(destinationFlagCopyFromImage));

  memcpy(sourceFlagCopyFromImage, source, 5);
  memcpy(destinationFlagCopyFromImage, destination, 5);
  sourceFlagCopyFromImage[4] = '\0';
  destinationFlagCopyFromImage[4] = '\0';

  char* startPath = pwdCommand(stackDirectory);
  int strSource = strlen(source);
  int strStartPath = strlen(startPath);
  int strDestination = strlen(destination);

  int maxLength = strSource > strStartPath ? strSource : strStartPath;
  maxLength = strDestination > maxLength ? strDestination : maxLength;
  maxLength++;  // por causa do \0

  if (strstr(sourceFlagCopyFromImage, "img") != NULL) {
    // Realizo formatação de string para obter primeiro
    char firstPath[maxLength];
    char firstPathFile[maxLength + 1];
    memset(firstPath, '\0', maxLength);
    memset(firstPathFile, '\0', maxLength + 1);
    int offset = source[0] == '/' ? 5 : 4;
    int successFirstPath =
        formatFirstPath(source + offset, firstPathFile, firstPath, startPath);

    cpImageToComputer(stackDirectory, file, fat, bootSector, fsInfo, startPath,
                      firstPathFile, firstPath, source, destination);
  } else if (strstr(destinationFlagCopyFromImage, "img") != NULL) {
    char firstPath[maxLength];
    char firstPathFile[maxLength + 1];
    memset(firstPath, '\0', maxLength);
    memset(firstPathFile, '\0', maxLength + 1);
    int offset = (source[0] == '/') ? 1 : 0;
    int successFirstPath =
        formatFirstPath(source + 1, firstPathFile, firstPath, startPath);

    // Realizo formatação de string para obter primeiro
    char secondPath[maxLength];
    memset(secondPath, '\0', maxLength);
    offset = 4;

    char realDestination[maxLength + 2];
    memset(realDestination, '\0', maxLength + 2);
    strcat(realDestination, ".");
    strcat(realDestination, destination + offset);
    int successSecondPath =
        formatSecondPath(realDestination, startPath, secondPath);

    cpComputerToImage(stackDirectory, file, fat, bootSector, fsInfo, startPath,
                      successSecondPath, source, realDestination, firstPath);
  } else {
    // Realizo formatação de string para obter primeiro
    char firstPath[maxLength];
    char firstPathFile[maxLength + 1];
    memset(firstPath, '\0', maxLength);
    memset(firstPathFile, '\0', maxLength + 1);
    int successFirstPath =
        formatFirstPath(source, firstPathFile, firstPath, startPath);

    // Realizo formatação de string para obter segundo caminho
    char formattedDestination[maxLength];
    memset(formattedDestination, '\0', maxLength);
    int successDestination =
        formatSecondPath(destination, startPath, formattedDestination);

    if (successFirstPath) {
      cpImageToImage(stackDirectory, file, fat, bootSector, fsInfo, startPath,
                     successDestination, firstPathFile, firstPath, source,
                     destination, formattedDestination);
    }
  }

  cdCommand(fat, bootSector, stackDirectory, file, "/");
  cdCommand(fat, bootSector, stackDirectory, file, startPath);

  free(startPath);
}
