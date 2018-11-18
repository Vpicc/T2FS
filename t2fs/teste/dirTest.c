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
    if(mkdir2("../abra") == -1){
        printf("\nError\n");
    }
    if(mkdir2("../abra") == -1){
        printf("\nError pq tentou criar um direitorio ja existente\n");
    }    
    printf("***Folders da ROOT:\n");
    printFolders(2);
    printf("\n***Mudando para o diretorio recem criado '/abra'\n");
    if(chdir2("./abra") == -1){
        printf("\nError change abra\n");
    }
    printf("Folders do direitorio '/abra':\n");
    printFolders(currentPath.clusterNo);
    if(chdir2("..") == -1){
        printf("\nError change ..\n");
    }
    printFAT(0);
    printf("\n");
    printf("\n***Deletando o diretorio recem criado '/abra'\n");
    printf("\nPath atual antes da deleção: %s\n", currentPath.absolute);
    if(rmdir2("./abra") == -1){
        printf("\nERRO NA DELECAO DO ABRA\n");
    }
    printf("\nPath atual depois da deleção: %s\n", currentPath.absolute);
    printFAT(0);
    printf("***Folders da ROOT:\n");
    printFolders(currentPath.clusterNo);
    printf("***Folde do diretorio que foi deletado '/abra':\n");
    printFolders(11);
    printf("\n\n***Fazendo o direito 'abra2' dentro do diretorio 'dir1' estando na raiz ");
    if(mkdir2("../dir1/abra2") == -1){
        printf("\nError mkdir2 abra2\n");
    }    
    printf("\nPath atual depois da criação do abra2 dentro do dir1 estando na raiz: %s\n", currentPath.absolute);
    printf("\n**Indo para o direito 'abra'\n");
    if(chdir2("./abra") == -1){//esse diretorio n existe mais
        printf("\nError change ./abra, esse diretorio n existe\n");
    }
    printf("Path atual: %s\n", currentPath.absolute);
    printf("\n**Indo para o direito ' '\n");
    if(chdir2(" ") == -1){//piccoli falou q tem q retornar erro mesmo
        printf("\nError change " """\n");
    }
    printf("Path atual: %s\n", currentPath.absolute);

    printf("\n**Indo para o direito '..dir1/ '\n");
    if(chdir2("../dir1") == -1){
        printf("\nError change ../dir1\n");
    }
    printf("Path atual: %s\n", currentPath.absolute);
    printf("\n***Folders do DIR1:\n");
    printFolders(currentPath.clusterNo);
    printf("\n**Indo para o diretorio 'abra2'\n");
    if(chdir2("abra2") == -1){
        printf("\nError change abra2\n");
    }
    printFolders(currentPath.clusterNo);

    printf("Path atual: %s\n", currentPath.absolute);
    if(chdir2(".") == -1){
        printf("\nError change .\n");
    }
    printf("\n\n**Indo para o direito '.'\n");
    printf("Path atual: %s\n", currentPath.absolute);

    printf("\n**Indo para o direito '..' -> voltar um\n");
    if(chdir2("..") == -1){
        printf("\nError change ..\n");
    }
    printf("Path atual: %s\n", currentPath.absolute);

    printf("\n**Indo para o direito '/' -> raiz\n");
    if(chdir2("/") == -1){
        printf("\nError change /\n");
    }
    printf("Path atual: %s\n", currentPath.absolute);
    printf("\n*****Deletando o abra2, que está dentro do dir1 apartir da raiz:\n");
    if(rmdir2("../dir1/abra2") == -1){
        printf("Error na deleção do abra2");
    }
    printDataCluster(11);
    if(rmdir2("./file1.txt") == -1){
        printf("\nDelete Dir só remove diretorios!\n");
    }
    printf("\nPath atual depois da delecao: %s\n", currentPath.absolute);
    printf("Indo para o direito dir1\n");
    if(chdir2("../dir1") == -1){
        printf("\nError change ../dir1\n");
    }
    printf("Path atual: %s\n", currentPath.absolute);
    printf("***Folders do DIR1:\n");    
    printFolders(currentPath.clusterNo);

    if(chdir2("../") == -1){
        printf("\nERROR 1\n");
    }

    printf("Path atual: %s\n", currentPath.absolute);
    if(chdir2("./file1.txt") == -1){
        printf("\nChange diretorio só funciona para diretorios!\n");
    }
    printf("Path atual: %s\n", currentPath.absolute);
/*
Fazer o teste tentando deletar um diretorio apontado por um link
*/
   printFolders(currentPath.clusterNo);
   printf("*****TESTANDO CRIAR UM LINK*********");
   if(ln2("./link2", "dir1") == -1){
       printf("ERRO NA CRIACAO DO LINK2");
   }
   printf("\nPRINT DO DIRETORIO RAIZ ONDE FOI CRIADO O LINK2");
   printFolders(currentPath.clusterNo);
   printf("\n*****");
   printf("\nOq tem no arquivo do link2: ");
   printDataCluster(11);
   printFAT(0);
   if(rmdir2("./link2") == -1){
       printf("\nEsse diretorio nao esta vazio\n");
   }
   printFAT(0);
   printf("\nDANDO UM CHANGE DIR PARA O DIRETORIO APONTANDO PELO LINK2");
   if(chdir2("./link2") == -1){
       printf("\nERRO NO CHANGE DIR LINK2");
   }
   printf("\nPath atual: %s\n", currentPath.absolute);
   printf("\nCRIANDO UM DIRETORIO ABRA NO DIRETORIO DIR1");
   if(mkdir2("/dir1/abrameu") == -1){
       printf("erro na criação");
   }
   printf("\nPRINT DO DIRETORIO 1 ONDE FOI CRIADO O ABRA\n");
   printFolders(currentPath.clusterNo);
   if(chdir2("/") == -1){
       printf("ERRO NA CHANGE PARA '/'");
   }
   printf("\nPath atual: %s\n", currentPath.absolute);
   printf("\nCRIANDO UM LINK APONTANDO PARA O ABRAMEU QUE ESTA DENTRO DO DIR1");
   if(ln2("./link3", "dir1/abrameu") == -1){
       printf("ERRO NA CRIACAO DO LINK3");
   }
   printf("\nPRINT DO DIRETORIO RAIZ ONDE FOI CRIADO O LINK3\n");
   printFolders(currentPath.clusterNo);

   if(chdir2("link3") == -1){
       printf("\nERRO NO CHANGE LINK3");
   }
   printf("\nPath atual: %s\n", currentPath.absolute);
   printf("\nPRINT DO DIRETORIO ONDE APONTA O LINK3, o ABRAMEU\n");
   printFolders(currentPath.clusterNo);

   if(chdir2("/dir1") == -1){
       printf("\nERRO NO CHANGE DIR 1 ");
   }
   printf("\nPath atual: %s\n", currentPath.absolute);
   printf("\nPRINT DO DIRETORIO 1\n");
   printFolders(currentPath.clusterNo);

   if(rmdir2("../link3") == -1){
       printf("ERRO NO DELETE DIR ../LINK3");
   }
   printf("\nDELECAO DO LINK3 QUE APONTA PARA UM DIRETORIO\n");
   printf("\nPath atual: %s\n", currentPath.absolute);
   printFolders(currentPath.clusterNo);

   if(rmdir2(".") == -1){
       printf("\nERROR: Nao pode deletar o diretorio .");
   }
   if(rmdir2("..") == -1){
       printf("\nERROR: Nao pode deletar o diretorio ..");
   }

   if(rmdir2("/") == -1){
       printf("ERROR: Nao pode deletar o diretorio raiz\n");
   }

}