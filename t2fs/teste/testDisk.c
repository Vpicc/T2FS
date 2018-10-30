#include <stdio.h>
#include "../include/disk.h"

//gcc -o testeDisk testDisk.c ../src/disk.c ../lib/apidisk.o -Wall -ggdb && ./testeDisk

int main() {
    init_disk();
    printf("\nall done\n");
    return 0;
}