#include "mv.h"

static inline uint32_t getNextCluster(FAT32_Clusters* fat, uint32_t cluster) {
  if (fat[cluster] == END_OF_CHAIN || fat[cluster] >= END_OF_CHAIN_2) return -1;
  return fat[cluster];
}

// Função responsável por realizar o comando mv para shortEntries
static inline int mvShortDirEntry(ListDirEntry* listDirEntry, FILE* file,
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

  return saveDirEntryOnFile(fat, fsInfo, bootSector, file, listDirEntry,
                            nodeDirEntryCopy, dirEntryCopy);
}

// Função auxiliar para realizar tokenização do caminho de origem
static inline int formatFirstPath(char* source, char* folderPathOfFirstFile,
                                  char* sourceName, char* pwd) {
  char** sourceFile;
  int amountOfTokenizedPath = tokenizePath(&sourceFile, source);

  if (amountOfTokenizedPath == -1) return 0;

  if (sourceFile == NULL) return 0;

  for (int i = 0; i < amountOfTokenizedPath - 1; i++) {
    strcat(folderPathOfFirstFile, sourceFile[i]);
    if (i != amountOfTokenizedPath - 2) strcat(folderPathOfFirstFile, "/");
  }

  if ((amountOfTokenizedPath == 1) || (strcmp(folderPathOfFirstFile, ".") == 0))
    strcpy(folderPathOfFirstFile, pwd);
  else if (amountOfTokenizedPath > 1 && strcmp(folderPathOfFirstFile, "") == 0)
    strcpy(folderPathOfFirstFile, "/");

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

// Função responsável por realizar o comando cp para shortEntries
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

// Move arquivo do computador para a imagem
static inline void mvComputerToImage(StackDirectory* stackDirectory, FILE* file,
                                     FAT32_Clusters* fat,
                                     struct bootSectorStruct bootSector,
                                     struct FSInfoStruct* fsInfo,
                                     char* startPath, int successDestination,
                                     char* source, char* destination,
                                     char* filename) {
  struct stat statbuf;
  stat(source, &statbuf);

  if ((statbuf.st_mode & S_IFMT) == S_IFDIR) {
    printf("Não é possivel mover um diretório: %s\n", source);
    return;
  }

  FILE* fileComputer = fopen(source, "r");

  if (fileComputer == NULL || !successDestination) {
    printf("Arquivo não encontrado: %s\n", source);
    return;
  }

  fseek(fileComputer, 0, SEEK_END);
  int sizeOfFile = ftell(fileComputer) + 1;
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

  // Apagando arquivo fora da imagem
  free(bufferToSave);
  remove(source);
}

// Move arquivo do imagem para o computador
static inline void mvImageToComputer(StackDirectory* stackDirectory, FILE* file,
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
    printf("[mv] Não há nenhum arquivo com o nome informado: %s\n", source);
    return;
  }

  if (sourceNodeDirEntry->entry->shortEntry.DIR_Attr == DIRECTORY) {
    printf("[mv] Não é possivel mover um diretório: %s\n", source);
    return;
  }

  // Verifico se consigo alcançar diretório destino, senão volto ao que estava e
  // encerro
  int strDestination = strlen(destination) + strlen(sourceFile) + 3;
  char destinationFile[strDestination];
  memset(destinationFile, '\0', strDestination);
  strcat(destinationFile, destination);
  strcat(destinationFile, "/");
  strcat(destinationFile, sourceFile);
  FILE* fileComputer = fopen(destinationFile, "w");

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

  rmCommand(listDirEntry, sourceFile, file, fat, bootSector, fsInfo);

  free(buffer);
}

