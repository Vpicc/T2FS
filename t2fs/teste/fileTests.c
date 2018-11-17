#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/disk.h"
#include "../include/t2fs.h"
#include "../include/apidisk.h"

//gcc -m32 -o fileTests fileTests.c ../src/disk.c ../src/t2fs.c ../lib/apidisk.o -Wall -ggdb && ./fileTests

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

    printf("\n\n*******TESTE PARA A CREATEFILE");
    int handleFile5;
    int handleFile6;
    int handleFile7;
    int handleFile8;
    int handleFile9;
    int handleFile10;
    int handleFile11;
    int handleFile12;
    int handleFile13;
    int handleFile14;
    int handleFile15;

    //printf("current: %s", currentPath.absolute);
    if((handleFile5 = createFile("./dir1/file5.txt")) == -1){
        printf("\nERRROR createFile5\n");
    }
    //printOpenFiles();
    if((handleFile6 = createFile("./dir1/file6.txt")) == -1){
        printf("\nERRROR createFile6\n");
    }
    //printOpenFiles();
    if((handleFile7 = createFile("./dir1/file7.txt")) == -1){
        printf("\nERRROR createFile7\n");
    }
    //printOpenFiles();
    if((handleFile8 = createFile("./dir1/file8.txt")) == -1){
        printf("\nERRROR createFile8\n");
    }
    printOpenFiles();
    if((handleFile9 = createFile("./dir1/file9.txt")) == -1){
        printf("\nERRROR createFile9\n");
    }
    if((handleFile10 = createFile("./dir1/file10.txt")) == -1){
        printf("\nERRROR createFile10\n");
    }
    if((handleFile11 = createFile("./dir1/file11.txt")) == -1){
        printf("\nERRROR createFile511\n");
    }
    if((handleFile12 = createFile("./dir1/file12.txt")) == -1){
        printf("\nERRROR createFile12\n");
    }
    if((handleFile13 = createFile("./dir1/file13.txt")) == -1){
        printf("\nERRROR createFile13\n");
    }
    if((handleFile14 = createFile("./dir1/file14.txt")) == -1){
        printf("\nERRROR createFile14\n");
    }
    if((handleFile15 = createFile("./dir1/file15.txt")) == -1){
        printf("\nERRROR createFile15\n");
    }
    if(closeFile(handleFile13) == -1){
        printf("\nERROR closeFile 13\n");
    }
    if((handleFile15 = createFile("./dir1/file15.txt")) == -1){
        printf("\nERRROR createFile15 second time\n");
    }
    printf("\nMudando para o direotrio './dir1' apois a criação de 11 arquivos(file5 ... file 14)");
    if(changeDir("./dir1") == -1){
        printf("\nERRROR 3\n");
    }

    printf("\nFOLDERS DO DIREOTIRO DIR1, ONDE FOI CRIADO OS ARQUIVOS");
    printFolders(currentPath.clusterNo);
    deleteFile("./file15.txt");
    printf("\nFOLDERS DO DIREOTIRO DIR1, ONDE FOI CRIADO OS ARQUIVOS e deletado o 15");
    printFolders(currentPath.clusterNo);

    changeDir("/");//Se colocar "/" aqui também está dando segmentation
    openFile("link1");
    printf("\nPRINT DA FAT\n");
    printFAT(0);
    printf("\nPRINT DOS ARQUIVOS ABERTOS\n");
    printOpenFiles();

}