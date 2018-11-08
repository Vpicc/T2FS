#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/apidisk.h"
#include "../include/disk.h"
#include "../include/t2fs.h"

int disk_initialized = 0;

DISK_FILE openFiles[10];

DWORD convertToDword(unsigned char* buffer) {
    return (DWORD) ((DWORD)buffer[0] | (DWORD)buffer[1] << 8 |(DWORD)buffer[2] << 16 |(DWORD)buffer[3] << 24 );
}

WORD convertToWord(unsigned char* buffer) {
    return (WORD) ((WORD)buffer[0] | (WORD)buffer[1] << 8);
}

unsigned char* wordToLtlEnd(WORD entry) {
    unsigned char* buffer = malloc(sizeof(unsigned char)*2);

    buffer[0] = entry;
    buffer[1] = entry >> 8;

    return buffer;
}

unsigned char* dwordToLtlEnd(DWORD entry) {
    unsigned char* buffer = malloc(sizeof(unsigned char)*4);

    buffer[0] = entry;
    buffer[1] = entry >> 8;
    buffer[2] = entry >> 16;
    buffer[3] = entry >> 24;

    return buffer;
}



int init_disk() {
    if(!disk_initialized){
        int i;
        unsigned char buffer[SECTOR_SIZE] = {0};
        
        if (read_sector(0,buffer) != 0) {
            return -1;
        }

        memcpy(superBlock.id, buffer, 4);
        superBlock.version = convertToWord(buffer + 4);
        superBlock.superblockSize = convertToWord(buffer + 6);
        superBlock.DiskSize = convertToDword(buffer + 8);
        superBlock.NofSectors = convertToDword(buffer + 12);
        superBlock.SectorsPerCluster = convertToDword(buffer + 16);
        superBlock.pFATSectorStart = convertToDword(buffer + 20);
        superBlock.RootDirCluster = convertToDword(buffer + 24);
        superBlock.DataSectorStart = convertToDword(buffer + 28); // K+1


        for (i = 0; i < 10; i++) {
            openFiles[i].file = -1;
            openFiles[i].currPointer = -1;
            strcpy(openFiles[i].name,"");
        }

        currentPath.absolute = malloc(sizeof(char)*5); // Valor inicial arbitrario
        strcpy(currentPath.absolute, "/");
        currentPath.clusterNo = superBlock.RootDirCluster;
        
        disk_initialized = 1;
        
    }
    return 0;
}


int writeInFAT(int clusterNo, DWORD value) {
    int offset = clusterNo/64;
    unsigned int sector = superBlock.pFATSectorStart + offset;
    int sectorOffset = (clusterNo % 64)*4;
    unsigned char buffer[SECTOR_SIZE] = {0};
    unsigned char* writeValue = malloc(sizeof(unsigned char)*4);
    DWORD badSectorCheck;

    if (value == 0x00000001) {
        return -1;
    }

    if (sector >= superBlock.pFATSectorStart && sector < superBlock.DataSectorStart) { // se nao acabou a FAT
        readInFAT(clusterNo, &badSectorCheck);
        if (badSectorCheck == BAD_SECTOR) { // checa se setor nao esta danificado
            return -1;
        }

        read_sector(sector,buffer);
        writeValue = dwordToLtlEnd(value);
        memcpy(buffer + sectorOffset, writeValue,4);
        write_sector(sector,buffer);
        
        return 0;
    }
    return -1;
}

int readInFAT(int clusterNo, DWORD* value) {
    int offset = clusterNo/64;
    unsigned int sector = superBlock.pFATSectorStart + offset;
    int sectorOffset = (clusterNo % 64)*4;
    unsigned char buffer[SECTOR_SIZE];

    if (sector >= superBlock.pFATSectorStart && sector < superBlock.DataSectorStart) { // se nao acabou a FAT
        read_sector(sector,buffer);
        *value = convertToDword(buffer + sectorOffset);
        return 0;
    }
    return -1;
}


int writeDataClusterFolder(int clusterNo, struct t2fs_record folder) {
    int i;
    int k = 0;
    int written = 0;
    unsigned int sectorToWrite;
    int clusterByteSize = sizeof(unsigned char)*SECTOR_SIZE*superBlock.SectorsPerCluster;
    unsigned char* buffer = malloc(clusterByteSize);
    unsigned int sector = superBlock.DataSectorStart + superBlock.SectorsPerCluster*clusterNo;
    
    if (sector >= superBlock.DataSectorStart && sector < superBlock.NofSectors) {
        readCluster(clusterNo, buffer);

        for(i = 0; i < clusterByteSize; i+= sizeof(struct t2fs_record)) {
            if ( ((BYTE) buffer[i]) == 0 && !written ) {
                memcpy(buffer + i,&(folder.TypeVal),1);
                memcpy((buffer + i + 1),folder.name,51);
                memcpy((buffer + i + 52),dwordToLtlEnd(folder.bytesFileSize),4);
                memcpy((buffer + i + 56),dwordToLtlEnd(folder.clustersFileSize),4);
                memcpy((buffer + i + 60),dwordToLtlEnd(folder.firstCluster),4);
                written = 1;
            } 
        }

        for(sectorToWrite = sector; sectorToWrite < (sector + superBlock.SectorsPerCluster); sectorToWrite++) {
            write_sector(sectorToWrite, buffer + k);
            k += 256;
        }

        if (written) {
            return 0;
        } else {
            return -1;
        }
    }
    return -1;
}

