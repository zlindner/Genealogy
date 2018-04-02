#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "GEDCOMparser.h"
#include "LinkedListAPI.h"
#include "GEDCOMutilities.h"

int main(void) {
	GEDCOMobject *obj;
	GEDCOMerror err = createGEDCOM("../uploads/shakespeare.ged", &obj);

	//char *info = getFileInfo("../uploads/shakespeare.ged");
	//int val = createIndividual("shakespeare.ged", "Henry", "Innis");
	char *desc = getDesc("shakespeare.ged", "Mary", "Arden", 0);

	return 0;
}
