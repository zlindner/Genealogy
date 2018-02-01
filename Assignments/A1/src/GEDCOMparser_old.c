#include <ctype.h>
#include <inttypes.h>
#include "GEDCOMparser.h"
#include "GEDCOMutilities.h"

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
	
	Header *header = NULL;
	Submitter *submitter = NULL;
	
	char line[257]; // max line length + null terminator + extra char for error checking
	int lineNum = 0;
	
	int level;
	int prevLevel;
	
	char *tag;
	char *parentTag;
	
	while (fgets(line, sizeof(line), file)) {
		// todo remove newline from line? 
		//    -> would have to fix checking for max line length
		
		if (strlen(line) > 255) { 
			if (header == NULL) { // todo same here
				err.type = INV_HEADER;
			} else {
				err.type = INV_RECORD;
			}
			
			err.line = lineNum;
			return err;
		}
		
		lineNum++; // line was successfully read, increment line number
		
		if (!isdigit(line[0])) {
			if (header == NULL) { // todo same here
				err.type = INV_HEADER;
			} else {
				err.type = INV_RECORD;
			}
			
			err.line = lineNum;
			return err;
		}
		
		prevLevel = level;
		
		if (prevLevel != level) {
			parentTag = tag;
		}
		
		level = strtoimax(&line[0], NULL, 10);
		tag = line + 2;
		char *value = line + 7;
		
		// todo check for leading zero's in level
		// todo move error checking for level, tag, value etc. to utils
		
		if (level < 0 || level > 99) {
			if (header == NULL) { // todo this doesn't rly work, need better way to know where ya at
				err.type = INV_HEADER;
			} else {
				err.type = INV_RECORD;
			}
			
			err.line = lineNum;
			return err;
		}
		
		if (strlen(tag) > 31) { // todo first 15 characters must be unique
			if (header == NULL) { // todo same here
				err.type = INV_HEADER;
			} else {
				err.type = INV_RECORD;
			}
			
			err.line = lineNum;
			return err;
		}
		
		if (level == 0) {
			if (strcmp(tag, "HEAD\n") == 0) {
				if (header == NULL) {
					if (value[0] == 0) {
						header = malloc(sizeof(Header));
					} else {
						err.type = INV_HEADER;
						err.line = lineNum;
						return err;
					}
				} else {
					err.type = INV_GEDCOM;
					err.line = -1;
					return err;
				}
			}
		} else if (level == (prevLevel + 1)) {
			if (strcmp(tag, "SOUR") == 0) {
				if (header == NULL || value[0] == 0) {
					err.type = INV_HEADER;
					err.line = lineNum;
					return err;
				}
				
				strcpy(header->source, value);
			}
		} else if (level == prevLevel) {
			if (strcmp(parentTag, "SOUR") == 0) {
				if (value[0] != 0) {
					insertFront(&(header->otherFields), value);
				} else {
					err.type = INV_HEADER;
					err.line = lineNum;
					return err;
				}
			}
		} else {
			// todo invalid level number
		}
	}
	
	fclose(file); // parsing is done, close file
	
	// todo build GEDCOMobject
	/*gedcom = malloc(sizeof(GEDCOMobject));
	
	if (submitter != NULL) {
		header->submitter = submitter;
	} else {
		err.type = INV_GEDCOM;
		err.line = -1;
		return err;
	}
	
	if (header != NULL) {
		gedcom->header = header;
	} else {
		err.type = INV_GEDCOM;
		err.line = -1;
		return err;
	}*/
		
	err.type = OK;
	err.line = -1;
	return err;
}

char *printGEDCOM(const GEDCOMobject *obj) {
	
}

void deleteGEDCOM(GEDCOMobject *obj) {
	
}


char *printError(GEDCOMerror err) {
	char *errMsg = malloc(sizeof(char));
	char *name = getErrorName(err.type);
	char line[25];
	
	strcpy(errMsg, "");
	
	if (name == NULL) {
		return NULL;
	}
	
	sprintf(line, "%d", err.line);
	
	errMsg = realloc(errMsg, strlen(name) * sizeof(char) + strlen(line));
	
	strcat(errMsg, name);
	strcat(errMsg, " (line ");
	strcat(errMsg, line);
	strcat(errMsg, ")");
	
	return errMsg;
}

Individual *findPerson(const GEDCOMobject *familyRecord, bool (*compare)(const void *first, const void *second), const void *person) {
	
}

List getDescendants(const GEDCOMobject *familyRecord, const Individual *person) {
	
}

void deleteEvent(void *toBeDeleted) {
	
}

int compareEvents(const void *first, const void *second) {
	
}

char *printEvent(void *toBePrinted) {
	
}

void deleteIndividual(void *toBeDeleted) {
	
}

int compareIndividuals(const void *first, const void *second) {
	
}

char *printIndividual(void *toBePrinted) {
	
}

void deleteFamily(void *toBeDeleted) {
	
}

int compareFamilies(const void *first, const void *second) {
	
}

char *printFamily(void *toBePrinted) {
	
}

void deleteField(void *toBeDeleted) {
	
}

int compareFields(const void *first, const void *second) {
	
}

char *printField(void *toBePrinted) {
	
}
