#include "info.h"

void infoCommand(FILE *file, struct bootSectorStruct bootSector,
                 FAT32_Clusters *fat, struct FSInfoStruct *fsInfo) {
  // Obtendo informações para serem exibidas
  uint32_t fatSize = bootSector.BPB_FATSz32 * bootSector.BPB_BytsPerSec;
  uint32_t bytesPerCluster =
      bootSector.BPB_SecPerClus * bootSector.BPB_BytsPerSec;

  uint64_t totalDataCluster = getTotalNumberOfDataSectors(bootSector);

  uint64_t countOffreeCluster = getFreeClusterCount(fsInfo);

  uint64_t sizeOfImage = getSizeOfImage(file);
  double sizeOfImageInMB = sizeOfImage / (double)(1024 * 1024);
  uint64_t dataSize = getDataSize(sizeOfImage, bootSector);
  double dataSizeInMB = dataSize / (double)(1024 * 1024);

  double freeSpaceInMB =
      countOffreeCluster * bytesPerCluster / (double)(1024 * 1024);
  double usedSpaceInMB = dataSizeInMB - freeSpaceInMB;

  // Exibindo informações
  printf("Informações do Sistema de Arquivos FAT32\n\n");
  printf("Tipo:");
  for (int i = 0; i < 8; i++) printf("%c", bootSector.BS_FilSysType[i]);

  printf("\nOEM: %s\n", bootSector.BS_OEMName);
  printf("Total de Setores: %d\n", bootSector.BPB_TotSec32);
  printf("Total de Clusters de Dados: %ld\n", totalDataCluster);

  printf("Tamanho de Dados: %ld (%.2lfM)\n", dataSize, dataSizeInMB);
  printf("Tamanho da Imagem: %ld (%.2lfM)\n", sizeOfImage, sizeOfImageInMB);

  printf("Bytes por Setores: %d\n", bootSector.BPB_BytsPerSec);
  printf("Setores por Clusters: %d\n", bootSector.BPB_SecPerClus);
  printf("Bytes por Clusters: %d\n", bytesPerCluster);
  printf("Setores Reservados: %d\n", bootSector.BPB_RsvdSecCnt);
  printf("Setores por FAT: %d\n", bootSector.BPB_FATSz32);
  printf("Tamanho da FAT: %d (%d KiB)\n", fatSize, fatSize / 1024);
  printf("Endereço inicial da FAT1: %016lX\n", getFat1Address(bootSector));
  printf("Endereço inicial da FAT2: %016lX\n", getFat2Address(bootSector));
  printf("Endereço de inicio dos Dados: %016lX\n",
         getFirstSectorOfCluster(bootSector, getClusterOfRootDir(bootSector)));
  printf("Cluster do Diretório Raiz (/): %d\n", bootSector.BPB_RootClus);
  printf("\n");

  double percentage = (100 * countOffreeCluster) / totalDataCluster;
  printf("Rótulo do Disco\n");
  printf("Clusters Livres: %ld/%ld (%.4lf%%)\n", countOffreeCluster,
         totalDataCluster, percentage);

  printf("Espaço Livre: %ld (%.4fM)\n", countOffreeCluster * bytesPerCluster,
         freeSpaceInMB);

  printf("Espaço Utilizado: %ld (%.4fM)\n",
         (totalDataCluster * bytesPerCluster) -
             (countOffreeCluster * bytesPerCluster),
         usedSpaceInMB);
}
