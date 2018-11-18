#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/apidisk.h"
#include "../include/disk.h"
#include "../include/t2fs.h"

int disk_initialized = 0;

//aa

DISK_DIR openDirectories[10];

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
            openFiles[i].clusterNo = -1;
            openDirectories[i].handle = -1;
            openDirectories[i].noReads=-1;
            openDirectories[i].clusterDir= -1;
            openDirectories[i].directory=setNullDirent();
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

        free(writeValue);

        
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
        free(buffer);
        if (written) {
            return 0;
        } else {
            return -1;
        }
    }
    free(buffer);
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
        free(buffer);
        return folderContent;
    }
    free(buffer);
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

    for(sectorToWrite = sector; sectorToWrite < (sector + superBlock.SectorsPerCluster); sectorToWrite++) {
        write_sector(sectorToWrite, newBuffer + k);
        k += 256;
    }
    free(newBuffer);
    return position + size;
}

int truncateCluster(int clusterNo, int position) {
    int i;
    int k = 0;
    unsigned int sectorToWrite;
    unsigned int sector = superBlock.DataSectorStart + superBlock.SectorsPerCluster*clusterNo;
    unsigned char* newBuffer = malloc(sizeof(unsigned char)*SECTOR_SIZE*superBlock.SectorsPerCluster);
    
    readCluster(clusterNo, newBuffer);

    for(i = position; i < SECTOR_SIZE*superBlock.SectorsPerCluster; i++){
        newBuffer[i] = '\0';
    }
    for(sectorToWrite = sector; sectorToWrite < (sector + superBlock.SectorsPerCluster); sectorToWrite++) {
        write_sector(sectorToWrite, newBuffer + k);
        k += 256;
    }
    free(newBuffer);

    return 0;
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

    if (strcmp(pathcpy,"/") == 0) {
        return superBlock.RootDirCluster;
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
        free(pathcpy);
        free(folderContent);
        return -1;
    }

    if (!pathComplete) {
        free(pathcpy);
        free(folderContent);
        return -1;
    }
    free(pathcpy);
    free(folderContent);
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
    free(pathcpy);
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
        buffer[0] = '\0';
    } else {
        strcpy(buffer,currPath);
    }

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
        } else{ 
            if (strcmp(tokenizedPath[i],".") != 0) {
                if(strcmp(buffer,"/") != 0){
                    strcat(buffer,"/");
                }
                strcat(buffer,tokenizedPath[i]);
            }
        }
    }

    *output = malloc(sizeof(char)*(strlen(buffer)+ 1));

    strcpy(*output, buffer);

    free(buffer);
    free(pathcpy);

    return 0;

}
/*
* INPUT:/aaa/ccc/aa/bb OUT: /aaa/ccc/aa/ AND bb
*/
int separatePath(char * path, char ** FristStringOutput, char ** SecondStringOutput) {
    const char dir_div = '/';
    int lenghtAux;
    int lenghtPath = strlen(path);
    char *aux =  malloc(lenghtPath);
    //Nunca vão ter um tamanho maior que o path
    *SecondStringOutput = malloc(lenghtPath);
    memset(*SecondStringOutput,'\0',lenghtPath);
    *FristStringOutput = malloc(lenghtPath);
    memset(*FristStringOutput,'\0',lenghtPath);

    aux = strrchr(path, dir_div);
    lenghtAux = strlen(aux);
    memcpy(*SecondStringOutput,aux+1,lenghtAux);
    memcpy(*FristStringOutput, path, lenghtPath-lenghtAux);
    strcat(*FristStringOutput,"/");
    return 0;
}
int changeDir(char * path){

    char * absolute;
    char * firstOut;
    char * secondOut;
    int clusterNewPath;
    char *linkOutput;

//Variaveis para a validação do tipo:
    int i;
    int isDir = 0;
    int clusterByteSize = sizeof(unsigned char)*SECTOR_SIZE*superBlock.SectorsPerCluster;
    unsigned char* buffer = malloc(clusterByteSize);
    int clusterOfDir;

    if(strlen(path) == 0){ //Se a string for vazia n altera o lugar
        return 0;
    }
    if(strcmp(path,"/") == 0){ // "/"" vai para a RAIZ
        currentPath.clusterNo = superBlock.RootDirCluster;
        free(currentPath.absolute);
        currentPath.absolute = malloc(sizeof(char)*2);
        //memset(currentPath.absolute, '/0', 2);
        strcpy(currentPath.absolute,"/");
        return 0;
    }
    //faço depois da comparação do path com vazio e "/", pq se nao tava dando segmentation..

        if(link(path, &linkOutput) == -1)
            return -1;

    if(toAbsolutePath(linkOutput, currentPath.absolute, &absolute) == -1){

        free(absolute);
        return -1;
    }

    if(separatePath(absolute, &firstOut, &secondOut) == -1){
        return -2;
    }

    clusterOfDir = pathToCluster(firstOut);

    readCluster(clusterOfDir, buffer);
    if(strlen(secondOut) > 0){
        for(i = 0; i < clusterByteSize; i+= sizeof(struct t2fs_record)) {
            if ( (strcmp((char *)buffer+i+1, secondOut) == 0) && (((BYTE) buffer[i]) == TYPEVAL_DIRETORIO) && !isDir ) {
                isDir = 4;
            } 
        }
        if(isDir == 0){
            return -3;
        }
    }
//se o absoluto do atual com o path for /, então é pq é o ROOT.
    if(strlen(absolute)== 1 && absolute[0] == '/'){
        clusterNewPath = superBlock.RootDirCluster;
    }
    else{
        clusterNewPath = pathToCluster(absolute);
    }

    if(clusterNewPath == -1){//se o pathname n existir
        free(absolute);
        return -5;
    }

    free(currentPath.absolute);
    currentPath.absolute = malloc(sizeof(char)*(strlen(absolute)+1));
    strcpy(currentPath.absolute, absolute);
    currentPath.clusterNo = clusterNewPath;

    free(absolute);
    
    return 0;    
}
int mkdir(char * path){

    char * absolute;
    char * firstOut;
    char * secondOut;
    int firstClusterFreeInFAT;
    int clusterDotDot;
    toAbsolutePath(path, currentPath.absolute, &absolute);
    separatePath(absolute, &firstOut, &secondOut);

    if(findFATOpenCluster(&firstClusterFreeInFAT) == -1){//se n achar um cluster livre na fat
        free(absolute);
        free(firstOut);
        free(secondOut);
        return -1;
    }
    if(strlen(secondOut) == 0){//diretorio sem nome
        free(absolute);
        free(firstOut);
        free(secondOut);
        return -1;
    }
    if(!(isRightName(secondOut))){//diretorios n podem ter esse nome
        free(absolute);
        free(firstOut);
        free(secondOut);
        return -1;
    }

//se o firstOut do absolute for '/', então DotDot vai ser o raiz
    if(strlen(firstOut) == 1 && firstOut[0]== '/'){
        clusterDotDot = superBlock.RootDirCluster;
    }
    else//se nao for, então tem q achar onde fica
        {
            clusterDotDot = pathToCluster(firstOut);
            if(clusterDotDot == -1){
                free(absolute);
                free(firstOut);
                free(secondOut);
                return -1;
            }
        }

    if(isInCluster(clusterDotDot, secondOut, TYPEVAL_DIRETORIO)){
        free(absolute);
        free(firstOut);
        free(secondOut);
        return -1;
    }

    struct t2fs_record one_dot;
    struct t2fs_record two_dot;
    struct t2fs_record folder;


    one_dot.TypeVal = TYPEVAL_DIRETORIO;
    strcpy(one_dot.name, ".");
    one_dot.bytesFileSize = SECTOR_SIZE*superBlock.SectorsPerCluster;
    one_dot.clustersFileSize = 1;
    one_dot.firstCluster = firstClusterFreeInFAT;
    writeDataClusterFolder(firstClusterFreeInFAT, one_dot);//vai ter sempre lugar pq o cluster estava vazio


    two_dot.TypeVal = TYPEVAL_DIRETORIO;
    strcpy(two_dot.name, "..");
    two_dot.bytesFileSize = SECTOR_SIZE*superBlock.SectorsPerCluster;
    two_dot.clustersFileSize = 1;
    two_dot.firstCluster = clusterDotDot;
    writeDataClusterFolder(firstClusterFreeInFAT, two_dot);//vai ter sempre lugar pq o cluster estava vazio


    folder.TypeVal = TYPEVAL_DIRETORIO;
    strcpy(folder.name, secondOut);
    folder.bytesFileSize = SECTOR_SIZE*superBlock.SectorsPerCluster;;
    folder.clustersFileSize = 1;
    folder.firstCluster = firstClusterFreeInFAT;
    if(writeDataClusterFolder(clusterDotDot, folder) == -1){//se n estiver lugar para escrever dentro da folder
        return -1;
    }

    writeInFAT(firstClusterFreeInFAT, END_OF_FILE);

    free(absolute);
    free(firstOut);
    free(secondOut);

    return 0;
}
int isEmptyDir(int clusterNo){
    int i;
    int folderSize = ( (SECTOR_SIZE*superBlock.SectorsPerCluster) / sizeof(struct t2fs_record) );
    struct t2fs_record* folderContent = malloc(sizeof(struct t2fs_record)*( (SECTOR_SIZE*superBlock.SectorsPerCluster) / sizeof(struct t2fs_record) ));
    folderContent = readDataClusterFolder(clusterNo);
    unsigned int tam=  0;
    for(i = 0; i < folderSize; i++) {
        if(strlen(folderContent[i].name) > 0){
            tam++;
        }
    }
    // 2 é pq ele só pode encontrar o . e o ..
    if(tam > 2){//is empty
        return 0;
    }
    else
        return 1;
}
int deleteDir(char * path){
    char * absolute;
    char * firstOut;
    char * secondOut;
    unsigned int clusterDirFather;
    unsigned int clusterDir;
    int sucess = 0;
    char *linkOutput;
    unsigned char* bufferWithNulls = malloc(sizeof(unsigned char)*SECTOR_SIZE*superBlock.SectorsPerCluster);
    
    //buffer usado para a delecao
    memset(bufferWithNulls,'\0',SECTOR_SIZE*superBlock.SectorsPerCluster);// coloca /0 em todo o buffer

    if(strlen(path) == 0){ //Se a string for vazia n tem q deletar
        return -1;
    }


    if(link(path, &linkOutput)== -1)
            return -1; 

    if(toAbsolutePath(linkOutput, currentPath.absolute, &absolute)== -1){
        return -1;
    }
    if(separatePath(absolute, &firstOut, &secondOut)==-1){//secondOut tem o nome da pasta que tem q apagar
        return -1;
    }
    if(!isRightName(secondOut)){ // "/", ".", ".." da erro
        return -1;
    }

//diretorio que quer apagar
    if((clusterDir = pathToCluster(linkOutput)) == -1){
        return -1;
    }

    if(isEmptyDir(clusterDir) && clusterDir != superBlock.RootDirCluster){
        //diretorio que pertence o diretorio que quer apagar
        if((clusterDirFather = pathToCluster(firstOut)) == - 1){
            return -1;
        }
        int i;
        int folderSize = ( (SECTOR_SIZE*superBlock.SectorsPerCluster) / sizeof(struct t2fs_record) );
        struct t2fs_record* folderContent = malloc(sizeof(struct t2fs_record)*( (SECTOR_SIZE*superBlock.SectorsPerCluster) / sizeof(struct t2fs_record) ));
        folderContent = readDataClusterFolder(clusterDirFather);
        for(i = 0; i < folderSize; i++) {
            if((strcmp(folderContent[i].name, secondOut) == 0) && (folderContent[i].TypeVal == TYPEVAL_DIRETORIO)){
                    folderContent[i].TypeVal = TYPEVAL_INVALIDO;
                    strcpy(folderContent[i].name, "\0");
                    folderContent[i].bytesFileSize = 0;
                    folderContent[i].clustersFileSize = 0;
                    folderContent[i].firstCluster = 0;
                    writeInFAT(clusterDir, 0);
                    writeZeroClusterFolderByName(clusterDirFather, folderContent[i], secondOut, TYPEVAL_DIRETORIO);              
                    writeCluster(clusterDir,bufferWithNulls,0,SECTOR_SIZE*superBlock.SectorsPerCluster);                    
                    sucess = 1;
            }
        }
        if(sucess){
            return 0;
        }
        else{
            return -1;
        }
    }
    return -1;
}

