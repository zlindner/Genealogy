#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>

#include "TestHarness.h"
#include "GEDCOMTestCases.h"
#include "ListTestCases.h"

//#define TESTS 38  //A1, including list
//32 tests for A2
#define TESTS 70
#define DEBUG 0
#define OUT stdout

static int testNo;
static testRec * * testRecords;
static int studentScore;  //globals  required to handle segfaults gracefully
//------------ Something went wrong and student code segfaulted --------------

/*
static void segfaultCatcher(int signal,  siginfo_t* si, void *arg)
{
    printf("\n\n************** Code Segfaulted: Partial Report ********************\n");
    int j;
    for(j=0; j<TESTS; j++){
        if(testRecords[j] != NULL){
            printRecord(testRecords[j]);
        }
    }
    
    printf("*******************\nPARTIAL RESULTS\nProgram caused segfault\n*******************\n");
    printf("Partial Score:%d\n",studentScore);
    
    
    exit(EXIT_FAILURE);
}
*/
static void addTestResult(testRec* tmpRec){
    testRecords[testNo-1] = tmpRec;
    studentScore = studentScore + getScore(testRecords[testNo-1]);
    testNo++;
}

static float calcGrade(void){
    float weights[] = {	0,0,0,0,0,0,0,0,0,  			//Lists
                       	0,0,0,0,0,0,0,0,0,0,0,			//Valid GEDCOM
                      	0,0,0,0, 						//Invalid GEDCOM
        				0,0,0,0,						//Delete GEDCOM
                        0,0,							//printGEDCOM
                        0,								//printError
                        0,0,                            //Find person
                        0,0,0,0,0,						//Get descendants
                        //13 tests for Module 1
                        2,2,2,2,2,2,2,2,4,				//Valid writeGEDCOM - ref objects
                        3,								//Valid writeGEDCOM - from file
                        2,								//Valid writeGEDCOM - invalid args
                        3,2,                            //validateGEDCOM
                        //8 tests for Module 2
                        6,6,5,5,						//get descendants
                        6,6,5,5,                        //get ancestors
                        //11 tests for Module 3
                        3,1,                             //indToJSON
                        3,2,                             //JSONtoInd
                        5,2,                            //JSONtoGEDCOM
                        2,                              //addIndividual
                        3,1,                             //iListToJSON
                        3,1                             //gListToJSON
    };
    float totalScore = 0;
    int i = 0;
    for (i = 0; i < TESTS; i++){
        totalScore += weights[i]*(float)getScore(testRecords[i]);
        
        //Regreession
        
       
        
    }
    return totalScore;
}