int readCluster(int clusterNo, unsigned char* buffer) {
    int i = 0;
    unsigned int sectorToRead;
    unsigned int sector = superBlock.DataSectorStart + superBlock.SectorsPerCluster*clusterNo;

    for(sectorToRead = sector; sectorToRead < (sector + superBlock.SectorsPerCluster); sectorToRead++) {
        read_sector(sectorToRead,buffer + i);
        i += 256;
    }
    return 0;
}

struct t2fs_record* readDataClusterFolder(int clusterNo) {
    int j;
    int folderSizeInBytes = sizeof(struct t2fs_record)*( (SECTOR_SIZE*superBlock.SectorsPerCluster) / sizeof(struct t2fs_record) );
    unsigned int sector = superBlock.DataSectorStart + superBlock.SectorsPerCluster*clusterNo;
    unsigned char* buffer = malloc(sizeof(unsigned char)*SECTOR_SIZE*superBlock.SectorsPerCluster);
    struct t2fs_record* folderContent = malloc(folderSizeInBytes);

    if (sector >= superBlock.DataSectorStart && sector < superBlock.NofSectors) {
        readCluster(clusterNo, buffer);

        for(j = 0; j < folderSizeInBytes/sizeof(struct t2fs_record); j++) {
            folderContent[j].TypeVal = (BYTE) *(buffer + sizeof(struct t2fs_record)*j);
            memcpy(folderContent[j].name, buffer + 1 + sizeof(struct t2fs_record)*j, 51);
            folderContent[j].bytesFileSize = convertToDword(buffer + 52 + sizeof(struct t2fs_record)*j);
            folderContent[j].clustersFileSize = convertToDword(buffer + 56 + sizeof(struct t2fs_record)*j);
            folderContent[j].firstCluster = convertToDword(buffer + 60 + sizeof(struct t2fs_record)*j);
        }
        return folderContent;
    }
    return NULL;
}

unsigned char* readDataCluster (int clusterNo){
    unsigned int sector = superBlock.DataSectorStart + superBlock.SectorsPerCluster*clusterNo;
    unsigned char* buffer = malloc(sizeof(unsigned char)*SECTOR_SIZE*superBlock.SectorsPerCluster); 
    if (sector >= superBlock.DataSectorStart && sector < superBlock.NofSectors) {
        readCluster(clusterNo, buffer);
        return buffer;
    }
    return NULL;
}

int writeCluster(int clusterNo, unsigned char* buffer, int position, int size) {
    int i;
    int j;
    int k = 0;
    unsigned int sectorToWrite;
    unsigned int sector = superBlock.DataSectorStart + superBlock.SectorsPerCluster*clusterNo;
    unsigned char* newBuffer = malloc(sizeof(unsigned char)*SECTOR_SIZE*superBlock.SectorsPerCluster);

    if (size > SECTOR_SIZE*superBlock.SectorsPerCluster || (position + size) > SECTOR_SIZE*superBlock.SectorsPerCluster) {
        return -1;
    }

    readCluster(clusterNo, newBuffer);

    for(j = position; j < size + position; j++){
        newBuffer[j] = buffer[j - position];
    }

    for(i = position + size; i < SECTOR_SIZE*superBlock.SectorsPerCluster; i++){
        newBuffer[i] = '\0';
    }
    for(sectorToWrite = sector; sectorToWrite < (sector + superBlock.SectorsPerCluster); sectorToWrite++) {
        write_sector(sectorToWrite, newBuffer + k);
        k += 256;
    }
    return position + size;
}