//O fileName n pode ser path
int writeZeroClusterFolderByName(int clusterNo, struct t2fs_record folder, char * fileName, BYTE TypeValEntrada) {
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
            if ( (strcmp((char *)buffer+i+1, fileName) == 0) && (((BYTE) buffer[i]) == TypeValEntrada) && !written ) {
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
        free(buffer);
        if (written) {
            return 0;
        } else {
            return -1;
        }
    }
    free(buffer);
    return -1;
}
int isInCluster(int clusterNo, char * fileName, BYTE TypeValEntrada) {
    int i;
    int wasFound = 0;
    int clusterByteSize = sizeof(unsigned char)*SECTOR_SIZE*superBlock.SectorsPerCluster;
    unsigned char* buffer = malloc(clusterByteSize);
    unsigned int sector = superBlock.DataSectorStart + superBlock.SectorsPerCluster*clusterNo;
    
    if (sector >= superBlock.DataSectorStart && sector < superBlock.NofSectors) {
        readCluster(clusterNo, buffer);

        for(i = 0; i < clusterByteSize; i+= sizeof(struct t2fs_record)) {
            if ( (strcmp((char *)buffer+i+1, fileName) == 0) && (((BYTE) buffer[i]) == TypeValEntrada) && !wasFound ) {
                wasFound = 1;
            } 
        }
        free(buffer);
        if (wasFound) {
            return 1;
        } else {
            return 0;
        }
    }
    free(buffer);
    return 0;
}

