#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/disk.h"
#include "../include/apidisk.h"
#include "../include/t2fs.h"

//gcc -o testeSoft softlinkTest.c ../src/disk.c ../src/t2fs.c ../lib/apidisk.o -Wall -ggdb && ./testeSoft
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


    DIR2 openD1,openD2,openD3;
    DIRENT2 directoryentry;
    DWORD value;
    int i;

    init_disk();
    fprintf(stderr,"ABRINDO DIRETORIOs\n\n");
    openD1=opendir2("/dir1");
    fprintf(stderr,"Retorno da funcao opendir procurando por dir1 = %d\n\n",openD1);

    openD2=opendir2("./dir1");
     fprintf(stderr,"Retorno da funcao opendir procurando por dir1 = %d\n\n",openD2);

    openD3=opendir2("/");
        fprintf(stderr,"Retorno da funcao opendir procurando por root(/) = %d\n\n",openD3);
    
    

    printOpenDirectories();
    

    fprintf(stderr,"FECHANDO DIRETORIOs\n\n");

       if(closedir2(openD2) != 0)
    fprintf(stderr,"Erro ao fechar handle %d\n\n", openD2);
        else
    fprintf(stderr,"Fechando handle %d\n\n", openD2);

    printOpenDirectories();

    fprintf(stderr,"\n\nAbrindo um root(/)\n\n");

    openD2=opendir2("/");

    printOpenDirectories();

int aux;
     fprintf(stderr,"\n\nCriando um softlink\n\n");
     aux=ln2("/sossosofo_file2.txt","/file2.txt");
  if(aux!= 0){
     fprintf(stderr,"\n\nERRO AO CRIAR SOFTLINK(/): %d\n\n", aux);
    }else{
        for(i=0;i<10;i++){
        if(readdir2(openD2,&directoryentry)==-1)
        fprintf(stderr,"Erro ao ler diretorio\n\n");
    else{
        fprintf(stderr,"Entry name: %s\n",directoryentry.name);
        fprintf(stderr,"Entry fileType: %x\n",directoryentry.fileType);
        fprintf(stderr,"Entry size: %x\n\n",directoryentry.fileSize);
    }    
    }
    }

         fprintf(stderr,"\n\nPrintando o softlink criado anteriormente para ROOT\n\n");
        printDataCluster(30);
         fprintf(stderr,"\n\nPrintando o softlink criado anteriormente para dir1\n\n");
        printDataCluster(29);
            fprintf(stderr,"\n\nPrintando o softlink criado anteriormente para /file1.txt\n\n");
        printDataCluster(32);
                    fprintf(stderr,"\n\nPrintando o softlink criado anteriormente para /file2.txt\n\n");
        printDataCluster(37);
    printFAT(0);
    readInFAT(30, &value);
    printf("\nCLUSTER 30 NA FAT APONTA PARA CLUSTER 2: %x\n\n",value);

        readInFAT(29, &value);
    printf("\nCLUSTER 29 NA FAT APONTA PARA CLUSTER 5: %x\n\n",value);
            readInFAT(32, &value);
    printf("\nCLUSTER 32 NA FAT APONTA PARA CLUSTER 3: %x\n\n",value);
            readInFAT(37, &value);
    printf("\nCLUSTER 37 NA FAT APONTA PARA CLUSTER 4: %x\n\n",value);
closedir2(openD1);
closedir2(openD2);
closedir2(openD3);


return 0;
}