// Move arquivo da imagem para a imagem
static inline void mvImageToImage(
    StackDirectory* stackDirectory, FILE* file, FAT32_Clusters* fat,
    struct bootSectorStruct bootSector, struct FSInfoStruct* fsInfo,
    char* startPath, int successDestination, char* pathSource, char* sourceFile,
    char* source, char* destination, char* formattedDestination) {
  // Verifico se consigo alcançar diretório de origem do arquivo que desejo
  // mover, senão volto ao que estava e encerro
  if (!cdCommand(fat, bootSector, stackDirectory, file, pathSource)) {
    return;
  }

  ListDirEntry* listDirEntry = stackDirectory->currentDirectory->listDirEntry;

  int sourcePositionOfDirEntryInList = 0;
  NodeDirEntry* sourceNodeDirEntry = getNodeDirEntryByFilename(
      listDirEntry, sourceFile, &sourcePositionOfDirEntryInList);

  // Verificando se existe um arquivo com o nome informado
  if (sourceNodeDirEntry == NULL) {
    printf("[mv] Não há nenhum arquivo com o nome informado: %s\n", source);
    return;
  }

  if (sourceNodeDirEntry->entry->shortEntry.DIR_Attr == DIRECTORY) {
    printf("Não é possivel mover um diretório: %s\n", source);
    return;
  }

  // Copiando para nao perder referencia
  NodeDirEntry nodeDirEntryCopy;
  DirEntry dirEntryCopy;
  memcpy(&dirEntryCopy, sourceNodeDirEntry->entry, sizeof(dirEntryCopy));
  nodeDirEntryCopy.entry = &dirEntryCopy;

  // Verifico se consigo alcançar diretório destino, senão volto ao que estava e
  // encerro
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
  int positionInserted = mvShortDirEntry(listDirEntry, file, fat, bootSector,
                                         fsInfo, &nodeDirEntryCopy);

  // Voltando para diretório da origem do arquivo e busco referencia para ele
  cdCommand(fat, bootSector, stackDirectory, file, "/");
  cdCommand(fat, bootSector, stackDirectory, file, pathSource);
  listDirEntry = stackDirectory->currentDirectory->listDirEntry;
  sourcePositionOfDirEntryInList = 0;
  sourceNodeDirEntry = getNodeDirEntryByFilename(
      listDirEntry, sourceFile, &sourcePositionOfDirEntryInList);

  // Verifico se o arquivo movido estava no fim ou meio da lista
  if (sourcePositionOfDirEntryInList == listDirEntry->qtdDirEntries - 1)
    sourceNodeDirEntry->entry->shortEntry.DIR_Name[0] =
        DIR_ENTRY_LAST_AND_UNUSED;
  else
    sourceNodeDirEntry->entry->shortEntry.DIR_Name[0] = DIR_ENTRY_UNUSED;

  int maxDirInOneCluster = MAX_DIR_IN_CLUSTER(bootSector);
  int dirEntryPosition = sourcePositionOfDirEntryInList % maxDirInOneCluster;

  // Caso a dirEntry for a última do cluster (e o cluster for o último), liberar
  // o cluster Não liberar caso for o diretório root
  if (dirEntryPosition == 0 &&
      sourceNodeDirEntry->cluster != getClusterOfRootDir(bootSector) &&
      fat[sourceNodeDirEntry->cluster] == END_OF_CHAIN) {
    fat[sourceNodeDirEntry->cluster] = FREE_CLUSTER;
    fat[sourceNodeDirEntry->previous->cluster] = END_OF_CHAIN;
    fsInfo->FSI_Free_Count++;

    // Atualizando Informações na FAT e no fsinfo
    saveFAT(bootSector, file, fat);
    saveFSInfo(fsInfo, bootSector, file);
  }

  // Salvo DirEntry na Imagem
  saveDirEntryOnPositionOfFile(bootSector, file, sourceNodeDirEntry->entry,
                               sourceNodeDirEntry->cluster, dirEntryPosition);
}

void mvCommand(StackDirectory* stackDirectory, FILE* file, FAT32_Clusters* fat,
               struct bootSectorStruct bootSector, struct FSInfoStruct* fsInfo,
               char* source, char* destination) {
  // Verificando préfixo de source e destination para saber qual das três
  // funções executar
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
    // Realizo formatação de string
    char firstPath[maxLength];
    char firstPathFile[maxLength + 1];
    memset(firstPath, '\0', maxLength);
    memset(firstPathFile, '\0', maxLength + 1);
    int offset = source[0] == '/' ? 5 : 4;
    int successFirstPath =
        formatFirstPath(source + offset, firstPathFile, firstPath, startPath);

    mvImageToComputer(stackDirectory, file, fat, bootSector, fsInfo, startPath,
                      firstPathFile, firstPath, source, destination);
  } else if (strstr(destinationFlagCopyFromImage, "img") != NULL) {
    // Realizo formatação de string
    char firstPath[maxLength];
    char firstPathFile[maxLength + 1];
    memset(firstPath, '\0', maxLength);
    memset(firstPathFile, '\0', maxLength + 1);
    int offset = (source[0] == '/') ? 1 : 0;
    int successFirstPath =
        formatFirstPath(source + 1, firstPathFile, firstPath, startPath);

    char secondPath[maxLength];
    memset(secondPath, '\0', maxLength);
    offset = 4;

    char realDestination[maxLength + 2];
    memset(realDestination, '\0', maxLength + 2);
    strcat(realDestination, ".");
    strcat(realDestination, destination + offset);
    int successSecondPath =
        formatSecondPath(realDestination, startPath, secondPath);

    mvComputerToImage(stackDirectory, file, fat, bootSector, fsInfo, startPath,
                      successSecondPath, source, realDestination, firstPath);
  } else {
    // Realizo formatação de string
    char firstFilename[maxLength];
    char firstPath[maxLength + 1];
    memset(firstFilename, '\0', maxLength);
    memset(firstPath, '\0', maxLength + 1);
    int successFirstPath =
        formatFirstPath(source, firstPath, firstFilename, startPath);

    char formattedDestination[maxLength];
    memset(formattedDestination, '\0', maxLength);
    int successDestination =
        formatSecondPath(destination, startPath, formattedDestination);

    if (successFirstPath) {
      mvImageToImage(stackDirectory, file, fat, bootSector, fsInfo, startPath,
                     successDestination, firstPath, firstFilename, source,
                     destination, formattedDestination);
    }
  }

  // Volto para o diretório origem
  cdCommand(fat, bootSector, stackDirectory, file, "/");
  cdCommand(fat, bootSector, stackDirectory, file, startPath);
  free(startPath);
}