int isRightName(char * name){
    if(strcmp(name, ".") == 0){
        return 0;
    }
    if(strcmp(name, "..") == 0){
        return 0;
    }
    if(name[0] == '/'){
        return 0;
    }

    return 1;
}



DIRENT2 setNullDirent()
{
    DIRENT2 dir;
    strcpy(dir.name,"");
    dir.fileType=(DWORD)6;
    dir.fileSize=(DWORD)0;

    return dir;
}

DIRENT2 searchDirByHandle(DIR2 handle){

    int i;
    struct t2fs_record* folderContent = malloc(sizeof(struct t2fs_record)*( (SECTOR_SIZE*superBlock.SectorsPerCluster) / sizeof(struct t2fs_record) ));
    int folderSize = ( (SECTOR_SIZE*superBlock.SectorsPerCluster) / sizeof(struct t2fs_record) );

    for(i=0;i<10;i++){
        if(openDirectories[i].handle==handle){
            
            folderContent=readDataClusterFolder(openDirectories[i].clusterDir);
            if(openDirectories[i].noReads<folderSize){
                openDirectories[i].directory.fileSize=folderContent[openDirectories[i].noReads].bytesFileSize;
                openDirectories[i].directory.fileType=folderContent[openDirectories[i].noReads].TypeVal;
                strcpy(openDirectories[i].directory.name,folderContent[openDirectories[i].noReads].name);
                openDirectories[i].noReads++;
            return openDirectories[i].directory;
            }
        }
    }
    return setNullDirent();
}


void setCurrentPathToRoot(){
    strcpy(currentPath.absolute,"/");
    currentPath.clusterNo=superBlock.RootDirCluster;
}
DWORD getTypeVal(char *absolute){

    char *firstOut;
    char *secondOut;
    struct t2fs_record* folderContent = malloc(sizeof(struct t2fs_record)*( (SECTOR_SIZE*superBlock.SectorsPerCluster) / sizeof(struct t2fs_record) ));
    int folderSize = ( (SECTOR_SIZE*superBlock.SectorsPerCluster) / sizeof(struct t2fs_record) );
    int i;
    int clusterDir;

    separatePath(absolute, &firstOut, &secondOut);

    clusterDir=pathToCluster(firstOut);
    folderContent=readDataClusterFolder(clusterDir);
    for(i=0;i<folderSize;i++){
        if(strcmp(folderContent[i].name,secondOut) ==0 ){
            return folderContent[i].TypeVal;
        }
    }
return TYPEVAL_INVALIDO;

}

DIR2 openDir(char *path){
    int i;
    char *absolute=malloc(sizeof(char)*2);
    int dirCluster;
    char *linkOutput;
    int retornoLink;

    if(strcmp(path,"/") == 0){
        strcpy(absolute,"/");
    }else{
        retornoLink=link(path, &linkOutput);
        if(retornoLink == -1)
            return -1; 
        if(retornoLink < 0){
            return -4;
        }else if(retornoLink ==1){
            if(toAbsolutePath(linkOutput, currentPath.absolute, &absolute)){
            return -1;
            } 
        }else{
            if(toAbsolutePath(path, currentPath.absolute, &absolute) == -1)
            return -2;
        }
        //checking type
        //fprintf(stderr,"\n\nABSOLUTE NO OPENDIR: %s\n\n",absolute);
        if(getTypeVal(absolute) != TYPEVAL_DIRETORIO){
            return -5;
        }
    }
    
     dirCluster=pathToCluster(absolute);
    for(i=0;i<10;i++){
        if(openDirectories[i].handle == -1){
            openDirectories[i].handle = i;
            openDirectories[i].noReads=0;
            if(strcmp(absolute,"/") == 0 || strcmp(path,"/") ==0 ){
            openDirectories[i].clusterDir=superBlock.RootDirCluster;
            }
            else{
                openDirectories[i].clusterDir=dirCluster;
            }
            return openDirectories[i].handle;
        }
    }
    return -1;
}

void printOpenDirectories(){
    int i;
    printf("\nLista de diretorios abertos:\n\n");
    for(i=0;i<10;i++)
        if(openDirectories[i].handle!=-1)
            fprintf(stderr,"HANDLE:%d: \tCLUSTER:%d\n",openDirectories[i].handle,openDirectories[i].clusterDir);
}

void freeOpenDirectory(DISK_DIR *opendirectory){
    opendirectory->handle=-1;
    opendirectory->noReads=-1;
    opendirectory->clusterDir=-1;
    opendirectory->directory=setNullDirent();
}

int closeDir(DIR2 handle){
    int i;

    for(i=0;i<10;i++){
        if(openDirectories[i].handle==handle){
            freeOpenDirectory(&openDirectories[openDirectories[i].handle]);
            return 0;
        }
    }
    return -1;

}

