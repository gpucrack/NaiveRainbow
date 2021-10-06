#include "rainbow.h"

char char_in_range(unsigned char n) {
    assert(n >= 0 && n <= 63);

    // 0..9
    if (n < 10) {
        return '0' + n;
    }

    // A..Z
    if (n < 36) {
        return 'A' + (n - 10);
    }

    // a..z
    if (n < 62) {
        return 'a' + (n - 36);
    }

    if (n == 62) {
        return '-';
    }

    return '_';
}

void reduce_cipher(unsigned char cipher_text[], unsigned int iteration,
                   char* plain_text) {
    unsigned int sum[MAX_PASSWORD_LENGTH] = {0};
    unsigned int current_index = 0;

    for (unsigned int i = 0; i < HASH_LENGTH; i++) {
        sum[i % MAX_PASSWORD_LENGTH] += cipher_text[i];
    }

    for (unsigned int i = 0; i < MAX_PASSWORD_LENGTH; i++) {
        unsigned int mod = (sum[i] + iteration) % 65;

        // if mod == 64, skip a character (smaller password)
        if (mod != 64) {
            plain_text[current_index] = char_in_range(mod);
            current_index++;
        }
    }

    plain_text[current_index] = '\0';
}

void create_startpoint(unsigned int counter, char* plain_text) {
    int i;
    for (i = 0; i < MAX_PASSWORD_LENGTH; i++) {
        // get the character corresponding to the last 6 bits
        plain_text[i] = char_in_range(counter & 63);
        // right shift the counter by 6 bits (2^6 == 64 values)
        counter >>= 6;

        if (!counter) {
            break;
        }
    }

    plain_text[i + 1] = '\0';
}

void insert_chain(RainbowTable* table, char* startpoint, char* endpoint) {
    for (unsigned int i = 0; i < table->length; i++) {
        if (!strcmp(table->chains[i].endpoint, endpoint)) {
            return;
        }
    }

    strcpy(table->chains[table->length].startpoint, startpoint);
    strcpy(table->chains[table->length].endpoint, endpoint);
    table->length++;
}

RainbowTable gen_table(unsigned int counter_start) {
    /*
       Allocate in one contiguous block the chains. This should reduce cache
       misses, malloc calls, and make it easier later on the GPU.
    */
    RainbowChain* chains = malloc(sizeof(RainbowChain) * TABLE_M);
    if (!chains) {
        perror("Cannot allocate enough memory");
        exit(EXIT_FAILURE);
    }

    RainbowTable table = {chains, 0};

    // generate all rows
    for (unsigned int i = 0; i < TABLE_M; i++) {
        // generate the chain
        char last_plain_text[MAX_PASSWORD_LENGTH + 1];
        char startpoint[MAX_PASSWORD_LENGTH + 1];
        create_startpoint(counter_start + i, startpoint);
        strcpy(last_plain_text, startpoint);

        /*
            Apply a round of hash + reduce `TABLE_T - 1` times.
            The chain should look like this:

            n -> r0(h(n)) -> r1(h(r0(h(n))) -> ...
        */
        for (unsigned int j = 0; j < TABLE_T - 1; j++) {
            unsigned char cipher_text[HASH_LENGTH];
            HASH_FUNCTION(last_plain_text, strlen(last_plain_text),
                          cipher_text);
            reduce_cipher(cipher_text, j, last_plain_text);
        }

        // try to insert the chain in the rainbow table
        insert_chain(&table, startpoint, last_plain_text);
    }

    return table;
}

void del_table(RainbowTable table) { free(table.chains); }

void print_hash(unsigned char cipher_text[]) {
    for (int i = 0; i < HASH_LENGTH; i++) {
        printf("%02x", cipher_text[i]);
    }
}

void print_table(RainbowTable table) {
    for (unsigned int i = 0; i < table.length; i++) {
        printf("%s -> ... -> %s\n", table.chains[i].startpoint,
               table.chains[i].endpoint);
    }
}

void print_matrix(RainbowTable table) {
    for (unsigned int i = 0; i < TABLE_M; i++) {
        char plain_text[MAX_PASSWORD_LENGTH + 1];
        unsigned char cipher_text[HASH_LENGTH];
        strcpy(plain_text, table.chains[i].startpoint);

        for (unsigned int j = 0; j < TABLE_T - 1; j++) {
            HASH_FUNCTION(plain_text, strlen(plain_text), cipher_text);
            printf("%s -> ", plain_text);
            print_hash(cipher_text);
            printf(" -> ");
            reduce_cipher(cipher_text, j, plain_text);
        }

        printf("%s\n", plain_text);
    }
}

void offline(RainbowTable* rainbow_tables) {
    for (int i = 0; i < TABLE_COUNT; i++) {
        rainbow_tables[i] = gen_table(i * TABLE_M);
    }
}

void online(RainbowTable* rainbow_tables, unsigned char* cipher,
            char* password) {
    for (int i = 0; i < TABLE_COUNT; i++) {
        // iterate column by column, starting from the last plaintext
        // https://stackoverflow.com/questions/3623263/reverse-iteration-with-an-unsigned-loop-variable
        for (unsigned int j = TABLE_T - 1; j-- > 0;) {
            char column_plain_text[MAX_PASSWORD_LENGTH + 1];
            unsigned char column_cipher_text[HASH_LENGTH];
            memcpy(column_cipher_text, cipher, HASH_LENGTH);

            // get the reduction corresponding to the current
            // column
            for (unsigned int k = j; k < TABLE_T - 2; k++) {
                reduce_cipher(column_cipher_text, k, column_plain_text);
                HASH_FUNCTION(column_plain_text, strlen(column_plain_text),
                              column_cipher_text);
            }
            reduce_cipher(column_cipher_text, TABLE_T - 2, column_plain_text);

            // iterate through all rows to check if it'a an endpoint
            for (unsigned int k = 0; k < TABLE_M; k++) {
                // we found a matching endpoint
                if (!strcmp(rainbow_tables[i].chains[k].endpoint,
                            column_plain_text)) {
                    // re-construct the chain
                    char chain_plain_text[MAX_PASSWORD_LENGTH + 1];
                    unsigned char chain_cipher_text[HASH_LENGTH];
                    strcpy(chain_plain_text,
                           rainbow_tables[i].chains[k].startpoint);

                    for (unsigned int l = 0; l < j; l++) {
                        HASH_FUNCTION(chain_plain_text,
                                      strlen(chain_plain_text),
                                      chain_cipher_text);
                        reduce_cipher(chain_cipher_text, l, chain_plain_text);
                    }
                    HASH_FUNCTION(chain_plain_text, strlen(chain_plain_text),
                                  chain_cipher_text);

                    /*
                       The cipher was indeed present in the chain, this was
                       not a false positive from a reduction. We found a plain
                       text that matches the cipher text!
                    */
                    if (!memcmp(chain_cipher_text, cipher, HASH_LENGTH)) {
                        strcpy(password, chain_plain_text);
                        return;
                    }
                }
            }
        }
    }

    // no match found
    password[0] = '\0';
}