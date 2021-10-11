#include <math.h>
#include <rainbow.h>
#include <stdio.h>
#include <stdlib.h>

/*
    Calculates the coverage of one rainbow table.
    Set these parameters in rainbow.h beforehand:
    TABLE_COUNT 1
    TABLE_T 100
    MAX_PASSWORD_LENGTH 3
*/
int main() {
    RainbowTable rainbow_tables[TABLE_COUNT];
    offline(rainbow_tables);

    unsigned int n = 0;
    for (int i = 1; i <= MAX_PASSWORD_LENGTH; i++) {
        n += pow(64, i);
    }

    unsigned int found = 0;
    for (unsigned int i = 0; i < n; i++) {
        char plain_text[MAX_PASSWORD_LENGTH + 1];
        char password[MAX_PASSWORD_LENGTH + 1];
        unsigned char digest[HASH_LENGTH];

        create_startpoint(i, plain_text);
        HASH_FUNCTION(plain_text, strlen(plain_text), digest);
        online(rainbow_tables, digest, password);

        if (!strcmp(plain_text, password)) {
            found++;
        }

        printf("\r%f%%", (float)i / n * 100);
        fflush(stdout);
    }

    double success_rate = (double)found / n * 100;
    printf("\nSuccess rate for one table: %lf%%", success_rate);

    return 0;
}