FILE2 createFile(char * filename){
    char * absolute;
    char * firstOut;
    char * secondOut;
    int firstClusterFreeInFAT;
    int handle;
    handle = makeAnewHandle();
    int clusterToRecordFile;
    char *linkOutput;
    

    if(link(filename, &linkOutput)== -1)
            return -1; 

    if(toAbsolutePath(linkOutput, currentPath.absolute, &absolute)){
        //printf("\nERRO INESPERADO\n");//se der erro aqui eu n sei pq, tem q ver ainda
        return -1;
    }

    if(separatePath(absolute, &firstOut, &secondOut)){
        //printf("\nERRO INESPERADo\n");//se der erro aqui eu n sei pq, tem q ver ainda
        return -1;
    }

    if(!isRightName(secondOut)){
        return -1;
    }  

    clusterToRecordFile = pathToCluster(firstOut);
//caminho inexistente
    if(clusterToRecordFile == -1){
        return -1;
    }

//arquivo sem nome
    if(strlen(secondOut) == 0){
        free(absolute);
        free(firstOut);
        free(secondOut);
        return -1;
    }
//diretorios n podem ter esse nome
    if(!(isRightName(secondOut))){
        free(absolute);
        free(firstOut);
        free(secondOut);
        return -1;
    }
//n tinha espaço para adicionar um novo arquivos
    if(handle == -1){
        free(absolute);
        free(firstOut);
        free(secondOut);
        return -1; 
    }

//se ja tiver um arquivo com esse nome nesse diretorio
//TODO: TEM Q APGAR O TEM E COLOCAR O NOVO.
    if(isInCluster(clusterToRecordFile, secondOut, TYPEVAL_REGULAR)){
        deleteFile(filename);
    }

//se n achar um cluster livre na fat
    if(findFATOpenCluster(&firstClusterFreeInFAT) == -1){
        free(absolute);
        free(firstOut);
        free(secondOut);
        return -1;
    }

//criação das estruturas
    struct t2fs_record toRecord;

//declaração de seus atributos
    toRecord.TypeVal = TYPEVAL_REGULAR;
    strcpy(toRecord.name, secondOut);
    toRecord.bytesFileSize = 0;
    toRecord.clustersFileSize = 1;
    toRecord.firstCluster = firstClusterFreeInFAT;

//escrita no diretorio
    if(writeDataClusterFolder(clusterToRecordFile, toRecord) == - 1){//se n tiver espaço na folder
        return -1;
    }
//marcação na fat de cluster ocupado
    writeInFAT(firstClusterFreeInFAT, END_OF_FILE);

//retorna o handle já colocando ele no array de opens
    return (openFile (filename));
}

int makeAnewHandle(){
    int i;
    
    for(i = 0; i < MAX_NUM_FILES; i++){
        if(openFiles[i].file == -1){
            return (i+1);
        }
    }
    //se chegou até aqui é pq n encontrou nenhuma posição no array de 10 para botar um novo arquivo
    return -1;
}

FILE2 openFile (char * filename){
    char * absolute;
    char * firstOut;
    char * secondOut;
    //int firstClusterFreeInFAT;
    int handle;
    handle = makeAnewHandle();
    int firstClusterOfFile;
    char *linkOutput;

    int i;
    int isFile= 0;
    int clusterByteSize = sizeof(unsigned char)*SECTOR_SIZE*superBlock.SectorsPerCluster;
    unsigned char* buffer = malloc(clusterByteSize);
    int clusterOfDir;

    if(link(filename, &linkOutput)== -1)
            return -1; 

    if(toAbsolutePath(linkOutput, currentPath.absolute, &absolute)){
        //printf("\nERRO INESPERADO\n");//se der erro aqui eu n sei pq, tem q ver ainda
        return -2;
    }

    if(separatePath(absolute, &firstOut, &secondOut)){
        //printf("\nERRO INESPERADo\n");//se der erro aqui eu n sei pq, tem q ver ainda
        return -2;
    }

    if(!isRightName(secondOut)){ 
        return -1;
    }  
//verificação
    clusterOfDir = pathToCluster(firstOut);

    readCluster(clusterOfDir, buffer);
    if(strlen(secondOut) > 0){
        for(i = 0; i < clusterByteSize; i+= sizeof(struct t2fs_record)) {
            if ( (strcmp((char *)buffer+i+1, secondOut) == 0) && (((BYTE) buffer[i]) == TYPEVAL_REGULAR) && !isFile ) {
                isFile = 1;
            } 
        }
        if(isFile == 0){
            return -3;
        }
    }
//fim da verificação

    firstClusterOfFile = pathToCluster(absolute);
//caminho inexistente
    if(firstClusterOfFile == -1){
        return -4;
    }
//n tinha espaço para adicionar um novo arquivos
    if(handle == -1){
        free(absolute);
        free(firstOut);
        free(secondOut);
        return -5; 
    }
    struct diskf newFileToRecord;
   
    newFileToRecord.clusterNo = firstClusterOfFile;
    newFileToRecord.currPointer = 0;
    newFileToRecord.file = handle;
    //adicionei essas linhas pois uso o path na hora de saber o tamanho do arquivo - SAMUEL
    newFileToRecord.clusterDir=pathToCluster(firstOut);

//atualização do openFiles
    memcpy(&openFiles[handle-1], &newFileToRecord, sizeof(struct diskf));
    
    return newFileToRecord.file;
}

int closeFile(FILE2 handle){
    int i;
    for(i = 0; i < MAX_NUM_FILES; i++){
        if(openFiles[i].file == handle){//então tava aberto
            openFiles[i].file = -1;
            openFiles[i].clusterNo = -1;
            openFiles[i].currPointer = -1;
            return 0;
        }
    }
    return -1;
}

