#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <openssl/sha.h>
#include <assert.h>

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

/*
	A rainbow table.

	The endpoints are an array of plain texts.
	The startpoint is the plain text of the first raw,
	as an unsigned int.

	A NULL endpoint means that the chain has been pruned
	from the table.
*/
typedef struct {
	unsigned int startpoint;
	char** endpoints;
} RainbowTable;

/*
	Returns a char in the [a-zA-Z0-9_-] range given a parameter in the [0-63] range.
	Look at an ASCII table to better understand this function (https://www.asciitable.com/).
*/
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

/*
	A reduce operation, which returns a plain text
	for a given cipher text and a given `iteration`.

	The nth `iteration` reduction function should give
	the nth+1 plain text reduction.

	We sum in an array of the size of the plain text and
	take the modulo to get an alphanumeric char.
*/
void reduce_cipher(unsigned char cipher_text[], unsigned int iteration, char* plain_text) {
	unsigned int sum[MAX_PASSWORD_LENGTH] = { 0 };
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

/*
	Transforms a startpoint from a counter to a valid password.
	Note that the last character of the counter is the MSB.

	The implementation uses bit shifting as I didn't think of
	another way of doing this, as a result it only works if the
	range of characters is a multiple of 2.
*/
void reduce_startpoint(unsigned int counter, char* plain_text) {
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

/*
	Generates a rainbow table of size `TABLE_M*TABLE_T`, where
	`TABLE_M` is the number of rows
	`TABLE_T` is the number of plain texts in a chain.

	The `counter_start` parameter is used to discriminate
	rainbow tables so they're not all similar.
*/
RainbowTable gen_table(unsigned int counter_start) {
	char** endpoints = malloc(sizeof(char*) * TABLE_M);
	assert(endpoints != NULL);

	// generate all rows
	for (unsigned int i = 0; i < TABLE_M; i++) {
		// generate the chain
		endpoints[i] = malloc(sizeof(char) * MAX_PASSWORD_LENGTH + 1);
		assert(endpoints[i] != NULL);

		char last_plain_text[MAX_PASSWORD_LENGTH + 1];
		reduce_startpoint(counter_start + i, last_plain_text);

		/*
			Apply a round of hash + reduce `TABLE_T - 1` times.
			The chain should look like this:

			n -> r0(h(n)) -> r1(h(r0(h(n))) -> ...
		*/
		for (unsigned int j = 0; j < TABLE_T - 1; j++) {
			unsigned char cipher_text[HASH_LENGTH];
			HASH_FUNCTION(last_plain_text, strlen(last_plain_text), cipher_text);
			reduce_cipher(cipher_text, j, last_plain_text);
		}

		// insert only if the row is unique.
		// else, we mark the endpoint as NULL.
		bool merge = false;
		for (unsigned int j = 0; j < i; j++) {
			if (endpoints[j] && !strcmp(last_plain_text, endpoints[j])) {
				merge = true;
				break;
			}
		}

		if (merge) {
			free(endpoints[i]);
			endpoints[i] = NULL;
		}
		else {
			strcpy(endpoints[i], last_plain_text);
		}
	}

	return (RainbowTable) { counter_start, endpoints };
}

// Deletes a table.
void del_table(RainbowTable table) {
	for (unsigned int i = 0; i < TABLE_M; i++) {
		free(table.endpoints[i]);
	}

	free(table.endpoints);
}

// Pretty-prints the hash of a cipher text.
void print_hash(unsigned char cipher_text[]) {
	for (int i = 0; i < HASH_LENGTH; i++) {
		printf("%02x", cipher_text[i]);
	}
}

// Pretty-prints a rainbow table.
void print_table(RainbowTable table) {
	for (unsigned int i = 0; i < TABLE_M; i++) {
		char startpoint[MAX_PASSWORD_LENGTH + 1];
		reduce_startpoint(table.startpoint + i, startpoint);

		printf(
			"%s -> ... -> %s\n",
			startpoint,
			table.endpoints[i]
		);
	}
}

// Pretty prints the rainbow matrix corresponding to a rainbow table.
void print_matrix(RainbowTable table) {
	for (unsigned int i = 0; i < TABLE_M; i++) {
		char plain_text[MAX_PASSWORD_LENGTH + 1];
		unsigned char cipher_text[HASH_LENGTH];
		reduce_startpoint(table.startpoint + i, plain_text);

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

/*
	Offline phase of the attack.
	Generates all rainbow tables needed.
*/
void offline(RainbowTable* rainbow_tables) {
	for (int i = 0; i < TABLE_COUNT; i++) {
		rainbow_tables[i] = gen_table(i * TABLE_M);
	}
}

/*
	Online phase of the attack.

	Uses the pre-generated rainbow tables to guess
	the plain text of the given cipher text.

	Returns in `password` the match if any, or returns an empty string.
*/
void online(RainbowTable* rainbow_tables, unsigned char* cipher, char* password) {
	for (int i = 0; i < TABLE_COUNT; i++) {
		// iterate column by column, starting from the last plaintext
		// https://stackoverflow.com/questions/3623263/reverse-iteration-with-an-unsigned-loop-variable
		for (unsigned int j = TABLE_T - 1; j-- > 0;) {
			char column_plain_text[MAX_PASSWORD_LENGTH + 1];
			unsigned char column_cipher_text[HASH_LENGTH];
			memcpy(column_cipher_text, cipher, HASH_LENGTH);

			// get the reduction of the cipher text corresponding to the current column
			for (unsigned int k = j; k < TABLE_T - 2; k++) {
				reduce_cipher(column_cipher_text, k, column_plain_text);
				HASH_FUNCTION(column_plain_text, strlen(column_plain_text), column_cipher_text);
			}
			reduce_cipher(column_cipher_text, TABLE_T - 2, column_plain_text);

			// iterate through all rows to check if it'a an endpoint
			for (unsigned int k = 0; k < TABLE_M; k++) {
				// we found a matching endpoint
				if (rainbow_tables[i].endpoints[k] && !strcmp(rainbow_tables[i].endpoints[k], column_plain_text)) {
					// re-construct the chain
					char chain_plain_text[MAX_PASSWORD_LENGTH + 1];
					unsigned char chain_cipher_text[HASH_LENGTH];
					reduce_startpoint(rainbow_tables[i].startpoint + k, chain_plain_text);

					for (unsigned int l = 0; l < j; l++) {
						HASH_FUNCTION(chain_plain_text, strlen(chain_plain_text), chain_cipher_text);
						reduce_cipher(chain_cipher_text, l, chain_plain_text);
					}
					HASH_FUNCTION(chain_plain_text, strlen(chain_plain_text), chain_cipher_text);

					/*
						The cipher was indeed present in the chain, this was
						not a false positive from a reduction.
						We found a plain text that matches the cipher text!
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
	}
	else {
		printf("Password '%s' found for the given hash!\n", found);
	}

	for (int i = 0; i < TABLE_COUNT; i++) {
		del_table(rainbow_tables[i]);
	}

	return 0;
}
