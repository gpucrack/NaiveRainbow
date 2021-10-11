#include <rainbow.h>
#include <stdio.h>
#include <stdlib.h>

/*
    Useful for debug purposes.
    Use it like that: ./print_matrix > matrix.txt
*/
int main() {
    RainbowTable rainbow_tables[TABLE_COUNT];
    offline(rainbow_tables);

    for (int i = 0; i < TABLE_COUNT; i++) {
        print_table(rainbow_tables[i]);
        printf("\n\n\n");
        del_table(rainbow_tables[i]);
    }

    return 0;
}