int deleteFile(char * filename){

    char * absolute;
    char * firstOut;
    char * secondOut;
    int clusterOfDir;//cluster que contem o diretorio que contem o arquivo
    int clusterToDelete;//cluster que tem q apagar
    unsigned char* bufferWithNulls = malloc(sizeof(unsigned char)*SECTOR_SIZE*superBlock.SectorsPerCluster);
    DWORD FATrepresentation = 0;
    BYTE typeToDelete = TYPEVAL_REGULAR;
    char *linkOutput;
    //variaveis para a verificação
    int i;
    int isFile = 0;
    int clusterByteSize = sizeof(unsigned char)*SECTOR_SIZE*superBlock.SectorsPerCluster;
    unsigned char* buffer = malloc(clusterByteSize);
    //

    if(link(filename, &linkOutput)== -1)
            return -1; 

    if(link(filename, &linkOutput)== 1){
        typeToDelete = TYPEVAL_LINK;
    }

    memset(bufferWithNulls,'\0',SECTOR_SIZE*superBlock.SectorsPerCluster);// coloca /0 em todo o buffer

    if(toAbsolutePath(filename, currentPath.absolute, &absolute)){
        //printf("\nERRO INESPERADO\n");//se der erro aqui eu n sei pq, tem q ver ainda
        free(absolute);
        free(firstOut);
        free(secondOut);
        return -1;
    }

    if(separatePath(absolute, &firstOut, &secondOut)){
        //printf("\nERRO INESPERADo\n");//se der erro aqui eu n sei pq, tem q ver ainda
        free(absolute);
        free(firstOut);
        free(secondOut);
        return -1;
    }

    if(!isRightName(secondOut)){
        return -1;
    }  

    if((clusterOfDir = pathToCluster(firstOut)) == -1){
        free(absolute);
        free(firstOut);
        free(secondOut);
         return -1;
    }

    if((clusterToDelete = pathToCluster(absolute))== -1){
        free(absolute);
        free(firstOut);
        free(secondOut);
        return -1;
    }

//Verificação de se é um TYPE FILE mesmo
    readCluster(clusterOfDir, buffer);
    for(i = 0; i < clusterByteSize; i+= sizeof(struct t2fs_record)) {
        if ( (strcmp((char *)buffer+i+1, secondOut) == 0) && (((BYTE) buffer[i]) == typeToDelete) && !isFile ) {
            isFile = 1;
        } 
    }
    //se n for do tipo File, n pode deletar.
    if(isFile == 0){
        free(absolute);
        free(firstOut);
        free(secondOut);
        return -1;
    }
//Fim da verificação

    struct t2fs_record folderContent;

    folderContent.TypeVal = TYPEVAL_INVALIDO;
    strcpy(folderContent.name, "\0");
    folderContent.bytesFileSize = 0;
    folderContent.clustersFileSize = 0;
    folderContent.firstCluster = 0;

    if(writeZeroClusterFolderByName(clusterOfDir, folderContent, secondOut, typeToDelete) == -1){
        free(absolute);
        free(firstOut);
        free(secondOut);
        return -1;
    }

    closeFileByFristCluster(clusterToDelete);

    while( FATrepresentation != END_OF_FILE && FATrepresentation != BAD_SECTOR){

        readInFAT(clusterToDelete,&FATrepresentation);//le o proximo cluster que tem conteudo do arquivo
        writeInFAT(clusterToDelete, 0);//marca 0 naquela represetanção, pra libera-lá

        //escreve o buffer cheio de nulls dentro daquele cluster q tinha o arquivo.
        writeCluster(clusterToDelete,bufferWithNulls,0,SECTOR_SIZE*superBlock.SectorsPerCluster);

        //atualiza o novo cluster
        if(FATrepresentation != END_OF_FILE && FATrepresentation != BAD_SECTOR){
            clusterToDelete = (int) FATrepresentation;
        }

    }
    free(absolute);
    free(firstOut);
    free(secondOut);
    return 0;
}

int closeFileByFristCluster(int clusterToClose){
    int i;
    for(i = 0; i < MAX_NUM_FILES; i++){
        if(openFiles[i].clusterNo == clusterToClose){//então tava aberto
            openFiles[i].file = -1;
            openFiles[i].clusterNo = -1;
            openFiles[i].currPointer = -1;
            return 0;
        }
    }
    return -1;
}

void printOpenFiles(){
    int i;
    printf("\nLista de arquivos abertos:");
    for(i=0;i<10;i++){
        if(openFiles[i].file != -1){
            printf("\nArquivo de handle: %d\n", openFiles[i].file);
            printf("Cluster inicial deste arquivo: %d\n", openFiles[i].clusterNo);
            printf("CurrPointer: %d\n", openFiles[i].currPointer);
        }
    }
}

int link(char * path, char ** output) {
    int i;
    int isLink = 0;
    int clusterByteSize = sizeof(unsigned char)*SECTOR_SIZE*superBlock.SectorsPerCluster;
    unsigned char* buffer = malloc(clusterByteSize);
    char * absolute;
    char * pathToFile;
    char * fileName;
    int pathClusterNo;
    int linkClusterNo;


    toAbsolutePath(path, currentPath.absolute, &absolute);

    //printf("\nAboslute: %s", absolute);
    separatePath(absolute, &pathToFile, &fileName);

    pathClusterNo = pathToCluster(pathToFile);

    if(pathClusterNo == -1) {
        free(buffer);
        free(absolute);
        free(fileName);
        free(pathToFile);
        return -1;
    }

    readCluster(pathClusterNo, buffer);

    // printf("\nfileName: %s", fileName);
    for(i = 0; i < clusterByteSize; i+= sizeof(struct t2fs_record)) {
        // printf("\nNumero de vezes do for %d\n", i);
        if ( (strcmp((char *)buffer+i+1, fileName) == 0) && (((BYTE) buffer[i]) == TYPEVAL_LINK) && !isLink ) {
            isLink = 1;
        } 
    }

    if(!isLink) {
        free(buffer);
        free(absolute);
        free(fileName);
        free(pathToFile);
        *output = malloc(sizeof(char)*(strlen(path)+1));
        strcpy(*output,path);
        return 0;//BOTEI RETORNO COMO 0 PARA NAO SOFTLINK E COMO 1 PARA SOFTLINK -- LUCAS
    }
    
    linkClusterNo = pathToCluster(path);

    memset(buffer,0,clusterByteSize);

    readCluster(linkClusterNo,buffer);

    *output = malloc(sizeof(char)*(strlen((char*)buffer)+1));
    strcpy(*output,(char*)buffer);

    free(buffer);
    free(absolute);
    free(fileName);
    free(pathToFile);
    //BOTEI RETORNO COMO 0 PARA NAO SOFTLINK E COMO 1 PARA SOFTLINK -- LUCAS
    return 1;
}


