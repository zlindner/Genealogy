/*Header *header = NULL;
   Submitter *subm = NULL;

   bool allocHeader = false;
   bool allocSubm = false;

   char line[257];
   int lineNum = 0;

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

        lineNum++;

        if (tag[0] == '@' && tag[strlen(tag) - 1] == '@') {
                if (strcmp(val, "INDI") == 0) {
                        Individual *indiv = malloc(sizeof(Individual));

                        if (indiv == NULL) {
                                err.type = OTHER_ERROR;
                                err.line = -1;
                                goto CLEANUP;
                        }

                        indiv->givenName = calloc(sizeof(char), 100);
                        sprintf(indiv->givenName, "%d", lineNum);
                        indiv->surname = calloc(sizeof(char), 100);
                        indiv->events = initializeList(&printEvent, &deleteEvent, &compareEvents);
                        indiv->families = initializeList(&printFamily, &deleteFamily, &compareFamilies);
                        indiv->otherFields = initializeList(&printField, &deleteField, &compareFields);

                        insertData(hashTable, tag, indiv);
                } else if (strcmp(val, "FAM") == 0) {
                        Family *fam = malloc(sizeof(Family));

                        if (fam == NULL) {
                                err.type = OTHER_ERROR;
                                err.line = -1;
                                goto CLEANUP;
                        }

                        fam->wife = calloc(sizeof(Individual), 1);
                        fam->husband = calloc(sizeof(Individual), 1);
                        fam->children = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);
                        fam->events = initializeList(&printEvent, &deleteEvent, &compareEvents);
                        fam->otherFields = initializeList(&printField, &deleteField, &compareFields);

                        insertData(hashTable, tag, fam);
                } else if (strcmp(val, "SUBM") == 0) {
                        allocSubm = true;

                        subm = calloc(sizeof(Submitter) + 100, 1);

                        if (subm == NULL) {
                                err.type = OTHER_ERROR;
                                err.line = -1;
                                return err;
                        }

                        subm->otherFields = initializeList(&printField, &deleteField, &compareFields);

                        insertData(hashTable, tag, subm);
                }
        }
   }

   rewind(file);

   // HEADER
   bool buildHeader = false;
   bool hasHeader = false;
   bool buildHeaderGEDC = false;
   bool buildingHeaderSOUR = false;
   bool hasVersion = false;
   bool hasEncoding = false;

   bool buildSubm = false;
   bool hasSubm = false;

   Individual *indivPtr = NULL;
   bool buildIndiv = false;

   Family *famPtr = NULL;
   bool buildFam = false;

   Event *event = NULL;
   bool buildEvent = false;

   bool hasTRLR;

   lineNum = 0;

   while (fgets(line, sizeof(line), file)) {
        //TODO possibly check for line terminators -> error if dont exist
        line[strcspn(line, "\r\n")] = 0;

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

        printf("%d\n", lineNum);

        if (level == 0 && buildHeader) {
                buildHeader = false;

                if (header->source[0] != '\0' && hasVersion && hasEncoding && header->submitter != NULL) {
                        hasHeader = true;

                        (*obj)->header = header;
                } else {
                        err.type = INV_HEADER;
                        err.line = lineNum;
                        goto CLEANUP;
                }
        }

        if (level == 0 && buildSubm) {
                buildSubm = false;

                if (subm->submitterName[0] != '\0') {
                        hasSubm = true;

                        (*obj)->submitter = subm;
                } else {
                        err.type = INV_RECORD;
                        err.line = lineNum;
                        goto CLEANUP;
                }
        }

        if ((level == 0 || level == 1) && buildEvent) {
                buildEvent = false;

                if (buildIndiv) {
                        insertBack(&indivPtr->events, event);
                } else if (buildFam) {
                        insertBack(&famPtr->events, event);
                }
        }

        if (level == 0 && buildIndiv) {
                buildIndiv = false;
                insertBack(&(*obj)->individuals, indivPtr);
        }

        if (level == 0 && buildFam) {
                buildFam = false;
                insertBack(&(*obj)->families, famPtr);
        }

        if ((level == 0 || level == 1) && buildHeaderGEDC) {
                if (hasVersion) {
                        buildHeaderGEDC = false;
                } else {
                        err.type = INV_HEADER;
                        err.line = lineNum;
                        goto CLEANUP;
                }
        }

        if ((level == 0 || level == 1) && buildingHeaderSOUR) {
                buildingHeaderSOUR = false;
        }

        if (tag[0] == '@' && tag[strlen(tag) - 1] == '@') {
                if (strcmp(val, "INDI") == 0) {
                        buildIndiv = true;
                        indivPtr = lookupData(hashTable, tag);
                        continue;
                } else if (strcmp(val, "FAM") == 0) {
                        buildFam = true;
                        famPtr = (Family *) lookupData(hashTable, tag);
                        continue;
                } else if (strcmp(val, "SUBM") == 0) {
                        buildSubm = true;
                } else {
                        err.type = INV_RECORD;
                        err.line = lineNum;
                        goto CLEANUP;
                }
        }

        if (strcmp(tag, "HEAD") == 0) {
                if (level != 0 || val[0] != '\0') {
                        err.type = INV_HEADER;
                        err.line = lineNum;
                        goto CLEANUP;
                }

                allocHeader = true;

                buildHeader = true;

                header = calloc(sizeof(Header), 1);
                if (header == NULL) {
                        err.type = OTHER_ERROR;
                        err.line = -1;
                        goto CLEANUP;
                }

                header->otherFields = initializeList(&printField, &deleteField, &compareFields);

                continue;
        }

        if (buildHeader) {
                if (tag[0] == '\0' || val[0] == '\0') {
                        if (strcmp(tag, "GEDC") == 0) {
                                buildHeaderGEDC = true;
                        } else {
                                err.type = INV_HEADER;
                                err.line = lineNum;
                                goto CLEANUP;
                        }
                } else {
                        if (strcmp(tag, "SOUR") == 0) {
                                if (level == 1) {
                                        buildingHeaderSOUR = true;

                                        strcpy(header->source, val);
                                        header->source[strlen(header->source)] = '\0';
                                } else {
                                        err.type = INV_HEADER;
                                        err.line = lineNum;
                                        goto CLEANUP;
                                }
                        } else if (strcmp(tag, "SUBM") == 0) {
                                header->submitter = subm;
                        } else if (strcmp(tag, "VERS") == 0) {
                                if (buildHeaderGEDC) {
                                        header->gedcVersion = strtoimax(val, NULL, 0);
                                        hasVersion = true;
                                } else if (!buildingHeaderSOUR) {
                                        err.type = INV_HEADER;
                                        err.line = lineNum;
                                        goto CLEANUP;
                                }
                        } else if (strcmp(tag, "CHAR") == 0) {
                                header->encoding = getEncoding(val);

                                // TODO validate encoding

                                hasEncoding = true;
                        } else {
                                insertBack(&header->otherFields, initializeField(tag, val));
                        }
                }
        }

        if (buildIndiv) {
                if (strcmp(tag, "NAME") == 0) {
                        char *tok = strtok(val, "/");
                        if (tok != NULL) {
                                if (indivPtr->givenName[0] == '\0') {
                                        indivPtr->givenName = realloc(indivPtr->givenName, strlen(tok) + 1);
                                        strcpy(indivPtr->givenName, tok);
                                        indivPtr->givenName[strlen(indivPtr->givenName)] = '\0';
                                }

                                tok = strtok(NULL, "/");
                                if (tok != NULL) {
                                        if (indivPtr->surname[0] == '\0') {
                                                indivPtr->surname = realloc(indivPtr->surname, strlen(tok) + 1);
                                                strcpy(indivPtr->surname, tok);
                                                indivPtr->surname[strlen(indivPtr->surname)] = '\0';
                                        }
                                }
                        }
                } else if (strcmp(tag, "GIVN") == 0) {
                        if (indivPtr->givenName[0] == '\0') {
                                indivPtr->givenName = realloc(indivPtr->givenName, strlen(val) + 1);
                                strcpy(indivPtr->givenName, val);
                                indivPtr->givenName[strlen(indivPtr->givenName)] = '\0';
                        }
                } else if (strcmp(tag, "SURN") == 0) {
                        if (indivPtr->surname[0] == '\0') {
                                indivPtr->surname = realloc(indivPtr->surname, strlen(val) + 1);
                                strcpy(indivPtr->surname, val);
                                indivPtr->surname[strlen(indivPtr->surname)] = '\0';
                        }
                } else if (strcmp(tag, "FAMS") == 0) {
                        insertBack(&indivPtr->families, (Family *) lookupData(hashTable, val));
                } else if (strcmp(tag, "FAMC") == 0) {
                        insertBack(&indivPtr->families, (Family *) lookupData(hashTable, val));
                } else if (isIndivEvent(tag) && !buildEvent) {
                        buildEvent = true;

                        event = malloc(sizeof(Event));

                        strcpy(event->type, tag);
                        event->type[4] = '\0';

                        event->date = calloc(sizeof(char), 1);
                        event->place = calloc(sizeof(char), 1);

                        event->otherFields = initializeList(&printEvent, &deleteEvent, &compareEvents);
                        continue;
                } else if (!buildEvent) {
                        if (tag[0] == '\0' || val[0] == '\0') {
                                err.type = INV_RECORD;
                                err.line = lineNum;
                                goto CLEANUP;
                        }

                        insertBack(&indivPtr->otherFields, initializeField(tag, val));
                }
        }

        if (buildFam) {
                if (strcmp(tag, "HUSB") == 0) {
                        if (val[0] == '\0') {
                                err.type = INV_RECORD;
                                err.line = lineNum;
                                goto CLEANUP;
                        }

                        famPtr->husband = (Individual *) lookupData(hashTable, val);
                } else if (strcmp(tag, "WIFE") == 0) {
                        if (val[0] == '\0') {
                                err.type = INV_RECORD;
                                err.line = lineNum;
                                goto CLEANUP;
                        }

                        famPtr->wife = (Individual *) lookupData(hashTable, val);
                } else if (strcmp(tag, "CHIL") == 0) {
                        if (val[0] == '\0') {
                                err.type = INV_RECORD;
                                err.line = lineNum;
                                goto CLEANUP;
                        }

                        insertBack(&famPtr->children, (Individual *) lookupData(hashTable, val));
                } else if (isFamEvent(tag)  && !buildEvent) {
                        buildEvent = true;

                        event = malloc(sizeof(Event));

                        strcpy(event->type, tag);
                        event->type[4] = '\0';

                        event->date = calloc(sizeof(char), 1);
                        event->place = calloc(sizeof(char), 1);

                        event->otherFields = initializeList(&printEvent, &deleteEvent, &compareEvents);

                        continue;
                } else if (!buildEvent) {
                        if (tag[0] == '\0' || val[0] == '\0') {
                                err.type = INV_RECORD;
                                err.line = lineNum;
                                goto CLEANUP;
                        }

                        insertBack(&famPtr->otherFields, initializeField(tag, val));
                }
        }

        if (buildSubm) {
                if (strcmp(tag, "NAME") == 0) {
                        if (val[0] == '\0') {
                                err.type = INV_RECORD;
                                err.line = lineNum;
                                goto CLEANUP;
                        }

                        strcpy(subm->submitterName, val);
                        subm->submitterName[strlen(subm->submitterName)] = '\0';
                } else if (strcmp(tag, "ADDR") == 0) {
                        if (val[0] != '\0') {
                                strcpy(subm->address, val);
                        } else {
                                err.type = INV_RECORD;
                                err.line = lineNum;
                                goto CLEANUP;
                        }
                } else if (strcmp(tag, "CONT") == 0) {
                        if (val[0] == '\0') {
                                err.type = INV_RECORD;
                                err.line = lineNum;
                                goto CLEANUP;
                        }

                        if (subm->address[0] != '\0') {
                                sprintf(subm->address + strlen(subm->address), "\\n%s", val);
                        }
                } else if (strcmp(tag, "CONC") == 0) {
                        if (val[0] == '\0') {
                                err.type = INV_RECORD;
                                err.line = lineNum;
                                goto CLEANUP;
                        }

                        if (subm->address[0] != '\0') {
                                strcat(subm->address, val);
                        }
                } else {
                        if (tag[0] == '\0' || val[0] == '\0') {
                                err.type = INV_RECORD;
                                err.line = lineNum;
                                goto CLEANUP;
                        }

                        insertBack(&subm->otherFields, initializeField(tag, val));
                }
        }

        if (buildEvent) {
                if (strcmp(tag, "DATE") == 0) {
                        if (val[0] == '\0') {
                                err.type = INV_RECORD;
                                err.line = lineNum;
                                goto CLEANUP;
                        }

                        event->date = realloc(event->date, strlen(val) + 1);
                        strcpy(event->date, val);
                        event->date[strlen(event->date)] = '\0';
                } else if (strcmp(tag, "PLAC") == 0) {
                        if (val[0] == '\0') {
                                err.type = INV_RECORD;
                                err.line = lineNum;
                                goto CLEANUP;
                        }

                        event->place = realloc(event->place, strlen(val) + 1);
                        strcpy(event->place, val);
                        event->place[strlen(event->place)] = '\0';
                } else {
                        if (tag[0] == '\0' || val[0] == '\0') {
                                err.type = INV_RECORD;
                                err.line = lineNum;
                                goto CLEANUP;
                        }

                        insertBack(&event->otherFields, initializeField(tag, val));
                }
        }

        if (strcmp(tag, "TRLR") == 0) {
                if (level == 0 && val[0] == '\0') {
                        hasTRLR = true;
                } else {
                        err.type = INV_RECORD;
                        err.line = lineNum;
                        goto CLEANUP;
                }
        }
   }

   //printf("%s\n", printGEDCOM(*obj));
   //printf("%s\n", printHeader((*obj)->header));

   //printf("%s\n", printIndividual(lookupData(hashTable, "@I1@")));

   ListIterator iter = createIterator((*obj)->individuals);
   Individual *indiboy;
   while ((indiboy = nextElement(&iter)) != NULL) {
        printf("%s\n", printIndividual(indiboy));
   }

   if (!hasHeader || !hasTRLR) {
        err.type = INV_GEDCOM;
        err.line = -1;
        goto CLEANUP;
   }

   fclose(file);

   destroyTable(hashTable);

   err.type = OK;
   err.line = -1;
   return err;

   CLEANUP:
   destroyTable(hashTable);

   if (allocHeader) {
        deleteHeader(header);
   } else if (allocSubm) {
        deleteSubmitter(subm);
   }

   clearList(&((*obj)->families));
   clearList(&((*obj)->individuals));

   return err;*/
