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

HashTable *initHashTable(size_t size);

HashTableNode *initHashTableNode(char *key, void *data);

void insertData(HashTable *hashTable, char *key, void *data);

void *lookupData(HashTable *hashTable, char *key);

int hash(size_t tableSize, char *key);

void destroyTable(HashTable *hashTable);
/* Hash Table End */

char *getErrorName(ErrorCode code);

CharSet getEncoding(char *name);

char *getEncodingName(CharSet encoding);

void deleteHeader(void *toBeDeleted);

char *printHeader(void *toBePrinted);

void deleteSubmitter(void *toBeDeleted);

char *printSubmitter(void *toBePrinted);

Field *initializeField(char *tag, char *val);

bool isIndivEvent(char *tag);

bool isFamEvent(char *tag);

#endif