int truncateFile(FILE2 handle) {
    int i = 0;
    int fileNo;
    int found = 0;
    int currentPointerInCluster;
    int currentCluster;
    int nextCluster;
    int truncatedCluster;
    int previousCluster;
    DWORD value;

    while(i < MAX_NUM_FILES && !found){
        if (handle == openFiles[i].file) {
            fileNo = i;
            found = 1;
        }
        i += 1;
    }

    if(!found) {
        return -1;
    }

    currentPointerInCluster = openFiles[fileNo].currPointer;
    nextCluster = openFiles[fileNo].clusterNo;
    currentCluster = nextCluster;
	
    while(currentPointerInCluster >= SECTOR_SIZE*superBlock.SectorsPerCluster) {
        if(readInFAT(nextCluster,&value) != 0) {
            return -1;
        }
        nextCluster = (int)value;
        if((DWORD)nextCluster != END_OF_FILE) {
            currentCluster = nextCluster;
        }
        currentPointerInCluster -= SECTOR_SIZE*superBlock.SectorsPerCluster;
        
    }

    truncateCluster(currentCluster,currentPointerInCluster);
    truncatedCluster = currentCluster;


    while((DWORD)nextCluster != END_OF_FILE) {
        if(readInFAT(nextCluster,&value) != 0) {
            return -1;
        }
        nextCluster = (int)value;
        if((DWORD)nextCluster != END_OF_FILE) {
            previousCluster = currentCluster;
            currentCluster = nextCluster;
            truncateCluster(currentCluster,0);
            writeInFAT(previousCluster,0);
        }
    }

    writeInFAT(currentCluster,0);
    writeInFAT(truncatedCluster,END_OF_FILE);

    //ATUALIZO O FILE SIZE
    if(updateFileSize(handle,(DWORD)openFiles[fileNo].currPointer) != 0){
        return -1;
    }

    return 0;
}

// WORK IN PROGRESS
int writeFile(FILE2 handle, char * buffer, int size) {
    int i = 0;
    int fileNo;
    int found = 0;
    int remainingSize = size;
    int bytesWritten = 0;
    int currentPointerInCluster;
    int currentCluster;
    int nextCluster;
    // Arredondado para cima size/clustersize
    int clusterSize = superBlock.SectorsPerCluster*SECTOR_SIZE;
     

    DWORD value;

    while(i < MAX_NUM_FILES && !found){
        if (handle == openFiles[i].file) {
            fileNo = i;
            found = 1;
        }
        i += 1;
    }

    if(!found) {
        return -1;
    }

    currentPointerInCluster = openFiles[fileNo].currPointer;
    nextCluster = openFiles[fileNo].clusterNo;
    currentCluster = nextCluster;
	
    while(currentPointerInCluster >= clusterSize) {
        if(readInFAT(nextCluster,&value) != 0) {
            return -1;
        }
        nextCluster = (int)value;
        if((DWORD)nextCluster != END_OF_FILE) {
            currentCluster = nextCluster;
        }
        currentPointerInCluster -= (clusterSize);
    }

    // Escreve primeiro cluster que o current pointer está
    if((remainingSize + currentPointerInCluster) <= (clusterSize)){
        writeCluster(currentCluster,(unsigned char*)(buffer),currentPointerInCluster,size);
        bytesWritten += size;
        remainingSize -= size;
    } else {
        writeCluster(currentCluster,(unsigned char*)(buffer),currentPointerInCluster,(clusterSize - currentPointerInCluster));
        remainingSize -= (clusterSize - currentPointerInCluster);
        bytesWritten += (clusterSize - currentPointerInCluster);
    }


    // Escreve nos clusters que já existem no arquivo
    while((DWORD)nextCluster != END_OF_FILE && remainingSize > 0) {
        if(readInFAT(nextCluster,&value) != 0) {
            return -1;
        }
        nextCluster = (int)value;
        if((DWORD)nextCluster != END_OF_FILE) {
            currentCluster = nextCluster;

            if(remainingSize <= (clusterSize)){
                writeCluster(currentCluster,(unsigned char*)(buffer + bytesWritten),0,remainingSize);
                bytesWritten += remainingSize;
                remainingSize -= remainingSize;
            } else {
                writeCluster(currentCluster,(unsigned char*)(buffer + bytesWritten),0,(clusterSize));
                remainingSize -= (clusterSize);
                bytesWritten += (clusterSize);
            }
        }
    }

    // Cria novos clusters e escreve no arquivo
    while(remainingSize > 0) {
        if(remainingSize <= (clusterSize)){
            if(findFATOpenCluster(&nextCluster) != 0) {
                return -1;
            }
            writeInFAT(currentCluster,nextCluster);
            writeInFAT(nextCluster,END_OF_FILE);
            writeCluster(nextCluster,(unsigned char*)(buffer + bytesWritten),0,remainingSize);
            bytesWritten += remainingSize;
            remainingSize -= remainingSize;
        } else {
            if(findFATOpenCluster(&nextCluster) != 0) {
                return -1;
            }
            writeInFAT(currentCluster,nextCluster);
            writeInFAT(nextCluster,END_OF_FILE);
            writeCluster(nextCluster,(unsigned char*)(buffer + bytesWritten),0,(clusterSize));
            currentCluster = nextCluster;
            remainingSize -= (clusterSize);
            bytesWritten += (clusterSize);
        }
    }

    openFiles[fileNo].currPointer += bytesWritten; 

    if(setRealDealFileSizeOfChaos(handle) != 0)
        return -2;

    
    return bytesWritten;
}
int readFile (FILE2 handle, char *buffer, int size){ //IN PROGRESS

    int found=0;
    int currentPointerInCluster;
    int currentCluster;
    int nextCluster;
    int fileNo;
    DWORD value;
    int j;
    int i=0;
    int clusterCount=0;
    unsigned char *prebuffer=malloc(sizeof(unsigned char)*SECTOR_SIZE*superBlock.SectorsPerCluster+1);


    //procura o arquivo pelo handle
    for(j=0;j<MAX_NUM_FILES && found==0;j++){
        if(openFiles[j].file == handle){
            found=1;
            fileNo=j;
        }

    }
    if(found==0){
        //fprintf(stderr,"\n\nNao achou o handle\n\n");
        return -1;
    }

    //atribuicao dos parametros do arquivo
    currentPointerInCluster = openFiles[fileNo].currPointer;
    currentCluster = openFiles[fileNo].clusterNo;

    //le o cluster atual
    prebuffer=readDataCluster(currentCluster);

    while((DWORD)currentCluster != END_OF_FILE && i<size && (DWORD)currentCluster != BAD_SECTOR){

        //percorre o buffer até achar o final do arquivo ou do cluster, transferindo os dados para saida
        while(currentPointerInCluster < SECTOR_SIZE*superBlock.SectorsPerCluster  && prebuffer[currentPointerInCluster] != '\0' && i<size){
            buffer[i]=(unsigned char)prebuffer[currentPointerInCluster];
            //fprintf(stderr,"\n%d - %c:%c",i,prebuffer[i],buffer[i]);
            currentPointerInCluster++;
            i++;
        }
        if(i>=size){
            //fprintf(stderr,"numero lido: %d",i);
            return -1;
        }
        //    fprintf(stderr,"\n\nPREBUFFER:%s\n\n",prebuffer);
        //    fprintf(stderr,"\n\nBUFFER:%s\n\n",buffer);

    //se ainda nao preencheu o tamanho descrito
        if(i<size || i>=clusterCount*SECTOR_SIZE*superBlock.SectorsPerCluster){
            if(readInFAT(currentCluster,&value) != 0) {
                return -2;
            }else{
                    nextCluster = (int)value;
                    free(prebuffer);
                    prebuffer=malloc(sizeof(unsigned char)*SECTOR_SIZE*superBlock.SectorsPerCluster+1);
                    prebuffer=readDataCluster(nextCluster);
                    currentPointerInCluster=0;
                    currentCluster = nextCluster;
            }
            }
                clusterCount++;
        }
    free(prebuffer);
    if(i == 0)
    return -3;
    openFiles[fileNo].currPointer +=i;
    return i;
}
int realFileSize (FILE2 handle){ //IN PROGRESS

    int found=0;
    int currentPointerInCluster;
    int currentCluster;
    int nextCluster;
    int fileNo;
    DWORD value;
    int j;
    int i=0;
    unsigned char *prebuffer=malloc(sizeof(unsigned char)*SECTOR_SIZE*superBlock.SectorsPerCluster+1);


    //procura o arquivo pelo handle
    for(j=0;j<MAX_NUM_FILES && found==0;j++){
        if(openFiles[j].file == handle){
            found=1;
            fileNo=j;
        }

    }
    if(found==0){
        //fprintf(stderr,"\n\nNao achou o handle\n\n");
        return -1;
    }

    //atribuicao dos parametros do arquivo
    currentPointerInCluster = 0;
    currentCluster = openFiles[fileNo].clusterNo;

    //le o cluster atual
    prebuffer=readDataCluster(currentCluster);

    while((DWORD)currentCluster != END_OF_FILE && (DWORD)currentCluster != BAD_SECTOR){

        //percorre o buffer até achar o final do arquivo ou do cluster, transferindo os dados para saida
        while(currentPointerInCluster <  SECTOR_SIZE*superBlock.SectorsPerCluster && prebuffer[currentPointerInCluster] != '\0') {
            currentPointerInCluster++;
            i++;
        }

    //se ainda nao preencheu o tamanho descrito
        if(readInFAT(currentCluster,&value) != 0) {
            return -2;
        }
        nextCluster = (int)value;
        free(prebuffer);
        prebuffer=malloc(sizeof(unsigned char)*SECTOR_SIZE*superBlock.SectorsPerCluster);
        if((DWORD)nextCluster != END_OF_FILE){
            prebuffer=readDataCluster(nextCluster);
        }
        currentPointerInCluster=0;
        currentCluster = nextCluster;
        }
    
    free(prebuffer);
    if(i == 0)
    return -3;
    return i;
}

