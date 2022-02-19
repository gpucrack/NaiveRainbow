#include <rainbow.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
    Example showing how we create a rainbow table given its start and end point files.
*/
int main() {
    RainbowTable rainbow_tables[TABLE_COUNT];

    const char* start_path = "/home/dorian/CLionProjects/NaiveRainbow/examples/testStart.txt";
    const char* end_path = "/home/dorian/CLionProjects/NaiveRainbow/examples/testEnd.txt";

    // the password we will be looking to crack, after it's hashed
    const char* password = "aaaaaaa";

    // `digest` now contains the hashed password
    unsigned char digest[HASH_LENGTH];
    //HASH(password, strlen(password), digest);
    ntlm(password, digest);

    printf("\nLooking for password '%s', hashed as %s", password, digest);
    // print_hash(digest);
    printf(".\nStarting attack...\n");

    // try to crack the password
    char found[PASSWORD_LENGTH + 1];
    online_from_files(start_path, end_path, digest, found);

    // if `found` is not empty, then we successfully cracked the password
    if (!strcmp(found, "")) {
        printf("No password found for the given hash.\n");
    } else {
        printf("Password '%s' found for the given hash!\n", found);
    }

    return 0;
}
