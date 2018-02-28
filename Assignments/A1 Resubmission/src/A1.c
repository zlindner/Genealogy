#include <stdio.h>
#include <stdlib.h>
#include "GEDCOMparser.h"
#include "LinkedListAPI.h"
#include "GEDCOMutilities.h"

bool compare(const void *first, const void *second) {
	Individual *a = (Individual *) first;
	Individual *b = (Individual *) second;

	return strcmp(a->givenName, b->givenName) && strcmp(a->surname, b->surname);
}

int main(void) {
	GEDCOMobject *obj;
	GEDCOMerror err = createGEDCOM("assets/simpleValid2GenE1.ged", &obj);

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
