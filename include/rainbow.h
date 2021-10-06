#ifndef RAINBOW_H
#define RAINBOW_H

#include <assert.h>
#include <openssl/sha.h>
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

/*
    A chain of the rainbow table.
    It contains a startpoint and an endpoint.
*/
typedef struct {
    char startpoint[MAX_PASSWORD_LENGTH + 1];
    char endpoint[MAX_PASSWORD_LENGTH + 1];
} RainbowChain;

/*
    A rainbow table.
    It's an array of chains, with a length.
*/
typedef struct {
    RainbowChain* chains;
    unsigned int length;
} RainbowTable;

/*
   Returns a char in the [a-zA-Z0-9_-] range given a parameter in the
   [0-63] range. Look at an ASCII table to better understand this function
   (https://www.asciitable.com/).
*/
char char_in_range(unsigned char n);

/*
    A reduce operation, which returns a plain text
    for a given cipher text and a given `iteration`.

    The nth `iteration` reduction function should give
    the nth+1 plain text reduction.

    We sum in an array of the size of the plain text and
    take the modulo to get an alphanumeric char.
*/
void reduce_cipher(unsigned char cipher_text[], unsigned int iteration,
                   char* plain_text);

/*
    Transforms a startpoint from a counter to a valid password.
    Note that the last character of the counter is the MSB.

    The implementation uses bit shifting as I didn't think of
    another way of doing this, as a result it only works if the
    range of characters is a multiple of 2.
*/
void create_startpoint(unsigned int counter, char* plain_text);

/*
    Generates a rainbow table of size `TABLE_M*TABLE_T`, where
    `TABLE_M` is the number of rows
    `TABLE_T` is the number of plain texts in a chain.

    The `counter_start` parameter is used to discriminate
    rainbow tables so they're not all similar.
*/
RainbowTable gen_table(unsigned int counter_start);

/*
    Inserts a chain in the rainbow `table` if it's not already present. This is
    not very efficient and a HashSet data structure would be better, but let's
    keep it simple for now.
*/
void insert_chain(RainbowTable* table, char* startpoint, char* endpoint);

// Deletes a table.
void del_table(RainbowTable table);

// Pretty-prints the hash of a cipher text.
void print_hash(unsigned char cipher_text[]);

// Pretty-prints a rainbow table.
void print_table(RainbowTable table);

// Pretty prints the rainbow matrix corresponding to a rainbow table.
void print_matrix(RainbowTable table);

/*
    Offline phase of the attack.
    Generates all rainbow tables needed.
*/
void offline(RainbowTable* rainbow_tables);

/*
    Online phase of the attack.

    Uses the pre-generated rainbow tables to guess
    the plain text of the given cipher text.

    Returns in `password` the match if any, or returns an empty string.
*/
void online(RainbowTable* rainbow_tables, unsigned char* cipher,
            char* password);

#endif