int setRealDealFileSizeOfChaos(FILE2 handle){

    int filesize;

    //OBTENHO O TAMANHO REAL DO ARQUIVO -- OBS: AQUI TEM Q TER UM BUFFER ENORME PRA GARANTIR
    filesize=realFileSize(handle);
    if(filesize <0)
    {
        return -2;
    }
    //ATUALIZO O FILE SIZE
    if(updateFileSize(handle,(DWORD)filesize) != 0){
        return -3;
    }

    return 0;
}

int updateFileSize(FILE2 handle,DWORD newFileSize){

    int found=0;
    int fileNo;
    int j;
    struct t2fs_record newStruct;
    struct t2fs_record* folderContent = malloc(sizeof(struct t2fs_record)*( (SECTOR_SIZE*superBlock.SectorsPerCluster) / sizeof(struct t2fs_record) ));
    int folderSize = ( (SECTOR_SIZE*superBlock.SectorsPerCluster) / sizeof(struct t2fs_record) );
    int i;
    int foundinfolder =0;
    int count;
    unsigned char * buffer = malloc(sizeof(struct t2fs_record));

    //procura o arquivo pelo handle
    for(j=0;j<MAX_NUM_FILES && found==0;j++){
        if(openFiles[j].file == handle){
            found=1;
            fileNo=j;
        }

    }
    if(found==0){
        //fprintf(stderr,"\n\nNao achou o handle\n\n");
        return -1;
    }

    //verifica o tamanho do arquivo com o nome dado;
    folderContent=readDataClusterFolder(openFiles[fileNo].clusterDir);
    for(i=0;i<folderSize && foundinfolder ==0;i++){

        if(folderContent[i].firstCluster == openFiles[fileNo].clusterNo){
            newStruct.bytesFileSize=newFileSize;
            newStruct.clustersFileSize= (int)(newFileSize % (superBlock.SectorsPerCluster*SECTOR_SIZE)) == 0 ? (int)newFileSize/(superBlock.SectorsPerCluster*SECTOR_SIZE) : (int)newFileSize/(superBlock.SectorsPerCluster*SECTOR_SIZE) + 1;
            newStruct.firstCluster=openFiles[fileNo].clusterNo;
            strcpy(newStruct.name,folderContent[i].name);
            newStruct.TypeVal=folderContent[i].TypeVal;
            foundinfolder=1;
            count=(i*sizeof(struct t2fs_record));
            //fprintf(stderr,"\n\nCOUNT:%d\n\n",count);
            //fprintf(stderr,"DATASECTORSTART: %x",superBlock.DataSectorStart);

        }
    }

    if (!foundinfolder)
        return -1;

    memcpy(buffer,&(newStruct.TypeVal),1);
    memcpy((buffer + 1),newStruct.name,51);
    memcpy((buffer + 52),dwordToLtlEnd(newStruct.bytesFileSize),4);
    memcpy((buffer + 56),dwordToLtlEnd(newStruct.clustersFileSize),4);
    memcpy((buffer + 60),dwordToLtlEnd(newStruct.firstCluster),4);

    //fprintf(stderr,"\n\nBUFFER: %s\n\n",buffer);
    writeCluster(openFiles[fileNo].clusterDir,buffer,count,sizeof(struct t2fs_record));

return 0;      
}

