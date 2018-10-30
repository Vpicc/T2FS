#include <stdio.h>
#include "../include/disk.h"

//gcc -o testeDisk testDisk.c ../src/disk.c ../lib/apidisk.o -Wall -ggdb && ./testeDisk

int main() {
    int num = 13333;
    unsigned char* numLtlEnd;
    int num2 = 91532899;
    unsigned char* num2LtlEnd;
    init_disk();

    printf("\nNUM: %d\n", num);

    printf("\nWORD: %x\n", num);

    numLtlEnd = wordToLtlEnd(num);
    printf("\nWORD LITTLE END: %x %x\n",numLtlEnd[0], numLtlEnd[1]);

    printf("\nNUM2: %d\n", num2);

    printf("\nWORD: %x\n", num2);

    num2LtlEnd = dwordToLtlEnd(num2);
    printf("\nWORD LITTLE END: %x %x %x %x\n",num2LtlEnd[0], num2LtlEnd[1], num2LtlEnd[2], num2LtlEnd[3]);




    return 0;
}