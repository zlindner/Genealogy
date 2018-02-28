#include "GEDCOMparser.h"
#include "GEDCOMutilities.h"
#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>

GEDCOMerror createGEDCOM(char *fileName, GEDCOMobject **obj) {
	GEDCOMerror err;
	FILE *file;

	file = fopen(fileName, "r");

	if (file == NULL) {
		fclose(file);
		err.type = INV_FILE;
		err.line = -1;
		return err;
	}

	Submitter *submitter = malloc(sizeof(Submitter) + 100);
	if (submitter == NULL) {
		err.type = OTHER_ERROR;
		err.line = -1;
		return err;
	}
	submitter->address[0] = '\0';
	submitter->otherFields = initializeList(&printField, &deleteField, &compareFields);

	Header *header = malloc(sizeof(Header));
	if (header == NULL) {
		err.type = OTHER_ERROR;
		err.line = -1;
		return err;
	}
	header->submitter = submitter;
	header->otherFields = initializeList(&printField, &deleteField, &compareFields);

	*obj = malloc(sizeof(GEDCOMobject));
	if (*obj == NULL) {
		err.type = OTHER_ERROR;
		err.line = -1;
		return err;
	}

	(*obj)->header = header;
	(*obj)->submitter = submitter;
	(*obj)->families = initializeList(&printFamily, &deleteFamily, &compareFamilies);
	(*obj)->individuals = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);

	HashTable *hashTable = initHashTable(127);
	if (hashTable == NULL) {
		err.type = OTHER_ERROR;
		err.line = -1;
		return err;
	}

	char line[255];
	int lineNum = 0;
	while (fgets(line, sizeof(line), file)) {
		if (line[0] == '\n') { // skip blank lines
			continue;
		}

		if (memchr(line, '\n', strlen(line)) == NULL && memchr(line, '\r', strlen(line)) == NULL) {
			err.type = INV_GEDCOM;
			err.line = lineNum;
			goto ERROR;
		}

		line[strcspn(line, "\n")] = 0;
		line[strcspn(line, "\r")] = 0;
		lineNum++;

		strtok(line, " ");
		char *tag = strtok(NULL, " ");
		if (tag == NULL) {
			err.type = INV_RECORD;
			err.line = lineNum;
			goto ERROR;
		}

		char *value = line + 3 + strlen(tag);

		if (strstr(tag, "@") == NULL) { // only concered with pointers this pass
			continue;
		}

		if (tag[0] == '@') {
			if (tag[strlen(tag) - 1] == '@' && strlen(tag) > 2) {
				if (strcmp(value, "SUBM") == 0) {
					insertData(hashTable, tag, submitter);
				} else if (strcmp(value, "INDI") == 0) {
					Individual *individual = malloc(sizeof(Individual));
					individual->givenName = calloc(1, sizeof(char));
					individual->surname = calloc(1, sizeof(char));
					individual->events = initializeList(&printEvent, &deleteEvent, &compareEvents);
					individual->families = initializeList(&printFamily, &deleteFamily, &compareFamilies);
					individual->otherFields = initializeList(&printField, &deleteField, &compareFields);

					insertData(hashTable, tag, individual);
				} else if (strcmp(value, "FAM") == 0) {
					Family *family = malloc(sizeof(Family));
					family->wife = NULL;
					family->husband = NULL;
					family->children = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);
					family->events = initializeList(&printEvent, &deleteEvent, &compareEvents);
					family->otherFields = initializeList(&printField, &deleteField, &compareFields);

					insertData(hashTable, tag, family);
				} else {
					err.type = INV_RECORD;
					err.line = lineNum;
					goto ERROR;
				}
			} else {
				err.type = INV_RECORD;
				err.line = lineNum;
				goto ERROR;
			}
		}
	}

	rewind(file);

	bool buildHeader = false;
	bool buildHeaderGEDC = false;
	bool hasHeader = false;
	bool hasSource = false;
	bool hasVersion = false;
	bool hasEncoding = false;
	bool acceptField = true;

	bool buildSubmitter = false;
	bool hasName = false;
	bool hasSubmitter = false;

	bool buildIndividual = false;
	Individual *individual;

	bool buildFamily = false;
	Family *family;

	bool buildEvent = false;
	Event *event;

	bool hasTRLR = false;

	lineNum = 0;
	while (fgets(line, sizeof(line), file)) {
		lineNum++;
		if (line[0] == '\n') { // skip blank lines
			continue;
		}

		if (memchr(line, '\n', strlen(line)) == NULL && memchr(line, '\r', strlen(line)) == NULL) {
			err.type = INV_GEDCOM;
			err.line = lineNum;
			goto ERROR;
		}

		line[strcspn(line, "\n")] = 0;
		line[strcspn(line, "\r")] = 0;

		int level = -1;
		if (!isdigit(line[0])) {
			err.type = INV_RECORD;
			err.line = lineNum;
			goto ERROR;
		} else {
			level = line[0] - '0';
		}

		strtok(line, " ");
		char *tag = strtok(NULL, " ");
		if (tag == NULL) {
			err.type = INV_RECORD;
			err.line = lineNum;
			goto ERROR;
		}

		char *value = line + 3 + strlen(tag);
		if (strlen(value) == 0) {
			value = NULL;
		}

		if (lineNum == 1) {
			if (strcmp(tag, "HEAD") == 0 && level == 0 && value == NULL) {
				buildHeader = true;
				continue;
			} else {
				err.type = INV_GEDCOM;
				err.line = -1;
				goto ERROR;
			}
		}

		if (hasTRLR) {
			if (tag != NULL || value != NULL) {
				err.type = INV_GEDCOM;
				err.line = -1;
				goto ERROR;
			}
		}

		if (strcmp(tag, "TRLR") == 0) {
			if (level != 0 || value != NULL) {
				err.type = INV_RECORD;
				err.line = lineNum;
				goto ERROR;
			}

			hasTRLR = true;
		}

		if (buildIndividual) {
			if (level == 0) {
				if (buildEvent) {
					insertBack(&individual->events, event);
					buildEvent = false;
				}

				buildIndividual = false;
				insertBack(&(*obj)->individuals, individual);
			}

			if (strcmp(tag, "NAME") == 0) {
				if (value == NULL) {
					err.type = INV_RECORD;
					err.line = lineNum;
					goto ERROR;
				}

				char *tok = strtok(value, "/");
				if (tok != NULL) {
					if (individual->givenName[0] == '\0') {
						individual->givenName = realloc(individual->givenName, strlen(tok) + 1);
						strcpy(individual->givenName, tok);
						individual->givenName[strlen(individual->givenName)] = '\0';
					}

					tok = strtok(NULL, "/");
					if (tok != NULL) {
						if (individual->surname[0] == '\0') {
							individual->surname = realloc(individual->surname, strlen(tok) + 1);
							strcpy(individual->surname, tok);
							individual->surname[strlen(individual->surname)] = '\0';
						}
					}
				}
			} else if (strcmp(tag, "GIVN") == 0) {
				if (individual->givenName[0] == '\0') {
					individual->givenName = realloc(individual->givenName, strlen(value) + 1);
					strcpy(individual->givenName, value);
					individual->givenName[strlen(individual->givenName)] = '\0';
				}
			} else if (strcmp(tag, "SURN") == 0) {
				if (individual->surname[0] == '\0') {
					individual->surname = realloc(individual->surname, strlen(value) + 1);
					strcpy(individual->surname, value);
					individual->surname[strlen(individual->surname)] = '\0';
				}
			} else if (strcmp(tag, "FAMS") == 0) {
				if (buildEvent) {
					insertBack(&individual->events, event);
					buildEvent = false;
				}

				if (value == NULL) {
					err.type = INV_RECORD;
					err.line = lineNum;
					goto ERROR;
				}

				Family *temp = lookupData(hashTable, value);

				if (temp == NULL) {
					err.type = INV_RECORD;
					err.line = lineNum;
					goto ERROR;
				}

				bool F = false;
				bool M = false;
				ListIterator iter = createIterator(individual->otherFields);
				Field *field;
				while ((field = nextElement(&iter)) != NULL) {
					if (strcmp(field->tag, "SEX") == 0) {
						if (strcmp(field->value, "F") == 0) {
							F = true;
						} else if (strcmp(field->value, "M") == 0) {
							M = true;
						}

						break;
					}
				}

				if (F && temp->wife == NULL) {
					temp->wife = individual;
				}

				if (M && temp->husband == NULL) {
					temp->wife = individual;
				}
			} else if (strcmp(tag, "FAMC") == 0) {
				if (buildEvent) {
					insertBack(&individual->events, event);
					buildEvent = false;
				}

				if (value == NULL) {
					err.type = INV_RECORD;
					err.line = lineNum;
					goto ERROR;
				}

				Family *temp = lookupData(hashTable, value);

				if (temp == NULL) {
					err.type = INV_RECORD;
					err.line = lineNum;
					goto ERROR;
				}

				bool add = true;
				ListIterator iter = createIterator(temp->children);
				Individual *child;
				while ((child = nextElement(&iter)) != NULL) {
					if (compareIndividuals(child, individual) == 0) {
						add = false;
					}
				}

				if (add) {
					insertBack(&temp->children, individual);
				}
			} else if (isIndivEvent(tag)) {
				if (buildEvent) {
					insertBack(&individual->events, event);
					buildEvent = false;
				}

				if (value != NULL) {
					err.type = INV_RECORD;
					err.line = lineNum;
					goto ERROR;
				}

				event = malloc(sizeof(Event));
				strcpy(event->type, tag);
				event->type[4] = '\0';
				event->date = calloc(sizeof(char), 1);
				event->place = calloc(sizeof(char), 1);
				event->otherFields = initializeList(&printEvent, &deleteEvent, &compareEvents);

				buildEvent = true;

				continue;
			} else if (!buildEvent && buildIndividual && !isIndivEvent(tag)) {
				if (value == NULL) {
					err.type = INV_RECORD;
					err.line = lineNum;
					goto ERROR;
				}

				Field *field = initializeField(tag, value);
				insertBack(&individual->otherFields, field);
			}
		}

		if (buildFamily) {
			if (level == 0) {
				if (buildEvent) {
					insertBack(&family->events, event);
					buildEvent = false;
				}

				buildFamily = false;
				insertBack(&(*obj)->families, family);
			}

			if (strcmp(tag, "HUSB") == 0) {
				if (value[0] == '\0') {
					err.type = INV_RECORD;
					err.line = lineNum;
					goto ERROR;
				}

				Individual *temp = lookupData(hashTable, value);

				if (temp == NULL) {
					err.type = INV_RECORD;
					err.line = lineNum;
					goto ERROR;
				}

				family->husband = temp;
			} else if (strcmp(tag, "WIFE") == 0) {
				if (value[0] == '\0') {
					err.type = INV_RECORD;
					err.line = lineNum;
					goto ERROR;
				}

				Individual *temp = lookupData(hashTable, value);

				if (temp == NULL) {
					err.type = INV_RECORD;
					err.line = lineNum;
					goto ERROR;
				}

				family->wife = temp;
			} else if (strcmp(tag, "CHIL") == 0) {
				if (value[0] == '\0') {
					err.type = INV_RECORD;
					err.line = lineNum;
					goto ERROR;
				}

				Individual *temp = lookupData(hashTable, value);

				if (temp == NULL) {
					err.type = INV_RECORD;
					err.line = lineNum;
					goto ERROR;
				}

				bool add = true;
				ListIterator iter = createIterator(family->children);
				Individual *child;
				while ((child = nextElement(&iter)) != NULL) {
					if (compareIndividuals(child, temp) == 0) {
						add = false;
					}
				}

				if (add) {
					insertBack(&family->children, temp);
				}
			} else if (isFamEvent(tag) && !buildEvent) {
				if (value != NULL) {
					err.type = INV_RECORD;
					err.line = lineNum;
					goto ERROR;
				}

				event = malloc(sizeof(Event));
				strcpy(event->type, tag);
				event->type[4] = '\0';
				event->date = calloc(sizeof(char), 1);
				event->place = calloc(sizeof(char), 1);
				event->otherFields = initializeList(&printEvent, &deleteEvent, &compareEvents);

				buildEvent = true;

				continue;
			} else if (!buildEvent && buildFamily) {
				if (value == NULL) {
					err.type = INV_RECORD;
					err.line = lineNum;
					goto ERROR;
				}

				Field *field = initializeField(tag, value);
				insertBack(&family->otherFields, field);
			}
		}

		if (buildSubmitter) {
			if (level == 0) {
				if (hasName) {
					buildSubmitter = false;
				} else {
					err.type = INV_RECORD;
					err.line = lineNum;
					goto ERROR;
				}
			}

			if (strcmp(tag, "NAME") == 0) {
				if (value != NULL && strlen(value) <= 61) {
					strcpy(submitter->submitterName, value);
					hasName = true;
				} else {
					err.type = INV_RECORD;
					err.line = lineNum;
					goto ERROR;
				}
			} else if (strcmp(tag, "ADDR") == 0) {
				if (value != NULL) {
					strcpy(submitter->address, value);
				} else {
					err.type = INV_RECORD;
					err.line = lineNum;
					goto ERROR;
				}
			} else if (strcmp(tag, "CONT") == 0) {
				if (value == NULL) {
					err.type = INV_RECORD;
					err.line = lineNum;
					goto ERROR;
				}

				if (submitter->address[0] != '\0') {
					sprintf(submitter->address + strlen(submitter->address), "\\n%s", value);
				}
			} else if (strcmp(tag, "CONC") == 0) {
				if (value == NULL) {
					err.type = INV_RECORD;
					err.line = lineNum;
					goto ERROR;
				}

				if (submitter->address[0] != '\0') {
					strcat(submitter->address, value);
				}
			}
		}

		if (buildHeader) {
			if (level == 0) {
				if (hasSource && hasVersion && hasEncoding) {
					buildHeader = false;
					acceptField = false;
					hasHeader = true;
				} else {
					err.type = INV_HEADER;
					err.line = lineNum;
					goto ERROR;
				}
			}

			if (level == 1 && buildHeaderGEDC) {
				buildHeaderGEDC = false;
			}

			if (strcmp(tag, "SOUR") == 0) {
				if (level == 1 && value != NULL) {
					strcpy(header->source, value);
					hasSource = true;
				}
			} else if (strcmp(tag, "GEDC") == 0) {
				if (level == 1 && value == NULL) {
					buildHeaderGEDC = true;
					continue;
				}
			} else if (strcmp(tag, "VERS") == 0 && buildHeaderGEDC) {
				if (level == 2 && value != NULL) {
					float version = atof(value);
					header->gedcVersion = version;
					hasVersion = true;
				} else {
					err.type = INV_HEADER;
					err.line = lineNum;
					goto ERROR;
				}
			} else if (strcmp(tag, "CHAR") == 0) {
				if (level == 1 && value != NULL) {
					if (strcmp(value, "ANSEL") == 0) {
						header->encoding = ANSEL;
						hasEncoding = true;
					} else if (strcmp(value, "UTF8") == 0 || strcmp(value, "UTF-8") == 0) {
						header->encoding = UTF8;
						hasEncoding = true;
					} else if (strcmp(value, "UNICODE") == 0) {
						header->encoding = UNICODE;
						hasEncoding = true;
					} else if (strcmp(value, "ASCII") == 0) {
						header->encoding = ASCII;
						hasEncoding = true;
					} else {
						err.type = INV_HEADER;
						err.line = lineNum;
						goto ERROR;
					}
				} else {
					err.type = INV_HEADER;
					err.line = lineNum;
					goto ERROR;
				}
			} else if (strcmp(tag, "SUBM") == 0) {
				if (level == 1 && value != NULL) {
					Submitter *temp = lookupData(hashTable, value);

					if (temp == NULL) {
						err.type = INV_HEADER;
						err.line = lineNum;
						goto ERROR;
					}

					header->submitter = temp;
					hasSubmitter = true;
				} else {
					err.type = INV_HEADER;
					err.line = lineNum;
					goto ERROR;
				}
			} else if (value != NULL && acceptField) {
				Field *field = initializeField(tag, value);
				insertBack(&header->otherFields, field);
			} else if (acceptField) {
				err.type = INV_HEADER;
				err.line = lineNum;
				goto ERROR;
			}
		}

		if (buildEvent) {
			if (strcmp(tag, "DATE") == 0) {
				if (value == NULL) {
					err.type = INV_RECORD;
					err.line = lineNum;
					goto ERROR;
				}

				event->date = realloc(event->date, strlen(value) + 1);
				strcpy(event->date, value);
				event->date[strlen(event->date)] = '\0';
			} else if (strcmp(tag, "PLAC") == 0) {
				if (value == NULL) {
					err.type = INV_RECORD;
					err.line = lineNum;
					goto ERROR;
				}

				event->place = realloc(event->place, strlen(value) + 1);
				strcpy(event->place, value);
				event->place[strlen(event->place)] = '\0';
			} else if (level > 1) {
				if (value == NULL) {
					err.type = INV_RECORD;
					err.line = lineNum;
					goto ERROR;
				}

				Field *field = initializeField(tag, value);
				insertBack(&event->otherFields, field);
			}

			if (level <= 1) {
				if (buildIndividual) {
					insertBack(&individual->events, event);
					buildEvent = false;
				} else if (buildFamily) {
					insertBack(&family->events, event);
					buildEvent = false;
				}
			}
		}

		if (tag[0] == '@') {
			if (tag[strlen(tag) - 1] == '@' && strlen(tag) > 2) {
				if (strcmp(value, "SUBM") == 0) {
					Submitter *temp = lookupData(hashTable, tag);

					if (temp == NULL) {
						err.type = INV_RECORD;
						err.line = lineNum;
						goto ERROR;
					}

					buildSubmitter = true;
					submitter = temp;

					continue;
				} else if (strcmp(value, "INDI") == 0) {
					Individual *temp = lookupData(hashTable, tag);

					if (temp == NULL) {
						err.type = INV_RECORD;
						err.line = lineNum;
						goto ERROR;
					}

					buildIndividual = true;
					individual = temp;
				} else if (strcmp(value, "FAM") == 0) {
					Family *temp = lookupData(hashTable, tag);

					if (temp == NULL) {
						err.type = INV_RECORD;
						err.line = lineNum;
						goto ERROR;
					}

					buildFamily = true;
					family = temp;
				} else {
					err.type = INV_RECORD;
					err.line = lineNum;
					goto ERROR;
				}
			} else {
				err.type = INV_RECORD;
				err.line = lineNum;
				goto ERROR;
			}
		}
	}

	fclose(file);

	if (!hasHeader || !hasTRLR || !hasSubmitter) {
		err.type = INV_GEDCOM;
		err.line = -1;
		goto ERROR;
	}

	destroyTable(hashTable);

	err.type = OK;
	err.line = -1;
	return err;

	ERROR:

	fclose(file);

	destroyTable(hashTable);
	deleteGEDCOM(*obj);

	return err;
}

