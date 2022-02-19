#include "rainbow.h"

int compare_rainbow_chains(const void *p1, const void *p2) {
    const RainbowChain *chain1 = p1;
    const RainbowChain *chain2 = p2;
    return strcmp(chain1->endpoint, chain2->endpoint);
}

unsigned char charset[CHARSET_LENGTH] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E',
                                         'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
                                         'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c',
                                         'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
                                         's', 't',
                                         'u', 'v', 'w', 'x', 'y', 'z'};

char char_in_range(unsigned char n) {
    assert(n >= 0 && n <= 63);
    static const char *chars =
            "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-_";

    return chars[n];
}

void reduce_digest(unsigned char *digest, unsigned long iteration,
                   unsigned char table_number, char *plain_text) {
    // pseudo-random counter based on the hash
    unsigned long counter = digest[7];
    for (char i = 6; i >= 0; i--) {
        counter <<= 8;
        counter |= digest[i];
    }

    /*
        Get a plain text using the above counter.
        We multiply the table number by the iteration to have
        tables with reduction functions that are different enough
        (just adding the table number isn't optimal).

        The overflow that can happen on the counter variable isn't
        an issue since it happens reliably.

        https://www.gnu.org/software/autoconf/manual/autoconf-2.63/html_node/Integer-Overflow-Basics.html
    */
    create_startpoint(counter + iteration * table_number, plain_text);
}

void create_startpoint(unsigned long counter, char *plain_text) {
    for (int i = PASSWORD_LENGTH - 1; i >= 0; i--) {
        plain_text[i] = char_in_range(counter % 64);
        counter /= 64;
    }

    plain_text[PASSWORD_LENGTH] = '\0';
}

inline void insert_chain(RainbowTable *table, char *startpoint,
                         char *endpoint) {
    strcpy(table->chains[table->length].startpoint, startpoint);
    strcpy(table->chains[table->length].endpoint, endpoint);
    table->length++;
}

RainbowChain *binary_search(RainbowTable *table, char *endpoint) {
    int start = 0;
    int end = table->length - 1;
    while (start <= end) {
        int middle = start + (end - start) / 2;
        if (!strcmp(table->chains[middle].endpoint, endpoint)) {
            return &table->chains[middle];
        }

        if (strcmp(table->chains[middle].endpoint, endpoint) < 0) {
            start = middle + 1;
        } else {
            end = middle - 1;
        }
    }
    return NULL;
}

void dedup_endpoints(RainbowTable *table) {
    unsigned long dedup_index = 1;
    for (unsigned long i = 1; i < table->length; i++) {
        if (strcmp(table->chains[i - 1].endpoint, table->chains[i].endpoint)) {
            table->chains[dedup_index] = table->chains[i];
            dedup_index++;
        }
    }

    table->length = dedup_index;
}

RainbowTable gen_table(unsigned char table_number, unsigned long m0) {
    DEBUG_PRINT("\nGenerating table %hhu\n", table_number);

    RainbowChain *chains = malloc(sizeof(RainbowChain) * m0);
    if (!chains) {
        perror("Cannot allocate enough memory for the rainbow table");
        exit(EXIT_FAILURE);
    }

    RainbowTable table = {chains, 0, table_number};

    // generate all rows
    for (unsigned long i = 0; i < m0; i++) {
        // generate the chain
        char last_plain_text[PASSWORD_LENGTH + 1];
        char startpoint[PASSWORD_LENGTH + 1];
        create_startpoint(i, startpoint);
        strcpy(last_plain_text, startpoint);

        /*
            Apply a round of hash + reduce `TABLE_T - 1` times.
            The chain should look like this:

            n -> r0(h(n)) -> r1(h(r0(h(n))) -> ...
        */
        for (unsigned long j = 0; j < TABLE_T - 1; j++) {
            unsigned char digest[HASH_LENGTH];
            HASH(last_plain_text, strlen(last_plain_text), digest);
            reduce_digest(digest, j, table_number, last_plain_text);
        }

        insert_chain(&table, startpoint, last_plain_text);

        if (i % 1000 == 0) {
            DEBUG_PRINT("\rprogress: %.2f%%", (float) (i + 1) / m0 * 100);
        }
    }
    // the debug macro requires at least one variadic argument
    DEBUG_PRINT("%s", " DONE\n");

    // sort the rainbow table by the endpoints
    DEBUG_PRINT("%s", "Sorting table...");
    qsort(table.chains, table.length, sizeof(RainbowChain),
          compare_rainbow_chains);
    DEBUG_PRINT("%s", " DONE\n");

    // deduplicates chains with similar endpoints
    DEBUG_PRINT("%s", "Deleting duplicate endpoints...");
    dedup_endpoints(&table);
    DEBUG_PRINT("%s", " DONE\n");

    return table;
}

