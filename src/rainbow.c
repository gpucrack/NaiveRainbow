#include "rainbow.h"

char char_in_range(unsigned char n) {
    assert(n >= 0 && n <= 63);
    static const char* chars =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-_";

    return chars[n];
}

void reduce_cipher(unsigned char cipher_text[], unsigned int iteration,
                   char* plain_text) {
    unsigned int sum[MAX_PASSWORD_LENGTH] = {0};
    unsigned int current_index = 0;

    for (int i = 0; i < HASH_LENGTH; i++) {
        sum[i % MAX_PASSWORD_LENGTH] += cipher_text[i];
    }

    for (int i = 0; i < MAX_PASSWORD_LENGTH; i++) {
        sum[i] = (sum[i] + iteration) % 65;
        if (sum[i] != 64) {
            plain_text[current_index] = char_in_range(sum[i]);
            current_index++;
        }
    }

    plain_text[current_index] = '\0';
}

void create_startpoint(unsigned int counter, char* plain_text) {
    int startpoint_length = 1;
    unsigned int old_thresold = 0;
    unsigned int thresold = 64;

    // get the length of the startpoint
    while (counter >= thresold) {
        startpoint_length++;
        old_thresold = thresold;
        thresold += pow(64, startpoint_length);
    }

    // the startpoint of length `startpoint_length` should start at 0
    counter -= old_thresold;

    int i;
    // fill the startpoint with the corresponding characters
    for (i = startpoint_length - 1; i >= 0; i--) {
        plain_text[i] = char_in_range(counter % 64);
        counter /= 64;
    }

    // fill with zeros if the length is inferior to the computed length
    for (; i >= 0; i--) {
        plain_text[i] = char_in_range(0);
    }

    plain_text[startpoint_length] = '\0';
}

void _create_startpoint(unsigned int counter, char* plain_text) {
    int i;
    for (i = 0; i < MAX_PASSWORD_LENGTH; i++) {
        // get the character corresponding to the last 6 bits
        plain_text[i] = char_in_range(counter & 63);

        if (!counter) {
            break;
        }

        // right shift the counter by 6 bits (2^6 == 64 values)
        counter >>= 6;
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

RainbowTable gen_table(unsigned char table_number, unsigned int m0) {
    /*
       Allocate in one contiguous block the chains. This should reduce cache
       misses, malloc calls, and make it easier later on the GPU.
    */
    RainbowChain* chains = malloc(sizeof(RainbowChain) * m0);
    if (!chains) {
        perror("Cannot allocate enough memory");
        exit(EXIT_FAILURE);
    }

    RainbowTable table = {chains, 0};

    // generate all rows
    for (unsigned int i = 0; i < m0; i++) {
        // generate the chain
        char last_plain_text[MAX_PASSWORD_LENGTH + 1];
        char startpoint[MAX_PASSWORD_LENGTH + 1];
        create_startpoint(table_number * m0 + i, startpoint);
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
    for (unsigned int i = 0; i < table.length; i++) {
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
    // the number of possible passwords
    unsigned int n = 0;
    for (int i = 0; i <= MAX_PASSWORD_LENGTH; i++) {
        n += pow(64, i);
    }

    // the expected number of unique chains
    unsigned int mtmax = 2 * n / (TABLE_T + 2);

    // the starting number of chains, given the alpha coefficient
    unsigned int m0 = TABLE_ALPHA / (1 - TABLE_ALPHA) * mtmax;

    // make sure there are at least some chains for the smaller password spaces
    if (m0 < 10) {
        m0 = 10;
    }

    for (int i = 0; i < TABLE_COUNT; i++) {
        rainbow_tables[i] = gen_table(i, m0);
    }
}

void online(RainbowTable* rainbow_tables, unsigned char* cipher,
            char* password) {
    /*
        Iterate column by column, starting from the last plaintext.
        https://stackoverflow.com/questions/3623263/reverse-iteration-with-an-unsigned-loop-variable

        We iterate through all tables at the same time because it's faster to
        find a match in the last columns.
    */
    for (unsigned int i = TABLE_T - 1; i-- > 0;) {
        for (int j = 0; j < TABLE_COUNT; j++) {
            char column_plain_text[MAX_PASSWORD_LENGTH + 1];
            unsigned char column_cipher_text[HASH_LENGTH];
            memcpy(column_cipher_text, cipher, HASH_LENGTH);

            // get the reduction corresponding to the current column
            for (unsigned int k = i; k < TABLE_T - 2; k++) {
                reduce_cipher(column_cipher_text, k, column_plain_text);
                HASH_FUNCTION(column_plain_text, strlen(column_plain_text),
                              column_cipher_text);
            }
            reduce_cipher(column_cipher_text, TABLE_T - 2, column_plain_text);

            // iterate through all rows to check if it'a an endpoint
            for (unsigned int k = 0; k < rainbow_tables[j].length; k++) {
                // we found a matching endpoint
                if (!strcmp(rainbow_tables[j].chains[k].endpoint,
                            column_plain_text)) {
                    // re-construct the chain
                    char chain_plain_text[MAX_PASSWORD_LENGTH + 1];
                    unsigned char chain_cipher_text[HASH_LENGTH];
                    strcpy(chain_plain_text,
                           rainbow_tables[j].chains[k].startpoint);

                    for (unsigned int l = 0; l < i; l++) {
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