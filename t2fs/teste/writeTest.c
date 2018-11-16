#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/disk.h"
#include "../include/apidisk.h"
#include "../include/t2fs.h"
// gcc -o testeWrite writeTest.c ../src/disk.c ../src/t2fs.c ../lib/apidisk.o -Wall -ggdb && ./testeWrite
// gcc -m32 -o testeWrite writeTest.c ../src/disk.c ../src/t2fs.c ../lib/apidisk.o -Wall -ggdb && ./testeWrite

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
    char aEveryWhere[8096];
    FILE2 handle = 0;

    init_disk();

    writeInFAT(60,END_OF_FILE);

    openFiles[0].file = handle;
    openFiles[0].clusterNo = 60;
    openFiles[0].currPointer = 0;
    printf("\n\nEscrevendo no Cluster 23 a letra 'b' 1024 vezes, tamanho de um cluster");  

    for(i = 0; i < 8096; i++){
        aEveryWhere[i] = 'b';
    }

    writeInFAT(60,61);
    writeInFAT(61,62);
    writeInFAT(62,63);
    writeInFAT(63,END_OF_FILE);

    writeFile(handle,aEveryWhere,4000);

    printDataCluster(61);
    printFAT(0);


    openFiles[0].currPointer = 3200;
    truncateFile(handle);

    printDataCluster(61);
    printDataCluster(62);
    printDataCluster(63);
    printFAT(0);


   //writeFile(handle,"aEveryWhere",11);

    


/*
    printDataSector(60);
    printDataSector(11);
    printDataSector(13);
    printDataSector(15);
    printDataSector(16);
    printDataSector(17);
    printDataSector(18);
    printDataSector(19);
*/

    //writeFile(handle,aEveryWhere,8092);


    
    

    


    return 0;
}