void store_table(RainbowTable *table, const char *file_path) {
    FILE *file = fopen(file_path, "w");

    if (!file) {
        perror("Could not open file on disk");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "%lu %hhu\n", table->length, table->number);

    for (unsigned long i = 0; i < table->length; i++) {
        fprintf(file, "%s %s\n", table->chains[i].startpoint,
                table->chains[i].endpoint);
    }

    fclose(file);
}

RainbowTable load_table(const char *file_path) {
    FILE *file = fopen(file_path, "r");

    if (!file) {
        perror("Could not open file on disk");
        exit(EXIT_FAILURE);
    }

    RainbowTable table;
    table.length = 0;

    unsigned long table_length;
    fscanf(file, "%lu %hhu\n", &table_length, &table.number);

    RainbowChain *chains = malloc(sizeof(RainbowChain) * table_length);
    table.chains = chains;
    if (!chains) {
        perror("Cannot allocate enough memory to load this rainbow table");
        exit(EXIT_FAILURE);
    }

    char startpoint[PASSWORD_LENGTH + 1];
    char endpoint[PASSWORD_LENGTH + 1];
    while (fscanf(file, "%s %s\n", startpoint, endpoint) != EOF) {
        insert_chain(&table, startpoint, endpoint);
    }
    assert(table.length == table_length);

    fclose(file);
    return table;
}

void del_table(RainbowTable *table) { free(table->chains); }

void print_hash(const unsigned char *digest) {
    for (int i = 0; i < HASH_LENGTH; i++) {
        printf("%02x", digest[i]);
    }
}

void print_table(const RainbowTable *table) {
    for (unsigned long i = 0; i < table->length; i++) {
        printf("%s -> ... -> %s\n", table->chains[i].startpoint,
               table->chains[i].endpoint);
    }
}

void print_matrix(const RainbowTable *table) {
    for (unsigned long i = 0; i < table->length; i++) {
        char plain_text[PASSWORD_LENGTH + 1];
        unsigned char digest[HASH_LENGTH];
        strcpy(plain_text, table->chains[i].startpoint);

        for (unsigned long j = 0; j < TABLE_T - 1; j++) {
            HASH(plain_text, strlen(plain_text), digest);
            printf("%s -> ", plain_text);
            print_hash(digest);
            printf(" -> ");
            reduce_digest(digest, j, table->number, plain_text);
        }

        assert(!strcmp(table->chains[i].endpoint, plain_text));
        printf("%s\n", plain_text);
    }
}

int search_endpoint(char** endpoints, char* plain_text, int mt) {
    for(int i = 0; i < mt; i++){
        //printf("Comparing %s and ", endpoints[i]);
        //printf("%s", plain_text);
        //printf("\n");
        if (!strcmp(endpoints[i], plain_text)) {
            printf("Match found when comparing %s and %s (row %d).\n", endpoints[i], plain_text, i);
            return i;
        }
    }
    return -1;
}

void char_to_password(char text[], Password *password) {
    for (int i = 0; i < PASSWORD_LENGTH; i++) {
        password->bytes[i] = text[i];
    }
}

void password_to_char(Password *password, char text[]) {
    for (int i = 0; i < PASSWORD_LENGTH; i++) {
        text[i] = password->bytes[i];
    }
}

void char_to_digest(char text[], Digest *digest) {
    for (int i = 0; i < HASH_LENGTH; i++) {
        char hex[2];
        hex[0] = text[i*2];
        hex[1] = text[(i*2)+1];
        uint32_t num = (int)strtol(hex, NULL, 16);
        digest->bytes[i] = num;
    }
}

