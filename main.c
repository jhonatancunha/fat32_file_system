#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./utils/utils.h"
#include "FAT/fat.h"
#include "ListDirEntry/listDirEntry.h"
#include "StackDirectory/stackDirectory.h"
#include "commands/attr/attr.h"
#include "commands/cd/cd.h"
#include "commands/cluster/cluster.h"
#include "commands/cp/cp.h"
#include "commands/help/help.h"
#include "commands/info/info.h"
#include "commands/ls/ls.h"
#include "commands/mkdir/mkdir.h"
#include "commands/mv/mv.h"
#include "commands/pwd/pwd.h"
#include "commands/rename/rename.h"
#include "commands/rm/rm.h"
#include "commands/rmdir/rmdir.h"
#include "commands/touch/touch.h"

int tokenizeArrayOfCommands(char ***commands, char *arg,
                            int *amountOfCommands) {
  int size = strlen(arg);

  if (size < 1) return -1;

  char argsTokenized[size];
  memset(argsTokenized, '\0', size);
  strcpy(argsTokenized, arg);
  argsTokenized[size - 1] = '\0';

  // Contando quantas strings devemos armazenar
  int amountOfStrings = 1;
  char *token = strchr(argsTokenized, ' ');
  while (token != NULL) {
    amountOfStrings++;
    token++;
    token = strchr(token, ' ');
  }

  char **tokenizedCommands = (char **)calloc(amountOfStrings, sizeof(char *));
  int i = 0;
  token = strtok(argsTokenized, " ");
  char *buffer;

  while (token != 0x0) {
    buffer = (char *)calloc(strlen(token) + 1, sizeof(char));
    strcpy(buffer, token);
    tokenizedCommands[i++] = buffer;
    token = strtok(NULL, " ");
  }

  *commands = tokenizedCommands;
  return amountOfStrings;
}