char *printGEDCOM(const GEDCOMobject *obj) {
	if (obj == NULL) {
		return NULL;
	}

	ListIterator iter;
	Family *fam;
	Individual *indiv;

	char *header = printHeader(obj->header);
	char *str = malloc(strlen(header) + 50);

	sprintf(str, "GEDCOM:\n%s\n", header);
	free(header);

	iter = createIterator(obj->families);
	while ((fam = (Family *) nextElement(&iter)) != NULL) {
		char *famStr = printFamily(fam);
		str = realloc(str, strlen(str) + strlen(famStr) + 50);
		sprintf(str + strlen(str), "%s\n", famStr);
		free(famStr);
	}

	iter = createIterator(obj->individuals);
	while ((indiv = (Individual *) nextElement(&iter)) != NULL) {
		char *indivStr = printIndividual(indiv);
		str = realloc(str, strlen(str) + strlen(indivStr) + 50);
		sprintf(str + strlen(str), "%s\n", indivStr);
		free(indivStr);
	}

	str[strlen(str)] = '\0';

	return str;
}

void deleteGEDCOM(GEDCOMobject *obj) {
	if (obj == NULL) {
		return;
	}

	deleteHeader(obj->header);

	clearList(&obj->families);
	clearList(&obj->individuals);

	free(obj);
	obj = NULL;
}