void display_digest(Digest *digest) {
    for (unsigned char i = 0; i < HASH_LENGTH; i++) {
            printf("%02X", (unsigned char) digest->bytes[i]);
    }
}

void display_password(Password *pwd) {
    for (unsigned char i = 0; i < PASSWORD_LENGTH; i++) {
        printf("%c", (unsigned char) pwd->bytes[i]);
    }
}

void reduce_digest2(char* char_digest, unsigned int index, char* char_plain, int pwd_length) {
    Digest *digest = (Digest *) malloc(sizeof(Digest));
    // printf("\nDEBUT REDUCTION : %s", char_digest);
    //print_hash(char_digest);
    char_to_digest(char_digest, digest);

    // printf("\nDigest : ");
    // display_digest(digest);

    Password *plain_text = (Password *) malloc(sizeof(Password));
    char_to_password("abcdefg", plain_text);

    // printf("   ---   Password : ");
    // display_password(plain_text);

    (*plain_text).i[0] =
            charset[((*digest).bytes[0] + index) % CHARSET_LENGTH] |
            (charset[((*digest).bytes[1] + index) % CHARSET_LENGTH] << 8)|
            (charset[((*digest).bytes[2] + index) % CHARSET_LENGTH] << 16)|
            (charset[((*digest).bytes[3] + index) % CHARSET_LENGTH] << 24);
    (*plain_text).i[1] =
            charset[((*digest).bytes[4] + index) % CHARSET_LENGTH] |
            (charset[((*digest).bytes[5] + index) % CHARSET_LENGTH] << 8)|
            (charset[((*digest).bytes[6] + index) % CHARSET_LENGTH] << 16)|
            (charset[((*digest).bytes[7] + index) % CHARSET_LENGTH] << 24);
    password_to_char(plain_text, char_plain);

    // printf("\n");
    // printf(" %s a été réduit en '%s'\n", char_digest, char_plain);
}

