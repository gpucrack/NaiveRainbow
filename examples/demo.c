#include <rainbow.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
    Example showcasing rainbow table generation (offline phase)
    and password attack (online phase).
*/
int main() {
    RainbowTable rainbow_tables[TABLE_COUNT];

    // generate the rainbow tables, according to our parameters set in rainbow.h
    offline(rainbow_tables);

    // print a table
    printf("\nTable 1:\n");
    print_table(&rainbow_tables[0]);

    // to print the full matrix
    // print_matrix(&rainbow_tables[0]);

    // the password we will be looking to crack, after it's hashed
    const char* password = "pwd";
    if (strlen(password) != PASSWORD_LENGTH) {
        fprintf(stderr, "Wrong password size, check rainbow.h settings!");
        exit(EXIT_FAILURE);
    }

    // `digest` now contains the hashed password
    unsigned char digest[HASH_LENGTH];
    HASH(password, strlen(password), digest);

    printf("\nLooking for password '%s', hashed as ", password);
    print_hash(digest);
    printf(".\nStarting attack...\n");

    // try to crack the password
    char found[PASSWORD_LENGTH + 1];
    online(rainbow_tables, digest, found);

    // if `found` is not empty, then we successfully cracked the password
    if (!strcmp(found, "")) {
        printf("No password found for the given hash.\n");
    } else {
        printf("Password '%s' found for the given hash!\n", found);
    }

    // free the tables (not needed if the program returns right after)
    for (int i = 0; i < TABLE_COUNT; i++) {
        del_table(&rainbow_tables[i]);
    }

    return 0;
}