void destroyArrayOfCommands(char **commands, int amountOfCommands) {
  for (int i = 0; i < amountOfCommands; i++) {
    free(commands[i]);
  }

  free(commands);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Formato de entrada inválido.");
    printf("Por favor, utilize o seguinte formato: ./main imagem_fat32\n");
    return 0;
  }

  struct bootSectorStruct bootSector;
  struct FSInfoStruct fsInfo;
  FAT32_Clusters *fat1 = NULL;

  char buffer[1024];
  memset(buffer, '\0', sizeof(buffer));
  char **commands;

  StackDirectory *stackDirectory = createStackDirectory();

  FILE *file = fopen(argv[1], "rb+");

  if (file == NULL) {
    printf("Erro ao abrir a imagem\n");
    return 1;
  }

  // Carregando Boot Sector e BPB Structure
  fread(&bootSector, sizeof(bootSector), 1, file);

  int fs_struct_offset = bootSector.BPB_BytsPerSec * bootSector.BPB_FSInfo;

  fseek(file, fs_struct_offset, SEEK_SET);
  fread(&fsInfo, sizeof(fsInfo), 1, file);

  // Alocando vetor de cluster para cada fat
  uint64_t totalNumbersOfDataSectors =
      getTotalNumberOfDataSectors(bootSector) + 1;

  fat1 = (uint32_t *)calloc(totalNumbersOfDataSectors, sizeof(uint32_t));

  // Lendo dados da FAT1
  fseek(file, getFat1Address(bootSector), SEEK_SET);
  fread(fat1, totalNumbersOfDataSectors, sizeof(*fat1), file);

  // Lendo o diretorio root
  readAllClusterOfDirectoryAndPushIntoStack(
      fat1, bootSector, file, stackDirectory, getClusterOfRootDir(bootSector),
      "/");

  system("clear");
  char *pwd = pwdCommand(stackDirectory);
  printf(GREEN("fatshell:") BLUE("[img%s]") "$ ", pwd);
  while (fgets(buffer, 1024, stdin) != NULL) {
    int amountOfCommands =
        tokenizeArrayOfCommands(&commands, buffer, &amountOfCommands);

    if (*commands != NULL) {
      // Verificando comando digitado
      if (!strcmp(commands[0], "ls")) {
        lsCommand(stackDirectory->currentDirectory->listDirEntry);
      } else if (!strcmp(commands[0], "cd")) {
        if (amountOfCommands != 2) {
          printf("Quantidade de argumentos inválidos para o comando cd.\n");
        } else {
          cdCommand(fat1, bootSector, stackDirectory, file, commands[1]);
        }
      } else if (!strcmp(commands[0], "info")) {
        infoCommand(file, bootSector, fat1, &fsInfo);
      } else if (!strcmp(commands[0], "touch")) {
        if (amountOfCommands != 2) {
          printf("Quantidade de argumentos inválidos para o comando touch.\n");
        } else {
          touchCommand(fat1, &fsInfo, bootSector, file,
                       stackDirectory->currentDirectory->listDirEntry,
                       commands[1]);
        }
      } else if (!strcmp(commands[0], "rm")) {
        if (amountOfCommands != 2) {
          printf("Quantidade de argumentos inválidos para o comando rm.\n");
        } else {
          rmCommand(stackDirectory->currentDirectory->listDirEntry, commands[1],
                    file, fat1, bootSector, &fsInfo);
        }
      } else if (!strcmp(commands[0], "cp")) {
        if (amountOfCommands != 3) {
          printf("Quantidade de argumentos inválidos para o comando cp.\n");
        } else {
          cpCommand(stackDirectory, file, fat1, bootSector, &fsInfo,
                    commands[1], commands[2]);
        }
      } else if (!strcmp(commands[0], "pwd")) {
        printf("%s\n", pwd);
      } else if (!strcmp(commands[0], "attr")) {
        if (amountOfCommands != 2) {
          printf("Quantidade de argumentos inválidos para o comando attr.\n");
        } else {
          attrCommand(stackDirectory->currentDirectory->listDirEntry,
                      commands[1]);
        }
      } else if (!strcmp(commands[0], "cluster")) {
        if (amountOfCommands != 2) {
          printf(
              "Quantidade de argumentos inválidos para o comando cluster.\n");
        } else {
          cluster(bootSector, file, atoi(commands[1]));
        }
      } else if (!strcmp(commands[0], "rename")) {
        if (amountOfCommands != 3) {
          printf("Quantidade de argumentos inválidos para o comando rename.\n");
        } else {
          renameCommand(bootSector, file,
                        stackDirectory->currentDirectory->listDirEntry,
                        commands[1], commands[2]);
        }
      } else if (!strcmp(commands[0], "mv")) {
        if (amountOfCommands != 3) {
          printf("Quantidade de argumentos inválidos para o comando mv.\n");
        } else {
          mvCommand(stackDirectory, file, fat1, bootSector, &fsInfo,
                    commands[1], commands[2]);
        }
      } else if (!strcmp(commands[0], "mkdir")) {
        if (amountOfCommands != 2) {
          printf("Quantidade de argumentos inválidos para o comando mkdir.\n");
        } else {
          mkdirCommand(fat1, &fsInfo, bootSector, file,
                       stackDirectory->currentDirectory->listDirEntry,
                       stackDirectory, commands[1]);
        }
      } else if (!strcmp(commands[0], "rmdir")) {
        if (amountOfCommands != 2) {
          printf("Quantidade de argumentos inválidos para o comando rmdir.\n");
        } else {
          rmdirCommand(stackDirectory, commands[1], file, fat1, bootSector,
                       &fsInfo);
        }
      } else if (!strcmp(commands[0], "clear")) {
        system("clear");
      } else if (!strcmp(commands[0], "help")) {
        helpCommand();
      } else if (!strcmp(commands[0], "exit")) {
        break;
      } else {
        printf(RED("[Comando Inválido]") ": Utilize " BLUE(
            "help") " para ver as opções disponíveis.\n");
      }
    }

    fclose(file);
    file = fopen("myimagefat32.img", "rb+");

    destroyArrayOfCommands(commands, amountOfCommands);
    free(pwd);
    pwd = pwdCommand(stackDirectory);
    printf(GREEN("fatshell:") BLUE("[img%s]") "$ ", pwd);
  }

  free(fat1);
  destroyStack(stackDirectory);
  fclose(file);

  return 0;
}
