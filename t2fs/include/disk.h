#ifndef __DISK___
#define __DISK___
#include "t2fs.h"

DWORD convertToDword(unsigned char* buffer);

WORD convertToWord(unsigned char* buffer);

unsigned char* wordToLtlEnd(WORD entry);

unsigned char* dwordToLtlEnd(DWORD entry);

int init_disk();


typedef struct diskf {
    FILE2 file;
    int currPointer;
    char name[54];
} DISK_FILE;


#endif