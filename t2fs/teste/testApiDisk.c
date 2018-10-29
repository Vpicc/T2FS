#include <stdio.h>
#include "../include/apidisk.h"

int main() {
    unsigned char buffer[256] = {0};
    buffer[1] = 1;
    printf("\nHello\n");
    read_sector(0,buffer);
    printf("%s\n", buffer);
    getchar();
    return 0;
}