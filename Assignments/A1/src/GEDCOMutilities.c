#include "GEDCOMutilities.h"

HashTable *initHashTable(size_t size) {
	HashTable *hashTable = malloc(sizeof(HashTable));
	if (hashTable == NULL) {
		return NULL;
	}

	hashTable->table = malloc(sizeof(HashTableNode) * size);
	if (hashTable->table == NULL) {
		return NULL;
	}

	for (int i = 0; i < size; i++) {
		hashTable->table[i] = NULL;
	}

	hashTable->size = size;

	return hashTable;
}

HashTableNode *initHashTableNode(char *key, void *data) {
	HashTableNode *hashTableNode = malloc(sizeof(HashTableNode));

	hashTableNode->key = key;
	hashTableNode->data = data;
	hashTableNode->next = NULL;

	return hashTableNode;
}

void insertData(HashTable *hashTable, char *key, void *data) {
	HashTableNode *node, *last;

	int index = hash(hashTable->size, key);

	HashTableNode *next = hashTable->table[index];

	while (next != NULL && next->key != NULL && strcmp(key, next->key) > 0) {
		last = next;
		next = next->next;
	}

	if (next != NULL && next->key != NULL && strcmp(key, next->key) == 0) {
		free(next->data);
		next->data = data;
	} else {
		node = initHashTableNode(key, data);

		if (next == hashTable->table[index]) {                 // insert node at front
			node->next = next;
			hashTable->table[index] = node;
		} else if (next == NULL) {                 // insert node at end
			last->next = node;
		} else {                 // insert node in middle
			node->next = next;
			last->next = node;
		}
	}
}

void *lookupData(HashTable *hashTable, char *key) {
	int index = hash(hashTable->size, key);
	HashTableNode *node = hashTable->table[index];

	while (node != NULL && node->key != NULL && strcmp(key, node->key) > 0) {
		node = node->next;                     // traverse the list
	}

	if (node == NULL || node->key == NULL || strcmp(key, node->key) != 0) {
		return NULL;
	}

	return node->data;
}

int hash(size_t tableSize, char *key) {
	size_t hash, i;

	for (hash = i = 0; i < strlen(key); i++) {
		hash += key[i];
		hash += hash << 8;
		hash ^= hash >> 6;
	}

	return hash % tableSize;
}

void destroyTable(HashTable *hashTable) {
	for (int i = 0; i < hashTable->size; i++) {
		if (hashTable->table[i] != NULL) {
			free(hashTable->table[i]);
			hashTable->table[i] = NULL;
		}
	}

	free(hashTable->table);
	free(hashTable);
}

char *getErrorName(ErrorCode code) {
	switch(code) {
	case OK:
		return "OK";
	case INV_FILE:
		return "Invalid file";
	case INV_GEDCOM:
		return "Invalid GEDCOM";
	case INV_HEADER:
		return "Invalid header";
	case INV_RECORD:
		return "Invalid record";
	case OTHER:
		return "Other";
	default:
		return NULL;
	}
}

CharSet getEncoding(char *name) {
	if (strcmp(name, "ANSEL") == 0) {
		return ANSEL;
	}

	if (strcmp(name, "UTF8") == 0) {
		return UTF8;
	}

	if (strcmp(name, "UNICODE") == 0) {
		return UNICODE;
	}

	if (strcmp(name, "ASCII") == 0) {
		return ASCII;
	}

	return ANSEL;             // todo default val?
}

char *getEncodingName(CharSet encoding) {
	char *names[] = {"Ansel", "UTF-8", "Unicode", "ASCII"};

	return names[encoding];
}

void deleteHeader(void *toBeDeleted) {
	Header *header = (Header *) toBeDeleted;

	if (header == NULL) {
		return;
	}

	deleteSubmitter(header->submitter);

	clearList(&header->otherFields);

	free(header);
	header = NULL;
}

char *printHeader(void *toBePrinted) {
	Header *header = (Header *) toBePrinted;

	if (header == NULL) {
		return NULL;
	}

	ListIterator iter;
	Field *field;

	char *encoding = getEncodingName(header->encoding);
	char *submitter = printSubmitter(header->submitter);
	char *str = malloc(strlen(header->source) + strlen(encoding) + strlen(submitter) + 100);
	sprintf(str, "\nHEADER:\nSource: %s\nVersion: %f\nEncoding: %s\n%s", header->source, header->gedcVersion, encoding, submitter);

	free(submitter);

	iter = createIterator(header->otherFields);
	while ((field = nextElement(&iter)) != NULL) {
		char *fieldStr = printField(field);
		str = realloc(str, strlen(str) + strlen(fieldStr) + 50);
		sprintf(str + strlen(str), "\n%s", fieldStr);
		free(fieldStr);
	}

	str[strlen(str)] = '\0';

	return str;
}

void deleteSubmitter(void *toBeDeleted) {
	Submitter *subm = (Submitter *) toBeDeleted;

	if (subm == NULL) {
		return;
	}

	clearList(&subm->otherFields);

	//TODO maybe have to free address
	//free(subm->address);

	free(subm);
	subm = NULL;
}

char *printSubmitter(void *toBePrinted) {
	Submitter *subm = (Submitter *) toBePrinted;

	if (subm == NULL) {
		return NULL;
	}

	ListIterator iter;
	Field *field;

	char *str = malloc(strlen(subm->submitterName) + 50);
	sprintf(str, "\nSUBMITTER:\nName: %s", subm->submitterName);

	iter = createIterator(subm->otherFields);
	while ((field = nextElement(&iter)) != NULL) {
		char *fieldStr = printField(field);
		str = realloc(str, strlen(str) + strlen(fieldStr) + 50);
		sprintf(str + strlen(str), "\n%s", fieldStr);
		free(fieldStr);
	}

	//str = realloc(str, strlen(str) + strlen(subm->address) + 50);
	//sprintf(str + strlen(str), ", Address: %s", subm->address);

	str[strlen(str)] = '\0';

	return str;
}

Field *initializeField(char *tag, char *val) {
	Field *field = malloc(sizeof(Field));

	field->tag = malloc(strlen(tag) + 1);
	strcpy(field->tag, tag);
	field->tag[strlen(field->tag)] = '\0';

	field->value = malloc(strlen(val) + 1);
	strcpy(field->value, val);
	field->value[strlen(field->value)] = '\0';

	return field;
}
