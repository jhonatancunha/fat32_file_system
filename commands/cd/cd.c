#include "cd.h"

/**
 * @brief Desempilha os diretórios da stackDirectory até chegar no diretório
 * raíz.
 * @param[in] stackDirectory Pilha de diretórios de um dado diretório.
 */
static inline void popToRootDir(StackDirectory* stackDirectory) {
  // Se houver apenas um diretório, significa que ele já está no diretório raíz.
  if (stackDirectory->qtdDirectory == 1) return;

  // Desempilha os diretórios até chegar no diretório raíz.
  while (strcmp(stackDirectory->currentDirectory->name, "/") != 0) {
    pop(stackDirectory);
  }
}

/**
 * @brief Validação da entrada do comando cd e mudança do diretório atual na
 * stackDirectory.
 * @param[in] fat Ponteiro FAT32_Clusters da tabela FAT que será salva.
 * @param[in] bootSector Instância da struct bootSectorStruct.
 * @param[in] stackDirectory Pilha de diretórios de um dado diretório.
 * @param[in] file Ponteiro FILE para o arquivo.
 * @param[in] path Diretório destino a ser acessado.
 * @param[in] pathTokenized String de diretórios tokenizada.
 * @return Valor inteiro para o status da operação, -1 entrada inválida e 1 para
 * entrada válida.
 */
static inline int cdShortEntryCommand(FAT32_Clusters* fat,
                                      struct bootSectorStruct bootSector,
                                      StackDirectory* stackDirectory,
                                      FILE* file, char* path,
                                      char** pathTokenized) {
  // Varredura por todo o diretório
  int i = 0;
  while (pathTokenized[i] != 0x0) {
    // Atingiu o diretório atual e vai para o próximo
    if (strcmp(pathTokenized[i], ".") == 0) {
      i++;
      continue;
    }

    // Atingiu o diretório anterior
    if (strcmp(pathTokenized[i], "..") == 0) {
      // Caso o diretório raíz seja o atual, ele não possui diretórios
      // anteriores
      if (stackDirectory->qtdDirectory == 1) {
        printf("Não existe diretório anterior!\n");
        break;
      }
      // Desempilha o diretório atingido e vai para o próximo
      pop(stackDirectory);
      i++;
      continue;
    }

    // Atribui ao nodeDirEntry o diretório do índice i
    NodeDirEntry* nodeDirEntry = getNodeDirEntryByFilename(
        stackDirectory->currentDirectory->listDirEntry, pathTokenized[i], NULL);

    // Verifica se o diretório é válido
    if (nodeDirEntry == 0x0) {
      // printf("[cd] ");
      printf("Caminho inválido: %s\n", path);
      return -1;
    }

    // Verifica se a entrada é um diretório
    if (nodeDirEntry->entry->shortEntry.DIR_Attr == ARCHIVE) {
      // printf("[cd] ");
      printf("Caminho não é um diretório: %s\n", pathTokenized[i]);
      return -1;
    }

    // Atribui as posições do cluster do diretório
    uint16_t lowCluster = nodeDirEntry->entry->shortEntry.DIR_FstClusLO;
    uint16_t highCluster = nodeDirEntry->entry->shortEntry.DIR_FstClusHI;
    uint32_t cluster = JOIN_LOW_HIGH_CLUSTER(lowCluster, highCluster);

    // Lê o diretorio
    readAllClusterOfDirectoryAndPushIntoStack(
        fat, bootSector, file, stackDirectory, cluster, pathTokenized[i]);

    // Incrementa o índice para o próximo diretório, se houver
    i++;
  }

  return 1;
}

int cdCommand(FAT32_Clusters* fat, struct bootSectorStruct bootSector,
              StackDirectory* stackDirectory, FILE* file, char* path) {
  // Entrada vazia é inválida
  if (strlen(path) == 0) return 0;

  // Obtêm o caminho absoluto do diretório atual
  char* currentPath = pwdCommand(stackDirectory);
  // Se o caminho a ser atingido é o mesmo do caminho atual,
  // a função é apenas encerrada retornando sucesso
  if (strcmp(path, currentPath) == 0) {
    free(currentPath);
    return 1;
  }

  // Caso o destino for o diretório raíz, atribui o diretório raiz à
  // stackDirectory
  if (strlen(path) == 1 && path[0] == '/') {
    popToRootDir(stackDirectory);
    return 1;
  }

  // Tokeniza o path, separando os diretórios
  char** pathTokenized;
  int countTokenized = tokenizePath(&pathTokenized, path);

  // Retorna erro caso caminho seja nulo
  if (countTokenized == -1) return 0;

  int i = 0;

  // Obtém o caminho absoluto atual
  char* starterPath = pwdCommand(stackDirectory);
  int isStackInSourceDestination = strcmp(starterPath, path);

  // Caso já esteja no diretório destino, a função é encerrada retornando
  // sucesso
  if (!isStackInSourceDestination) {
    for (i = 0; i < countTokenized; i++) free(pathTokenized[i]);
    free(pathTokenized);
    free(starterPath);
    return 1;
  }

  // Verifica todas as entradas, e caso algum diretório seja inválido, retorna
  // erro
  while (pathTokenized[i] != 0x0) {
    if (strlen(pathTokenized[i]) > 11) {
      printf(
          "[cd] Nome proibido! Número máximo de caracteres para arquivo é 11 : "
          "%s\n",
          pathTokenized[i]);

      for (i = 0; i < countTokenized; i++) free(pathTokenized[i]);
      free(pathTokenized);
      free(starterPath);
      return 0;
    }
    i++;
  }

  // Verifica a validade do caminho inserido e realiza as operações de
  // mudança de diretório na stackDirectory
  int cdSuccess = cdShortEntryCommand(fat, bootSector, stackDirectory, file,
                                      path, pathTokenized);

  // Limpa a memória HEAP para cada caminho inserido
  for (i = 0; i < countTokenized; i++) free(pathTokenized[i]);
  free(pathTokenized);

  // Caso a operação tenha falhado, volta o stackDirectory para o diretório
  // atual
  if (cdSuccess == -1) {
    free(currentPath);
    currentPath = pwdCommand(stackDirectory);
    if (strcmp(currentPath, starterPath) != 0) {
      popToRootDir(stackDirectory);
      cdCommand(fat, bootSector, stackDirectory, file, starterPath);
    }
  }

  free(starterPath);
  free(currentPath);

  if (cdSuccess == -1) return 0;
  return 1;
}