#include "mkdir.h"

// Implementação do comando mkdir para ShortEntries retorna o cluster usado ou
// -1 caso não haja espaço
static inline uint32_t mkdirShortEntry(FAT32_Clusters *fat,
                                       struct FSInfoStruct *fsInfo,
                                       struct bootSectorStruct bootSector,
                                       FILE *file, ListDirEntry *listDirEntry,
                                       char *fatShortDirEntryName,
                                       uint32_t *clusterOfFatherFolder) {
  uint64_t totalFatEntries = getTotalNumberOfDataSectors(bootSector) + 1;
  uint32_t freeCluster;

  // Verificando se é necessário obter um novo cluster ou se é possível usar o
  // cluster do fatherFolder
  if (clusterOfFatherFolder == NULL)
    freeCluster = getOneFreeClusterInFat(fat, fsInfo, totalFatEntries);
  else
    freeCluster = *clusterOfFatherFolder;

  // Verificando se há ou não espaço disponível
  if (freeCluster == -1) {
    printf("Não há espaço disponível para criar o diretório.\n");
    return -1;
  }

  uint16_t currentTime = getCurrentTime();
  uint16_t currentDate = getCurrentDate();
  uint16_t lowCluster = GET_LOW_CLUSTER(freeCluster);
  uint16_t highCluster = GET_HIGH_CLUSTER(freeCluster);

  NodeDirEntry *nodeListDirEntry =
      (NodeDirEntry *)calloc(1, sizeof(NodeDirEntry));

  DirEntry *dirEntry = (DirEntry *)calloc(1, sizeof(DirEntry));

  // Adicionando informações necessárias para o diretório
  strcpy(dirEntry->shortEntry.DIR_Name, fatShortDirEntryName);
  dirEntry->shortEntry.DIR_CrtTime = currentTime;
  dirEntry->shortEntry.DIR_WrtTime = currentTime;
  dirEntry->shortEntry.DIR_LstAccDate = currentDate;
  dirEntry->shortEntry.DIR_CrtDate = currentDate;
  dirEntry->shortEntry.DIR_WrtDate = currentDate;
  dirEntry->shortEntry.DIR_Attr = DIRECTORY;
  dirEntry->shortEntry.DIR_FstClusLO = lowCluster;
  dirEntry->shortEntry.DIR_FstClusHI = highCluster;

  nodeListDirEntry->entry = dirEntry;
  nodeListDirEntry->next = NULL;
  nodeListDirEntry->cluster = freeCluster;

  // Se realmente foi obtido um novo cluster para a criação dos dados, atualiza
  // a quantidade de clusters livres no fsInfo e atualiza a FAT
  if (clusterOfFatherFolder == NULL) {
    fat[freeCluster] = END_OF_CHAIN;
    fsInfo->FSI_Nxt_Free = freeCluster;
    fsInfo->FSI_Free_Count--;
  }

  // Salvando alterações na imagem
  saveDirEntryOnFile(fat, fsInfo, bootSector, file, listDirEntry,
                     nodeListDirEntry, dirEntry);

  return freeCluster;
}

void mkdirCommand(FAT32_Clusters *fat, struct FSInfoStruct *fsInfo,
                  struct bootSectorStruct bootSector, FILE *file,
                  ListDirEntry *listDirEntry, StackDirectory *stack,
                  char *name) {
  // Verificando se o nome informado é válido
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

  // Verificando se o nome possui caracteres especiais
  if (!validateDirName(fatDirEntryName)) {
    printf("Nome inválido, não utilize caracteres especiais.\n");
    return;
  }
  int zero = 0;

  // Obtendo cluster da área de dados do diretório
  uint32_t fatherFolderCluster = mkdirShortEntry(
      fat, fsInfo, bootSector, file, listDirEntry, fatDirEntryName, NULL);

  // Verificando se existe um cluster para o diretório na área de dados
  if (fatherFolderCluster == -1) {
    printf("erro\n");
    return;
  }

  cdCommand(fat, bootSector, stack, file, name);
  listDirEntry = stack->currentDirectory->listDirEntry;

  char fatDotDirEntryName[11];
  char fatDotDotDirEntryName[11];
  memset(fatDotDirEntryName, ' ', sizeof(fatDotDirEntryName));
  memset(fatDotDotDirEntryName, ' ', sizeof(fatDotDotDirEntryName));

  fatDotDirEntryName[0] = '.';
  fatDotDotDirEntryName[0] = '.';
  fatDotDotDirEntryName[1] = '.';

  // Zerar cluster da pasta
  // Aloca todo o espaço do buffer
  uint64_t sizeOfCluster =
      bootSector.BPB_BytsPerSec * bootSector.BPB_SecPerClus;
  uint8_t buffer[sizeOfCluster + 1];
  memset(buffer, '\0', sizeOfCluster + 1);
  fseek(file, getFirstSectorOfCluster(bootSector, fatherFolderCluster),
        SEEK_SET);

  fwrite(buffer, 1, sizeOfCluster, file);

  while (listDirEntry->qtdDirEntries != 0) removeLastDirEntry(listDirEntry);
  listDirEntry->amountOfCluster = 1;  // para caso zerar na remoção acima

  // Criando os diretórios . e .. no diretório
  mkdirShortEntry(fat, fsInfo, bootSector, file, listDirEntry,
                  fatDotDirEntryName, &fatherFolderCluster);
  mkdirShortEntry(fat, fsInfo, bootSector, file, listDirEntry,
                  fatDotDotDirEntryName, &zero);

  cdCommand(fat, bootSector, stack, file, "..");
  free(fatDirEntryName);
}
