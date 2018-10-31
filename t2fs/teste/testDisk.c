#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/disk.h"
#include "../include/apidisk.h"

//gcc -o testeDisk testDisk.c ../src/disk.c ../lib/apidisk.o -Wall -ggdb && ./testeDisk

void printFAT(int sector) {
    int j;
    unsigned char buffer[SECTOR_SIZE];
    printf("\n");
    read_sector(superBlock.pFATSectorStart + sector,buffer);
    for (j = 0; j < 256; j++){
        printf("%x ",buffer[j]);
    }
    printf("\n");
}

int main() {
    int num = 13333;
    unsigned char* numLtlEnd;
    int num2 = 91532899;
    unsigned char* num2LtlEnd;
    struct t2fs_record* folderContent = malloc(sizeof(struct t2fs_record)*( (SECTOR_SIZE*superBlock.SectorsPerCluster) / sizeof(struct t2fs_record) ));

    init_disk();

    printf("\nNUM: %d\n", num);

    printf("\nWORD: %x\n", num);

    numLtlEnd = wordToLtlEnd(num);
    printf("\nWORD LITTLE END: %x %x\n",numLtlEnd[0], numLtlEnd[1]);

    printf("\nNUM2: %d\n", num2);

    printf("\nWORD: %x\n", num2);

    num2LtlEnd = dwordToLtlEnd(num2);
    printf("\nWORD LITTLE END: %x %x %x %x\n",num2LtlEnd[0], num2LtlEnd[1], num2LtlEnd[2], num2LtlEnd[3]);

    printFAT(0);

    printf("\nCLUSTER 2 NA FAT: %x\n",readInFAT(2));

    printf("\nESCREVENDO EOF\n");
    writeInFAT(2,END_OF_FILE);

    printf("\nCLUSTER 2 NA FAT: %x\n",readInFAT(2));

    printf("\nESCREVENDO 0xFF00FF00\n");
    writeInFAT(2,0xFF00FF00);

    printf("\nCLUSTER 2 NA FAT: %x\n",readInFAT(2));

    printFAT(0);

    printf("\nVOLTANDO PARA ZERO\n");

    writeInFAT(2,0);

    printf("\nCLUSTER 2 NA FAT: %x\n",readInFAT(2));

    folderContent = readDataCluster(0);



    return 0;
}