#ifndef __DISK___
#define __DISK___
#include "t2fs.h"

#define END_OF_FILE 0xFFFFFFFF

#define BAD_SECTOR 0xFFFFFFFE

#define MAX_NUM_FILES 10

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

int isInCluster(int clusterNo, char * fileName, BYTE TypeValEntrada);

int isRightName(char * name);

int writeZeroClusterFolderByName(int clusterNo, struct t2fs_record folder, char * fileName, BYTE TypeValEntrada);

DIRENT2 searchDirByHandle(DIR2 handle);

DIR2 openDir(char *path);

DIRENT2 setNullDirent();

void printOpenDirectories();

void setCurrentPathToRoot();

int closeDir(DIR2 handle);

int link(char * path, char ** output);

int truncateCluster(int clusterNo, int position);

int truncateFile(FILE2 handle);

FILE2 createFile(char * filename);

int makeAnewHandle();

void printOpenFiles();

FILE2 openFile (char * filename);

int writeFile(FILE2 handle, char * buffer, int size);


typedef struct diskf {
    FILE2 file;
    int currPointer;
    int clusterNo;
} DISK_FILE;

typedef struct currp {
    char* absolute;
    int clusterNo;
} CURRENT_PATH;

CURRENT_PATH currentPath;

typedef struct diskd {
    DIR2 handle;
    int noReads;
    struct currp path;
    DIRENT2 directory;
} DISK_DIR;

DISK_FILE openFiles[10];

#endif