#include "pwd.h"

char* pwdCommand(StackDirectory* stackDirectory) {
  // Quantidade de caracteres que a string de saída do pwd precisa ter
  int size = (11 * stackDirectory->qtdDirectory) + stackDirectory->qtdDirectory;
  // Alocação da string que armazena a saída do comando pwd
  char* output = (char*)calloc(size + 1, sizeof(char));

  // Variável auxiliar da NodeStackDirectory para navegação na pilha de
  // diretórios
  NodeStackDirectory* aux = stackDirectory->rootDirectory;

  // Itera por todos os diretórios da NodeStackDirectory
  int i;
  for (i = 0; i < stackDirectory->qtdDirectory; i++) {
    // Concatena na string o diretório da variável auxiliar
    strcat(output, aux->name);
    // Caso o diretório atual seja diferente do diretório raiz ou diferente do
    // diretório corrente, o separador '/' é concatenado na string de saída
    if ((i + 1) != stackDirectory->qtdDirectory) {
      if (i) strcat(output, "/");
      aux = aux->next;
    }
  }
  return output;
}
