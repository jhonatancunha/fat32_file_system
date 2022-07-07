#include "help.h"

void helpCommand() {
  printf(BLUE("info:") " exibe informações do disco e da FAT.\n");

  printf(BLUE("cluster ") CYAN(
      "<num>:") " exibe o conteúdo do bloco num no formato texto.\n");

  printf(BLUE("pwd:") " exibe o diretório corrente (caminho absoluto).\n");

  printf(BLUE("attr ") CYAN(
      "<file | dir>:") " exibe os atributos de um arquivo (file) ou diretório (dir).\n");

  printf(BLUE("cd ") CYAN(
      "<path>:") " altera o diretório corrente para o definido como path.\n");

  printf(BLUE("touch ")
             CYAN("<file>:") " cria o arquivo file com conteúdo vazio.\n");

  printf(BLUE("mkdir ") CYAN("<dir>:") " cria o diretório dir vazio.\n");

  printf(BLUE("rm ") CYAN("<file>:") " remove o arquivo file do sistema.\n");

  printf(BLUE("rmdir ")
             CYAN("<dir>:") " remove o diretório dir, se estiver vazio.\n");

  printf(BLUE("cp ") CYAN("<source_path> ") CYAN(
      "<target_path>:") " copia um arquivo de origem (source_path) para destino (target_path).\n");

  printf(BLUE("mv ") CYAN("<source_path> ") CYAN(
      "<target_path>:") " move um arquivo de origem (source_path) para destino (target_path).\n");

  printf(BLUE("rename ") CYAN("<file> ") CYAN(
      "<newfilename>:") " renomeia arquivo file para newfilename.\n");

  printf(
      BLUE("ls:") " listar os arquivos e diretórios do diretório corrente.\n");
}
