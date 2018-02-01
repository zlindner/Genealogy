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

	*obj = malloc(sizeof(GEDCOMobject));
	if (*obj == NULL) {
		err.type = OTHER;
		err.line = -1;
		return err;
	}

	(*obj)->families = initializeList(&printFamily, &deleteFamily, &compareFamilies);
	(*obj)->individuals = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);

	Header *header = NULL;
	bool buildHeader = false;
	bool hasHeader = false;

	Submitter *subm = NULL;
	bool buildSubm = false;
	bool hasSubm = false;

	Individual **indivs = NULL;
	int numIndiv = 0;
	bool buildIndiv = false;

	Family **fams = NULL;
	int numFam = 0;
	bool buildFam = false;

	HashTable *hashTable = initHashTable(128);
	if (hashTable == NULL) {
		err.type = OTHER;
		err.line = -1;
		return err;
	}

	char line[257];

	while (fgets(line, sizeof(line), file)) {
		line[strcspn(line, "\r\n")] = 0;

		char copy[strlen(line) + 50];
		strcpy(copy, line);

		strtok(copy, " ");

		char tag[strlen(line) + 50];
		strcpy(tag, strtok(NULL, " "));
		tag[strlen(tag)] = '\0';

		char val[strlen(line) + 50];
		if (strlen(line) > strlen(tag) + 2) {
			strcpy(val, line + 2 + strlen(tag) + 1);
		} else {
			strcpy(val, "");
		}
		val[strlen(val)] = '\0';

		if (tag[0] == '@' && tag[strlen(tag) - 1] == '@') {
			if (strcmp(val, "INDI") == 0) {
				indivs = realloc(indivs, sizeof(Individual) * (numIndiv + 1));

				indivs[numIndiv] = malloc(sizeof(Individual));
				if (indivs[numIndiv] == NULL) {
					err.type = OTHER;
					err.line = -1;
					return err;
				}
				indivs[numIndiv]->events = initializeList(&printEvent, &deleteEvent, &compareEvents);
				indivs[numIndiv]->families = initializeList(&printFamily, &deleteFamily, &compareFamilies);
				indivs[numIndiv]->otherFields = initializeList(&printField, &deleteField, &compareFields);

				insertData(hashTable, tag, indivs[numIndiv]);

				numIndiv++;
			} else if (strcmp(val, "FAM") == 0) {
				fams = realloc(fams, sizeof(Family) * (numFam + 1));

				fams[numFam] = malloc(sizeof(Family));
				if (fams[numFam] == NULL) {
					err.type = OTHER;
					err.line = -1;
					return err;
				}
				fams[numFam]->children = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);
				fams[numFam]->otherFields = initializeList(&printField, &deleteField, &compareFields);

				insertData(hashTable, tag, fams[numFam]);

				numFam++;
			} else if (strcmp(val, "SUBM") == 0) {
				subm = malloc(sizeof(Submitter));
				if (subm == NULL) {
					err.type = OTHER;
					err.line = -1;
					return err;
				}
				subm->otherFields = initializeList(&printField, &deleteField, &compareFields);

				insertData(hashTable, tag, subm);
			}
		}
	}

	free(indivs);
	free(fams);

	rewind(file);

	Individual *indivPtr = NULL;
	Family *famPtr = NULL;

	bool hasTRLR;

	int lineNum = 0;

	while (fgets(line, sizeof(line), file)) {
		line[strcspn(line, "\r\n")] = 0;                                                             // remove line terminators

		char copy[strlen(line) + 50];
		strcpy(copy, line);

		int level = strtok(copy, " ")[0] - '0';

		char tag[strlen(line) + 50];
		strcpy(tag, strtok(NULL, " "));
		tag[strlen(tag)] = '\0';

		char val[strlen(line) + 50];
		if (strlen(line) > strlen(tag) + 2) {
			strcpy(val, line + 2 + strlen(tag) + 1);
		} else {
			strcpy(val, "");
		}
		val[strlen(val)] = '\0';

		lineNum++;

		if (level == 0 && buildHeader) {
			buildHeader = false;
			//TODO validate Header

			hasHeader = true;

			(*obj)->header = header;
		}

		if (level == 0 && buildSubm) {
			buildSubm = false;

			//TODO validate subm

			hasSubm = true;

			(*obj)->submitter = subm;
		}

		if (level == 0 && buildIndiv) {
			buildIndiv = false;
			insertBack(&(*obj)->individuals, indivPtr);
		}

		if (level == 0 && buildFam) {
			buildFam = false;
			insertBack(&(*obj)->families, famPtr);
		}

		if (tag[0] == '@' && tag[strlen(tag) - 1] == '@') {
			if (strcmp(val, "INDI") == 0) {
				buildIndiv = true;
				indivPtr = (Individual *) lookupData(hashTable, tag);
			} else if (strcmp(val, "FAM") == 0) {
				buildFam = true;
				famPtr = (Family *) lookupData(hashTable, tag);
			} else if (strcmp(val, "SUBM") == 0) {
				buildSubm = true;
			}

			continue;
		}

		if (strcmp(tag, "HEAD") == 0) {
			if (level != 0 || lineNum != 1) {
				err.type = INV_HEADER;
				err.line = lineNum;
				return err;
			}

			buildHeader = true;

			header = malloc(sizeof(Header));
			if (header == NULL) {
				err.type = OTHER;
				err.line = -1;
				return err;
			}

			header->otherFields = initializeList(&printField, &deleteField, &compareFields);
			continue;
		}

		if (buildHeader) {
			if (strcmp(tag, "SOUR") == 0) {
				strcpy(header->source, val);
				header->source[strlen(header->source)] = '\0';
			} else if (strcmp(tag, "SUBM") == 0) {
				header->submitter = subm;
			} else if (strcmp(tag, "GEDC") == 0) {

			} else if (strcmp(tag, "VERS") == 0) {                                                 //TODO fix
				header->gedcVersion = strtoimax(val, NULL, 0);
			} else if (strcmp(tag, "CHAR") == 0) {
				header->encoding = getEncoding(val);
			} else {
				insertBack(&header->otherFields, initializeField(tag, val));
			}
		}

		if (buildIndiv) {
			if (strcmp(tag, "NAME") == 0) {
				int first = -1;
				int lastStart = -1;
				int lastEnd = -1;

				for (int i = 0; val[i]; i++) {
					if (val[i] == ' ') {
						first = i;
					} else if (val[i] == '/') {
						if (lastStart == -1) {
							lastStart = i;
						} else if (lastEnd == -1) {
							lastEnd = i;
						}
					}
				}

				//TODO only malloc if name isn't already set

				char *given = malloc(first + 1);
				if (given == NULL) {
					err.type = OTHER;
					err.line = -1;
					return err;
				}
				strncpy(given, val, first);
				given[strlen(given)] = '\0';

				char *surname = malloc(lastEnd - lastStart + 1);
				if (surname == NULL) {
					err.type = OTHER;
					err.line = -1;
					return err;
				}
				strcpy(surname, val + lastStart + 1);
				surname[strlen(surname) - 1] = ' ';
				surname[strlen(surname)] = '\0';

				indivPtr->givenName = given;
				indivPtr->surname = surname;
			} else if (strcmp(tag, "GIVN") == 0) {
				char *given = malloc(strlen(val) + 1);
				if (given == NULL) {
					err.type = OTHER;
					err.line = -1;
					return err;
				}

				strcpy(given, val);
				given[strlen(given)] = '\0';

				indivPtr->givenName = given;
			} else if (strcmp(tag, "SURN") == 0) {
				char *surname = malloc(strlen(val) + 1);
				if (surname == NULL) {
					err.type = OTHER;
					err.line = -1;
					return err;
				}

				strcpy(surname, val);
				surname[strlen(surname)] = '\0';

				indivPtr->surname = surname;
			} else if (strcmp(tag, "FAMS") == 0) {
				insertBack(&indivPtr->families, lookupData(hashTable, val));
			} else if (strcmp(tag, "FAMC") == 0) {
				insertBack(&indivPtr->families, lookupData(hashTable, val));
			} else {
				//TODO figure out event vs field
			}
		}

		if (buildFam) {
			if (strcmp(tag, "HUSB") == 0) {
				famPtr->husband = (Individual *) lookupData(hashTable, val);
			} else if (strcmp(tag, "WIFE") == 0) {
				famPtr->wife = (Individual *) lookupData(hashTable, val);
			} else if (strcmp(tag, "CHIL") == 0) {
				insertBack(&famPtr->children, (Individual *) lookupData(hashTable, val));
			} else {
				insertBack(&famPtr->otherFields, initializeField(tag, val));
			}
		}

		if (buildSubm) {
			if (strcmp(tag, "NAME") == 0) {
				strcpy(subm->submitterName, val);
				subm->submitterName[strlen(subm->submitterName)] = '\0';
			} else if (strcmp(tag, "ADDR") == 0) {

			} else {
				insertBack(&subm->otherFields, initializeField(tag, val));
			}
		}

		if (strcmp(tag, "TRLR") == 0) {
			hasTRLR = true;
		}
	}

	if (!hasHeader || !hasTRLR) {
		err.type = INV_GEDCOM;
		err.line = -1;
		return err;
	}

	fclose(file);

	destroyTable(hashTable);

	err.type = OK;
	err.line = -1;
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

	sprintf(str, "GEDCOM:\n%s", header);
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
	sprintf(str, "EVENT:\nType: %s\nDate: %s\nPlace: %s", event->type, event->date, event->place);

	iter = createIterator(event->otherFields);
	while ((field = (Field *) nextElement(&iter)) != NULL) {
		char *fieldStr = printField(field);
		str = realloc(str, strlen(str) + strlen(fieldStr) + 50);
		sprintf(str + strlen(str), ", %s", fieldStr);
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
	clearList(&indiv->families);
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
	Family *fam;
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

	iter = createIterator(indiv->families);
	while ((fam = (Family *) nextElement(&iter)) != NULL) {
		char *famStr = printFamily(fam);
		str = realloc(str, strlen(str) + strlen(famStr) + 50);
		sprintf(str + strlen(str), "%s\n", famStr);
		free(famStr);
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

	if (fam->wife != NULL) {
		deleteIndividual(fam->wife);
	}

	if (fam->husband != NULL) {
		deleteIndividual(fam->husband);
	}

	clearList(&fam->children);
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
	Field *field;

	char *str = malloc(1);
	*str = 0;

	if (fam->wife != NULL) {
		str = realloc(str, strlen(str) + strlen(printIndividual(fam->wife)) + 50);
		sprintf(str, "\nFAMILY: \nWife: %s", printIndividual(fam->wife));
	}

	if (fam->husband != NULL) {
		str = realloc(str, strlen(str) + strlen(printIndividual(fam->husband)) + 50);

		if (fam->wife != NULL) {
			sprintf(str + strlen(str), "\nHusband: %s", printIndividual(fam->husband));
		} else {
			sprintf(str, "\nFAMILY: \nHusband: %s", printIndividual(fam->husband));
		}
	}

	iter = createIterator(fam->children);
	while ((child = (Individual *) nextElement(&iter)) != NULL) {
		char *childStr = printIndividual(child);
		str = realloc(str, strlen(str) + strlen(childStr) + 50);
		sprintf(str + strlen(str), "\n%s", childStr);
		free(childStr);
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
