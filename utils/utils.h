/**
 * Descrição:
 *  Especificação de funções e estruturas auxiliares
 *  para manipulação do FAT.
 *
 * Autores:
 *  Gustavo Sengling Favaro
 *  Jessé Pires Barbato Rocha,
 *  Jhonatan Guilherme de Oliveira Cunha,
 *
 *  Criado em: 03/06/2022
 *
 *  Última atualização: 29/06/2022
 */

#pragma once
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define rootDirSectors 0  // 0 para o fat 32
#define MAX_DIR_IN_CLUSTER(bootSector) \
  (bootSector.BPB_SecPerClus * bootSector.BPB_BytsPerSec) / 32

#define ARCHIVE 32
#define DIRECTORY 16

#define FREE_CLUSTER 0x00
#define END_OF_CHAIN 0xFFFFFFF
#define END_OF_CHAIN_2 0xFFFFFF8
#define FSI_NXT_FREE_UNKNOWN 0xffffffff

#define DIR_ENTRY_LAST_AND_UNUSED 0x0
#define DIR_ENTRY_UNUSED 0xE5

#include "../FAT/fat.h"
#include "../ListDirEntry/listDirEntry.h"

// Obtêm 16 bits mais significativos do cluster
#define GET_HIGH_CLUSTER(cluster) (cluster >> 16)

// Obtêm 16 bits menos significativos do cluster
#define GET_LOW_CLUSTER(cluster) (cluster & 0xFFFF)

// Realiza  junção da parte baixa e alto do cluster
#define JOIN_LOW_HIGH_CLUSTER(low, high) (low | (high << 16))

// Tranforma Bytes em KiB
#define TRANSFORM_BYTES_IN_KIBIBYTES(valueInBytes) (valueInBytes / (double)1024)

// COLORS
#define BLUE(string) "\x1b[34m" string "\x1b[0m"
#define RED(string) "\x1b[31m" string "\x1b[0m"
#define GREEN(string) "\x1B[32m" string "\x1b[0m"
#define YELLOW(string) "\x1B[33m" string "\x1b[0m"
#define CYAN(string) "\x1B[36m" string "\x1b[0m"

/**
 * @brief Obtêm a posição no arquivo do primeiro setor dos clusters de dados.
 * @param[in] bootSector Instância da struct bootSectorStruct.
 * @return O valor uint64_t referente a posição no arquivo.
 */
uint64_t getFirstDataSector(struct bootSectorStruct bootSector);

/**
 * @brief Obtêm a posição no arquivo da FAT1.
 * @param[in] bootSector Instância da struct bootSectorStruct.
 * @return O valor uint64_t referente a posição no arquivo da FAT1.
 */
uint64_t getFat1Address(struct bootSectorStruct bootSector);

/**
 * @brief Obtêm a posição no arquivo da FAT2.
 * @param[in] bootSector Instância da struct bootSectorStruct.
 * @return O valor uint64_t referente a posição no arquivo da FAT2.
 */
uint64_t getFat2Address(struct bootSectorStruct bootSector);

/**
 * @brief Obtêm a posição inicial do setor de um cluster.
 * @param[in] bootSector Instância da struct bootSectorStruct.
 * @param[in] N Cluster a ser procurado.
 * @return O valor uint64_t referente a posição do primeiro setor do cluster no
 * arquivo.
 */
uint64_t getFirstSectorOfCluster(struct bootSectorStruct bootSector,
                                 uint32_t N);

/**
 * @brief Obtêm a posição do arquivo do primeiro setor do cluster do diretório
 * raiz (/)
 * @param[in] bootSector Instância da struct bootSectorStruct.
 * @return O valor uint64_t referente a posição no arquivo do diretório raiz
 * (/).
 */
uint64_t getClusterOfRootDir(struct bootSectorStruct bootSector);

/**
 * @brief Obtêm a quantidade total de setores de dados.
 * @param[in] bootSector Instância da struct bootSectorStruct.
 * @return A quantidade total de setores de dados.
 */
uint64_t getTotalNumberOfDataSectors(struct bootSectorStruct bootSector);

/**
 * @brief Obtêm a quantidade total de setores de dados.
 * @param[in] fsInfo Ponteiro da estrutura FSInfoStruct que será salva.
 * @return A quantidade total de setores de dados.
 */
uint32_t getFreeClusterCount(struct FSInfoStruct *fsInfo);

/**
 * @brief Obtêm o tamanho total do arquivo.
 * @param[in] file Ponteiro FILE para o arquivo.
 * @return O tamanho total do arquivo.
 */
uint64_t getSizeOfImage(FILE *file);

/**
 * @brief Obtêm o tamanho total da região de dados do arquivo.
 * @param[in] sizeOfImage Tamanho da imagem.
 * @param[in] bootSector Instância da struct bootSectorStruct.
 * @return O tamanho total da região de dados do arquivo.
 */
uint64_t getDataSize(uint64_t sizeOfImage, struct bootSectorStruct bootSector);

