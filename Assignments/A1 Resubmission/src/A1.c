#include <stdio.h>
#include <stdlib.h>
#include "GEDCOMparser.h"
#include "LinkedListAPI.h"
#include "GEDCOMutilities.h"

bool compare(const void *first, const void *second) {
	Individual *a = (Individual *) first;
	Individual *b = (Individual *) second;

	return strcmp(a->givenName, b->givenName) == 0 && strcmp(a->surname, b->surname) == 0;
}

int main(void) {
	GEDCOMobject *obj;
	GEDCOMerror err = createGEDCOM("assets/shakespeare.ged", &obj);

	Individual *i = malloc(sizeof(Individual));
	i->givenName = malloc(100);
	i->surname = malloc(100);
	strcpy(i->givenName, "John");
	strcpy(i->surname, "Shakespeare");

	Individual *person = findPerson(obj, compare, i);

	List d = getDescendants(obj, person);

	if (err.type == OK) {
		char *str = printGEDCOM(obj);
		printf("%s\n", str);
		free(str);

		deleteGEDCOM(obj);
	}

	char *errStr = printError(err);
	printf("%s\n", errStr);

	free(errStr);

	return 0;
}
