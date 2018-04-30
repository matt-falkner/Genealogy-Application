#ifndef GEDCOMUTILITIES__H
#define GEDCOMUTILITIES__H

#include "ctype.h"
#include "stdbool.h"
#include "assert.h"
#include "stddef.h"

typedef struct 
{
	Individual * data;
	char * xref;
	
} Individualp;


typedef struct 
{
	Family * data;
	char * xref;
} Familyp;


//PARSE DATE FUNCTIONS
typedef struct {
	int day, month, year;
}Date;


/* GLUE */
char * getFileDetails( char * filename);
char * printCharSetJSON(CharSet item);
char * writeGEDCOMfromJSON(char * filename, char * json);
char * getIndividualsInFile(char * filename);
char * injectIndividualIntoFile(char * filename, char * json);
char * getDescendantsForPersonInFile(char * filename, char * individualJson, int number);
Individual* findMatchInRecord(const GEDCOMobject* familyRecord, int (*compare)(const void* first, const void* second), const void* person);
char * getAncestorsForPersonInFile(char * filename, char * individualJson, int number);



char * findSex(const Individual * ind);



char* indToJSONExtra(const Individual* ind);

char * indvsToJSON(GEDCOMobject * obj);

Date * initDate(void);
void parseDate(Date ** date, char * rawDate);
int rawMonthParse(char * rawMonth);
int compareDate(Date * temp1, Date * temp2);


char * concatStrings(char * before, char * new);
void printIndividualFully( void * person);

/* 		PARSERS LEVEL 2: Individuals.
 * 		~Helper Functions.
 */		
bool customIndividualMapCompare(const void* first,const void* second);
Individual * initIndividual(void);
bool inWhitelistInvidual(char * token);
Event * initEvent();

Family * initFamily(void);
Submitter * initSubmitter(void);


char * helloWorld(void);


List * findListChildrenDataComparison(const Individual * person);


List * findChildren(const Individual * person);


/* 		PARSERS LEVEL 2: Headers
 *  	~Helper Functions.
 */
bool inWhitelistHead(char * token);
Header *  initHeader(void);

char * printCharSet(CharSet item);

/* ~~~~GENERIC FUNCTIONS ~~~~
 * 	Can be used anywhere in the project as helper functions.
 */
float parseVersionNumber(char * string);
char * duplicateString(char * original);
//does a quick check if the error is OK or Not, IE. Whether to chain throw to top 
bool actualError(GEDCOMerror error);
//better version of STRCPY
void copyString(char * destination, char * source);
//Prints errors 
char * printError(GEDCOMerror err);
//LL printer with iterator
void printAllElements(List list);

bool inWhitelistFamily(char * token);
bool inWhitelistSubmitter(char * token);

char * cleanup(char * original);


//Currently I don't use 2 compare functions, so I use this as a dummy for init.

/*
 * 	EVENT Utility Functions for LinkedListAPI
 */ 
bool customComapreEvent(const void* first,const void* second);
char * printFuncEvent(void * toBePrinted);
void deleteFuncEvent(void * toBeDeleted);
//Event * initEvent();

/*
 * 	FIELD Utility Functions for LinkedListAPI
 */ 
char * printFuncField(void * toBePrinted);
void deleteFuncField(void * toBeDeleted);
bool customComapreField(const void* first,const void* second);

/*
 * 	FIELD CONT...
 */ 
Field * initField();



/*
 * 	Invidual POINTER Maps Utility Functions for LinkedListAPI
 * 	This is a hash map substituation!!!. 
 */ 
char * printFuncINDVMap(void * toBePrinted);
void deleteFuncINDVMap(void * toBeDeleted);


/*
 * 	Family POINTER Maps Utility Functions for LinkedListAPI
 * 	This is a hash map substituation!!!. 
 */ 
bool customComapreFamilyP(const void* first,const void* second);
char * printFuncFamilyP(void * toBePrinted);
void deleteFuncFamilyP(void * toBeDeleted);



GEDCOMerror writeHEADER(FILE ** buffer, const GEDCOMobject * obj);
GEDCOMerror writeINDIVIDUAL(FILE ** buffer, const GEDCOMobject * obj, List * individualPointersMap, List * familyPointersMap);
GEDCOMerror writeFAMILIES(FILE ** buffer, const GEDCOMobject * obj, List * individualPointersMap, List * familyPointersMap);
GEDCOMerror writeSUBMITTER(FILE ** buffer, const GEDCOMobject * obj);

bool comparePointersBool(const void * first, const void * second);
//Custom Printers 
int comparePointers(const void * first, const void * second);
char * printFieldValue(void * toBePrinted);
char * printFieldTag(void * toBePrinted);
int headDepthEvaluator(char * tag);
void searchAndReplaceCONT(char ** string, int currentDepth);
char * generateXREF(int index, char type);
char * printIndividualToGEDCOM(Individual * temp);

void printFamOtherToGEDCOM(Family * family, FILE * buffer);

List deepCopyListofLists(List original);

void appendChildrenToGeneration(const Individual * person, List * nextGeneration);
List * getNextGeneration(List * currentGeneration);

bool comparePointersFamilyPBool(const void * first, const void * second);

int comparePointersFamilyP(const void * first, const void * second);


void printIndOtherToGEDCOM(Individual * temp, FILE * buffer);


//GEDCOMerror validateGEDCOM(const GEDCOMobject* obj);


bool findPersonInList(const List * list, int (*compare)(const void* first, const void* second), const void * person);


/** Function to return a list of up to N generations of descendants of an individual in a GEDCOM
 *@pre GEDCOM object exists, is not null, and is valid
*@post GEDCOM object has not been modified in any way, and a list of descendants has been created
*@return a list of descendants. The list may be empty. All list members must be of type List.  
 *@param familyRecord - a pointer to a GEDCOMobject struct
 *@param person - the Individual record whose descendants we want
 *@param maxGen - maximum number of generations to examine (must be >= 1)
 **/
//List getDescendantListN(const GEDCOMobject* familyRecord,const Individual* person,unsigned int maxGen);


void appendChildrenToGeneration(const Individual * person, List * nextGeneration);


List * getFirstGeneration(const Individual * person);
/*
void deleteList(void* toBeDeleted);
int compareList(const void* first,const void* second);
char* printList(void* toBePrinted);
*/


List * duplicateList(List list);


/* Deep copy helper functions */
Event * deepCopyEvent(const Event * original);
Field * deepCopyField(const Field * original);
Individual * deepCopyIndividual(const Individual * original);



/* Get ancestors */
List * getPreviousGeneration(List * currentGeneration);
void appendParentsToGeneration(const Individual * person, List * prevGeneration);
//List getAncestorListN(const GEDCOMobject * familyRecord, const Individual * person, int maxGen);
List * getFirstAncestors(const Individual * person);

#endif
