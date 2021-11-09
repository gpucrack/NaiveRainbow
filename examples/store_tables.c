#include <rainbow.h>
#include <stdio.h>
#include <stdlib.h>

/*
    Computes rainbow tables and stores it on the disk.
*/
int main() {
    RainbowTable tables[TABLE_COUNT];
    offline(tables);

    for (unsigned char i = 0; i < TABLE_COUNT; i++) {
        char file_path[32];
        sprintf(file_path, "length%hhu_%hhu.rt", PASSWORD_LENGTH, i);
        store_table(&tables[i], file_path);
    }

    return 0;
}