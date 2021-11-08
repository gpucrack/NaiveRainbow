#include <assert.h>
#include <rainbow.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    RainbowTable rainbow_tables[TABLE_COUNT];

    printf("Generating tables...\n");
    offline(rainbow_tables);

    // to print a table
    print_table(&rainbow_tables[0]);

    // to print the full matrix
    // print_matrix(rainbow_tables[0]);

    char* password = "crak";
    assert(strlen(password) == PASSWORD_LENGTH);

    unsigned char digest[HASH_LENGTH];
    HASH(password, strlen(password), digest);

    printf("Looking for password '%s', hashed as ", password);
    print_hash(digest);
    printf(".\n\n");

    char found[PASSWORD_LENGTH + 1];

    printf("Starting attack...\n");
    online(rainbow_tables, digest, found);

    if (!strcmp(found, "")) {
        printf("No password found for the given hash.\n");
    } else {
        printf("Password '%s' found for the given hash!\n", found);
    }

    for (int i = 0; i < TABLE_COUNT; i++) {
        del_table(&rainbow_tables[i]);
    }

    return 0;
}