/**
 * @brief Imprimi na tela a data de maneira formada.
 * @param[in] data Valor em uint16_t referente a data do arquivo na FAT.
 */
void getDateFromFile(uint16_t data);

/**
 * @brief Imprime hora, minuto e segundos de maneira formatada.
 * @param[in] time Valor em uint16_t referente ao horário do arquivo na FAT.
 */
void getTimestampFromFile(uint16_t time);

/**
 * @brief Busca uma DirEntry baseado em seu nome.
 * @param[in] listDirEntry Ponteiro para a ListDirEntry que realizaremos a
 * busca.
 * @param[in] filename Nome que será buscado na lista.
 * @param[out] positionOfFile Variável que armazenará a posição onde a DirEntry
 * foi encontrada.
 * @retval Retorna um ponteiro NodeDirEntry referente ao DirEntry
 * encontrado.
 * @retval Retorna NULL caso o arquivo procurado não for encontrado.
 */
NodeDirEntry *getNodeDirEntryByFilename(ListDirEntry *listDirEntry,
                                        char *filename, int *positionOfFile);

/**
 * @brief Obtêm o DirName da ShortDirEntry de maneira formatada.
 * @param[in] name Nome da ShortDirEntry a ser formatada
 * @param[in] attrOfEntry DIR_Attr referente a ShortDirEntry que será formatada.
 * @return Uma string contendo a versão formatada do nome da ShortDirEntry.
 */
char *prettyShortDirName(char *name, uint8_t attrOfEntry);

/**
 * @brief Converte o nome do arquivo/diretório para um formtado aceitado pela
 * FAT 32.
 * @param[in] name Nome da arquivo/diretório ser formatada
 * @return Uma string contendo a versão formatada do nome do arquivo/diretório.
 */
char *fatWayShortDirName(char *name);

/**
 * @brief Salva o DirEntry em sua devida posição no arquivo.
 * @param[in] bootSector Instância da struct bootSectorStruct.
 * @param[in] file Ponteiro FILE para o arquivo.
 * @param[in] entry Ponteiro DirEntry que será salvo no arquivo.
 * @param[in] cluster Cluster onde o @p entry será salvo.
 * @param[in] dirEntryPosition Posição do @p entry em sua ListDirEntry (usado
 * para cálculo de offset).
 */
void saveDirEntryOnPositionOfFile(struct bootSectorStruct bootSector,
                                  FILE *file, DirEntry *entry, uint32_t cluster,
                                  int dirEntryPosition);

/**
 * @brief Realiza a validão de um DIR_Name para o formato aceito pela FAT 32.
 * @param[in] dirname DIR_Name da DirEntry a ser válidada.
 * @retval Retorna 0 caso nome seja inválido.
 * @retval Retorna 1 caso nome seja válido.
 */
int validateDirName(char *dirname);

/**
 * @brief Obtêm a hora, minuto e segundo atual do sistema no formato aceito pelo
 * FAT32.
 * @return O valor em uint16_t referente a hora, minuto e segundo atual.
 */
uint16_t getCurrentTime();

/**
 * @brief Obtêm a data atual do sistema no formato aceito pelo FAT32.
 * @return O valor em uint16_t referente a data atual.
 */
uint16_t getCurrentDate();

/**
 * @brief Converte um número para binário.
 * @param[in] number Número a ser convertido.
 * @param[out] binary Ponteiro para vetor onde será armazenado o binário.
 */
void convertNumberToBinary(uint64_t number, int *binary);

/**
 * @brief Atualiza as duas tabelas FAT no arquivo.
 * @param[in] bootSector Instância da struct bootSectorStruct.
 * @param[in] file Ponteiro FILE para o arquivo.
 * @param[in] fat Ponteiro FAT32_Clusters da tabela FAT que será salva.
 */
void saveFAT(struct bootSectorStruct bootSector, FILE *file,
             FAT32_Clusters *fat);

/**
 * @brief Atualiza a estrutura FsInfo no arquivo.
 * @param[in] fsInfo Ponteiro da estrutura FSInfoStruct que será salva.
 * @param[in] bootSector Instância da struct bootSectorStruct.
 * @param[in] file Ponteiro FILE para o arquivo.
 */
void saveFSInfo(struct FSInfoStruct *fsInfo, struct bootSectorStruct bootSector,
                FILE *file);

/**
 * @brief Verifica se ainda é possivel salvar mais dados no último cluster
 * utilizado pelo DirEntry.
 * @param[in] bootSector Instância da struct bootSectorStruct.
 * @param[in] amountOfDirEntries Quantidade de DirEntry que estão já estão
 * armazenados nos clusters.
 * @param[in] amountOfCluster Quantidade de clusters utilizados pelo DirEntry.
 * @retval Retorna 1 caso seja possivel utilizar o mesmo cluster.
 * @retval Retorna 0 caso não seja possivel utilizar o mesmo cluster, ou seja,
 * necessitamos alocar outro cluster para a DirEntry.
 */
