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

	hashTableNode->key = malloc(strlen(key) + 1);
	strcpy(hashTableNode->key, key);
	hashTableNode->data = data;
	hashTableNode->next = NULL;

	return hashTableNode;
}

void insertData(HashTable *hashTable, char *key, void *data) {
	HashTableNode *node = initHashTableNode(key, data);
	int index = hash(hashTable->size, key);

	if (hashTable->table[index] == NULL) {
		hashTable->table[index] = node;
	} else {
		HashTableNode *temp = hashTable->table[index];
		while (temp->next != NULL) {
			temp = temp->next;
		}

		temp->next = node;
	}
}

void *lookupData(HashTable *hashTable, char *key) {
	int index = hash(hashTable->size, key);
	HashTableNode *node = hashTable->table[index];

	if (node == NULL) {
		return NULL;
	}

	if (strcmp(node->key, key) == 0) {
		return node->data;
	}

	while (node->next != NULL) {
		node = node->next;

		if (strcmp(node->key, key) == 0) {
			return node->data;
		}
	}

	return NULL;
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
			HashTableNode *node = hashTable->table[i];
			if (node->next == NULL) {
				free(hashTable->table[i]->key);
				free(hashTable->table[i]);
				hashTable->table[i] = NULL;
			} else {
				HashTableNode *prev = NULL;
				while (node->next != NULL) {
					prev = node;
					node = node->next;

					free(prev->key);
					free(prev);
					prev = NULL;
				}

				free(node->key);
				free(node);
				node = NULL;
			}
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
	case OTHER_ERROR:
		return "Other";
	default:
		return NULL;
	}
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
	sprintf(str, "\nHEADER:\nSource: %s\nVersion: %.2f\nEncoding: %s\n%s", header->source, header->gedcVersion, encoding, submitter);

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

	if (subm->address[0] != '\0') {
		str = realloc(str, strlen(str) + strlen(subm->address) + 50);
		sprintf(str + strlen(str), "\nAddress: %s", subm->address);
	}

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

bool isIndivEvent(char *tag) {
	char *events[] = {
		"BIRT", "CHR", "DEAT", "BURI", "CREM", "ADOP", "BAPM", "BARM", "BASM",
		"BLES", "CHRA", "CONF", "FCOM", "ORDN", "NATU", "EMIG", "IMMI", "CENS",
		"PROB", "WILL", "GRAD", "RETI", "EVEN"
	};

	for (int i = 0; i < 23; i++) {
		if (strcmp(tag, events[i]) == 0) {
			return true;
		}
	}

	return false;
}

bool isFamEvent(char *tag) {
	char *events[] = {
		"ANUL", "CENS", "DIV", "DIVF", "ENGA", "MARB", "MARC", "MARR", "MARL",
		"MARS", "RESI", "EVEN"
	};

	for (int i = 0; i < 12; i++) {
		if (strcmp(tag, events[i]) == 0) {
			return true;
		}
	}

	return false;
}

void clearListFake(List *list) {
	if (list == NULL) {
		return;
	}

	Node *node;

	while (list->head != NULL) {
		node = list->head;
		list->head = list->head->next;

		free(node);
	}

	list->head = NULL;
	list->tail = NULL;
}

void descendants(List *d, const Individual *person) {
	ListIterator iterFam = createIterator(person->families);
	Family *fam;

	while ((fam = nextElement(&iterFam)) != NULL) {
		bool spouse = false;

		if (fam->wife != NULL && strcmp(fam->wife->givenName, person->givenName) == 0 && strcmp(fam->wife->surname, person->surname) == 0) {
			spouse = true;
		}

		if (fam->husband != NULL && strcmp(fam->husband->givenName, person->givenName) == 0 && strcmp(fam->husband->surname, person->surname) == 0) {
			spouse = true;
		}

		ListIterator iterChild = createIterator(fam->children);
		Individual *child;

		while ((child = nextElement(&iterChild)) != NULL) {
			if (strcmp(child->givenName, person->givenName) == 0 && strcmp(child->surname, person->surname) == 0) {
				break;
			}

			Individual *copy = malloc(sizeof(Individual));
			copy->givenName = calloc(strlen(child->givenName), sizeof(char) + 20);
			copy->surname = calloc(strlen(child->surname), sizeof(char) + 20);
			copy->events = initializeList(&printEvent, &deleteEvent, &compareEvents);

			strcpy(copy->givenName, child->givenName);
			strcpy(copy->surname, child->surname);

			ListIterator iterEvent = createIterator(child->events);
			Event *event;
			Event *birth = NULL;

			while ((event = nextElement(&iterEvent)) != NULL) {
				if (strcmp(event->type, "BIRT") == 0) {
					birth = malloc(sizeof(Event));
					strcpy(birth->type, "BIRT");

					birth->date = malloc(strlen(event->date) + 50);
					strcpy(birth->date, event->date);
					break;
				}
			}

			if (birth != NULL) {
				insertBack(&copy->events, birth);
			}

			ListIterator iter = createIterator(*d);
			Individual *i;
			bool exists = false;

			while ((i = nextElement(&iter)) != NULL) {
				if (strcmp(i->givenName, child->givenName) == 0 && strcmp(i->surname, child->surname) == 0) {
					char b1[100];
					char b2[100];

					b1[0] = 0;
					b2[0] = 0;

					iterEvent = createIterator(i->events);

					while ((event = nextElement(&iterEvent)) != NULL) {
						if (strcmp(event->type, "BIRT") == 0) {
							strcpy(b1, event->date);
						}
					}

					iterEvent = createIterator(child->events);

					while ((event = nextElement(&iterEvent)) != NULL) {
						if (strcmp(event->type, "BIRT") == 0) {
							strcpy(b2, event->date);
						}
					}

					if (b1[0] != 0 && b2[0] != 0) {
						if (strcmp(b1, b2) == 0) {
							exists = true;
						}
					} else if (b1[0] == 0 && b2[0] == 0) {
						exists = true;
					}
				}
			}

			if (!exists) {
				insertBack(d, copy);
			}

			if (getLength(child->families) == 1 && !spouse) {
				continue;
			}

			descendants(d, child);
		}
	}
}
