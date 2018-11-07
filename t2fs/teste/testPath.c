#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/disk.h"
#include "../include/apidisk.h"

//gcc -o testePath testPath.c ../src/disk.c ../lib/apidisk.o -Wall -ggdb && ./testePath

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

    printFolders(2);

    printf("\n\n****INICIANDO NA PASTA ROOT****\n\n");

    printf("\nRetorno do pathToCluster './file1.txt': %d\n", pathToCluster("./file1.txt"));


    printf("\nRetorno do pathToCluster '/dir1': %d\n", pathToCluster("/dir1"));

    printFolders(5);

    printf("\nRetorno do pathToCluster './dir1/file1.txt': %d\n", pathToCluster("./dir1/file1.txt"));

    printf("\nRetorno do pathToCluster './file1.txt/..': %d\n", pathToCluster("./file1.txt/.."));

    printf("\nRetorno do pathToCluster './dir1/file3.txt': %d\n", pathToCluster("./dir1/file3.txt"));






    return 0;
}