#include <math.h>
#include <rainbow.h>
#include <stdio.h>
#include <stdlib.h>

/*
    Calculates the coverage of one rainbow table.

    Set these parameters in rainbow.h beforehand:
    TABLE_COUNT 1
    TABLE_T 10000
    MAX_PASSWORD_LENGTH 3
*/
int main() {
    RainbowTable rainbow_tables[TABLE_COUNT];
    offline(rainbow_tables);
    // store_table(&rainbow_tables[0], "length4.rt");
    // rainbow_tables[0] = load_table("length4.rt");
    // print_table(&rainbow_tables[0]);

    unsigned long n = 1000;
    // for (int i = 1; i <= MAX_PASSWORD_LENGTH; i++){
    //     n += pow(64, i;
    //}

    unsigned long found = 0;
    for (unsigned long i = 0; i < n; i++) {
        char plain_text[MAX_PASSWORD_LENGTH + 1];
        char password[MAX_PASSWORD_LENGTH + 1];
        unsigned char digest[HASH_LENGTH];

        create_startpoint(i, plain_text);
        HASH(plain_text, strlen(plain_text), digest);
        online(&rainbow_tables[0], digest, password);

        if (!strcmp(plain_text, password)) {
            found++;
        }

        float progress = (float)(i + 1) / n * 100;

        // use a confidence interval for a better approximation
        float f = (float)found / (i + 1);
        float confidence = 1.96 * sqrt(f * (1 - f) / (i + 1));

        if (confidence > 0.001) {
            printf("\rprogress: %.2f%% | success rate: [%.2f%%, %.2f%%]",
                   progress, (f - confidence) * 100, (f + confidence) * 100);
        } else {
            // our confidence is high enough, just display the success rate
            printf("\rprogress: %.2f%% | success rate: %.2f%%          ",
                   progress, f * 100);
        }
        fflush(stdout);
    }

    return 0;
}