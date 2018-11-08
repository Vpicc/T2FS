#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/disk.h"
#include "../include/apidisk.h"

//gcc -o testePath testPath.c ../src/disk.c ../lib/apidisk.o -Wall -ggdb && ./testePath
//gcc -m32 -o testePath testPath.c ../src/disk.c ../lib/apidisk.o -Wall -ggdb && ./testePath

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
    int cluster;
    char** testeToken;
    int testeTokenSize;
    char * testeAbsolute;
    init_disk();

    printFolders(2);

    printf("\n\n****INICIANDO NA PASTA ROOT****\n\n");

    printf("\nRetorno do pathToCluster './file1.txt': %d\n", pathToCluster("./file1.txt"));


    printf("\nRetorno do pathToCluster '/dir1': %d\n", pathToCluster("/dir1"));

    printFolders(5);

    printf("\nRetorno do pathToCluster './dir1/file1.txt': %d\n", pathToCluster("./dir1/file1.txt"));

    printf("\nRetorno do pathToCluster './file1.txt/..': %d\n", pathToCluster("./file1.txt/.."));

    printf("\nRetorno do pathToCluster './dir1/file3.txt': %d\n", pathToCluster("./dir1/file3.txt"));

    printFAT(0);
    if(findFATOpenCluster(&cluster) != 0) {
        printf("\nERRO!\n");
    }

    printf("\nCluster disponivel na FAT: %d\n", cluster);

    writeInFAT(11,END_OF_FILE);

    printf("\nEscrevendo EOF no cluster 11\n");

    if(findFATOpenCluster(&cluster) != 0) {
        printf("\nERRO!\n");
    }

    printf("\nCluster disponivel na FAT: %d\n", cluster);

    writeInFAT(11,0);

    printf("\nVoltando a zero no cluster 11\n");

    testeTokenSize = tokenizePath("./dir1/dir2/../dir3///////////", &testeToken);

    for(i = 0; i < testeTokenSize; i++) {
        printf("\n%s\n", testeToken[i]);
    }


    toAbsolutePath("../b/c/d/e/f/g/h/i/../j/./k///","/aaa/bbb",&testeAbsolute);

    printf("\nTESTE ABSOLUTE: %s\n", testeAbsolute);

    printf("\n***************TESTE separatePath***************");

    char * saidaUm;
    char * saidaDois;
    separatePath("/ddd/ccc/aaaaa", &saidaUm, &saidaDois);

    printf("\nSaida Um: %s\nTamanho da saida um: %d", saidaUm, strlen(saidaUm));
    printf("\n\nSaida Dois: %s\nTamanho da saida dois: %d", saidaDois, strlen(saidaDois));
    printf("\n");

    printf("\n***************TESTE ChangeDir***************");
    printf("\nDiretorio atual: %s\n", currentPath.absolute);
    changeDir("./dir1/file1.txt");
    printf("\nAlterando para:'./dir1/file1.txt'\n");
    printf("Diretorio alterado: %s\nCluster atual: %d\n", currentPath.absolute, currentPath.clusterNo);
    changeDir(".././aa/b/../cb");
    printf("\nAlterando para:'.././aa/b/../cb'\n");
    printf("Diretorio alterado2: %s\n", currentPath.absolute);

    printf("\n");


    return 0;
}