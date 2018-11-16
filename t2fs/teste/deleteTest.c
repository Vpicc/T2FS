#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/disk.h"
#include "../include/t2fs.h"
#include "../include/apidisk.h"

//gcc -m32 -o deleteTest deleteTest.c ../src/disk.c ../src/t2fs.c ../lib/apidisk.o -Wall -ggdb && ./deleteTest

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
    init_disk();
    printFolders(currentPath.clusterNo);
    printFAT(0);
    printf("\nDataCluster ANTES DA DELECAO 4:\n");
    printDataCluster(4);
    printf("\n");
    printf("\nDataCluster ANTES DA DELECAO 8:\n");
    printDataCluster(8);
    if(deleteFile("./file2.txt") == -1){
        printf("ERROR NA DELECAO DO FILE2.TXT");
    }
    printFolders(currentPath.clusterNo);
    printf("\nDataCluster 4 depois da delecao:\n");
    printDataCluster(4);
    printf("\n");
    printf("\nDataCluster 8 depois da delecao:\n");
    printDataCluster(8);
    printFAT(0);
    printf("\n");
    printf("\nDataCluster ANTES DA DELECAO 9:\n");    
    printDataCluster(9);
    if(deleteFile("./link1") == -1){
        printf("ERROR NA DELECAO DO LINK1");
    }
    printFolders(currentPath.clusterNo);
    printf("\nDataCluster 9 depois da delecao:\n");    
    printDataCluster(9);
    printFAT(0);
}