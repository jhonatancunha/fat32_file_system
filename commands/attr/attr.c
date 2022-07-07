#include "attr.h"

// Função que implementa o attr com shortEntries
static inline void attrShortEntry(struct ShortDirStruct shortEntry) {
  // Formatando nome do arquivo para exibição
  char* prettyDirName =
      prettyShortDirName(shortEntry.DIR_Name, shortEntry.DIR_Attr);

  // Transformando tamanho do arquivo para KibiBytes
  double fileSizeInKibibytes =
      TRANSFORM_BYTES_IN_KIBIBYTES(shortEntry.DIR_FileSize);

  // Exibição das informações
  printf("Nome: %11s %s \n", " ", prettyDirName);
  printf("Tipo: %11s %s\n", " ",
         shortEntry.DIR_Attr == DIRECTORY ? "Diretório" : "Arquivo");
  printf("Tamanho: %8s %.5f KiB\n", " ", fileSizeInKibibytes);
  printf("Criado em: %7s", " ");
  getDateFromFile(shortEntry.DIR_CrtDate);
  printf(" às ");
  getTimestampFromFile(shortEntry.DIR_CrtTime);
  printf("\n");

  printf("Modificado em: %3s", " ");
  getDateFromFile(shortEntry.DIR_WrtDate);
  printf(" às ");
  getTimestampFromFile(shortEntry.DIR_WrtTime);
  printf("\n");

  printf("Último acesso em: ");
  getDateFromFile(shortEntry.DIR_LstAccDate);
  printf("\n");

  printf("Cluster: %8s %d\n", " ",
         JOIN_LOW_HIGH_CLUSTER(shortEntry.DIR_FstClusLO,
                               shortEntry.DIR_FstClusHI));
}

void attrCommand(ListDirEntry* listDirEntry, char* entryName) {
  // Verificando se o nome é válido
  if (strlen(entryName) > 11) {
    printf("Nome proibido. Número máximo de caracteres para arquivo é 11.\n");
    return;
  }

  NodeDirEntry* nodeDirEntry =
      getNodeDirEntryByFilename(listDirEntry, entryName, NULL);

  // Verificando se há um DirEntry com o nome especificado
  if (nodeDirEntry == NULL) {
    printf("Não há nenhum arquivo ou diretório com o nome informado.\n");
    return;
  }

  attrShortEntry(nodeDirEntry->entry->shortEntry);
}
