#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/disk.h"
#include "../include/apidisk.h"
#include "../include/t2fs.h"

//gcc -o testeRead testRead.c ../src/disk.c ../src/t2fs.c ../lib/apidisk.o -Wall -ggdb && ./testeRead
//gcc -m32 -o testeDisk testDisk.c ../src/disk.c ../src/t2fs.c ../lib/apidisk.o -Wall -ggdb && ./testeSoft

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
int main(){

FILE2 openFile1,openFile2;
char *buffer=malloc(sizeof(char)*100);
DIR2 openD;
DIRENT2 direntry;
int i;

init_disk();

openD=openDir("/");
for(i=0;i<5;i++){
    if(readdir2(openD,&direntry)==-1)
        fprintf(stderr,"Erro ao ler diretorio\n\n");
    else{
        fprintf(stderr,"First entry name: %s\n",direntry.name);
        fprintf(stderr,"First entry fileType: %x\n",direntry.fileType);
        fprintf(stderr,"First entry size: %x\n\n",direntry.fileSize);
    }
}
openFile1=open2("/file1.txt");

    fprintf(stderr,"LENDO ARQUIVO /file1.txt com o handle %d\n\n", openFile1);
    fprintf(stderr,"Retorno do read:%d\n\n",read2(openFile,buffer,99));
    fprintf(stderr,"%s\n",buffer);

    openFile2=open2("/file2.txt");

    fprintf(stderr,"LENDO ARQUIVO /file2.txt");
    fprintf(stderr,"Retorno do read:%d\n\n",read2(openFile,buffer,99));
    fprintf(stderr,"%s\n",buffer);


return 0;

}