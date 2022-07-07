#include "stackDirectory.h"

StackDirectory* createStackDirectory() {
  StackDirectory* stack = (StackDirectory*)malloc(sizeof(StackDirectory));
  stack->currentDirectory = NULL;
  stack->rootDirectory = NULL;
  stack->qtdDirectory = 0;

  return stack;
}

void push(StackDirectory* stack, NodeStackDirectory* node, char* name) {
  // Verificando a existência da stack e do nó
  if (stack == NULL) return;
  if (node == NULL) return;

  strcpy(node->name, name);

  node->next = NULL;
  // Caso a stack esteja vazia, apenas adiciona o node, fazendo current e root
  // directory apontar para o node. Caso contrário, adiciona o node e atualiza
  // as outras variáveis.
  if (stack->qtdDirectory == 0) {
    node->previous = NULL;
    stack->currentDirectory = node;
    stack->rootDirectory = node;
  } else {
    node->previous = stack->currentDirectory;
    stack->currentDirectory->next = node;
    stack->currentDirectory = node;
  }

  stack->qtdDirectory++;
}

void pop(StackDirectory* stack) {
  // Verificando se há uma stack ou se a stack está vazia
  if (stack == NULL) return;
  if (stack->qtdDirectory == 0) return;

  NodeStackDirectory* node = stack->currentDirectory;

  // Faz o diretório atual apontar para o diretório anterior se este útlimo não
  // for nulo
  if (stack->currentDirectory->previous != NULL)
    stack->currentDirectory = stack->currentDirectory->previous;

  destroyListDirEntry(node->listDirEntry);
  free(node);

  stack->qtdDirectory--;
}

void destroyStack(StackDirectory* stack) {
  if (stack == NULL) return;

  // Esvazia toda a stack antes de liberar o espaço alocado para a stack
  while (stack->qtdDirectory != 0) {
    pop(stack);
  }

  free(stack);
}

void readAllClusterOfDirectoryAndPushIntoStack(
    FAT32_Clusters* fat, struct bootSectorStruct bootSector, FILE* file,
    StackDirectory* stack, uint32_t cluster, char* name) {
  ListDirEntry* listDirEntry = createListDirEntry();

  uint32_t currentCluster = cluster;

  while (1) {
    // Posicionando ponteiro no cluster informado
    fseek(file, getFirstSectorOfCluster(bootSector, currentCluster), SEEK_SET);
    readEntireClusterAndSaveInList(file, listDirEntry, currentCluster);
    listDirEntry->amountOfCluster++;  // Atualizando quantidade de clusters

    // Caso tenha chegado no fim, para o loop
    currentCluster = fat[currentCluster];
    if (currentCluster == END_OF_CHAIN || currentCluster >= END_OF_CHAIN_2)
      break;
  }

  NodeStackDirectory* node =
      (NodeStackDirectory*)malloc(sizeof(NodeStackDirectory));

  node->listDirEntry = listDirEntry;
  push(stack, node, name);
}
