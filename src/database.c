#include "database.h"

int table[NUM_KEYS];

/* Set a key and value in the database */
int set_key_value(int key, int value)
{
	if (key < 0 || key >= NUM_KEYS)
		return -1;

	table[key] = value;
	return 0;
}

/* Query the value associated with a key in the database */
int query_key(int key, int *value)
{
	if (key < 0 || key >= NUM_KEYS) {
		*value = -1;
		return -1;
	}

	*value = table[key];
	return 0;
}

#define NUM_KEYS (32 * 1024)