int createSoftlink(char *linkname,char *filename){ //Fruto do REUSO

    char * absolutefilename = malloc(sizeof(char)*2);
    char * absolute;
    char * firstOut;
    char * secondOut;
    int firstClusterFreeInFAT;
    int clusterFile;
    int clusterDotDot;

    if(strcmp(filename,"/") ==0)
        strcpy(absolutefilename,"/");
    else
        toAbsolutePath(filename, currentPath.absolute, &absolutefilename);
                        
    //adiciono um "/" no final se nao tiver pra poder utilizar a funcao pathToCluster direto

    toAbsolutePath(linkname, currentPath.absolute, &absolute);
    separatePath(absolute, &firstOut, &secondOut);

    if(findFATOpenCluster(&firstClusterFreeInFAT) == -1){//se n achar um cluster livre na fat
        free(absolute);
        free(absolutefilename);
        free(firstOut);
        free(secondOut);
        return -1;
    }
        if(strlen(secondOut) == 0){//softlink sem nome
        free(absolute);
        free(absolutefilename);
        free(firstOut);
        free(secondOut);
        return -2;
    }
    if(!(isRightName(secondOut))){//softlink n pode ter esse nome

        free(absolute);
        free(absolutefilename);
        free(firstOut);
        free(secondOut);
        return -3;
    }

    //se o absolute do filename for / entao ele aponta para o root
    if(strcmp(absolutefilename,"/") == 0){
        clusterFile = superBlock.RootDirCluster;
    }
    else//se nao for, então tem q achar onde fica
    {   
        clusterFile = pathToCluster(absolutefilename);

        if(clusterFile == -1){
        free(absolute);
        free(absolutefilename);
        free(firstOut);
        free(secondOut);
        return -4;
    }
    }
    //se o firstOut do absolute for '/', então DotDot vai ser o raiz
    if(strlen(firstOut) == 1 && firstOut[0]== '/'){
        clusterDotDot = superBlock.RootDirCluster;
    }
    if(clusterDotDot == -1 || isInCluster(clusterDotDot, secondOut,TYPEVAL_LINK) ==1 ){

        free(absolute);
        free(absolutefilename);
        free(firstOut);
        free(secondOut);
        return -5;
    }

    else//se nao for, então tem q achar onde fica
    {
        clusterDotDot = pathToCluster(firstOut);
        
        if(clusterDotDot == -1 || isInCluster(clusterDotDot, secondOut,TYPEVAL_LINK) ==1 ){

        free(absolute);
        free(absolutefilename);
        free(firstOut);
        free(secondOut);
        return -5;
    }
    }

    //fprintf(stderr,"CLUSTER PARA ONDE O SOFTLINK APONTA: %d\n\n", clusterFile);
    //fprintf(stderr,"CLUSTER DO SOFTLINK CRIADO: %d\n\n", firstClusterFreeInFAT);
    //fprintf(stderr,"CLUSTER DA PASTA DO SOFTLINK: %d\n\n", clusterDotDot);
    //fprintf(stderr, "ABSOLUTE FILENAME: %s\n\n", absolutefilename);

    struct t2fs_record link;

    link.TypeVal = TYPEVAL_LINK;
    strcpy(link.name, secondOut);
    link.bytesFileSize = sizeof(char)*strlen(absolutefilename);
    link.clustersFileSize = 1;
    link.firstCluster = firstClusterFreeInFAT;
    writeDataClusterFolder(clusterDotDot, link);
  //  sprintf(buffer,"%d",clusterFile);
    writeCluster(firstClusterFreeInFAT,(unsigned char *)absolutefilename,0,link.bytesFileSize);

/*    DWORD conv;
    fprintf(stderr,"\n\n%s\n\n",buffer);
    conv=convertToDword((unsigned char*)buffer);
        fprintf(stderr,"\n\n%x\n\n",(DWORD)clusterFile);
*/

    writeInFAT(firstClusterFreeInFAT, (DWORD)clusterFile);

    free(absolute);

    free(absolutefilename);

    free(firstOut);

    free(secondOut);

return 0;
}
int sizeOfFile(int clusterDir, int clusterFile){

    struct t2fs_record* folderContent = malloc(sizeof(struct t2fs_record)*( (SECTOR_SIZE*superBlock.SectorsPerCluster) / sizeof(struct t2fs_record) ));
    int folderSize = ( (SECTOR_SIZE*superBlock.SectorsPerCluster) / sizeof(struct t2fs_record) );
    int i;
    //fprintf(stderr,"\n\nCLUSTER FILE:%d CLUSTER DIR%d",clusterFile, clusterDir);
    //verifica o tamanho do arquivo com o nome dado;
    folderContent=readDataClusterFolder(clusterDir);
    for(i=0;i<folderSize;i++){
        if(folderContent[i].firstCluster == clusterFile){
            return (int)folderContent[i].bytesFileSize;
        }
    }
    return -2;
}
int moveCursor (FILE2 handle, DWORD offset){
    int found=0;
    int currentPointer;
    int currentCluster;
    int fileNo;
    int newCursorPointer;
    int j;

    //procura o arquivo pelo handle
    for(j=0;j<MAX_NUM_FILES && found==0;j++){
        if(openFiles[j].file == handle){
            found=1;
            fileNo=j;
        }

    }
    if(found==0){
        //fprintf(stderr,"\n\nNao achou o handle\n\n");
        return -1;
    }
    //atribuicao dos parametros do arquivo
    currentPointer = openFiles[fileNo].currPointer;
    currentCluster= openFiles[fileNo].clusterNo;

    //novo cp
    newCursorPointer = (int)offset;
    if(newCursorPointer<-1)
        return -3;

    if(newCursorPointer > sizeOfFile(openFiles[fileNo].clusterDir,openFiles[fileNo].clusterNo) || newCursorPointer<0){
        //fprintf(stderr,"SIZE OF FILE: %d\n\n",sizeOfFile(openFiles[fileNo].clusterDir,openFiles[fileNo].clusterNo));
        return -2;
    }
    if((int)offset == -1){
        newCursorPointer= sizeOfFile(openFiles[fileNo].clusterDir,openFiles[fileNo].clusterNo) + 1;
    }

    openFiles[fileNo].currPointer= newCursorPointer;

    return 0;
}