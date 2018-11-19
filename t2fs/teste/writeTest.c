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
        if(folderContent[i].firstCluster != 0){
            printf("\nTYPEVAL: %x\n", folderContent[i].TypeVal);
            printf("NAME: %s\n", folderContent[i].name);
            printf("BYTESFILESIZE: %x\n", folderContent[i].bytesFileSize);
            printf("CLUSTERSFILESIZE: %x\n", folderContent[i].clustersFileSize);
            printf("FIRSTCLUSTER: %x\n", folderContent[i].firstCluster);
        }
    }
}


int main() {
    int i;
    char aEveryWhere[8096];
    int handleFile5;

    if((handleFile5 = create2("file6.txt")) == -1){
        printf("\nERRROR create25\n");
    }

    seek2(handleFile5,0);
    printf("\n\nEscrevendo no Cluster 23 a letra 'b' 1024 vezes, tamanho de um cluster\n");  

    for(i = 0; i < 8096; i++){
        aEveryWhere[i] = 'b';
    }


    printf("\nRETORNO WRITE: %d\n",write2(handleFile5,aEveryWhere,4000));

    printFolders(2);
    printf("\nRETORNO SEEK: %d\n",seek2(handleFile5,2600));
    printf("\nRETORNO TRUNCATE: %d\n",truncate2(handleFile5));
    printFolders(2);
  


    return 0;
}