#include "rmdir.h"

/**
 * @brief Função responsável por marcar os cluster ocupados pelo diretório como
 * livres
 * @param[in] fat Ponteiro FAT32_Clusters da tabela FAT que será salva.
 * @param[in] cluster Endereço do cluster onde está localizado o diretório.
 * @param[in] fsInfo Ponteiro da estrutura FSInfoStruct que será salva.
 */
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

/**
 * @brief Função responsável por remover as shortDirEntries
 * @param[in] nodeDirEntry estrutura NodeDirEntry do diretório a ser removido.
 * @param[in] file Ponteiro FILE para o arquivo.
 * @param[in] fat Ponteiro FAT32_Clusters da tabela FAT que será salva.
 * @param[in] bootSector Instância da struct bootSectorStruct.
 * @param[in] fsInfo Ponteiro da estrutura FSInfoStruct que será salva.
 * @param[in] positionOfDirEntryInList Posição na lista da DirEntry do diretório
 * a ser removido
 * @param[in] finalPositionOfList Última posição da lista da DirEntry
 * @param[in] positionOfDirEntryOnFile Posição da dirEntry no arquivo em relação
 * ao seu cluster
 * @return Valor inteiro, retorna 1 se a DirEntry removida é a última da lista
 */
static inline int rmShortDirEntry(NodeDirEntry* nodeDirEntry, FILE* file,
                                  FAT32_Clusters* fat,
                                  struct bootSectorStruct bootSector,
                                  struct FSInfoStruct* fsInfo,
                                  int positionOfDirEntryInList,
                                  int finalPositionOfList,
                                  int positionOfDirEntryOnFile) {
  // Obtém os clusters do diretório
  uint32_t firstCluster =
      JOIN_LOW_HIGH_CLUSTER(nodeDirEntry->entry->shortEntry.DIR_FstClusLO,
                            nodeDirEntry->entry->shortEntry.DIR_FstClusHI);

  // Se a posição não estiver livre, limpa o espaço na fat
  if (fat[firstCluster] != FREE_CLUSTER)
    freeFATClusters(fat, firstCluster, fsInfo);

  int removedLastPosition = 0;

  // Verifica se o arquivo movido estava no fim ou meio da lista
  if (positionOfDirEntryInList == finalPositionOfList) {
    nodeDirEntry->entry->shortEntry.DIR_Name[0] = DIR_ENTRY_LAST_AND_UNUSED;
    removedLastPosition = 1;
    // Caso a dirEntry for a última do cluster, liberar o cluster
    // Não liberar caso for o diretório raíz
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

  // Atualizando Informações na FAT e no fsInfo
  saveFAT(bootSector, file, fat);
  saveFSInfo(fsInfo, bootSector, file);

  return removedLastPosition;
}

/**
 * @brief Função que verifica se o diretório está vazio.
 * @param[in] listDirEntry Lista de diretórios que possui os arquivos do
 * diretório atual.
 * @return Valor inteiro que indica se o diretório está vazio, 0 para não e 1
 * para sim.
 */
int verifyEmptyDirectory(ListDirEntry* listDirEntry) {
  // Contador de dirEntry no diretório a ser removido
  int dirCount = 0;
  // Variável auxiliar para navegação na lista
  NodeDirEntry* aux = listDirEntry->head;

  // Laço de repetição para cada entrada da lista
  while (1) {
    // Caso a entrada da lista não esteja vazia, aumenta o contador do dirEntry
    if (aux->entry->shortEntry.DIR_Name[0] != DIR_ENTRY_LAST_AND_UNUSED ||
        aux->entry->shortEntry.DIR_Name[0] != DIR_ENTRY_UNUSED)
      dirCount++;

    // Termina o laço caso o fim da lista seja atingido
    if (aux->next == NULL) break;
    aux = aux->next;
  }

  // Caso o diretório tenha apenas as entradas para '.' e '..', significa que
  // o diretório está vazio e a função retorna 1, caso contrário retorna 0
  return dirCount == 2;
}

void rmdirCommand(StackDirectory* stackDirectory, char* entryName, FILE* file,
                  FAT32_Clusters* fat, struct bootSectorStruct bootSector,
                  struct FSInfoStruct* fsInfo) {
  // Verificando se o nome do arquivo é válido
  if (strlen(entryName) > 11) {
    printf("Nome proibido! Número máximo de caracteres para arquivo é 11.\n");
    return;
  }

  char bufferPath[strlen(entryName) + 1];
  memset(bufferPath, '\0', sizeof(bufferPath));
  for (int i = 0, j = 0; i < strlen(entryName); i++) {
    if (entryName[i] != '.' && entryName[i] != '/')
      bufferPath[j++] = entryName[i];
  }

  if (strcmp(bufferPath, "") == 0) {
    printf("Impossível realizar a remoção, argumento inválido: '%s'\n",
           entryName);
    return;
  }

  // Obtém o caminho absoluto do diretório atual
  char* startPath = pwdCommand(stackDirectory);

  int positionOfDirEntryInList;
  NodeDirEntry* nodeDirEntry =
      getNodeDirEntryByFilename(stackDirectory->currentDirectory->listDirEntry,
                                bufferPath, &positionOfDirEntryInList);

  // Verificando se existe um diretório com o nome informado
  if (nodeDirEntry == NULL) {
    printf("Não há nenhum diretório com o nome informado: %s\n", entryName);
    free(startPath);

    return;
  }

  // Verificando se a entrada realmente é um arquivo
  if (nodeDirEntry->entry->shortEntry.DIR_Attr != DIRECTORY) {
    printf("Impossível remover! A entrada não é um diretório.\n");
    free(startPath);

    return;
  }

  // muda o stackDirectory para o diretório a ser removido
  cdCommand(fat, bootSector, stackDirectory, file, entryName);

  // Verificando se o diretório está vazio
  if (!verifyEmptyDirectory(stackDirectory->currentDirectory->listDirEntry)) {
    printf("Impossível remover! O diretório não está vazio.\n");
    cdCommand(fat, bootSector, stackDirectory, file, "..");
    free(startPath);
    return;
  }

  int maxDirInOneCluster = MAX_DIR_IN_CLUSTER(bootSector);
  int removedLastPosition;

  // Apaga o diretório '..'
  positionOfDirEntryInList = 0;
  // Pega a posição do diretório na DirEntry
  nodeDirEntry =
      getNodeDirEntryByFilename(stackDirectory->currentDirectory->listDirEntry,
                                "..", &positionOfDirEntryInList);

  // Obtém a posição do cluster do diretório
  int positionOfFile = positionOfDirEntryInList % maxDirInOneCluster;

  // Obtém se a posição removida foi a útlima da listDirEntry ao remover o
  // diretório
  removedLastPosition = rmShortDirEntry(
      nodeDirEntry, file, fat, bootSector, fsInfo, positionOfDirEntryInList,
      stackDirectory->currentDirectory->listDirEntry->qtdDirEntries - 1, 1);

  saveDirEntryOnPositionOfFile(bootSector, file, nodeDirEntry->entry,
                               nodeDirEntry->cluster, positionOfFile);

  // Se o diretório removido for o último da lista, é removida a última DirEntry
  // da lista
  if (removedLastPosition)
    removeLastDirEntry(stackDirectory->currentDirectory->listDirEntry);

  // Apaga o diretório '.'
  positionOfDirEntryInList = 0;
  // Pega a posição do diretório na DirEntry
  nodeDirEntry =
      getNodeDirEntryByFilename(stackDirectory->currentDirectory->listDirEntry,
                                ".", &positionOfDirEntryInList);

  // Obtém se a posição removida foi a útlima da listDirEntry ao remover o
  // diretório
  removedLastPosition = rmShortDirEntry(
      nodeDirEntry, file, fat, bootSector, fsInfo, positionOfDirEntryInList,
      stackDirectory->currentDirectory->listDirEntry->qtdDirEntries - 1, 0);

  // Obtém a posição do cluster do diretório
  positionOfFile = positionOfDirEntryInList % maxDirInOneCluster;
  saveDirEntryOnPositionOfFile(bootSector, file, nodeDirEntry->entry,
                               nodeDirEntry->cluster, positionOfFile);

  // Se o diretório removido for o último da lista, é removida a última DirEntry
  // da lista
  if (removedLastPosition)
    removeLastDirEntry(stackDirectory->currentDirectory->listDirEntry);

  // Volta um diretório para apagar o diretório alvo
  cdCommand(fat, bootSector, stackDirectory, file, "..");

  // Apaga diretório alvo
  positionOfDirEntryInList = 0;
  // Pega a posição do diretório na DirEntry
  nodeDirEntry =
      getNodeDirEntryByFilename(stackDirectory->currentDirectory->listDirEntry,
                                bufferPath, &positionOfDirEntryInList);

  // Obtém a posição do cluster do diretório
  positionOfFile = positionOfDirEntryInList % maxDirInOneCluster;

  // Obtém se a posição removida foi a útlima da listDirEntry ao remover o
  // diretório
  removedLastPosition = rmShortDirEntry(
      nodeDirEntry, file, fat, bootSector, fsInfo, positionOfDirEntryInList,
      stackDirectory->currentDirectory->listDirEntry->qtdDirEntries - 1,
      positionOfFile);

  saveDirEntryOnPositionOfFile(bootSector, file, nodeDirEntry->entry,
                               nodeDirEntry->cluster, positionOfFile);

  // Se o diretório removido for o último da lista, é removida a última DirEntry
  // da lista
  if (removedLastPosition)
    removeLastDirEntry(stackDirectory->currentDirectory->listDirEntry);

  free(startPath);
}