#ifndef __DISK___
#define __DISK___
#include "t2fs.h"

#define END_OF_FILE 0xFFFFFFFF

#define BAD_SECTOR 0xFFFFFFFE

struct t2fs_superbloco superBlock;

DWORD convertToDword(unsigned char* buffer);

WORD convertToWord(unsigned char* buffer);

unsigned char* wordToLtlEnd(WORD entry);

unsigned char* dwordToLtlEnd(DWORD entry);

int init_disk();

int writeInFAT(int clusterNo, DWORD value);

int readInFAT(int clusterNo, DWORD* value);

struct t2fs_record* readDataClusterFolder(int clusterNo);

int writeDataClusterFolder(int clusterNo,struct t2fs_record folder);

int readCluster(int clusterNo, unsigned char* buffer);

unsigned char* readDataCluster (int clusterNo);

int writeCluster(int clusterNo, unsigned char* buffer, int position, int size);

int pathToCluster(char* path);

int findFATOpenCluster(int* clusterReturn);

int tokenizePath(char* path, char*** tokenized);

int toAbsolutePath(char * path, char * currPath, char ** output);

int separatePath(char * path, char ** FristStringOutput, char ** SecondStringOutput) ;

int changeDir(char * path);

int mkdir(char * path);

int isEmptyDir(int clusterNo);

int deleteDir(char * path);

int writeZeroClusterFolderByName(int clusterNo, struct t2fs_record folder, char * fileName, BYTE TypeValEntrada);

typedef struct diskf {
    FILE2 file;
    int currPointer;
    char name[51];
} DISK_FILE;

typedef struct currp {
    char* absolute;
    int clusterNo;
} CURRENT_PATH;

CURRENT_PATH currentPath;


#endif