char *printError(GEDCOMerror err) {
	char *type = getErrorName(err.type);
	char *str = malloc(strlen(type) + 50);

	sprintf(str, "%s", type);

	if (err.line != -1) {
		sprintf(str + strlen(str), " (line %d)", err.line);
	}

	str[strlen(str)] = '\0';

	return str;
}

Individual *findPerson(const GEDCOMobject *familyRecord, bool (*compare)(const void *first, const void *second), const void *person) {
	ListIterator iter;
	Individual *current;

	iter = createIterator(familyRecord->individuals);
	while ((current = (Individual *) nextElement(&iter)) != NULL && !compare(current, person));

	if (compare(current, person)) {
		return current;
	}

	return NULL;
}

List getDescendants(const GEDCOMobject *familyRecord, const Individual *person) {
	List descendants = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);

	ListIterator iterFam = createIterator(person->families);
	Family *fam;
	while ((fam = nextElement(&iterFam)) != NULL) {
		ListIterator iterChild = createIterator(fam->children);
		Individual *child;

		while ((child = nextElement(&iterChild)) != NULL) {
			Individual *copy = malloc(sizeof(Individual));
			copy->givenName = malloc(strlen(child->givenName));
			copy->surname = malloc(strlen(child->surname));

			strcpy(copy->givenName, child->givenName);
			strcpy(copy->surname, child->surname);

			insertBack(&descendants, copy);
		}
	}

	return descendants;
}

