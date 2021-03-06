#include <math.h>
#include <rainbow.h>
#include <stdio.h>
#include <stdlib.h>

/*
    Calculates the coverage of rainbow tables.
    This generates `TABLE_COUNT` tables, and tries to
    crack `n` passwords with it.

    With 1 rainbow table, the succes rate should be close to 86.5%.
    With 4 rainbow tables, the success rate should be close to 99.96%.
*/
int main() {
    RainbowTable rainbow_tables[TABLE_COUNT];
    offline(rainbow_tables);

    const unsigned long n = 10000;
    printf("\nTrying to crack %lu passwords\n", n);

    unsigned long found = 0;
    for (unsigned long i = 0; i < n; i++) {
        char plain_text[PASSWORD_LENGTH + 1];
        char password[PASSWORD_LENGTH + 1];
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

        float p_low = (f - confidence) * 100;
        if (p_low < 0) p_low = 0;

        float p_high = (f + confidence) * 100;
        if (p_high > 100) p_high = 100;

        if (confidence > 0.001) {
            printf(
                "\rprogress: %.2f%% | success rate: %.2f%% [%.2f%%, %.2f%%]  ",
                progress, f * 100, p_low, p_high);
        } else {
            // our confidence is high enough, just display the success rate
            printf(
                "\rprogress: %.2f%% | success rate: %.2f%%                   ",
                progress, f * 100);
        }
        fflush(stdout);
    }

    return 0;
}