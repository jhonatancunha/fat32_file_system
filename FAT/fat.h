/**
 * Descrição:
 *  Especificação das estruturas da FAT32.
 *
 * Autores:
 *  Gustavo Sengling Favaro
 *  Jessé Pires Barbato Rocha,
 *  Jhonatan Guilherme de Oliveira Cunha,
 *
 *  Criado em: 15/05/2022
 *
 *  Última atualização: 29/06/2022
 */

#pragma once
#include <stdint.h>
#include <uchar.h>

// Variaveis com BS (Boot Sector)
// Variaveis com BPB (BIOS Parameter Block)
struct bootSectorStruct {
  uint8_t BS_jmpBoot[3];
  uint8_t BS_OEMName[8];
  uint16_t BPB_BytsPerSec;
  uint8_t BPB_SecPerClus;
  uint16_t BPB_RsvdSecCnt;
  uint8_t BPB_NumFATs;
  uint16_t BPB_RootEntCnt;
  uint16_t BPB_TotSec16;
  uint8_t BPB_Media;
  uint16_t BPB_FATSz16;
  uint16_t BPB_SecPerTrk;
  uint16_t BPB_NumHeads;
  uint32_t BPB_HiddSec;
  uint32_t BPB_TotSec32;

  // FAT32
  uint32_t BPB_FATSz32;
  uint16_t BPB_ExtFlags;
  uint16_t BPB_FSVer;
  uint32_t BPB_RootClus;
  uint16_t BPB_FSInfo;
  uint16_t BPB_BkBootSec;
  uint8_t BPB_Reserved[12];
  uint8_t BS_DrvNum;
  uint8_t BS_Reserved1;
  uint8_t BS_BootSig;
  uint32_t BS_VolID;
  uint8_t BS_VolLab[11];
  uint8_t BS_FilSysType[8];
  uint8_t boot_code[420];
  uint16_t bootable_partition_signature;
} __attribute__((packed, aligned(1)));

struct FSInfoStruct {
  uint32_t FSI_LeadSig;
  uint8_t FSI_Reserved1[480];
  uint32_t FSI_StrucSig;
  uint32_t FSI_Free_Count;
  uint32_t FSI_Nxt_Free;
  uint8_t FSI_Reserved2[12];
  uint32_t FSI_TrailSig;
} __attribute__((packed, aligned(1)));

// Estrutura para uma ShortDirEntry
struct ShortDirStruct {
  uint8_t DIR_Name[11];
  uint8_t DIR_Attr;
  uint8_t DIR_NTRes;
  uint8_t DIR_CrtTimeTenth;
  uint16_t DIR_CrtTime;
  uint16_t DIR_CrtDate;
  uint16_t DIR_LstAccDate;
  uint16_t DIR_FstClusHI;
  uint16_t DIR_WrtTime;
  uint16_t DIR_WrtDate;
  uint16_t DIR_FstClusLO;
  uint32_t DIR_FileSize;
} __attribute__((packed, aligned(1)));

// Estrutura para uma LongDirEntry
struct LongDirStruct {
  uint8_t LDIR_Ord;
  char16_t LDIR_Name1[5];
  uint8_t LDIR_Attr;
  uint8_t LDIR_Type;
  uint8_t LDIR_Chksum;
  char16_t LDIR_Name2[6];
  uint16_t LDIR_FstClusLO;
  char16_t LDIR_Name3[2];
} __attribute__((packed, aligned(1)));

typedef union DirEntry {
  struct LongDirStruct longEntry;
  struct ShortDirStruct shortEntry;
} DirEntry;

typedef uint32_t FAT32_Clusters;
