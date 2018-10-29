#include <stdio.h>
#include "../include/apidisk.h"

// -lm -m32
// gcc -o teste testApiDisk.c ../lib/apidisk.o -Wall -ggdb && ./teste
int main() {
    int i;
    unsigned char buffer[256] = {0};
    read_sector(0,buffer);
    for(i = 0; i < sizeof(buffer)/sizeof(unsigned char); i++){
        if ( i == 0 ) {
            printf("\nID:\n");
        }
        if ( i < 4 ) {
            printf("%c ", buffer[i]);
        }
        if ( i == 4 ) {
            printf("\nversion:\n");
        }
        if ( i == 6 ) {
            printf("\nSuperBlockSize\n");
        }
        if ( i == 8 ) {
            printf("\nDiskSize\n");
        }
        if ( i == 12 ) {
            printf("\nNofSectors\n");
        }
        if ( i == 16 ) {
            printf("\nSectorsPerCluster\n");
        }
        if ( i == 20 ) {
            printf("\npFATSectorStart\n");
        }
        if ( i == 24 ) {
            printf("\nRootDirCluster\n");
        }
        if ( i == 28 ) {
            printf("\nDataSectorStart\n");
        }
        if ( i >= 4 && i < 32) {
            printf("%x ", buffer[i]);
        }
    }
    printf("\n");
    return 0;
}