void deleteEvent(void *toBeDeleted) {
	Event *event = (Event *) toBeDeleted;

	if (event == NULL) {
		return;
	}

	free(event->date);
	event->date = NULL;

	free(event->place);
	event->place = NULL;

	clearList(&event->otherFields);

	free(event);
	event = NULL;
}

int compareEvents(const void *first, const void *second) {
	Event *a = (Event *) first;
	Event *b = (Event *) second;

	return strcmp(a->type, b->type);
}

char *printEvent(void *toBePrinted) {
	Event *event = (Event *) toBePrinted;

	if (event == NULL) {
		return NULL;
	}

	ListIterator iter;
	Field *field;

	char *str = malloc(5 + strlen(event->type) + strlen(event->date) + strlen(event->place) + 50);
	sprintf(str, "\nEVENT:\nType: %s\nDate: %s\nPlace: %s", event->type, event->date, event->place);

	iter = createIterator(event->otherFields);
	while ((field = (Field *) nextElement(&iter)) != NULL) {
		char *fieldStr = printField(field);
		str = realloc(str, strlen(str) + strlen(fieldStr) + 50);
		sprintf(str + strlen(str), "%s\n", fieldStr);
		free(fieldStr);
	}

	str[strlen(str)] = '\0';

	return str;
}

