#ifndef __DISK___
#define __DISK___
#include "t2fs.h"

#define END_OF_FILE 0xFFFFFFFF

#define BAD_SECTOR 0xFFFFFFFE

struct t2fs_superbloco superBlock;

DWORD convertToDword(unsigned char* buffer);

WORD convertToWord(unsigned char* buffer);

unsigned char* wordToLtlEnd(WORD entry);

unsigned char* dwordToLtlEnd(DWORD entry);

int init_disk();

int writeInFAT(int clusterNo, DWORD value);

DWORD readInFAT(int clusterNo);

struct t2fs_record* readDataCluster(int clusterNo);


typedef struct diskf {
    FILE2 file;
    int currPointer;
    char name[54];
} DISK_FILE;


#endif