int pathToCluster(char* path) {
    int i;
    int found = 0;
    int pathsNo = 0;
    int folderInPath = 1;
    int pathComplete = 0;
    unsigned int currentCluster;
    char* pathTok;
    char* pathcpy = malloc(sizeof(char)*(strlen(path)+1));
    int folderSize = ( (SECTOR_SIZE*superBlock.SectorsPerCluster) / sizeof(struct t2fs_record) );
    struct t2fs_record* folderContent = malloc(sizeof(struct t2fs_record)*( (SECTOR_SIZE*superBlock.SectorsPerCluster) / sizeof(struct t2fs_record) ));

    strcpy(pathcpy,path);

    if (pathcpy[0] == '/') {
        currentCluster = superBlock.RootDirCluster;
    }else {
        currentCluster = currentPath.clusterNo;
    }

    pathTok = strtok(pathcpy,"/");

    while(pathTok != NULL && pathsNo == found && folderInPath) {
        pathsNo += 1;
        folderContent = readDataClusterFolder(currentCluster);
        for(i = 0; i < folderSize; i++) {
            if (strcmp(folderContent[i].name,pathTok) == 0) {
                currentCluster = folderContent[i].firstCluster;
                found += 1;
                if (folderContent[i].TypeVal != TYPEVAL_DIRETORIO) {
                    folderInPath = 0;
                }
            }
        }
        pathTok = strtok(NULL,"/");
        if (pathTok == NULL) {
            pathComplete = 1;
        }
    }

    if (pathsNo != found) {
        return -1;
    }

    if (!pathComplete) {
        return -1;
    }

    return currentCluster;
}

int findFATOpenCluster(int* clusterReturn) { // deixei assim, caso usarmos cluster como unsigned int no futuro
    int functionReturn = 0;
    int clusterNo = 1;
    DWORD value = BAD_SECTOR;
    while(functionReturn == 0 && value != 0) {
        clusterNo += 1;
        functionReturn = readInFAT(clusterNo,&value);
    }
    if(functionReturn == -1) {
        *clusterReturn = -1;
    } else {
        *clusterReturn = clusterNo;
    }
    return functionReturn;
}

int tokenizePath(char* path, char*** tokenized) {
    int i;
    int countFolders = 1;
    char * pathcpy = malloc(sizeof(char)*(strlen(path)+1));
    char * pathTok;

    strcpy(pathcpy, path);

    pathTok = strtok(pathcpy,"/");

    while(pathTok != NULL) {
        pathTok = strtok(NULL,"/");
        if (pathTok != NULL) {
            countFolders += 1;
        }
    }

    *tokenized = malloc(sizeof(char*)*countFolders);

    strcpy(pathcpy, path);

    pathTok = strtok(pathcpy,"/");

    i = 0;
    while(pathTok != NULL) {
        (*tokenized)[i] = malloc(sizeof(char)*strlen(pathTok));
        strcpy((*tokenized)[i], pathTok);
        pathTok = strtok(NULL,"/");
        i += 1;
    }

    return countFolders;

}

int toAbsolutePath(char * path, char * currPath, char ** output) {
    int i;
    int numTokens;
    char ** tokenizedPath;
    char * cutToken;
    int bufferSize = (strlen(path) + 1 + strlen(currPath) + 1);
    char * buffer = malloc(sizeof(char)*bufferSize);
    char * pathcpy = malloc(sizeof(char)*(strlen(path) + 1));

    strcpy(pathcpy,path);

    if(pathcpy[0] == '/'){
        *output = malloc(sizeof(char)*(strlen(path)+ 1));
        strcpy(*output,path);
        return 0;
    }

    strcpy(buffer,currPath);

    numTokens = tokenizePath(pathcpy, &tokenizedPath);

    for(i = 0; i < numTokens; i++) {
        if (strcmp(tokenizedPath[i],"..") == 0) {
            if(strcmp(buffer,"/") != 0){
                cutToken = strrchr(buffer, '/');
                *cutToken = '\0';
            }
            if(strcmp(buffer,"") == 0) {
                strcpy(buffer,"/");
            }
        } else if (strcmp(tokenizedPath[i],".") != 0) {
            strcat(buffer,"/");
            strcat(buffer,tokenizedPath[i]);
        }
    }

    *output = malloc(sizeof(char)*(strlen(buffer)+ 1));

    strcpy(*output, buffer);

    free(buffer);

    return 0;

}
/*
* INPUT:/aaa/ccc/aa/bb OUT: /aaa/ccc/aa AND bb
*/
int separatePath(char * path, char ** FristStringOutput, char ** SecondStringOutput) {
    char *aux;
    const char dir_div = '/';
    int lenghtAux;
    int lenghtPath = strlen(path);
    //Nunca vão ter um tamanho maior que o path
    *SecondStringOutput = malloc(lenghtPath);
    memset(*SecondStringOutput,'\0',lenghtPath);
    *FristStringOutput = malloc(lenghtPath);
    memset(*FristStringOutput,'\0',lenghtPath);

    aux = strrchr(path, dir_div);
    lenghtAux = strlen(aux);
    memcpy(*SecondStringOutput,aux+1,lenghtAux);
    memcpy(*FristStringOutput, path, lenghtPath-lenghtAux);
    return 0;
}
