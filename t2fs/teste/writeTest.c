#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/disk.h"
#include "../include/apidisk.h"
#include "../include/t2fs.h"
// gcc -o testeWrite writeTest.c ../src/disk.c ../src/t2fs.c ../lib/apidisk.o -Wall -ggdb && ./testeWrite

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

void printDataSector(int clusterNo) {
    int j;
    int clusterByteSize = sizeof(unsigned char)*SECTOR_SIZE*superBlock.SectorsPerCluster;
    unsigned char* buffer = malloc(clusterByteSize);
    readCluster(clusterNo, buffer);
    printf("\n");
    for (j = 0; j < 1024; j++){
        printf("%x ",buffer[j]);
    }
    printf("\n");
}
void printDataCluster(int clusterNo) {
    int j;
    int clusterByteSize = sizeof(unsigned char)*SECTOR_SIZE*superBlock.SectorsPerCluster;
    unsigned char* buffer = malloc(clusterByteSize);
    buffer = readDataCluster(clusterNo);
    printf("\n");
    for (j = 0; j < 1024; j++){
        printf("%c",buffer[j]);
    }
    printf("\n");
}

void printFolders(int clusterNo) {
    int i;
    int folderSize = ( (SECTOR_SIZE*superBlock.SectorsPerCluster) / sizeof(struct t2fs_record) );
    struct t2fs_record* folderContent = malloc(sizeof(struct t2fs_record)*( (SECTOR_SIZE*superBlock.SectorsPerCluster) / sizeof(struct t2fs_record) ));
    folderContent = readDataClusterFolder(clusterNo);
    for(i = 0; i < folderSize; i++) {
        printf("\nTYPEVAL: %x\n", folderContent[i].TypeVal);
        printf("NAME: %s\n", folderContent[i].name);
        printf("BYTESFILESIZE: %x\n", folderContent[i].bytesFileSize);
        printf("CLUSTERSFILESIZE: %x\n", folderContent[i].clustersFileSize);
        printf("FIRSTCLUSTER: %x\n", folderContent[i].firstCluster);
    }
}


int main() {
    int i;
    char aEveryWhere[1024];
    FILE2 handle = 0;

    init_disk();

    openFiles[0].file = handle;
    openFiles[0].clusterNo = 23;
    openFiles[0].currPointer = 1060;
    printf("\n\nEscrevendo no Cluster 23 a letra 'b' 1024 vezes, tamanho de um cluster");  

    for(i = 0; i < 1024; i++){
        aEveryWhere[i] = 'b';
    }
    writeCluster(23, (unsigned char*) aEveryWhere,0,1024);

    writeCluster(23,(unsigned char*)"TESTANDO",6,9);

    writeInFAT(23,24);
    writeInFAT(24,25);
    writeInFAT(25,26);
    writeInFAT(26,END_OF_FILE);

    writeCluster(24, (unsigned char*) aEveryWhere,0,1024);
    writeCluster(25, (unsigned char*) aEveryWhere,0,1024);
    writeCluster(26, (unsigned char*) aEveryWhere,0,1024);

    printFAT(0);

    truncateFile(handle);

    printDataSector(23);
    printDataCluster(23);

    printDataSector(24);
    printDataCluster(24);

    printDataSector(25);
    printDataCluster(25);    
    printDataSector(26);
    printDataCluster(26);
    
    

    


    return 0;
}