void ntlm(char *key, char *hash) {

    // printf("Key: %s", key);
    int i, j;
    int length = strlen(key);
    unsigned int nt_buffer[16],
            a, b, c, d, sqrt2, sqrt3, n,
            output[4];
    static char hex_format[33];
    char itoa16[16] = "0123456789abcdef";

    memset(nt_buffer, 0, 16 * sizeof(unsigned int));
    //The length of key need to be <= 27
    for (i = 0; i < length / 2; i++)
        nt_buffer[i] = key[2 * i] | (key[2 * i + 1] << 16);

    //padding
    if (length % 2 == 1)
        nt_buffer[i] = key[length - 1] | 0x800000;
    else
        nt_buffer[i] = 0x80;
    //put the length
    nt_buffer[14] = length << 4;

    output[0] = a = 0x67452301;
    output[1] = b = 0xefcdab89;
    output[2] = c = 0x98badcfe;
    output[3] = d = 0x10325476;
    sqrt2 = 0x5a827999;
    sqrt3 = 0x6ed9eba1;

    /* Round 1 */
    a += (d ^ (b & (c ^ d))) + nt_buffer[0];
    a = (a << 3) | (a >> 29);
    d += (c ^ (a & (b ^ c))) + nt_buffer[1];
    d = (d << 7) | (d >> 25);
    c += (b ^ (d & (a ^ b))) + nt_buffer[2];
    c = (c << 11) | (c >> 21);
    b += (a ^ (c & (d ^ a))) + nt_buffer[3];
    b = (b << 19) | (b >> 13);

    a += (d ^ (b & (c ^ d))) + nt_buffer[4];
    a = (a << 3) | (a >> 29);
    d += (c ^ (a & (b ^ c))) + nt_buffer[5];
    d = (d << 7) | (d >> 25);
    c += (b ^ (d & (a ^ b))) + nt_buffer[6];
    c = (c << 11) | (c >> 21);
    b += (a ^ (c & (d ^ a))) + nt_buffer[7];
    b = (b << 19) | (b >> 13);

    a += (d ^ (b & (c ^ d))) + nt_buffer[8];
    a = (a << 3) | (a >> 29);
    d += (c ^ (a & (b ^ c))) + nt_buffer[9];
    d = (d << 7) | (d >> 25);
    c += (b ^ (d & (a ^ b))) + nt_buffer[10];
    c = (c << 11) | (c >> 21);
    b += (a ^ (c & (d ^ a))) + nt_buffer[11];
    b = (b << 19) | (b >> 13);

    a += (d ^ (b & (c ^ d))) + nt_buffer[12];
    a = (a << 3) | (a >> 29);
    d += (c ^ (a & (b ^ c))) + nt_buffer[13];
    d = (d << 7) | (d >> 25);
    c += (b ^ (d & (a ^ b))) + nt_buffer[14];
    c = (c << 11) | (c >> 21);
    b += (a ^ (c & (d ^ a))) + nt_buffer[15];
    b = (b << 19) | (b >> 13);

    /* Round 2 */
    a += ((b & (c | d)) | (c & d)) + nt_buffer[0] + sqrt2;
    a = (a << 3) | (a >> 29);
    d += ((a & (b | c)) | (b & c)) + nt_buffer[4] + sqrt2;
    d = (d << 5) | (d >> 27);
    c += ((d & (a | b)) | (a & b)) + nt_buffer[8] + sqrt2;
    c = (c << 9) | (c >> 23);
    b += ((c & (d | a)) | (d & a)) + nt_buffer[12] + sqrt2;
    b = (b << 13) | (b >> 19);

    a += ((b & (c | d)) | (c & d)) + nt_buffer[1] + sqrt2;
    a = (a << 3) | (a >> 29);
    d += ((a & (b | c)) | (b & c)) + nt_buffer[5] + sqrt2;
    d = (d << 5) | (d >> 27);
    c += ((d & (a | b)) | (a & b)) + nt_buffer[9] + sqrt2;
    c = (c << 9) | (c >> 23);
    b += ((c & (d | a)) | (d & a)) + nt_buffer[13] + sqrt2;
    b = (b << 13) | (b >> 19);

    a += ((b & (c | d)) | (c & d)) + nt_buffer[2] + sqrt2;
    a = (a << 3) | (a >> 29);
    d += ((a & (b | c)) | (b & c)) + nt_buffer[6] + sqrt2;
    d = (d << 5) | (d >> 27);
    c += ((d & (a | b)) | (a & b)) + nt_buffer[10] + sqrt2;
    c = (c << 9) | (c >> 23);
    b += ((c & (d | a)) | (d & a)) + nt_buffer[14] + sqrt2;
    b = (b << 13) | (b >> 19);

    a += ((b & (c | d)) | (c & d)) + nt_buffer[3] + sqrt2;
    a = (a << 3) | (a >> 29);
    d += ((a & (b | c)) | (b & c)) + nt_buffer[7] + sqrt2;
    d = (d << 5) | (d >> 27);
    c += ((d & (a | b)) | (a & b)) + nt_buffer[11] + sqrt2;
    c = (c << 9) | (c >> 23);
    b += ((c & (d | a)) | (d & a)) + nt_buffer[15] + sqrt2;
    b = (b << 13) | (b >> 19);

    /* Round 3 */
    a += (d ^ c ^ b) + nt_buffer[0] + sqrt3;
    a = (a << 3) | (a >> 29);
    d += (c ^ b ^ a) + nt_buffer[8] + sqrt3;
    d = (d << 9) | (d >> 23);
    c += (b ^ a ^ d) + nt_buffer[4] + sqrt3;
    c = (c << 11) | (c >> 21);
    b += (a ^ d ^ c) + nt_buffer[12] + sqrt3;
    b = (b << 15) | (b >> 17);

    a += (d ^ c ^ b) + nt_buffer[2] + sqrt3;
    a = (a << 3) | (a >> 29);
    d += (c ^ b ^ a) + nt_buffer[10] + sqrt3;
    d = (d << 9) | (d >> 23);
    c += (b ^ a ^ d) + nt_buffer[6] + sqrt3;
    c = (c << 11) | (c >> 21);
    b += (a ^ d ^ c) + nt_buffer[14] + sqrt3;
    b = (b << 15) | (b >> 17);

    a += (d ^ c ^ b) + nt_buffer[1] + sqrt3;
    a = (a << 3) | (a >> 29);
    d += (c ^ b ^ a) + nt_buffer[9] + sqrt3;
    d = (d << 9) | (d >> 23);
    c += (b ^ a ^ d) + nt_buffer[5] + sqrt3;
    c = (c << 11) | (c >> 21);
    b += (a ^ d ^ c) + nt_buffer[13] + sqrt3;
    b = (b << 15) | (b >> 17);

    a += (d ^ c ^ b) + nt_buffer[3] + sqrt3;
    a = (a << 3) | (a >> 29);
    d += (c ^ b ^ a) + nt_buffer[11] + sqrt3;
    d = (d << 9) | (d >> 23);
    c += (b ^ a ^ d) + nt_buffer[7] + sqrt3;
    c = (c << 11) | (c >> 21);
    b += (a ^ d ^ c) + nt_buffer[15] + sqrt3;
    b = (b << 15) | (b >> 17);

    output[0] += a;
    output[1] += b;
    output[2] += c;
    output[3] += d;
    //Iterate the integer
    for (i = 0; i < 4; i++)
        for (j = 0, n = output[i]; j < 4; j++) {
            unsigned int convert = n % 256;
            hex_format[i * 8 + j * 2 + 1] = itoa16[convert % 16];
            convert = convert / 16;
            hex_format[i * 8 + j * 2 + 0] = itoa16[convert % 16];
            n = n / 256;
        }
    //null terminate the string
    hex_format[33] = 0;
    strcpy(hash, hex_format);
    // printf("   ---   Hash: %s\n", hash);
}