int main(void)
{
    studentScore = 0;
    testNo = 1;
    testRec* tmpRec = NULL;
/*    struct sigaction segfaultSignaler;
    // set up the segfault handler
    memset(&segfaultSignaler, 0, sizeof(struct sigaction));
    sigemptyset(&segfaultSignaler.sa_mask);
    segfaultSignaler.sa_sigaction = segfaultCatcher;
    segfaultSignaler.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &segfaultSignaler, NULL);
*/
    //record keeping array
    testRecords =  malloc(sizeof(testRec *)* TESTS);
    
    if(DEBUG) fprintf(OUT, "************** Testing Details ********************\n\n");
    
    //Create list
    if (DEBUG) fprintf(OUT,"Testing list creation\n");
    tmpRec = initListTest(testNo);
    addTestResult(tmpRec);
    
    //Node Creation
    if (DEBUG) fprintf(OUT, "Testing Node Creation\n");
    tmpRec = initNodeTest(testNo);
    addTestResult(tmpRec);
    
    //Testing getLength
    if (DEBUG) fprintf(OUT, "Testing getLength\n");
    tmpRec = getLengthTest(testNo);
    addTestResult(tmpRec);
    
    //Insert Front and Back
    if (DEBUG) fprintf(OUT, "Testing insertion\n");
    tmpRec = insertTest(testNo);
    addTestResult(tmpRec);
    
    //Getter functions test
    if (DEBUG) fprintf(OUT, "Testing getter functions\n");
    tmpRec = getTest(testNo);
    addTestResult(tmpRec);
    
    //Insert Sorted
    if (DEBUG) fprintf(OUT, "Testing sorted insertion\n");
    tmpRec = insertSortedTest(testNo);
    addTestResult(tmpRec);
    
    //delete List
    if (DEBUG) fprintf(OUT, "Testing deletion functionality");
    //runTest(&deleteTest);
    tmpRec = deleteTest(testNo);
    addTestResult(tmpRec);
    
    //Printing
    if (DEBUG) fprintf(OUT, "Testing list printing");
    tmpRec = printTest(testNo);
    addTestResult(tmpRec);

    //search List
    if (DEBUG) fprintf(OUT, "Testing search list functionality");
    //runTest(&deleteTest);
    tmpRec = searchTest(testNo);
    addTestResult(tmpRec);
    
    
    //Create GEDCOM tests
    if (DEBUG) fprintf(OUT,"Testing GEDCOM creation\n");
    
    if (DEBUG) fprintf(OUT,"Test 1\n");
    tmpRec = _tValidFileTest1(testNo);
    addTestResult(tmpRec);
    
    if (DEBUG) fprintf(OUT,"Test 2\n");
    tmpRec = _tValidFileTest2(testNo);
    addTestResult(tmpRec);
    
    if (DEBUG) fprintf(OUT,"Test 3\n");
    tmpRec = _tValidFileTest3(testNo);
    addTestResult(tmpRec);
    
    if (DEBUG) fprintf(OUT,"Test 4\n");
    tmpRec = _tValidFileTest4(testNo);
    addTestResult(tmpRec);
    
    if (DEBUG) fprintf(OUT,"Test 5\n");
    tmpRec = _tValidFileTest5(testNo);
    addTestResult(tmpRec);

    if (DEBUG) fprintf(OUT,"Test 6\n");
    tmpRec = _tValidFileTest6(testNo);
    addTestResult(tmpRec);

    if (DEBUG) fprintf(OUT,"Test 7\n");
    tmpRec = _tValidFileTest7(testNo);
    addTestResult(tmpRec);
    
    if (DEBUG) fprintf(OUT,"Test 8\n");
    tmpRec = _tValidFileTest8(testNo);
    addTestResult(tmpRec);
    
    if (DEBUG) fprintf(OUT,"Test 9\n");
    tmpRec = _tValidFileTest9(testNo);
    addTestResult(tmpRec);
    
    if (DEBUG) fprintf(OUT,"Test 10\n");
    tmpRec = _tValidFileTest10(testNo);
    addTestResult(tmpRec);
  
    if (DEBUG) fprintf(OUT,"Test 11\n");
    tmpRec = _tValidFileTest11(testNo);
    addTestResult(tmpRec);
    
    
    
    if (DEBUG) fprintf(OUT,"INV_FILE Tests\n");
    tmpRec = _tinvalidFileTests1(testNo);
    addTestResult(tmpRec);
    
    if (DEBUG) fprintf(OUT,"INV_GEDCOM Tests\n");
    tmpRec = _tinvalidFileTests2(testNo);
    addTestResult(tmpRec);
    
    if (DEBUG) fprintf(OUT,"INV_HEADER Tests\n");
    tmpRec = _tinvalidFileTests3(testNo);
    addTestResult(tmpRec);
    
    if (DEBUG) fprintf(OUT,"INV_RECORD Tests\n");
    tmpRec = _tinvalidFileTests4(testNo);
    addTestResult(tmpRec);

    
    
    if (DEBUG) fprintf(OUT,"Delete Test1\n");
    tmpRec = _tDeleteGEDCOMTest1(testNo);
    addTestResult(tmpRec);
    
    if (DEBUG) fprintf(OUT,"Delete Test2\n");
    tmpRec = _tDeleteGEDCOMTest2(testNo);
    addTestResult(tmpRec);
    
    if (DEBUG) fprintf(OUT,"Delete Test3\n");
    tmpRec = _tDeleteGEDCOMTest3(testNo);
    addTestResult(tmpRec);
    
    if (DEBUG) fprintf(OUT,"Delete Test4\n");
    tmpRec = _tDeleteGEDCOMTest4(testNo);
    addTestResult(tmpRec);
    
    
    
    if (DEBUG) fprintf(OUT,"Print Tests\n");
    tmpRec = _tPrintGEDCOMTest(testNo);
    addTestResult(tmpRec);
    
    if (DEBUG) fprintf(OUT,"Print Test - NULL arg\n");
    tmpRec = _tPrintGEDCOMTestNULL(testNo);
    addTestResult(tmpRec);
    
    
    
    if (DEBUG) fprintf(OUT,"Print Error Tests\n");
    tmpRec = _tPrintGEDCOMerrorTest(testNo);
    addTestResult(tmpRec);
    
    
    
    if (DEBUG) fprintf(OUT,"Find Person Tests\n");
    tmpRec = _tFindPersonTest(testNo);
    addTestResult(tmpRec);

    if (DEBUG) fprintf(OUT,"Find Person Test - NULL arg\n");
    tmpRec = _tFindPersonTestNull(testNo);
    addTestResult(tmpRec);
    
    
    
    if (DEBUG) fprintf(OUT,"Get Descendants Tests\n");
    tmpRec = _tGetDescendantsTest1(testNo);
    addTestResult(tmpRec);

    tmpRec = _tGetDescendantsTest2(testNo);
    addTestResult(tmpRec);
    
    tmpRec = _tGetDescendantsTest3(testNo);
    addTestResult(tmpRec);
    
    tmpRec = _tGetDescendantsTest4(testNo);
    addTestResult(tmpRec);
    
    tmpRec = _tGetDescendantsTest5(testNo);
    addTestResult(tmpRec);
    

    //Module 1
    
    tmpRec = _tWriteTest1(testNo);
    addTestResult(tmpRec);
    tmpRec = _tWriteTest2(testNo);
    addTestResult(tmpRec);
    tmpRec = _tWriteTest3(testNo);
    addTestResult(tmpRec);
    tmpRec = _tWriteTest4(testNo);
    addTestResult(tmpRec);
    tmpRec = _tWriteTest5(testNo);
    addTestResult(tmpRec);
    tmpRec = _tWriteTest6(testNo);
    addTestResult(tmpRec);
    tmpRec = _tWriteTest7(testNo);
    addTestResult(tmpRec);
    tmpRec = _tWriteTest8(testNo);
    addTestResult(tmpRec);
    tmpRec = _tWriteTest9(testNo);
    addTestResult(tmpRec);
    
    tmpRec = _tWriteTestWFile(testNo);
    addTestResult(tmpRec);
    tmpRec = _tWriteTestNull(testNo);
    addTestResult(tmpRec);
    
    tmpRec = _tValidateTest(testNo);
    addTestResult(tmpRec);
    tmpRec = _tValidateTestInv(testNo);
    addTestResult(tmpRec);

    
    //Module 2
    tmpRec = _tGetDescNTestBasicCorrect(testNo);
    addTestResult(tmpRec);
    tmpRec = _tGetDescNTestAdvancedCorrect(testNo);
    addTestResult(tmpRec);
    tmpRec = _tGetDescNTestValidEdgeCases(testNo);
    addTestResult(tmpRec);
    tmpRec = _tGetDescNTestInvalidArguments(testNo);
    addTestResult(tmpRec);
    
    tmpRec = _tGetAnsNTestBasicCorrect(testNo);
    addTestResult(tmpRec);
    tmpRec = _tGetAnsNTestAdvancedCorrect(testNo);
    addTestResult(tmpRec);
    tmpRec = _tGetAnsNTestValidEdgeCases(testNo);
    addTestResult(tmpRec);
    tmpRec = _tGetAnsNTestInvalidArguments(testNo);
    addTestResult(tmpRec);

    
    //Module 3
    tmpRec = _tTestIndToJSON(testNo);
    addTestResult(tmpRec);
    tmpRec = _tTestIndToJSONInv(testNo);
    addTestResult(tmpRec);

    
    tmpRec = _tTestJSONtoInd(testNo);
    addTestResult(tmpRec);
    tmpRec = _tTestJSONtoIndInv(testNo);
    addTestResult(tmpRec);
    
    
    tmpRec = _tTestJSONtoGEDCOM(testNo);
    addTestResult(tmpRec);
    tmpRec = _tTestJSONtoGEDCOMInv(testNo);
    addTestResult(tmpRec);
    
    tmpRec = _tAddIndividual(testNo);
    addTestResult(tmpRec);
    
    tmpRec = _tTestiListToJSON(testNo);
    addTestResult(tmpRec);
    tmpRec = _tTestiListToJSONInv(testNo);
    addTestResult(tmpRec);

    
    tmpRec = _tTestgListToJSON(testNo);
    addTestResult(tmpRec);
    tmpRec = _tTestgListToJSONInv(testNo);
    addTestResult(tmpRec);

    
    int j;
    for(j=0; j<TESTS; j++)
    {
        
        if (j == 0) {
            printf("\n\nList API (No marks given/deducted)\n");
        }
        
        if (j == 9) {
            printf("\n\nASSIGNMENT 1 TESTING (No marks given/deducted)\n");
        }
        
        if (j == 38) {
            printf("\n\nASSIGNMENT 2 TESTING\n");
        }

        printRecord(testRecords[j]);
        //printf("\n");
    }
    
    printf("Score: %.0f\n", calcGrade());
    destroyRecords(testRecords, TESTS);
    
    return 0;
    
}
