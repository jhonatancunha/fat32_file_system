#include "rename.h"

void renameCommand(struct bootSectorStruct bootSector, FILE *file,
                   ListDirEntry *listDirEntry, char *oldName, char *newName) {
  // Verifica tamanho do novo nome
  if (strlen(newName) > 11) {
    printf("Nome proibido. Número máximo de caracteres para arquivo é 11.\n");
    return;
  }

  // Verificando se existe arquivo que desejo renomear
  int positionOfFile = 0;
  NodeDirEntry *nodeEntry =
      getNodeDirEntryByFilename(listDirEntry, oldName, &positionOfFile);

  if (nodeEntry == NULL || nodeEntry->entry == NULL) {
    printf("[rename] Arquivo inexistente: %s\n", oldName);
    return;
  }

  // Verificando se já existe algum outro arquivo com o novo nome que deseja-se
  // utilizar
  if (getNodeDirEntryByFilename(listDirEntry, newName, NULL) != NULL) {
    printf("Nome: %s já utilizado por outro arquivo.\n", newName);
    return;
  }

  DirEntry *entry = nodeEntry->entry;

  // Formato o novo nome para salvar no arquivo
  char *fatDirEntryName = fatWayShortDirName(newName);

  if (!validateDirName(fatDirEntryName)) {
    printf("Nome inválido, não utilize caracteres especiais.\n");
    return;
  }

  memcpy(entry->shortEntry.DIR_Name, fatDirEntryName, 11);

  free(fatDirEntryName);
  saveDirEntryOnPositionOfFile(bootSector, file, entry, nodeEntry->cluster,
                               positionOfFile);
}
