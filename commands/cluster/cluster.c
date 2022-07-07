#include "cluster.h"

void cluster(struct bootSectorStruct bootSector, FILE *file, uint32_t number) {
  uint32_t sizeOfBytesInCluster =
      bootSector.BPB_SecPerClus * bootSector.BPB_BytsPerSec;

  uint8_t buffer[sizeOfBytesInCluster];
  memset(buffer, '\0', sizeOfBytesInCluster);

  // Posicionando ponteiro no cluster informado
  fseek(file, getFirstSectorOfCluster(bootSector, number), SEEK_SET);
  fread(buffer, sizeof(uint8_t), sizeOfBytesInCluster, file);

  int maxBytesInOneLine = 16;
  int amountOfLines = sizeOfBytesInCluster / maxBytesInOneLine;
  int startByte;
  int endByte;

  // Descobrindo início e fim do cluster
  for (int i = 0; i < amountOfLines; i++) {
    startByte = i * maxBytesInOneLine;
    endByte = startByte + maxBytesInOneLine;

    // Exibe as informações em hexadecimal
    for (int j = startByte; j < endByte; j++) {
      printf("%02hhX ", buffer[j]);
    }

    printf("\t");

    // Exibe as informações com base na tabela ASCII
    // https://pt.m.wikipedia.org/wiki/Ficheiro:ASCII-Table.svg
    for (int j = startByte; j < endByte; j++) {
      if (buffer[j] <= 31 || buffer[j] == 127)
        printf(".");
      else
        printf("%c", buffer[j]);
    }

    printf("\n");
  }

  printf("\n");
}