int checkCanSaveInTheLastCluster(struct bootSectorStruct bootSector,
                                 int amountOfDirEntries, int amountOfCluster);

/**
 * @brief Obtêm um cluster livre na tabela FAT 32.
 * @param[in] fat Ponteiro FAT32_Clusters da tabela FAT que será salva.
 * @param[in] fsInfo Ponteiro da estrutura FSInfoStruct.
 * @param[in] totalFatEntries Total de entradas que a tabela FAT 32 possui.
 * @retval Retorna a posição livre na tabela FAT 32.
 * @retval Retorna -1 caso a tabela FAT não tenha nenhuma posição livre.
 */
uint32_t getOneFreeClusterInFat(FAT32_Clusters *fat,
                                struct FSInfoStruct *fsInfo,
                                uint64_t totalFatEntries);

/**
 * @brief Obtêm um cluster livre na tabela FAT 32 e encadeia o mesmo na corrente
 * de clusters referente a um DirEntry.
 * @param[in] fat Ponteiro FAT32_Clusters da tabela FAT que será salva.
 * @param[in] fsInfo Ponteiro da estrutura FSInfoStruct que será salva.
 * @param[in] totalFatEntries Total de entradas que a tabela FAT 32 possui.
 * @param[in] lastFatPos Último cluster da DirEntry que possuirá um novo
 * cluster.
 * @retval Retorna o novo cluster utilizado.
 * @retval Retorna -1 caso não encontre nenhum cluster livre.
 */
uint32_t getFreeClusterAndConcatenate(FAT32_Clusters *fat,
                                      struct FSInfoStruct *fsInfo,
                                      uint64_t totalFatEntries,
                                      uint32_t lastFatPos);

/**
 * @brief Realiza a tokenização de uma strig de comando para uma lista de string
 * de comandos.
 * @param[out] tokenizedPath Ponteiro para variável que será armazenado a
 * a lista de string de comandos.
 * @param[in] path String de comandos a ser tokenizada.
 * @return Retorna a quantidade de comandos inseridos.
 */
int tokenizePath(char ***tokenizedPath, char *path);

/**
 * @brief Verifica se existe alguma posição livre a ser utilizado na
 * ListDirEntry.
 * @param[in] fat Ponteiro FAT32_Clusters da tabela FAT.
 * @param[in] listDirEntry Lista de DirEntry onde será realizado a procura.
 * @param[out] clusterOfEntryclusterOfEntry Varíavel onde será armazenado o
 * cluster referente a posição encontrada.
 * @retval Retorna a posição livre da lista de DirEntry.
 * @retval Retorna -1 caso não encontre nenhuma posição livre na lista de
 * DirEntry.
 */
int findFreePositionOnDirEntryList(FAT32_Clusters *fat,
                                   ListDirEntry *listDirEntry,
                                   uint32_t *clusterOfEntryclusterOfEntry);

// Salva DirEntry na imagem e retorna a posição onde o dir entry foi inserido na
// lista

/**
 * @brief Salva DirEntry no arquivo e insere a mesma na lista de DirEntry.
 * Esta função também realiza a atualização das informações das seguintes
 * estruturas:
 *
 * - Tabela FAT
 *
 * - Estrutura FsInfo
 *
 * - Lista de DirEntry (Insere o DirEntry salvo na lista).
 *
 * @param[in] fat Ponteiro FAT32_Clusters da tabela FAT que será salva.
 * @param[in] fsInfo Ponteiro da estrutura FSInfoStruct que será salva.
 * @param[in] bootSector Instância da struct bootSectorStruct.
 * @param[in] file Ponteiro FILE para o arquivo.
 * @param[in] listDirEntry Lista de DirEntry onde será inserido a DirEntry.
 * @param[in] nodeListDirEntry Ponteiro para NodeDirEntry que contém o DirEntry
 * que será salvo
 * @param[out] dirEntry Ponteiro para a DirEntry onde armazenaremos informações
 * atualizadas sobre o DirEntry após o salvamento.
 * @return Retorna a posição de onde o DirEntry foi salvo na lista de DirEntry.
 */
int saveDirEntryOnFile(FAT32_Clusters *fat, struct FSInfoStruct *fsInfo,
                       struct bootSectorStruct bootSector, FILE *file,
                       ListDirEntry *listDirEntry,
                       NodeDirEntry *nodeListDirEntry, DirEntry *dirEntry);

/**
 * @brief Salva o buffer de dados no cluster informado.
 * @param[in] bootSector Instância da struct bootSectorStruct.
 * @param[in] file Ponteiro FILE para o arquivo.
 * @param[in] cluster Cluster onde será salvo os dados.
 * @param[in] buffer Dados que serão salvos.
 * @param[in] sizeOfBuffer Tamanho do buffer de dados que será salvo.
 */
void saveDataClusterInFile(struct bootSectorStruct bootSector, FILE *file,
                           uint32_t cluster, uint8_t *buffer,
                           uint32_t sizeOfBuffer);
