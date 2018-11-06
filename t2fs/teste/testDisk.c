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
    char* escrita = "Esse eh o teste no cluster 23..";
    int num = 13333;
    unsigned char* numLtlEnd;
    int num2 = 91532899;
    unsigned char* num2LtlEnd;

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
    printf("\n\n*******Print do Data Sector 2*******:\n");
    printDataSector(2);
    printf("\n\n*******FOLDERS*******:\n");
    printFolders(2);

    printf("\n\n*******CLUSTER 3*******:\n");
    printDataSector(3);
    printf("\n\n*******Conteudo do CLUSTER 3*******:\n");
    printDataCluster(3);

    printf("\n\n************************************:\n");
    printf("Escrevendo no Cluster 23: 'Esse eh o teste no cluster 23..'");
    writeCluster(23, (unsigned char*) escrita,0,32);
    printDataSector(23);
    printDataCluster(23);

    printf("\n\nEscrevendo no Cluster 23 a letra 'b' 1024 vezes, tamanho de um cluster");  

    for(i = 0; i < 1024; i++){
        aEveryWhere[i] = 'b';
    }
    writeCluster(23, (unsigned char*) aEveryWhere,0,1024);
    printDataSector(23);
    printDataCluster(23);

printf("\n\n\n");  

    return 0;
}