#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/disk.h"
#include "../include/t2fs.h"
#include "../include/apidisk.h"

//gcc -m32 -o dirTest dirTest.c ../src/disk.c ../src/t2fs.c ../lib/apidisk.o -Wall -ggdb && ./dirTest

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
    
    printf("\n\n***************TESTE MakeDir***************\n");

    printf("\n*****Fazendo o direito 'abra' na Root.\n");
    if(mkdir("../abra") == -1){
        printf("\nError\n");
    }
    if(mkdir("../abra") == -1){
        printf("\nError\n");
    }    
    printf("***Folders da ROOT:\n");
    printFolders(2);
    printf("\n***Mudando para o diretorio recem criado '/abra'\n");
    if(changeDir("abra") == -1){
        printf("\nError change abra\n");
    }
    printf("Folders do direitorio '/abra':\n");
    printFolders(currentPath.clusterNo);
    if(changeDir("..") == -1){
        printf("\nError change ..\n");
    }
    printFAT(0);
    printf("\n***Deletando o diretorio recem criado '/abra'\n");
    printf("\nPath atual antes da deleção: %s\n", currentPath.absolute);
    deleteDir("./abra");
    printf("\nPath atual depois da deleção: %s\n", currentPath.absolute);
    printFAT(0);
    printf("***Folders da ROOT:\n");
    printFolders(currentPath.clusterNo);
    printf("***Folde do diretorio que foi deletado '/abra':\n");
    printFolders(11);
    printf("\n\n***Fazendo o direito 'abra2' dentro do diretorio 'dir1' estando na raiz ");
    if(mkdir("../dir1/abra2") == -1){
        printf("\nError mkdir abra2\n");
    }    
    printf("\nPath atual depois da criação do abra2 dentro do dir1 estando na raiz: %s\n", currentPath.absolute);
    printf("\n**Indo para o direito 'abra'\n");
    if(changeDir("./abra") == -1){//esse diretorio n existe mais
        printf("\nError change ./abra\n");
    }
    printf("Path atual: %s\n", currentPath.absolute);
    printf("\n**Indo para o direito ' '\n");
    if(changeDir(" ") == -1){//piccoli falou q tem q retornar erro mesmo
        printf("\nError change " """\n");
    }
    printf("Path atual: %s\n", currentPath.absolute);

    printf("\n**Indo para o direito '..dir1/ '\n");
    if(changeDir("../dir1") == -1){
        printf("\nError change ../dir1\n");
    }
    printf("Path atual: %s\n", currentPath.absolute);
    printf("\n***Folders do DIR1:\n");
    printFolders(currentPath.clusterNo);
    printf("\n**Indo para o diretorio 'abra2'\n");
    if(changeDir("abra2") == -1){
        printf("\nError change abra2\n");
    }
    printFolders(currentPath.clusterNo);

    printf("Path atual: %s\n", currentPath.absolute);
    if(changeDir(".") == -1){
        printf("\nError change .\n");
    }
    printf("\n\n**Indo para o direito '.'\n");
    printf("Path atual: %s\n", currentPath.absolute);

    printf("\n**Indo para o direito '..' -> voltar um\n");
    if(changeDir("..") == -1){
        printf("\nError change ..\n");
    }
    printf("Path atual: %s\n", currentPath.absolute);

    printf("\n**Indo para o direito '/' -> raiz\n");
    if(changeDir("/") == -1){
        printf("\nError change /\n");
    }
    printf("Path atual: %s\n", currentPath.absolute);

    printf("\n**Deletando o abra2, que está dentro do dir1 apartir da raiz:\n");
    if(deleteDir("../dir1/abra2") == -1){
        printf("Error na deleção");
    }
    if(deleteDir("./file1.txt") == -1){
        printf("\nFEITO N REMOVEU O FILE1\n");
    }
    printf("Path atual depois da delecao: %s\n", currentPath.absolute);
    printf("Indo para o direito dir1\n");
    if(changeDir("../dir1") == -1){
        printf("\nError change ../dir1\n");
    }
    printf("Path atual: %s\n", currentPath.absolute);
    printf("***Folders do DIR1:\n");    
    printFolders(currentPath.clusterNo);

    if(changeDir("../") == -1){
        printf("\nERROR 1\n");
    }
printf("PRINT DA FAT ANTES DA CRIACAO DOS ARQUIVOS");
    printFAT(0);
//Teste para a função createFile
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

    changeDir("/");
    openFile("link1");
    printf("\nPRINT DA FAT\n");
    printFAT(0);
    printf("\nPRINT DOS ARQUIVOS ABERTOS\n");
    printOpenFiles();

/*
//Teste da função getcwd2
printf("\n\n*******TESTE PARA A FUNCAO GETCWD2");
    char * currentPathTest = malloc(15);
    
    if(getcwd2(currentPathTest, 15) == -1){
        printf("Erro tamanho insuficiente\n");
    }
    else
        printf("\ncurrentPathTest: %s\n", currentPathTest);

    char * currentPathTest2 = malloc(3);
    if(getcwd2(currentPathTest2, 3) == -1){
        printf("Erro tamanho insuficiente\n");
    }
    else
        printf("\ncurrentPathTest: %s\n", currentPathTest2);
*/
    return 0;
}