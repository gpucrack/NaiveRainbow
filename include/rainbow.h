#ifndef RAINBOW_H
#define RAINBOW_H

#define _CRT_SECURE_NO_WARNINGS

#include <assert.h>
#include <math.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The max password length in the rainbow tables.
#define MAX_PASSWORD_LENGTH 3

// The hash function used.
#define HASH_FUNCTION SHA1

// The length of the cipher text produced by the hash function.
#define HASH_LENGTH 20

/*
    The maximality factor, such as the number of chains at the end of the
    offline phase is alpha*mtmax, where mtmax is the expected maximum number of
    chains in a rainbow table.
*/
#define TABLE_ALPHA 0.952

// The length of a chain in the table.
#define TABLE_T 100

// The number of tables.
#define TABLE_COUNT 1

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
*/
void create_startpoint(unsigned int counter, char* plain_text);

/*
    Generates a rainbow table of size `m0*TABLE_T`, where
    `m0` is the number of rows (chains)
    `TABLE_T` is the number of plain texts in a chain.

    The `table_number` parameter is used to discriminate
    rainbow tables so they're not all similar.
*/
RainbowTable gen_table(unsigned char table_number, unsigned int m0);

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