void online_from_files(char *start_path, char *end_path, unsigned char *digest, char *password) {
    FILE *fp;
    char buff[255];

    fp = fopen(start_path, "r");

    // Retrieve the number of points
    fscanf(fp, "%s", buff);
    int mt;
    sscanf(buff, "%d", &mt);
    printf("Number of points: %d\n", mt);
    fgets(buff, 255, (FILE *) fp);

    // Retrieve the password length
    int pwd_length;
    fgets(buff, 255, (FILE *) fp);
    sscanf(buff, "%d", &pwd_length);
    printf("Password length: %d\n", pwd_length);

    // Retrieve the chain length (t)
    int t;
    fgets(buff, 255, (FILE *) fp);
    sscanf(buff, "%d", &t);
    printf("Chain length (t): %d\n", t);

    char **startpoints = malloc(sizeof(char*) * mt);
    for(int i = 0; i < mt; i++) {
        startpoints[i] = malloc(sizeof(char) * pwd_length);
    }
    for (int i = 0; i < mt; i++) {
        fgets(buff, 255, (FILE*)fp);
        startpoints[i] = strdup(buff);
    }

    // Close the start file
    fclose(fp);

    FILE *fp2;
    char buff2[255];
    fp2 = fopen(end_path, "r");
    fgets(buff2, 255, (FILE *) fp2);
    fgets(buff2, 255, (FILE *) fp2);
    fgets(buff2, 255, (FILE *) fp2);

    char **endpoints = malloc(sizeof(char*) * mt);
    for(int i = 0; i < mt; i++) {
        endpoints[i] = malloc(sizeof(char) * pwd_length);
    }
    for (int i = 0; i < mt; i++) {
        fgets(buff2, 255, (FILE*)fp);
        endpoints[i] = strdup(buff2);
    }

    // Close the end file
    fclose(fp2);

    for (long i = t - 1; i >= 0; i--) {
        char column_plain_text[pwd_length + 1];
        unsigned char column_digest[HASH_LENGTH];
        strcpy(column_digest, digest);

        // printf("\nstrcpy : digest: %s\n", digest);
        // printf("strcpy : column_digest: %s\n", column_digest);

        printf("\nWe suppose that the digest '%s' is in row %lu\n", digest, i);

        // get the reduction corresponding to the current column
        for (unsigned long k = i; k < t - 1; k++) {
            reduce_digest2(column_digest, k, column_plain_text, pwd_length);
            ntlm(column_plain_text, column_digest);
            printf("k=%d   -   password: '%s'   -   hash: '%s'\n", k, column_plain_text, column_digest);
        }
        reduce_digest2(column_digest, t - 1, column_plain_text, pwd_length);
        printf("k=%d   -   password: '%s'   -   hash: '%s'\n", t - 1, column_plain_text, column_digest);

        printf("Trying to find %s in endpoints...\n", column_plain_text);
        int found = search_endpoint(endpoints, column_plain_text, mt);

        if (found == -1) {
            continue;
        }

        printf("Found at index %d", found);

        // we found a matching endpoint, reconstruct the chain
        char chain_plain_text[pwd_length + 1];
        unsigned char chain_digest[HASH_LENGTH];
        strcpy(chain_plain_text, startpoints[found]);

        for (unsigned long k = 0; k < i; k++) {
            ntlm(chain_plain_text, chain_digest);
            reduce_digest2(chain_digest, k, chain_plain_text, pwd_length);
        }
        ntlm(chain_plain_text, chain_digest);

        if (!memcmp(chain_digest, digest, HASH_LENGTH)) {
            strcpy(password, chain_plain_text);
            return;
        }
        printf("   ---   False alert.\n", i);
    }

    strcpy(password, "");
}

