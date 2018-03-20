#ifndef GEDCOMUTILITIES_H
#define GEDCOMUTILITIES_H

#include <stdlib.h>
#include <string.h>
#include "GEDCOMparser.h"

/* Hash Table Start */
typedef struct hashTableNode HashTableNode;

struct hashTableNode {
    char *key;
    void *data;
    HashTableNode *next;
};

typedef struct hashTable {
    size_t size;
    HashTableNode **table;

} HashTable;

/**
 * Initializes a hash table
 * @param size the size of the hash table
 * @return a hash table
 */
HashTable *initHashTable(size_t size);

/**
 * Initializes a hash table node
 * @param key a string that represents the data
 * @param data generic pointer to a data type
 * @return a hash table node
 */
HashTableNode *initHashTableNode(char *key, void *data);

/**
 * Inserts a node in the hash table
 * @param hashTable pointer to the hash table
 * @param key string that represents the data
 * @param data pointer to generic data that is to be inserted into the list
 */
void insertData(HashTable *hashTable, char *key, void *data);

/**
 * Function to return the data from the key given
 * @param hashTable pointer to the hash table containing data nodes
 * @param key string that represents a piece of data in the table
 * @return returns a pointer to the data in the hash table. Returns NULL if no match is found.
 */
void *lookupData(HashTable *hashTable, char *key);

/**
 * Returns the hash code of the given key
 * @param tableSize the size of the hash table
 * @param key the string being hashed
 * @return the hash code of the string key
 */
int hash(size_t tableSize, char *key);

/**
 * Deletes all data allocated to the hash table, aside from data
 * @param hashTable the hash table being deleted
 */
void destroyTable(HashTable *hashTable);
/* Hash Table End */

/**
 * Returns a string representing the given error
 * @param code the error code
 * @return the string representation of the error code
 */
char *getErrorName(ErrorCode code);

/**
 * Returns a CharSet's name as a string
 * @param encoding the CharSet
 * @return the string representation of the encoding
 */
char *getEncodingName(CharSet encoding);

/**
 * Deletes all data allocated to the header
 * @param toBeDeleted void pointer to header
 */
void deleteHeader(void *toBeDeleted);

/**
 * Returns a human-readable string representation of a header
 * @param toBePrinted void pointer to header
 * @return human-readable string representation of the passed header
 */
char *printHeader(void *toBePrinted);

/**
 * Deletes all data allocated to the submitter
 * @param toBeDeleted void pointer to submitter
 */
void deleteSubmitter(void *toBeDeleted);

/**
 * Returns a human-readable string representation of a submitter
 * @param toBePrinted void pointer to submitter
 * @return human-readable string representation of the passed submitter
 */
char *printSubmitter(void *toBePrinted);

/**
 * Initializes a field with the given tag and value
 * @param tag the field's tag
 * @param val the field's value
 * @return the newly allocated field
 */
Field *initializeField(char *tag, char *val);

/**
 * Returns whether or not the given tag is an event for an individual
 * @param tag the tag being checked
 * @return true if the tag is an event, false if otherwise
 */
bool isIndivEvent(char *tag);

/**
 * Returns whether or not the given tag is an event for a family
 * @param tag the tag being checked
 * @return true if the tag is an event, false if otherwise
 */
bool isFamEvent(char *tag);

/**
 * Clears a list without actually deleting any node data
 * @param list the list
 */
void clearListFake(List *list);

void descendants(List *d, const Individual *person);

void descendantsN(List *g[], const Individual *person, int n, int max);

void ancestorsN(List *g[], const Individual *person, int n, int max);

int compareDesc(const void *a, const void *b);

#endif
