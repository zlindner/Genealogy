#include <stdio.h>
#include <stdlib.h>
#include "GEDCOMparser.h"
#include "LinkedListAPI.h"

int main(void) {
	GEDCOMobject *obj;
	GEDCOMerror err = createGEDCOM("assets/simpleGEDCOM.ged", &obj);

	char *errStr = printError(err);
	printf("%s\n", errStr);

	if (err.type == OK) {
		char *str = printGEDCOM(obj);
		printf("%s\n", str);
		free(str);

		deleteGEDCOM(obj);
	}

	free(errStr);

	return 0;
}