void offline(RainbowTable *rainbow_tables) {
    // the number of possible passwords
    unsigned long long n = pow(64, PASSWORD_LENGTH);

    // the expected number of unique chains
    unsigned long mtmax = 2 * n / (TABLE_T + 2);

    // the number of starting chains, given the alpha coefficient
    unsigned long m0 = TABLE_ALPHA / (1 - TABLE_ALPHA) * mtmax;

    DEBUG_PRINT(
            "Generating %hhu table(s)\n"
            "Password length = %hhu\n"
            "Chain length (t) = %lu\n"
            "Maximality factor (alpha) = %.3f\n"
            "Optimal number of starting chains given alpha (m0) = %lu\n",
            TABLE_COUNT, PASSWORD_LENGTH, TABLE_T, TABLE_ALPHA, m0);

    for (unsigned char i = 0; i < TABLE_COUNT; i++) {
        rainbow_tables[i] = gen_table(i + 1, m0);
    }
}

void online(RainbowTable *rainbow_tables, unsigned char *digest,
            char *password) {
    /*
        Iterate column by column, starting from the last digest.
        https://stackoverflow.com/questions/3623263/reverse-iteration-with-an-unsigned-loop-variable

        We iterate through all tables at the same time because it's faster
       to find a match in the last columns.
    */
    for (unsigned long i = TABLE_T - 1; i-- > 0;) {
        for (int j = 0; j < TABLE_COUNT; j++) {
            unsigned char tn = rainbow_tables[j].number;

            char column_plain_text[PASSWORD_LENGTH + 1];
            unsigned char column_digest[HASH_LENGTH];
            memcpy(column_digest, digest, HASH_LENGTH);

            // get the reduction corresponding to the current column
            for (unsigned long k = i; k < TABLE_T - 2; k++) {
                reduce_digest(column_digest, k, tn, column_plain_text);
                HASH(column_plain_text, strlen(column_plain_text),
                     column_digest);
            }
            reduce_digest(column_digest, TABLE_T - 2, tn, column_plain_text);

            RainbowChain *found =
                    binary_search(&rainbow_tables[j], column_plain_text);

            if (!found) {
                continue;
            }

            // we found a matching endpoint, reconstruct the chain
            char chain_plain_text[PASSWORD_LENGTH + 1];
            unsigned char chain_digest[HASH_LENGTH];
            strcpy(chain_plain_text, found->startpoint);

            for (unsigned long k = 0; k < i; k++) {
                HASH(chain_plain_text, strlen(chain_plain_text), chain_digest);
                reduce_digest(chain_digest, k, tn, chain_plain_text);
            }
            HASH(chain_plain_text, strlen(chain_plain_text), chain_digest);

            /*
                The digest was indeed present in the chain, this was
                not a false positive from a reduction. We found a
                plain text that matches the digest!
            */
            if (!memcmp(chain_digest, digest, HASH_LENGTH)) {
                strcpy(password, chain_plain_text);
                return;
            }
        }
    }

    // no match found
    password[0] = '\0';
}
