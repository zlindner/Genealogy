#include <stdio.h>
#include <stdlib.h>
#include "GEDCOMparser.h"
#include "LinkedListAPI.h"
#include "GEDCOMutilities.h"

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