void deleteIndividual(void *toBeDeleted) {
	Individual *indiv = (Individual *) toBeDeleted;

	if (indiv == NULL) {
		return;
	}

	free(indiv->givenName);
	indiv->givenName = NULL;

	free(indiv->surname);
	indiv->surname = NULL;

	clearList(&indiv->events);
	clearList(&indiv->otherFields);

	free(indiv);
	indiv = NULL;
}

int compareIndividuals(const void *first, const void *second) {
	Individual *a = (Individual *) first;
	Individual *b = (Individual *) second;

	char strA[strlen(a->givenName) + strlen(a->surname) + 1];
	sprintf(strA, "%s,%s", a->surname, a->givenName);
	strA[strlen(strA)] = '\0';

	char strB[strlen(b->givenName) + strlen(b->surname) + 1];
	sprintf(strB, "%s,%s", b->surname, b->givenName);
	strB[strlen(strB)] = '\0';

	return strcmp(strA, strB);
}

char *printIndividual(void *toBePrinted) {
	Individual *indiv = (Individual *) toBePrinted;

	if (indiv == NULL) {
		return NULL;
	}

	ListIterator iter;
	Event *event;
	Field *field;

	char *str = malloc(strlen(indiv->givenName) + strlen(indiv->surname) + 50);
	sprintf(str, "\nINDIVIDUAL:\nGiven name: %s\nSurname: %s", indiv->givenName, indiv->surname);

	iter = createIterator(indiv->events);
	while ((event = (Event *) nextElement(&iter)) != NULL) {
		char *eventStr = printEvent(event);
		str = realloc(str, strlen(str) + strlen(eventStr) + 50);
		sprintf(str + strlen(str), "%s\n", eventStr);
		free(eventStr);
	}

	iter = createIterator(indiv->otherFields);
	while ((field = (Field *) nextElement(&iter)) != NULL) {
		char *fieldStr = printField(field);
		str = realloc(str, strlen(str) + strlen(fieldStr) + 50);
		sprintf(str + strlen(str), "%s\n", fieldStr);
		free(fieldStr);
	}

	str[strlen(str)] = '\0';

	return str;
}

