#include "ls.h"

void lsCommand(ListDirEntry* listDirEntry) {
  struct NodeDirEntry* dirEntry = listDirEntry->head;
  DirEntry* entry = dirEntry->entry;

  // Se não houver a DirEntry informada, apenas termina a execução do comando
  if (dirEntry == NULL) return;

  printf("\n");

  while (1) {
    // Verificando se a entrada é válida
    if (VALIDATE_TYPE_ENTRY(entry)) {
      char type = entry->shortEntry.DIR_Attr == ARCHIVE ? 'f' : 'd';

      // Exibindo conteúdo da entrada apenas quando é um diretório
      switch (GET_TYPE_OF_ENTRY(entry)) {
        case 0x00:
        case ATTR_DIRECTORY:

          if (entry->shortEntry.DIR_Name[0] != DIR_ENTRY_LAST_AND_UNUSED) {
            printf("%c\t", type);
            getDateFromFile(entry->shortEntry.DIR_CrtDate);
            printf("\t");
            getTimestampFromFile(entry->shortEntry.DIR_CrtTime);

            char* prettyName = prettyShortDirName(entry->shortEntry.DIR_Name,
                                                  entry->shortEntry.DIR_Attr);

            if (entry->shortEntry.DIR_Attr == DIRECTORY)
              printf(BLUE("\t%s"), prettyName);
            else
              printf("\t%s", prettyName);

            printf("\n");

            free(prettyName);
          }

          break;
        case ATTR_VOLUME_ID:
          // printf("volume\n");
          break;
        default:
          // printf("inválido\n");
          break;
      }
    }

    // Se chegou no fim do conteúdo do diretório, interrompe o loop
    if (dirEntry->next == NULL) break;

    dirEntry = dirEntry->next;
    entry = dirEntry->entry;
  }
}
