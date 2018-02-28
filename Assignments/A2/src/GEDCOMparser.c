#include "GEDCOMparser.h"
#include "GEDCOMutilities.h"
#include <ctype.h>
#include <inttypes.h>

GEDCOMerror createGEDCOM(char *fileName, GEDCOMobject **obj) {
	GEDCOMerror err;
	FILE *file = NULL;

	if (fileName == NULL) {
		err.type = INV_FILE;
		err.line = -1;
		return err;
	}

	file = fopen(fileName, "r");

	if (file == NULL) {
		err.type = INV_FILE;
		err.line = -1;
		return err;
	}

	if (file == NULL || fileName == NULL || strcmp(fileName + strlen(fileName) - 4, ".ged") != 0) {
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
	header->source[0] = 0;
	header->gedcVersion = -1;
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

	char line[512];
	int lineNum = 0;
	while (fgets(line, sizeof(line), file)) {
		lineNum++;



		if (line[0] == '\n') {         // skip blank lines
			continue;
		}

		if (strlen(line) > 255) {
			err.type = INV_RECORD;
			err.line = lineNum;
			goto ERROR;
		}

		if (memchr(line, '\n', strlen(line)) == NULL && memchr(line, '\r', strlen(line)) == NULL) {
			if (strcmp(line, "0 TRLR") != 0) {
				err.type = INV_GEDCOM;
				err.line = -1;
				goto ERROR;
			}
		}

		line[strcspn(line, "\n")] = 0;
		line[strcspn(line, "\r")] = 0;

		strtok(line, " ");
		char *tag = strtok(NULL, " ");
		if (tag == NULL) {
			err.type = INV_RECORD;
			err.line = lineNum;
			goto ERROR;
		}

		char *value = line + 3 + strlen(tag);

		if (strstr(tag, "@") == NULL) {         // only concered with pointers this pass
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
		int len = strlen(line);

		if (line[0] == '\n') {         // skip blank lines
			continue;
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
		if (len <= strlen(tag) + 2) {
			value = NULL;
		}

		if (value != NULL && strlen(value) == 0) {
			value = NULL;
		}

		if (lineNum == 1) {
			if (strcmp(tag, "HEAD") == 0) {
				if (level == 0 && value == NULL) {
					buildHeader = true;
					continue;
				} else {
					err.type = INV_HEADER;
					err.line = lineNum;
					goto ERROR;
				}
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
						tok[strlen(tok) - 1] = '\0';
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

				insertBack(&individual->families, temp);

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
					temp->husband = individual;
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

				insertBack(&individual->families, temp);

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
				if (hasSource && hasVersion && hasEncoding && hasSubmitter) {
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
						err.type = INV_GEDCOM;
						err.line = -1;
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
	(*obj) = NULL;

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

	if (familyRecord == NULL || compare == NULL || person == NULL) {
		return NULL;
	}

	iter = createIterator(familyRecord->individuals);
	while ((current = nextElement(&iter)) != NULL && !compare(current, person));

	if (compare(current, person)) {
		return current;
	}

	return NULL;
}

List getDescendants(const GEDCOMobject *familyRecord, const Individual *person) {
	List d = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);

	if (familyRecord == NULL || person == NULL) {
		return d;
	}

	descendants(&d, person);

	return d;
}

GEDCOMerror writeGEDCOM(char* filename, const GEDCOMobject *obj) {
	GEDCOMerror err;

	if (validateGEDCOM(obj) != OK) {
		err.type = WRITE_ERROR;
		err.line = -1;
		return err;
	}

	FILE *file = NULL;

	if (filename == NULL) {
		err.type = WRITE_ERROR;
		err.line = -1;
		return err;
	}

	file = fopen(filename, "r");

	if (file == NULL) {
		err.type = WRITE_ERROR;
		err.line = -1;
		return err;
	}

	if (file == NULL || filename == NULL || strcmp(filename + strlen(filename) - 4, ".ged") != 0) {
		fclose(file);
		err.type = WRITE_ERROR;
		err.line = -1;
		return err;
	}

	// populate hashtable with xrefs
	HashTable *hashTable = initHashTable(127);

	ListIterator iterIndiv = createIterator(obj->individuals);
	Individual *indiv;
	int numIndivs = 0;

	while ((indiv = nextElement(&iterIndiv)) != NULL) {
		char xref[10];
		sprintf(xref, "@I%d@", numIndivs + 1);
		insertData(hashTable, xref, indiv);

		numIndivs++;
	}

	ListIterator iterFam = createIterator(obj->families);
	Family *fam;
	int numFams = 0;

	while ((fam = nextElement(&iterFam)) != NULL) {
		char xref[10];
		sprintf(xref, "@F%d@", numFams = 1);
		insertData(hashTable, xref, fam);

		numFams++;
	}

	// write header
	Header *header = obj->header;

	fprintf(file, "0 HEAD\n");
	fprintf(file, "1 SOUR %s\n", header->source);
	fprintf(file, "1 GEDC\n");
	fprintf(file, "2 VERS %f\n", header->gedcVersion);
	fprintf(file, "2 FORM LINEAGE-LINKED\n");
	fprintf(file, "1 CHAR %s\n", getEncodingName(header->encoding));
	fprintf(file, "1 SUBM @S1@\n");

	// write individuals
	iterIndiv = createIterator(obj->individuals);
	while ((indiv = nextElement(&iterIndiv)) != NULL) {
		char xref[10];

		for (int i = 0; i < numIndivs; i++) {
			sprintf(xref, "@I%d@", i + 1);

			Individual *indiv2 = lookupData(hashTable, xref);

			if (compareIndividuals(indiv, indiv2) == 0) {
				break;
			}
		}

		fprintf(file, "0 INDI %s\n", xref);
		fprintf(file, "1 NAME %s /%s/\n", indiv->givenName, indiv->surname);

		// write individual events
		ListIterator iterEvent = createIterator(indiv->events);
		Event *event;

		while ((event = nextElement(&iterEvent)) != NULL) {
			fprintf(file, "1 %s\n", event->type);

			if (event->date[0] != 0) {
				fprintf(file, "2 DATE %s\n", event->date);
			}

			if (event->place[0] != 0) {
				fprintf(file, "2 PLAC %s\n", event->place);
			}
		}

		// write individual families
		ListIterator iterFam = createIterator(indiv->families);
		Family *fam;

		while ((fam = nextElement(&iterFam)) != NULL) {
			char xref[10];

			for (int i = 0; i < numFams; i++) {
				sprintf(xref, "@F%d@", i + 1);

				Family *fam2 = lookupData(hashTable, xref);

				if (fam == NULL || fam2 == NULL) {
					continue;
				}

				if (compareFamilies(fam, fam2) == 0) {
					break;
				}
			}

			if (fam->wife != NULL) {
				if (strcmp(fam->wife->givenName, indiv->givenName) == 0 && strcmp(fam->wife->surname, indiv->surname) == 0) {
					fprintf(file, "1 FAMS %s\n", xref);
					continue;
				}
			}

			if (fam->husband != NULL) {
				if (strcmp(fam->husband->givenName, indiv->givenName) == 0 && strcmp(fam->husband->surname, indiv->surname) == 0) {
					fprintf(file, "1 FAMS %s\n", xref);
					continue;
				}
			}

			fprintf(file, "1 FAMC %s\n", xref);
		}

		// write individual fields
		ListIterator iterField = createIterator(indiv->otherFields);
		Field *field;

		while ((field = nextElement(&iterField)) != NULL) {
			fprintf(file, "1 %s %s\n", field->tag, field->value);
		}
	}

	// write families
	iterFam = createIterator(obj->families);
	while ((fam = nextElement(&iterFam)) != NULL) {
		char xref[10];

		for (int i = 0; i < numFams; i++) {
			sprintf(xref, "@F%d@", i + 1);

			Family *fam2 = lookupData(hashTable, xref);

			if (fam == NULL || fam2 == NULL) {
				continue;
			}

			if (compareFamilies(fam, fam2) == 0) {
				break;
			}
		}

		fprintf(file, "0 %s FAM\n", xref);

		// write family wife
		if (fam->wife != NULL) {
			char xref[10];

			for (int i = 0; i < numIndivs; i++) {
				sprintf(xref, "@I%d@", i + 1);

				Individual *wife = lookupData(hashTable, xref);

				if (compareIndividuals(fam->wife, wife) == 0) {
					break;
				}
			}

			fprintf(file, "1 WIFE %s\n", xref);
		}

		// write family husband
		if (fam->husband != NULL) {
			char xref[10];

			for (int i = 0; i < numIndivs; i++) {
				sprintf(xref, "@I%d@", i + 1);

				Individual *husb = lookupData(hashTable, xref);

				if (compareIndividuals(fam->husband, husb) == 0) {
					break;
				}
			}

			fprintf(file, "1 HUSB %s\n", xref);
		}

		// write family children
		ListIterator iterChild = createIterator(fam->children);
		Individual *child;

		while ((child = nextElement(&iterChild)) != NULL) {
			char xref[10];

			for (int i = 0; i < numIndivs; i++) {
				sprintf(xref, "@I%d@", i + 1);

				Individual *child2 = lookupData(hashTable, xref);

				if (compareIndividuals(child, child2) == 0) {
					break;
				}
			}

			fprintf(file, "1 CHIL %s\n", xref);
		}

		// write family events
		ListIterator iterEvent = createIterator(fam->events);
		Event *event;

		while ((event = nextElement(&iterEvent)) != NULL) {
			fprintf(file, "1 %s\n", event->type);

			if (event->date[0] != 0) {
				fprintf(file, "2 DATE %s\n", event->date);
			}

			if (event->place[0] != 0) {
				fprintf(file, "2 PLAC %s\n", event->place);
			}
		}
	}

	// write submitter
	Submitter *submitter = obj->submitter;

	fprintf(file, "0 @S1@ SUBM\n");
	fprintf(file, "1 NAME %s\n", submitter->submitterName);

	if (submitter->address[0] != 0) {
		fprintf(file, "1 ADDR %s\n", submitter->address);
	}

	// write trailer
	fprintf(file, "0 TRLR");

	fclose(file);

	destroyTable(hashTable);

	err.type = OK;
	err.line = -1;
	return err;
}

ErrorCode validateGEDCOM(const GEDCOMobject *obj) {
	if (obj == NULL) {
		return INV_GEDCOM;
	}

	// validate header
	Header *header = obj->header;

	if (header == NULL) {
		return INV_GEDCOM;
	}

	if (header->source[0] == 0) {
		return INV_HEADER;
	}

	if (header->gedcVersion == -1) {
		return INV_HEADER;
	}

	if (header->submitter == NULL) {
		return INV_HEADER;
	}

	// validate submitter
	Submitter *submitter = obj->submitter;

	if (submitter == NULL) {
		return INV_GEDCOM;
	}

	if (submitter->submitterName[0] == 0) {
		return INV_RECORD;
	}

	// validate families
	ListIterator iter = createIterator(obj->families);
	Family *fam;

	while ((fam = nextElement(&iter)) != NULL) {
		ListIterator iterEvent = createIterator(fam->events);
		Event *event;

		while ((event = nextElement(&iterEvent)) != NULL) {
			if (event->type[0] == 0) {
				return INV_RECORD;
			}

			if (event->date != NULL && strlen(event->date) > 200) {
				return INV_RECORD;
			}

			if (event->place != NULL && strlen(event->place) > 200) {
				return INV_RECORD;
			}

			ListIterator iterField = createIterator(event->otherFields);
			Field *field;

			while ((field = nextElement(&iterField)) != NULL) {
				if (field->tag == NULL || field->tag[0] == 0 || strlen(field->tag) > 200) {
					return INV_RECORD;
				}

				if (field->value == NULL || field->value[0] == 0 || strlen(field->value) > 200) {
					return INV_RECORD;
				}
			}
		}

		ListIterator iterField = createIterator(fam->otherFields);
		Field *field;

		while ((field = nextElement(&iterField)) != NULL) {
			if (field->tag == NULL || field->tag[0] == 0 || strlen(field->tag) > 200) {
				return INV_RECORD;
			}

			if (field->value == NULL || field->value[0] == 0 || strlen(field->value) > 200) {
				return INV_RECORD;
			}
		}
	}

	// validate individuals
	iter = createIterator(obj->individuals);
	Individual *indiv;

	while ((indiv = nextElement(&iter)) != NULL) {
		if (indiv->givenName == NULL || (indiv->givenName != NULL && strlen(indiv->givenName) > 100)) {
			return INV_RECORD;
		}

		if (indiv->surname == NULL || (indiv->surname != NULL && strlen(indiv->surname) > 100)) {
			return INV_RECORD;
		}

		ListIterator iterEvent = createIterator(indiv->events);
		Event *event;

		while ((event = nextElement(&iterEvent)) != NULL) {
			if (event->type[0] == 0) {
				return INV_RECORD;
			}

			if (event->date != NULL && strlen(event->date) > 200) {
				return INV_RECORD;
			}

			if (event->place != NULL && strlen(event->place) > 200) {
				return INV_RECORD;
			}

			ListIterator iterField = createIterator(event->otherFields);
			Field *field;

			while ((field = nextElement(&iterField)) != NULL) {
				if (field->tag == NULL || field->tag[0] == 0 || strlen(field->tag) > 200) {
					return INV_RECORD;
				}

				if (field->value == NULL || field->value[0] == 0 || strlen(field->value) > 200) {
					return INV_RECORD;
				}
			}
		}

		ListIterator iterField = createIterator(indiv->otherFields);
		Field *field;

		while ((field = nextElement(&iterField)) != NULL) {
			if (field->tag == NULL || field->tag[0] == 0 || strlen(field->tag) > 200) {
				return INV_RECORD;
			}

			if (field->value == NULL || field->value[0] == 0 || strlen(field->value) > 200) {
				return INV_RECORD;
			}
		}
	}

	return OK;
}

List getDescendantListN(const GEDCOMobject *familyRecord, const Individual *person, unsigned int maxGen) {
	List d = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);

	if (familyRecord == NULL || person == NULL) {
		return d;
	}

	int gens = 0;

	if (maxGen == 0) {
		gens = 10;
	} else {
		gens = maxGen;
	}

	List *g[gens];

	for (int i = 0; i < gens; i++) {
		g[i] = malloc(sizeof(List));
		g[i]->head = NULL;
		g[i]->tail = NULL;
		g[i]->length = 0;
		g[i]->deleteData = &deleteIndividual;
		g[i]->compare = &compareDesc;
		g[i]->printData = &printIndividual;
	}

	descendantsN(g, person, 0, maxGen);

	for (int i = 0; i < gens; i++) {
		if (getLength(*g[i]) != 0) {
			insertBack(&d, g[i]);
		}
	}

	return d;
}

List getAncestorListN(const GEDCOMobject *familyRecord, const Individual *person, int maxGen) {
	List d = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);

	if (familyRecord == NULL || person == NULL) {
		return d;
	}

	if (maxGen < 0) {
		return d;
	}

	int gens = 0;

	if (maxGen == 0) {
		gens = 10;
	} else {
		gens = maxGen;
	}

	List *g[gens];

	for (int i = 0; i < gens; i++) {
		g[i] = malloc(sizeof(List));
		g[i]->head = NULL;
		g[i]->tail = NULL;
		g[i]->length = 0;
		g[i]->deleteData = &deleteIndividual;
		g[i]->compare = &compareDesc;
		g[i]->printData = &printIndividual;
	}

	ancestorsN(g, person, 0, maxGen);

	for (int i = 0; i < gens; i++) {
		if (getLength(*g[i]) != 0) {
			insertBack(&d, g[i]);
		}
	}

	return d;
}

char *indToJSON(const Individual *ind) {
	char *strJSON;

	if (ind == NULL) {
		strJSON = calloc(1, sizeof(char));
		return strJSON;
	}

	strJSON = calloc(250, sizeof(char));

	sprintf(strJSON, "{\"givenName\":\"%s\",\"surname\":\"%s\"}", ind->givenName, ind->surname);

	return strJSON;
}

//TODO maybe parse by quotation idk
Individual *JSONtoInd(const char *str) {
	if (str == NULL) {
		return NULL;
	}

	Individual *ind = malloc(sizeof(Individual));
	ind->givenName = calloc(1, sizeof(char));
	ind->surname = calloc(1, sizeof(char));
	ind->events = initializeList(&printEvent, &deleteEvent, &compareEvents);
	ind->families = initializeList(&printFamily, &deleteFamily, &compareFamilies);
	ind->otherFields = initializeList(&printField, &deleteField, &compareFields);

	char *copy = malloc(strlen(str) + 1);
	strcpy(copy, str);

	char *given = copy + 14;
	if (given[0] != '\"') {
		given = strtok(given, "\"");

		ind->givenName = realloc(ind->givenName, strlen(given) + 1);
		strcpy(ind->givenName, given);
	}

	strcpy(copy, str);

	char *surname = copy + 14 + strlen(ind->givenName) + 13;
	if (surname[0] != '\"') {
		surname = strtok(surname, "\"");

		ind->surname = realloc(ind->surname, strlen(surname) + 1);
		strcpy(ind->surname, surname);
	}

	free(copy);

	return ind;
}

GEDCOMobject *JSONtoGEDCOM(const char *str) {
	if (str == NULL) {
		return NULL;
	}

	char *copy = malloc(strlen(str) + 1);
	strcpy(copy, str);

	Submitter *sub = malloc(sizeof(Submitter));
	sub->address[0] = '\0';
	sub->otherFields = initializeList(&printField, &deleteField, &compareFields);

	Header *header = malloc(sizeof(Header));
	header->submitter = sub;
	header->otherFields = initializeList(&printField, &deleteField, &compareFields);

	int count = 0;
	char *tok = strtok(copy, "\"");
	tok = strtok(NULL, "\"");
	tok = strtok(NULL, "\"");
	tok = strtok(NULL, "\"");

	while (tok != NULL) {
		if (isalpha(tok[0]) || isdigit(tok[0])) {
			switch(count) {
			case 0:
				strcpy(header->source, tok);
				break;
			case 1:
				header->gedcVersion = atof(tok);
				break;
			case 2:
				if (strcmp(tok, "ANSEL") == 0) {
					header->encoding = ANSEL;
				} else if (strcmp(tok, "UTF8") == 0 || strcmp(tok, "UTF-8") == 0) {
					header->encoding = UTF8;
				} else if (strcmp(tok, "UNICODE") == 0) {
					header->encoding = UNICODE;
				} else if (strcmp(tok, "ASCII") == 0) {
					header->encoding = ASCII;
				}
				break;
			case 3:
				strcpy(sub->submitterName, tok);
				break;
			case 4:
				strcpy(sub->address, tok);
				break;
			}
		}

		count++;

		for (int i = 0; i < 4; i++) {
			tok = strtok(NULL, "\"");
		}
	}

	GEDCOMobject *obj = malloc(sizeof(GEDCOMobject));
	obj->header = header;
	obj->families = initializeList(&printFamily, &deleteFamily, &compareFamilies);
	obj->individuals = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);
	obj->submitter = sub;

	return obj;
}

void addIndividual(GEDCOMobject *obj, const Individual *toBeAdded) {
	if (obj == NULL || toBeAdded == NULL) {
		return;
	}

	insertBack(&obj->individuals, (Individual *) toBeAdded);
}

char *iListToJSON(List iList) {
	char *json = calloc(3, sizeof(char));

	if (getLength(iList) == 0) {
		strcpy(json, "[]");
		return json;
	}

	json[0] = '[';

	ListIterator iter = createIterator(iList);
	Individual *indiv;
	int i = 0;

	while ((indiv = nextElement(&iter)) != NULL) {
		char *indivJSON = indToJSON(indiv);

		json = realloc(json, strlen(json) + strlen(indivJSON) + 50);
		sprintf(json + strlen(json), "%s", indivJSON);

		i++;

		if (i != getLength(iList)) {
			sprintf(json + strlen(json), ",");
		}
	}

	sprintf(json + strlen(json), "]");

	return json;
}

char *gListToJSON(List gList) {
	char *json = calloc(3, sizeof(char));

	if (getLength(gList) == 0) {
		strcpy(json, "[]");
		return json;
	}

	json[0] = '[';

	ListIterator iter = createIterator(gList);
	List *iList;
	int i = 0;

	while ((iList = nextElement(&iter)) != NULL) {
		char *iListJSON = iListToJSON(*iList);

		json = realloc(json, strlen(json) + strlen(iListJSON) + 50);
		sprintf(json + strlen(json), "%s", iListJSON);

		i++;

		if (i != getLength(gList)) {
			sprintf(json + strlen(json), ",");
		}
	}

	sprintf(json + strlen(json), "]");

	return json;
}

void deleteGeneration(void *toBeDeleted) {

}

int compareGenerations(const void *first, const void *second) {
	return 1;
}

char *printGeneration(void *toBePrinted) {
	return NULL;
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
	clearListFake(&indiv->families);

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
		sprintf(str, "\nFAMILY: \nWife: %s %s", fam->wife->givenName, fam->wife->surname);
	}

	if (fam->husband != NULL) {
		str = realloc(str, strlen(str) + strlen(fam->husband->givenName) + strlen(fam->husband->surname) + 50);

		if (fam->wife != NULL) {
			sprintf(str + strlen(str), "\nHusband: %s %s", fam->husband->givenName, fam->husband->surname);
		} else {
			sprintf(str, "\nFAMILY: \nHusband: %s %s", fam->husband->givenName, fam->husband->surname);
		}
	}

	iter = createIterator(fam->children);
	while ((child = (Individual *) nextElement(&iter)) != NULL) {
		char *childStr = malloc(strlen(child->givenName) + strlen(child->surname) + 50);
		sprintf(childStr, "Child: %s %s", child->givenName, child->surname);
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