void deleteFamily(void *toBeDeleted) {
	Family *fam = (Family *) toBeDeleted;

	if (fam == NULL) {
		return;
	}

	clearList(&fam->events);
	clearListFake(&fam->children);
	clearList(&fam->otherFields);

	free(fam);
	fam = NULL;
}

int compareFamilies(const void *first, const void *second) {
	Family *a = (Family *) first;
	Family *b = (Family *) second;
	int numA = 0;
	int numB = 0;
	ListIterator iter;

	if (a->wife != NULL) {
		numA++;
	}

	if (a->husband != NULL) {
		numA++;
	}

	iter = createIterator(a->children);
	while (nextElement(&iter) != NULL) {
		numA++;
	}

	if (b->wife != NULL) {
		numB++;
	}

	if (b->husband != NULL) {
		numB++;
	}

	iter = createIterator(b->children);
	while (nextElement(&iter) != NULL) {
		numB++;
	}

	return (numA > numB) - (numA < numB);
}

char *printFamily(void *toBePrinted) {
	Family *fam = (Family *) toBePrinted;

	if (fam == NULL) {
		return NULL;
	}

	ListIterator iter;
	Individual *child;
	Event *event;
	Field *field;

	char *str = calloc(sizeof(char), 1);

	if (fam->wife != NULL) {
		str = realloc(str, strlen(str) + strlen(fam->wife->givenName) + strlen(fam->wife->surname) + 50);
		sprintf(str, "\nFAMILY: \nWife: %s%s", fam->wife->givenName, fam->wife->surname);
	}

	if (fam->husband != NULL) {
		str = realloc(str, strlen(str) + strlen(fam->husband->givenName) + strlen(fam->husband->surname) + 50);

		if (fam->wife != NULL) {
			sprintf(str + strlen(str), "\nHusband: %s%s", fam->husband->givenName, fam->husband->surname);
		} else {
			sprintf(str, "\nFAMILY: \nHusband: %s%s", fam->husband->givenName, fam->husband->surname);
		}
	}

	iter = createIterator(fam->children);
	while ((child = (Individual *) nextElement(&iter)) != NULL) {
		char *childStr = malloc(strlen(child->givenName) + strlen(child->surname) + 50);
		sprintf(childStr, "Child: %s%s", child->givenName, child->surname);
		str = realloc(str, strlen(str) + strlen(childStr) + 50);
		sprintf(str + strlen(str), "\n%s", childStr);
		free(childStr);
	}

	iter = createIterator(fam->events);
	while ((event = (Event *) nextElement(&iter)) != NULL) {
		char *eventStr = printEvent(event);
		str = realloc(str, strlen(str) + strlen(eventStr) + 50);
		sprintf(str + strlen(str), "%s", eventStr);
		free(eventStr);
	}

	iter = createIterator(fam->otherFields);
	while ((field = (Field *) nextElement(&iter)) != NULL) {
		char *fieldStr = printField(field);
		str = realloc(str, strlen(str) + strlen(fieldStr) + 50);
		sprintf(str + strlen(str), "\n%s", fieldStr);
		free(fieldStr);
	}

	str[strlen(str)] = '\0';

	return str;
}

void deleteField(void *toBeDeleted) {
	Field *field = (Field *) toBeDeleted;

	if (field == NULL) {
		return;
	}

	free(field->tag);
	field->tag = NULL;

	free(field->value);
	field->value = NULL;

	free(field);
	field = NULL;
}

int compareFields(const void *first, const void *second) {
	Field *a = (Field *) first;
	Field *b = (Field *) second;

	char strA[strlen(a->tag) + strlen(a->value) + 1];
	sprintf(strA, "%s %s", a->tag, a->value);
	strA[strlen(strA)] = '\0';

	char strB[strlen(b->tag) + strlen(b->value) + 1];
	sprintf(strB, "%s %s", b->tag, b->value);
	strB[strlen(strB)] = '\0';

	return strcmp(strA, strB);
}

char *printField(void *toBePrinted) {
	Field *field = (Field *) toBePrinted;

	if (field == NULL) {
		return NULL;
	}

	char *str = malloc(strlen(field->tag) + strlen(field->value) + 50);
	sprintf(str, "\nFIELD:\nTag: %s\nValue: %s", field->tag, field->value);
	str[strlen(str)] = '\0';

	return str;
}
