#pragma once

/* Set a key and value in the database */
int set_key_value(int key, int value);

/* Query the value associated with a key in the database */
int query_key(int key, int *value);

#define NUM_KEYS (32 * 1024)

/* Array containing the database */
extern int table[NUM_KEYS];
