#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <assert.h>

// The max password length in the rainbow tables.
#define MAX_PASSWORD_LENGTH 4

// The hash function used.
#define HASH_FUNCTION SHA1

// The length of the cipher text produced by the hash function.
#define HASH_LENGTH 20

// The number of rows in the table.
#define TABLE_M 20

// The size of a chain in the table.
#define TABLE_T 5

/*
	The data contained in a rainbow table.
	It can be either a plain text, or a cipher text.
*/
typedef union {
	char plain_text[MAX_PASSWORD_LENGTH + 1];
	unsigned char cipher_text[HASH_LENGTH];

} Data;

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
	for a given cipher text and a given convolution.

	We sum in an array of the size of the plain text and
	take the modulo to get an alphanumeric char.
*/
void reduce(unsigned char cipher_text[], unsigned int conv, char* plain_text) {
	unsigned int sum[MAX_PASSWORD_LENGTH] = { 0 };
	unsigned int current_index = 0;

	for (unsigned int i = 0; i < HASH_LENGTH; i++) {
		sum[i % MAX_PASSWORD_LENGTH] += cipher_text[i];
	}

	for (unsigned int i = 0; i < MAX_PASSWORD_LENGTH; i++) {
		unsigned int mod = (sum[i] + conv) % 65;

		// if mod == 64, skip a character (smaller password)
		if (mod != 64) {
			plain_text[current_index] = char_in_range(mod);
			current_index++;
		}
	}

	plain_text[current_index] = '\0';
}

// Generates a rainbow table.
Data** gen_table() {
	// the chain length should be uneven so it starts and ends with a plain text.
	assert(TABLE_T % 2 == 1);

	Data** table = malloc(sizeof(Data*) * TABLE_M);
	assert(table != NULL);

	for (unsigned int i = 0; i < TABLE_M; i++) {
		table[i] = malloc(sizeof(Data) * TABLE_T);
		assert(table[i] != NULL);
	}

	assert(table != NULL);

	for (unsigned int i = 0; i < TABLE_M; i++) {
		// the starting point increments by one each loop
		sprintf(table[i][0].plain_text, "%u", i);

		for (unsigned int j = 1; j < TABLE_T; j++) {
			// the uneven links are cipher texts
			if (j % 2) {
				// hash the last plain text, giving this cipher text
				HASH_FUNCTION(table[i][j - 1].plain_text, strlen(table[i][j - 1].plain_text), table[i][j].cipher_text);
			}
			// the even links are plain texts
			else {
				// reduce the last cipher text, giving this plain text
				reduce(table[i][j - 1].cipher_text, i, table[i][j].plain_text);
			}
		}
	}

	return table;
}

// Pretty-prints the hash of a cipher text.
void print_hash(unsigned char cipher_text[]) {
	for (int i = 0; i < HASH_LENGTH; i++) {
		printf("%x", cipher_text[i]);
	}
}

// Pretty prints a rainbow table.
void print_table(Data** table) {
	for (unsigned int i = 0; i < TABLE_M; i++) {
		for (unsigned int j = 0; j < TABLE_T - 1; j++) {
			if (j % 2) {
				print_hash(table[i][j].cipher_text);
				printf(" -> ");
			}

			else {
				printf("%s -> ", table[i][j].plain_text);
			}
		}

		printf("%s\n", table[i][TABLE_T - 1].plain_text);
	}
}

int main() {
	Data** rainbow_table = gen_table();
	print_table(rainbow_table);

	return 0;
}
