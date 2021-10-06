#define _CRT_SECURE_NO_WARNINGS

#include <assert.h>
#include <rainbow.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The max password length in the rainbow tables.
#define MAX_PASSWORD_LENGTH 3

// The hash function used.
#define HASH_FUNCTION SHA1

// The length of the cipher text produced by the hash function.
#define HASH_LENGTH 20

// The number of rows in the table.
#define TABLE_M 1000

// The size of a chain in the table.
#define TABLE_T 10000

// The number of tables.
#define TABLE_COUNT 4

int main() {
    RainbowTable rainbow_tables[TABLE_COUNT];

    printf("Generating tables...\n");
    offline(rainbow_tables);

    // to print a table
    print_table(rainbow_tables[0]);

    // to print the full matrix
    // print_matrix(rainbow_tables[0]);

    char* password = "pls";
    unsigned char cipher[HASH_LENGTH];
    HASH_FUNCTION(password, strlen(password), cipher);

    printf("Looking for password '%s', hashed as ", password);
    print_hash(cipher);
    printf(".\n\n");

    char found[MAX_PASSWORD_LENGTH + 1];

    printf("Starting attack...\n");
    online(rainbow_tables, cipher, found);

    if (!strcmp(found, "")) {
        printf("No password found for the given hash.\n");
    } else {
        printf("Password '%s' found for the given hash!\n", found);
    }

    for (int i = 0; i < TABLE_COUNT; i++) {
        del_table(rainbow_tables[i]);
    }

    return 0;
}
