#include "GEDCOMparser.h"
#include "GEDCOMutilities.h"
#include "ctype.h"
#include "stdbool.h"
#include "assert.h"
#include "stddef.h"



#define RED "\033[0;31m" //Red Color for terminal
#define GREEN "\033[0;32m"
#define RESET "\033[0m" 



#define CHAR2INT(c) (c-'0')


/*		HEAD PARSERS 
 * 		
 * 		:)
 */ 
GEDCOMerror parseHead(FILE ** buffer, int * line, GEDCOMobject ** obj);
GEDCOMerror parseIndividual(Individual * person, FILE ** buffer, int * line, char * refID, GEDCOMobject ** obj);
GEDCOMerror addLineToHeader(GEDCOMobject ** obj, char * token, char * substring);
GEDCOMerror addLinetoParentinHeader(GEDCOMobject ** obj, char * token, char * substring, char * parent);
GEDCOMerror isHeadValid(GEDCOMobject ** obj, int line);
GEDCOMerror addToIndividual(FILE ** buffer,int * line, int depth, GEDCOMobject ** obj, Individual ** person, char * type, char * substring);
GEDCOMerror parseSubmitter(Submitter * submitter,FILE ** buffer, int * line, char * token, GEDCOMobject ** obj);
GEDCOMerror parseEvent(Event ** event, FILE ** buffer, int * line, int depth, char * type);
GEDCOMerror peekForConactination(FILE ** buffer, char ** lastLinevalue, int depth, int * line);
char * printSubmitter(Submitter * toBePrinted);
char * parseTypeToken(FILE ** buffer, int * line); 
char * parseEOL(FILE ** buffer, int * line);

//Parsefull Name, pass in an individual and we will tokenize it and put it your person struct
void parseFullName(Individual * person, char * token);
int comapreTest(const void * first, const void * second);
GEDCOMerror parseFamily(Family * family, FILE ** buffer, int * line, char * refID, GEDCOMobject ** obj, List * individualMap);
void parseRecursively(const Individual * person, List * descendants, bool firstCall);

char * parseNextJSONToken(const char * json, int * currentIndex);


List getAncestorListN(const GEDCOMobject * familyRecord, const Individual * person, int maxGen)
{

	List listOfLists = initializeList(&printGeneration,&deleteGeneration, &compareGenerations);
	/* error check */
	if (familyRecord == NULL)
	{
		return listOfLists; 
	}
	if (person == NULL)
	{
		return listOfLists; 
	}


	//Go to max generations
	if (maxGen == 0)
		maxGen = 1000; 


	int genCount = 0;
	List * currGeneration = NULL;

	//printf("%d %d\n", genCount, maxGen);

	while (genCount < maxGen)
	{

		if (genCount == 0 && currGeneration == NULL)
		{
			currGeneration = getFirstAncestors(person);

			if (currGeneration != NULL)
			{
				//puts("found first gen");

				insertBack(&listOfLists, (void *)currGeneration);
				genCount++;
				continue;
			}
			else {
				//puts("found no children to first person. stopping");
				break;

				//exit(0);
				//break;
			}
		}
		else {
			
			currGeneration = getPreviousGeneration(currGeneration);

			if (currGeneration != NULL)
			{
				insertBack(&listOfLists, (void *)currGeneration);
				genCount++;
				continue;	

			}
			else {
				//puts("found no children to iteration person. stopping");
				break;
				//exit(0);
			}
		
		}

	}



	List copyListofList = deepCopyListofLists(listOfLists);


	return copyListofList;



}


//Question about this
ErrorCode validateGEDCOM(const GEDCOMobject* obj)
{


	if (obj == NULL)
	{
		return INV_GEDCOM;
	}

	if (obj->header == NULL)
	{
		return INV_GEDCOM;
	}

	if (obj->submitter == NULL)
	{
		return INV_GEDCOM;
	}



	/* Invalid header cases */
	if (obj->header->submitter == NULL)
	{
		return INV_HEADER;
	}

	if (obj->header->gedcVersion == 0)
	{
		return INV_HEADER;
	}

	if (obj->header->source[0] == '\0')
	{
		return INV_HEADER;
	}


	if (obj->submitter->submitterName[0] == '\0')
	{
		return INV_RECORD;
	}


	ListIterator listIndivudals = createIterator(obj->individuals);
	void * person;
	
	while ((person = nextElement(&listIndivudals))) 
	{

		if (person == NULL)
		{
		
			return INV_RECORD;	
		}

	}



	ListIterator listFamily = createIterator(obj->families);
	void * family;
	
	while ((family = nextElement(&listFamily))) 
	{
		if (family == NULL)
		{
			return INV_RECORD; 
			
		}
	}

	return OK;
}




char* indToJSON(const Individual* ind)
{
	/*char * temp = calloc(1, sizeof(char) * 512);
	sprintf(temp, "{\"givenName\":\"%s\",\"surname\":\"%s\"}", ind->givenName, ind->surname);
	return temp;
		*/
	char * buffer = NULL;
	size_t needed; 
	
	needed = snprintf(NULL, 0, "{\"givenName\":\"%s\",\"surname\":\"%s\"}", ind->givenName, ind->surname) + 1; 
	buffer = (char*)malloc(needed);
		
	if (buffer == NULL)
	{
		return NULL;
	}
	
	snprintf(buffer, needed, "{\"givenName\":\"%s\",\"surname\":\"%s\"}", ind->givenName, ind->surname);
	
	return buffer;
	
}

Individual* JSONtoInd(const char* str)
{
	if (str == NULL)
	{
		return NULL;
	}

	Individual * person = initIndividual();


 	char * parent = NULL;
    int i = 0;
    while ((parent = parseNextJSONToken(str, &i)) != NULL)
    {
    	printf("found parent %s\n", parent);
    	
    	char * child = parseNextJSONToken(str, &i);
    	if (child != NULL)
    	{
	  		printf("found child %s\n", child);

	  	
	  		if (strcmp(parent, "givenName") == 0)
	  		{
	  			person->givenName = malloc(sizeof(char) * (strlen(child) + 1));
	  			strcpy(person->givenName, child);
	  		}
	  		if (strcmp(parent, "surname") == 0)
	  		{
	  			person->surname = malloc(sizeof(char) * (strlen(child) + 1));
	  			strcpy(person->surname, child);
	  		}
  			if (strcmp(parent, "sex") == 0)
	  		{
	  			char * text1 = "SEX";

				if (strcmp(child, "MALE") == 0 || strcmp(child, "M") == 0 )
				{
					char * text2 = "M";
					Field * sex = initField();
					sex->tag = malloc(sizeof(char) * (sizeof(text1) + 1));
					strcpy(sex->tag, text1);
					sex->value = malloc(sizeof(char) * (sizeof(text2) + 1));
					strcpy(sex->value, text2);
					insertBack(&(person->otherFields), sex);

				}
				else if (strcmp(child, "FEMALE") == 0 || strcmp(child, "F") == 0)
				{
					char * text2 = "F";
					Field * sex = initField();
					sex->tag = malloc(sizeof(char) * (sizeof(text1) + 1));
					strcpy(sex->tag, text1);
					sex->value = malloc(sizeof(char) * (sizeof(text2) + 1));
					strcpy(sex->value, text2);
					insertBack(&(person->otherFields), sex);


				}
	  		}
    	}
    }

    printf("Created: %s\n", printIndividual((void *)person));

    /* parse that JSON*/

	return person;

}

	/*char firstTag[200];
	memset(firstTag, '\0', 199);
	char firstValue[200];
	memset(firstValue, '\0', 199);

	char secondTag[200];
	memset(secondTag, '\0', 199);
	char secondValue[200];
	memset(secondValue, '\0', 199);



	int length = strlen(str);
	if (length < 3)
	{
		person = NULL;
		return person;
	}

	//could do a loop to reach 1 or I which reps 1 
	if (str[0] != '{' || str[1] != '\"')
	{
		person = NULL;
		return person;
	}
	

	int firstTokenEnd = 0;

	int firstTagIndex = 0;

	for (int i = 2; i < length; i++)
	{
		if(str[i] != '\"')
		{
			//printf("%c", str[i]);
			firstTag[firstTagIndex++] = str[i];

		}
		else {

			firstTokenEnd = i+1;
			break;
		}
	}

	//printf("1 TAG: '%s'\n", firstTag);

	if (str[firstTokenEnd] == ':')
	{

		firstTokenEnd++;
	}
	else {
		person = NULL;
		return person;
	}

	if (str[firstTokenEnd] == '\"')
	{

	 	firstTokenEnd++;
	}
	else {
		person = NULL;
		return person;
	}

	int firstObjectEnd = 0;
	int firstValueIndex = 0;

	for (int i = firstTokenEnd; i < length; i++)
	{

		if(str[i] != '\"')
		{
			firstValue[firstValueIndex++] = str[i];
		}		
		else {
			firstObjectEnd = i +1;
			break;
		}

	}

	//printf("1 VALUE: '%s'\n", firstValue);

	if (str[firstObjectEnd] == ',')
	{
		firstObjectEnd++;
	}
	else {
		person = NULL;
		return person;
	}

	if (str[firstObjectEnd] == '\"')
	{
		firstObjectEnd++;
	}
	else {
		person = NULL;
		return person;
	}


	int secondTagIndex = 0;
	int secondTokenEnd = 0;

	for (int i = firstObjectEnd; i < length; i++)
	{
		if(str[i] != '\"')
		{
			//printf("%c", str[i]);
			secondTag[secondTagIndex++] = str[i];

		}
		else {
			secondTokenEnd = i+1;
			break;
		}
	}

	//rintf("2 TAG: '%s'\n", secondTag);



	if (str[secondTokenEnd] == ':')
	{
		secondTokenEnd++;
	}
	else {
		person = NULL;
		return person;
	}

	if (str[secondTokenEnd] == '\"')
	{
		secondTokenEnd++;
	}
	else {
		person = NULL;
		return person;
	}

	int secondValueIndex = 0;

	for (int i = secondTokenEnd; i < length; i++)
	{

		if(str[i] != '\"')
		{
			secondValue[secondValueIndex++] = str[i];
		}		
		else {
			break;
		}

	}
	//printf("2 VALUE: '%s'\n", secondValue);

	if (strcmp(firstTag, "givenName") == 0)
	{
		person->givenName = (char *)calloc(1, sizeof(char) * (strlen(firstValue)+ 1));
		strcpy(person->givenName, firstValue);
	}

	if (strcmp(secondTag, "surname") == 0)
	{
		person->surname = (char *)calloc(1, sizeof(char) * (strlen(secondValue)+ 1));
		strcpy(person->surname, secondValue);
	}

	return person;
}
*/



GEDCOMobject* JSONtoGEDCOM(const char* str)
{
	Submitter * submitter = initSubmitter(); 
	Header * header = initHeader();
	GEDCOMobject * obj = malloc(sizeof(GEDCOMobject));
	obj->submitter = submitter;
	obj->header = header;
	obj->header->submitter = submitter;
	
    obj->families = initializeList(&printFamily, &deleteFamily, &compareFamilies);
    obj->individuals = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);
    
    printf("parsing this token %s\n", str);

    char * parent = NULL;
    int i = 0;
    while ((parent = parseNextJSONToken(str, &i)) != NULL)
    {
    	printf("found parent %s\n", parent);
    	
    	char * child = parseNextJSONToken(str, &i);
    	if (child != NULL)
    	{
	  		printf("found child %s\n", child);

	  		if (strcmp(parent, "subAddress") == 0)
	  		{
	  			strcpy(obj->submitter->address, child);
	  		}
	  		if (strcmp(parent, "subName") == 0)
	  		{
	  			strcpy(obj->submitter->submitterName, child);
	  		}
	  		if (strcmp(parent, "source") == 0)
	  		{
	  			strcpy(obj->header->source, child);
	  		}
  			if (strcmp(parent, "encoding") == 0)
	  		{
				if (strcmp(child, "ANSEL") == 0)
				{
					obj->header->encoding = ANSEL;
				}
				else if (strcmp(child, "UTF-8") == 0)
				{
					obj->header->encoding = UTF8;
				}
				else if (strcmp(child, "UTF8") == 0)
				{
					obj->header->encoding = UTF8;
				}
				else if (strcmp(child, "UNICODE") == 0)
				{
					obj->header->encoding = UNICODE;
				}
				else if (strcmp(child, "ASCII") == 0)
				{
					obj->header->encoding = ASCII;
				}	
	  		}
	  		if (strcmp(parent, "gedcVersion") == 0)
	  		{
	  			obj->header->gedcVersion = parseVersionNumber(child);
	  		}
    	}
    }
    /* parse that JSON*/

	return obj;
}

char * parseNextJSONToken(const char * json, int * currentIndex)
{
	int i = 0;
	//bool foundOpenBrace = false;

	char token[256];
	memset(token, '\0', 255);
	int tokenCount = 0;

	for (i = *currentIndex; i < strlen(json); i++)
	{
		if (json[i] == '{' || json[i] == ':' || json[i] == ',')
		{
			continue;
		}
		if (json[i] == '}')
		{
			return NULL;
		}

		if (json[i] == '\"')
		{
			i++;
			while (json[i] != '\"')
			{	
				token[tokenCount] = json[i];
				tokenCount++;
				//printf("%c", json[i]);
				i++;
			}
			token[tokenCount] = '\0';

			*currentIndex = i + 1; 

			char * result = malloc(sizeof(char) * (strlen(token) + 1));
			strcpy(result, token);

			return result;
			//terminate and return 

		}

	}
	return NULL;

}


char * gListToJSON(List gList)
{

	char * temp = calloc(1, sizeof(char) * 5000);
	char * final = calloc(1, sizeof(char) * 5000);

	if (getLength(gList) == 0)
	{
		sprintf(temp, "[]");
		return temp;
	}
	
	sprintf(temp, "[");
	final = concatStrings(final, temp);
	temp = calloc(1, sizeof(char) * 5000);

	ListIterator listOfList = createIterator(gList);
	void * list;

	int length = getLength(gList);
	int counter = 1;

	while ((list  = nextElement(&listOfList)) != NULL) 
	{

		char * result = iListToJSON(*((List *)list));
		sprintf(temp, "%s", result);
		free(result);
		final = concatStrings(final, temp);
		temp = calloc(1, sizeof(char) * 5000);
		
		if (counter < length)
		{
			sprintf(temp, ",");
			final = concatStrings(final, temp);
			temp = calloc(1, sizeof(char) * 5000);
			counter++;
		}
	
	}
		
	sprintf(temp, "]");
	final = concatStrings(final, temp);
	temp = calloc(1, sizeof(char) * 5000);

	return final;
}



void deleteGeneration(void* toBeDeleted)
{
	return;
}
int compareGenerations(const void* first,const void* second)
{

	List * list1 = (List *)first;
	List * list2 = (List *)second;

	if (list1 == NULL || list2 == NULL)
	{
		return -1; 
	}

	if (getLength(*list1) == getLength(*list2))
	{
		Individual * person1 = getFromFront(*list1);
		Individual * person2 = getFromFront(*list1);

		if (person1 == NULL || person2 == NULL)
		{
			return -1; 
		}

		if (compareIndividuals((void *) person1, (void *)person2) == 0)
		{
			return 0;
		}
		else {
			return -1;
		}
	}
	else {
		return -1;
	}

}
char* printGeneration(void* toBePrinted)
{
	return NULL;
}


void addIndividual(GEDCOMobject* obj, const Individual* toBeAdded)
{
	
	if (obj == NULL || toBeAdded == NULL)
	{
		return;
	}
	insertBack(&obj->individuals, (void *) toBeAdded);

	return;
}

char * iListToJSON(List iList)
{
	char * temp = calloc(1, sizeof(char) * 2000);
	char * final = calloc(1, sizeof(char) * 2000);

	if (getLength(iList) == 0)
	{
		sprintf(temp, "[]");
		return temp;
	}
	
	sprintf(temp, "[");
	final = concatStrings(final, temp);
	temp = calloc(1, sizeof(char) * 2000);

	ListIterator listIndivudals = createIterator(iList);
	void * person;

	int length = getLength(iList);
	int counter = 1;

	while ((person = nextElement(&listIndivudals)) != NULL) 
	{

		char * result = indToJSONExtra((Individual *)person);
		sprintf(temp, "%s", result);
		free(result);
		final = concatStrings(final, temp);
		temp = calloc(1, sizeof(char) * 2000);
		
		if (counter < length)
		{
			sprintf(temp, ",");
			final = concatStrings(final, temp);
			temp = calloc(1, sizeof(char) * 2000);
			counter++;
		}
	
	}
		
	sprintf(temp, "]");
	final = concatStrings(final, temp);
	temp = calloc(1, sizeof(char) * 2000);

	return final;
}



GEDCOMerror writeGEDCOM(char * filename, const GEDCOMobject * obj)
{
	/* Some quick error checking */
	GEDCOMerror error; 
	List individualPointersMap = initializeList(&printFuncINDVMap, &deleteFuncINDVMap, &comparePointers);
	List familyPointersMap = initializeList(&printFuncFamilyP, &deleteFuncFamilyP, &comparePointersFamilyP);
	
	if (filename == NULL)
	{
		error.type = WRITE_ERROR; 
		error.line = -1;
		return error;
	}
	if (obj == NULL)
	{
		error.type = WRITE_ERROR;
		error.line = -1;
		return error;
	}
	
	FILE * buffer = fopen(filename, "w");
	if (buffer == NULL) 
	{
		error.type = WRITE_ERROR; 
		error.line = -1;
		return error;
	}




	/* output generations begins */
	
	error = writeHEADER(&buffer, obj);
	
	error = writeINDIVIDUAL(&buffer, obj, &individualPointersMap, &familyPointersMap);
	
	/*
	printf("List length = %d\n", getLength(individualPointersMap));
    
    ListIterator individualIterator = createIterator(individualPointersMap);
	

	
	void * elem;
	while ((elem = nextElement(&individualIterator)) != NULL){
		printf("found item %s\n", (individualPointersMap).printData(elem));
	}
	
	
	ListIterator familyIterator = createIterator(familyPointersMap);
	
	void * family;
	while ((family = nextElement(&familyIterator)) != NULL){
		printf("found family %s\n", (familyPointersMap).printData(family));
		printf("%s", printFamily(((Familyp *)family)->data));
	}
	*/
	
	error = writeFAMILIES(&buffer, obj, &individualPointersMap, &familyPointersMap);
	
	error = writeSUBMITTER(&buffer, obj);
	
	
	//writeIndividuals
	//writeFamilies
	//writeSubmitter 
	
	fprintf(buffer, "0 TRLR");
	
	
	fclose(buffer);
		
	error.type = OK;
	error.line = -1; 
	return error;
}




List getDescendantListN(const GEDCOMobject* familyRecord,const Individual* person,unsigned int maxGen)
{
	List listOfLists = initializeList(&printGeneration,&deleteGeneration, &compareGenerations);
	/* error check */
	if (familyRecord == NULL)
	{
		return listOfLists; 
	}
	if (person == NULL)
	{
		return listOfLists; 
	}


	//Go to max generations
	if (maxGen == 0)
		maxGen = 1000; 


	int genCount = 0;
	List * currGeneration = NULL;

	//printf("%d %d\n", genCount, maxGen);

	while (genCount < maxGen)
	{
		//printf("COUNTERRRRR = %d\n", genCount);

		//puts("does this happen");
		if (genCount == 0 && currGeneration == NULL)
		{
			currGeneration = getFirstGeneration(person);

			if (currGeneration != NULL)
			{
				//puts("found first gen");

				insertBack(&listOfLists, (void *)currGeneration);
				genCount++;
				continue;
			}
			else {
				//puts("found no children to first person. stopping");
				break;

				//exit(0);
				//break;
			}
		}
		else {
			currGeneration = getNextGeneration(currGeneration);

			if (currGeneration != NULL)
			{
				insertBack(&listOfLists, (void *)currGeneration);
				genCount++;
				continue;	

			}
			else {
				//puts("found no children to iteration person. stopping");
				break;
				//exit(0);
			}
		
		}

	}



	//make list of copies, I can sort them here!


	List copyListofList = deepCopyListofLists(listOfLists);


	return copyListofList;

}






void parseRecursively(const Individual * person, List * descendants, bool firstCall)
{

	//for children of individual 
		//copy person
		//add (person copy to descendants) 
		//parseRecursively(parse for that found person)
	if (!firstCall)
	{
		insertBack(descendants, (void *)person); 
	}

	List * children = findChildren(person);
	
	//person has no children below them, back up
	if (children == NULL)
	{
		return; 
	}
	else {


		ListIterator childIterator = createIterator(*children);

		void * child;
		
		while ((child = nextElement(&childIterator)) != NULL) 
		{
			parseRecursively((Individual *)child, descendants, false);
		}

		return; 

	}
}


List getDescendants(const GEDCOMobject* familyRecord, const Individual* person)
{
	List descendants = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);

	if (person == NULL || familyRecord == NULL)
	{

		return descendants;
	}

	//
	parseRecursively(person, &descendants, true); 


	ListIterator descendantsIterator = createIterator(descendants);
	void * desc;
	
	while ((desc = nextElement(&descendantsIterator)) != NULL) 
	{
		printIndividual(desc);
	}


	return descendants;
}


void deleteGEDCOM(GEDCOMobject* obj)
{
	return;
}


//DUMMY will have to write one for each later. 
int compareTest(const void * first, const void * second)
{
	return 0;
}

GEDCOMerror createGEDCOM(char* fileName, GEDCOMobject** obj)
{	
	
	(*obj) = malloc(sizeof(GEDCOMobject));
	(*obj)->submitter = NULL;
	(*obj)->header = NULL;
	
	if (fileName == NULL)
	{
 		GEDCOMerror e;
        e.type = INV_FILE; 
        e.line = -1;
        (*obj) = NULL;
        return e; 

	}

	/* Check for valid find ending */
	char * check1 = strstr(fileName, ".ged");
	char * check2 = strstr(fileName, ".GED");

	if (check1 == NULL && check2 == NULL)
	{
		GEDCOMerror e;
        e.type = INV_FILE; 
        e.line = -1;
        (*obj) = NULL;
        return e; 

	}

    FILE * buffer = fopen(fileName, "r");   
    
    if (buffer == NULL)
    {
        GEDCOMerror e;
        e.type = INV_FILE; 
        e.line = -1;
        (*obj) = NULL;
        return e; 
    }
    
    bool trailer = false;


    //bool foundSubmitterRef = false;
    //bool foundSubmitterDeclare = false;
  
    
    //bool foundSubmitter = false;
    char c = '\0';
    int line = 1;
    
    List individualPointersMap = initializeList(&printFuncINDVMap, &deleteFuncINDVMap, &compareTest);
    List familyPointersMap = initializeList(&printFuncFamilyP, &deleteFuncFamilyP, &compareTest);
    
    (*obj)->families = initializeList(&printFamily, &deleteFamily, &compareFamilies);
    (*obj)->individuals = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);
    

    bool header = false;

    while ((c = fgetc(buffer)) != EOF)
    {
		if (c == '0')
		{
			
			char * token = NULL;
			token = parseTypeToken(&buffer, &line);
			
			if (token == NULL)
			{
				continue;
			}
			//printf("%s\n", token);
			
			if (strcmp(token, "TRLR") == 0)
			{
				trailer = true;
				//puts("found trailer");
				if (header == false)
				{
					 *(obj) = NULL;
					GEDCOMerror error; 
					error.type = INV_GEDCOM; 
					error.line = -1; 
					return error;
				}

				if ((*obj)->submitter == NULL || (*obj)->header->submitter == NULL)
				{
				   *(obj) = NULL;
					GEDCOMerror error;
					error.type = INV_GEDCOM; 
					error.line = -1;
					return error;
				}


				fclose(buffer);
				GEDCOMerror error;
				error.type = OK; 
				error.line = -1;
				return error;	
			}
			
			if (strcmp(token, "HEAD") == 0)
			{	

			  header = true;
							  
			  GEDCOMerror result = parseHead(&buffer, &line, obj);
			  
			  if (actualError(result))
			  {
				  fclose(buffer);
				  *(obj) = NULL;
				  return result;
			  }
			  
			  //VALIDATE HEAD
			  result = isHeadValid(obj, line);

			  if (actualError(result))
			  {
				  fclose(buffer);
				  *(obj) = NULL;
				  return result;
			  }	  

			  continue;			  
			}
			
			if (token[0] == '@')
			{
				if (header == false)
				{
					 *(obj) = NULL;	
					GEDCOMerror error; 
					error.type = INV_GEDCOM; 
					error.line = -1; 
					return error;
				}

				
				//TODO: Check to see if this is NULL: / Make function return null if nothing. THEREFORE no pointer, ERROR.
				char * substring = NULL;
				substring = parseEOL(&buffer, &line);


				if (strcmp(substring, "INDI") == 0)
				{
					  Individual * person = initIndividual();
					  
					  GEDCOMerror result = parseIndividual(person, &buffer, &line, substring, obj);
			  
					  if (actualError(result))
					  {
						  fclose(buffer);
						   *(obj) = NULL;
						  return result;
					  }
					  
					  insertBack(&(*obj)->individuals, (void *)person);
					  
					  
					  Individualp * ref = (Individualp *) calloc(1, sizeof(Individualp));
					  ref->xref = calloc(1, sizeof(char) * (strlen(token) + 1));
					  copyString(ref->xref, token);
					  ref->data = person;
					  insertBack(&individualPointersMap, (void *)ref);
	
					  continue;		
				}

				if (strcmp(substring, "FAM") == 0)
				{
  
					Family * family = initFamily();
					
					GEDCOMerror result = parseFamily(family, &buffer, &line, token, obj, &individualPointersMap);
					
					if (actualError(result))
					{
					   fclose(buffer);
					    *(obj) = NULL;
					   return result;
					}
					  
					//insert the family you just created
					insertBack(&(*obj)->families, (void *)family);

		
					//Add object to "hash map"
				    Familyp * ref = (Familyp *) calloc(1, sizeof(Familyp));
				    ref->xref = calloc(1, sizeof(char) * (strlen(token) + 1));
				    copyString(ref->xref, token);
				    ref->data = family;
				    insertBack(&familyPointersMap, (void *)ref);
				}
				
				//TODO:
				if (strcmp(substring, "SUBM") == 0)
				{


					//breaks because this happens

					(*obj)->submitter = initSubmitter();
					
					GEDCOMerror result = parseSubmitter((*obj)->submitter, &buffer, &line, token, obj);
					
					(*obj)->header->submitter = (*obj)->submitter;
					

					if (actualError(result))
					{
						fclose(buffer);
						 *(obj) = NULL;
						return result;
					}
					
				}	
			}			
			
		}
		else {

			//Invalid start
			if (line == 1)
			{
					fclose(buffer);
					 *(obj) = NULL;
					GEDCOMerror error;
					error.type = INV_HEADER; 
					error.line = -1;
					return error; 
			}


			fclose(buffer);
			 *(obj) = NULL;
			GEDCOMerror error;
			error.type = INV_RECORD; 
			error.line = line;
			return error; 
		}	
	}

	if (trailer == false)
	{
		fclose(buffer);
		*(obj) = NULL;
		GEDCOMerror error;
		error.type = INV_GEDCOM; 
		error.line = -1;
		return error;
	}
	else 
	{
		GEDCOMerror error;
		error.type = OK; 
		error.line = -1;
		return error;
	}



}

	/*	

	if ((*obj)->submitter == NULL || (*obj)->header->submitter == NULL)
	{
	   *(obj) = NULL;
		GEDCOMerror error;
   		error.type = INV_GEDCOM; 
    	error.line = -1;
    	return error;
	}
*/



GEDCOMerror addToIndividual(FILE ** buffer,int * line, int depth, GEDCOMobject ** obj, Individual ** person, char * token, char * substring)
{
	GEDCOMerror error;
	
	if (strcmp(token, "NAME") == 0)
	{
		if ((*person)->givenName == NULL || (*person)->surname == NULL)
		{
				parseFullName((*person), substring);
		}
	}
	else if (strcmp(token, "GIVN") == 0)
	{
		copyString((*person)->givenName, substring);
		
	}
	else if (strcmp(token, "SURN") == 0)
	{
		copyString((*person)->surname, substring);
	}
	
	else if (strcmp(token, "DEAT") == 0)
	{
		//parseEOL(buffer, line);
		Event * event = initEvent();
		parseEvent(&event, buffer, line, depth, token);
		insertBack(&((*person)->events), (void *) event);
	}
	else if (strcmp(token, "BIRT") == 0)
	{
		//parseEOL(buffer, line);
		Event * event = initEvent();
		parseEvent(&event, buffer, line, depth, token);
		insertBack(&((*person)->events), (void *) event);	
	}
	else if (strcmp(token, "BURI") == 0)
	{
		//parseEOL(buffer, line);
		Event * event = initEvent();
		parseEvent(&event, buffer, line, depth, token);
		insertBack(&((*person)->events), (void *) event);	
	}
	else if (strcmp(token, "CHR") == 0)
	{
		//parseEOL(buffer, line);
		Event * event = initEvent();
		parseEvent(&event, buffer, line, depth, token);
		insertBack(&((*person)->events), (void *) event);
		
	}
	else {

		if (strcmp(token, "FAMC") != 0 && strcmp(token, "FAMS") != 0 ) {
		
			Field * temp = initField();
			temp->tag = (char *)calloc(1, sizeof(char) * (strlen(token) + 1));
			temp->value = (char *)calloc(1, sizeof(char) * (strlen(substring) + 1));
			strcpy(temp->tag, token);
			strcpy(temp->value,substring);
			insertBack(&(*person)->otherFields, temp); 

		}
	}
		
	error.type = OK; 
	error.line = -1;
	return error;
}

GEDCOMerror parseSubmitter(Submitter * submitter,FILE ** buffer, int * line, char * token, GEDCOMobject ** obj)
{
	GEDCOMerror error;
	char c = '\0';
	int depth = 0;
	
	while ((c = fgetc(*buffer)) != EOF)
	{	
		if (isdigit(c)) 
		{
			depth = CHAR2INT(c); //Save the depth
			
			*line = *line + 1;	//You know its a new line, up the anti 
			if (depth == 0)
			{
				ungetc('0', *buffer); //If you find a zero, handler returns to CreateGEDCOM where it can reparse starting at 0
				GEDCOMerror error;
				error.type = OK; 	
				error.line = -1;
				return error; 
			}				
			if (depth == 1) //IF it's greater than 0, we are still in our object. Keep parsing
			{

				char * token = NULL; 
				token = parseTypeToken(buffer, line); //Parse the first token	
				
				if (inWhitelistSubmitter(token)) //Is it on the whitelist for Individuals?
				{
					char * substring = NULL; 
					substring = parseEOL(buffer, line); 
					peekForConactination(buffer, &substring, depth, line);
					
					if (strcmp(token, "NAME") == 0)
					{
						strcpy(submitter->submitterName, substring);
					}
					else if (strcmp(token, "ADDR") == 0)
					{
						//submitter->address = (char *) calloc(1, sizeof(char) * sizeof(substring));
						//strcpy(submitter->address, substring);
						
//Question about how to dynamically malloc for this space
//submitter = calloc(1, (sizeof(Submitter) + sizeof(char) * (strlen(substring) + 1)));
						strcpy(submitter->address, substring);
					}
					else {
						Field * temp = initField();
						temp->tag = (char *)calloc(1, sizeof(char) * (strlen(token) + 1));
						temp->value = (char *)calloc(1, sizeof(char) * (strlen(substring) + 1));
						strcpy(temp->tag, token);
						strcpy(temp->value,substring);
						
//HERE					
						//free(token);
						//free(substring);
		
										
						//What the fuck is add other fields because there is a memeory issue somewhere here

						insertBack(&(submitter)->otherFields, (void *)temp);
						
						//printAllElements((submitter)->otherFields);
					}
					
				}
				else {
					GEDCOMerror error;
					error.type = INV_RECORD; 	
					error.line = *line;
					return error; 
					
					
				}
			}
		}
	}
	
	
	
	error.type = OK;
	error.line = 0;
	return error;
}








Individual* findPerson(const GEDCOMobject* familyRecord, bool (*compare)(const void* first, const void* second), const void* person)
{
	if (familyRecord == NULL || person == NULL)
	{
		return NULL;
	}


	Individual * result = NULL;
	
	ListIterator recordIterator = createIterator(familyRecord->individuals);
	
	void * elem; 
	while ((elem = nextElement(&recordIterator)) != NULL)
	{
		if (compare(elem, person) == true)
		{
			result = (Individual *)elem;
		}
	}
	
	return result;
}

GEDCOMerror parseFamily(Family * family, FILE ** buffer, int * line, char * refID, GEDCOMobject ** obj, List * individualMap)
{
	GEDCOMerror error;
	int depth = 0;
	char c = '\0';
	
	while ((c = fgetc(*buffer)) != EOF)
	{
	
		if (isdigit(c)) 
		{
			depth = CHAR2INT(c); //Save the depth
			
			*line = *line + 1;	//You know its a new line, up the anti 
			if (depth == 0)
			{
				ungetc('0', *buffer); //If you find a zero, handler returns to CreateGEDCOM where it can reparse starting at 0
				GEDCOMerror error;
				error.type = OK; 	
				error.line = -1;
				return error; 
			}				
			if (depth == 1) //IF it's greater than 0, we are still in our object. Keep parsing
			{

				char * token = parseTypeToken(buffer, line); //Parse the first token	
				
				if (inWhitelistFamily(token)) //Is it on the whitelist for Individuals?
				{			
					char * substring = parseEOL(buffer, line); 
					peekForConactination(buffer, &substring, depth, line);
					
					if (strcmp(token, "HUSB") == 0)
					{
						void * result = findElement(*individualMap, &customIndividualMapCompare, (void *)substring);
						if (result == NULL) {
						//Don't throw error because it could just because no husband legit
						}	
						else {
							family->husband = ((Individualp * )result)->data;
							
							Individual * husband = (Individual *)((Individualp * )result)->data;
							insertBack(&(husband->families), family);		
						}
					}
					else if (strcmp(token, "WIFE") == 0)
					{

						void * result = findElement(*individualMap, &customIndividualMapCompare, (void *)substring);
						if (result == NULL) {
							//??
						}	
						else {
							
							family->wife = ((Individualp * )result)->data;
							Individual * wife = (Individual *)((Individualp * )result)->data;
							insertBack(&(wife->families), family);
						}
					}
					else if (strcmp(token, "CHIL") == 0)
					{
						void * result = findElement(*individualMap, &customIndividualMapCompare, (void *)substring);
						if (result == NULL) {
							//????
							
						}
						else {
							insertBack(&(family->children), (void*)((Individualp * )result)->data);
							Individual * child = (Individual *)((Individualp * )result)->data;
							insertBack(&(child->families), family);
						}
						
					}
					/*  EVENT HANDLING  */
					else if (strcmp(token, "MARR") == 0)
					{
					
						Event * event = initEvent(); 
						parseEvent(&event, buffer, line, depth, token);
						printEvent((void *)event);
						insertBack(&(family->events), (void *)event);
						
					}
					else {						
						Field * temp = initField();
						temp->tag = (char *)calloc(1, sizeof(char) * (strlen(token) + 1));
						temp->value = (char *)calloc(1, sizeof(char) * (strlen(substring) + 1));
						strcpy(temp->tag, token);
						strcpy(temp->value,substring);
								
						free(token);
						free(substring);
		
						insertBack(&(family)->otherFields, (void *)temp);
					}
					
				}
			}
		}
	}
	error.type = OK; 	
	error.line = -1;
	return error; 
	
}


//Pass the HashMap in here to store the pointer to Individual / xref
GEDCOMerror parseIndividual(Individual * person, FILE ** buffer, int * line, char * refID, GEDCOMobject ** obj)
{
	int depth = 0;
	char c = '\0';
	
	
	GEDCOMerror error;


	while ((c = fgetc(*buffer)) != EOF)
	{
		
		if (c == ' ' || c == '\n' || c == '\r')
		{
			continue;
		}
	
		if (isdigit(c)) 
		{
			depth = CHAR2INT(c); //Save the depth
			*line = *line + 1;	//You know its a new line, up the anti 
			
			if (depth == 0)
			{
				ungetc('0', *buffer); //If you find a zero, handler returns to CreateGEDCOM where it can reparse starting at 0
				GEDCOMerror error;
				error.type = OK; 	
				error.line = -1;
				return error; 
			}				
			if (depth == 1) //IF it's greater than 0, we are still in our object. Keep parsing
			{
				char * token = parseTypeToken(buffer, line); //Parse the first token	
				if (inWhitelistInvidual(token)) //Is it on the whitelist for Individuals?
				{ 
					char * substring = parseEOL(buffer, line); 
					
					peekForConactination(buffer, &substring, depth, line);
					error = addToIndividual(buffer, line, depth, obj, &person,token, substring); 
					
					if (actualError(error)){
						return error;
					} else {
						continue;
						
					}
				
				}
				else {
					//throw gedcom error at this line
					error.type = INV_RECORD; 	
					error.line = *line;
					return error; 
				}
				
			}
			if (depth == 2)
			{
				char * token = parseTypeToken(buffer, line); //Parse the first token	
				if (inWhitelistInvidual(token)) //Is it on the whitelist for Individuals?
				{
					//Do the magic. 
					char * substring = parseEOL(buffer, line); 
					
					peekForConactination(buffer, &substring, depth, line);
					error = addToIndividual(buffer, line, depth, obj, &person,token, substring); 
					
					if (actualError(error)){
						return error;
					} else {
						continue;
						
					}
				
				}
				else {
					//throw gedcom error at this line
					error.type = INV_RECORD; 	
					error.line = *line;
					return error; 
				}
				
				
			}
		}
		else {
			 
			error.type = INV_RECORD; 	
			error.line = *line;
			return error; 
		}
	}
	

	//Actually this SHOULD only run if it encounters EOF. Maybe edit this and others?

	error.type = OK; 	
	error.line = -1;
	return error; 
}


GEDCOMerror parseEvent(Event ** event, FILE ** buffer, int * line, int depth, char * type)
{
	
		//Events have no subtoken, parse to EOL \\ CLEAR
		//Adding the type to the event
		for (int i = 0; i < 5; i++)
		{
			(*event)->type[i] = type[i];
			
		}
		(*event)->type[4] = '\0';
	
		GEDCOMerror error; 
		char c = '\0';
		char depthAsChar = '\0'; 
	
		//This while should only deal with start of line
		while ((c = fgetc(*buffer)) != EOF)
		{
			//keep all data incase we have to put it back in the buffer;
			if (isdigit(c)) {
				
				depthAsChar = c;
				int lineDepth = CHAR2INT(c);
				
				if (lineDepth > depth)
				{
					char * token = parseTypeToken(buffer, line);
				
					
					if (strcmp(token, "DATE") == 0)
					{
						char * substring = parseEOL(buffer, line);
						peekForConactination(buffer, &substring, depth, line);

						(*event)->date = calloc(1, sizeof(char) * (strlen(substring) + 1));
						copyString((*event)->date, substring);
					
						*line = *line +1;
						
						continue;
					}
					else if (strcmp(token, "PLAC") == 0)
					{
						char * substring = parseEOL(buffer, line);
						peekForConactination(buffer, &substring, depth, line);
						
						(*event)->place = calloc(1, sizeof(char) * (strlen(substring) + 1));
						copyString((*event)->place, substring);
						
						*line = *line + 1;
						
						continue;
					}
					else {
						
						//if it's valid for events (CHECK THIS)
						char * substring = parseEOL(buffer, line);
						peekForConactination(buffer, &substring, depth, line);
						
						Field * temp = initField();
						temp->tag = (char *)calloc(1, sizeof(char) * (strlen(token) + 1));
						temp->value = (char *)calloc(1, sizeof(char) * (strlen(substring) + 1));
						strcpy(temp->tag, token);
						strcpy(temp->value,substring);
						
//HERE					
						free(token);
						free(substring);
		
										
						
						insertBack(&(*event)->otherFields, (void *)temp);

						
						*line = *line + 1;
						
					}
				}
				else 
				{
					//printf("can't possibly be a event detail, stopped reading, puttind '%d' back on buffer\n", lineDepth);
					ungetc(depthAsChar, *buffer);
					break;
					
				}
				
			}
			
		}
					
	error.type = OK; 
	error.line = -1; 
	
	return error;
}



GEDCOMerror parseHead(FILE ** buffer, int * line, GEDCOMobject ** obj)
{
	//until depth returns continue with current object. 
	//We are focusing on the ->header
	
	//puts("do we even get here");

	(*obj)->header = initHeader();
	
	(*obj)->header->otherFields = initializeList(&printField, &deleteField, &compareFields);
	
	
	if ((*obj)->header == NULL)
	{
		GEDCOMerror error;
		error.type = OTHER_ERROR; 
		error.line = *line;
		return error; 
	}
	
	//if beginning of line, find the number. 
	

	//bool foundSubmitterRef = false;


	char depth = '\0';
	
		//same line is true until false when new line is detected.
	
	char c = '\0';
	
	//Same line.??
	//Same object.??
	
	char * currentParent = NULL;

    bool foundSubmitterRef = false; 
    bool foundEncodingRef = false; 


    char lastdepth = '\0';
	
	//bug is happening because there is whitespace
	
	//might be a bug where I accidentally use Sameline or other boolean
	while ((c = fgetc(*buffer)) != EOF)
	{		
		/*Finding start of record.
		 * 
		 * Whenever we reach the end of a line, I set depth to '\0'. 
		 * It's safe to assume that under the gedcom standard that 
		 *  the next item is athis integer
		 */
		if (depth == '\0' && isdigit(c)) {
			depth = c;
		}
		//This should be a number. If it isn't, either approaching EOF or error in headers.
		if (depth != '\0')
		{
			//This code only executes if you reach a new line, so its safe to assume it's an acurate
			*line = *line + 1;
			
			if (c == '\n' || c == '\r')
			{
				//sameLine = false;
				//find out if its really the end of the line or not.
				if ((c = fgetc(*buffer)) != '\n' && c != '\r')
				{
					//puts("does this ever happen");
					ungetc(c, *buffer);
					depth = '\0';
					continue;
				}
				else //it wasn't the end of the line. Now it is.
				{
					//printf("does this happen");
					depth = '\0';
					continue;
					
				}	
			}
			else {
				//something funny is happening with my parser clearing \n and \r
				if (depth == '1')
				{

					lastdepth = depth;
					char * token = NULL; 
					token = parseTypeToken(buffer, line);

					if (strcmp(token, "CHAR") == 0)
					{
						foundEncodingRef = true;
					}
					if (strcmp(token, "SUBM") == 0)
					{

						foundSubmitterRef = true;
					}

					/*
					if (strcmp(token, "SUBM") == 0)
					{
						foundSubmitterRef = true;
					}
					*/
				
					if (token == NULL || strcmp(token, "") == 0)
					{
						GEDCOMerror error;
						error.type = INV_HEADER; 	
						error.line = *line;
						return error; 
					}
					else {					
						
						//Ensuring that 2 depth doesn't happen before 1.
		
					
							/* MARK - If it's a valid token for a header, procceed parsing it's attributes 
							 */
							if (inWhitelistHead(token))
							{
								//No you are going to parse rest of the line. 
								
								currentParent = malloc(sizeof(char) * (strlen(token) + 1));
								strcpy(currentParent, token);								
								
								char * string = NULL; 
								string = parseEOL(buffer, line);
								
								
								peekForConactination(buffer, &string, CHAR2INT(depth), line);
								
								

								
								if (string == NULL)
								{
									//puts("tried to unwrap null / attribute to token proved invalid");
										//puts("here? 2");
									GEDCOMerror error;
									error.type = INV_HEADER; 	
									error.line = *line;
									return error; 
								}
								
								GEDCOMerror error = addLineToHeader(obj, currentParent, string);
								
								if (actualError(error))
								{
									return error;
								}
								
								free(token);
								free(string);
								depth = '\0';
							}
							else 
							{
								//free(token);
								//Invalid token.
								GEDCOMerror error;
								error.type = INV_HEADER; 	
								error.line = *line;
								return error; 	
							}
							

						}
								
				}
				//IE. a subtoken
				if (depth == '2')
				{

					if (CHAR2INT(depth) > CHAR2INT(lastdepth) + 1)
					{
							GEDCOMerror error;
							error.type = INV_RECORD; 	
							error.line = *line;
							return error; 	
					}
					else {
						lastdepth = depth;	
					}

				
							char * token = parseTypeToken(buffer, line);
	
							if (inWhitelistHead(token))
							{							
									char * string = NULL; 
									string = parseEOL(buffer, line);
									
									
									if (string == NULL)
									{
											GEDCOMerror error;
												//puts("here? asdasdasdasdasdadadasdas");

											error.type = INV_HEADER; 	
											error.line = *line;
											return error; 
									}
									
									peekForConactination(buffer, &string, CHAR2INT(depth), line);
								
									//printf("tag %s  value %s\n", token, string);
								
								
									//Add the parsed line to the domain of its parent. 
									GEDCOMerror error = addLinetoParentinHeader(obj,token,string,currentParent);
									free(token);
									free(string);
									
									if (actualError(error))
									{
										return error;
									}	
									//IE. Search find next depth when it comes.
									depth = '\0';
						
							}
							else 
							{
									//puts("here? 6");
								free(token);
								//Invalid token.
								GEDCOMerror error;
								error.type = INV_HEADER; 	
								error.line = *line;
								return error; 	
							}

					
				
					/*
					 * MARK- if not does have parent while depth == 2
					 * All this does is protect against a 2 before a 1
				
					else 
					{
							puts("here?  asdasd");
						GEDCOMerror error;
						error.type = INV_HEADER; 
						
						
						error.line = *line;
						return error; 

					}	
						 */		
				}
				else if (depth == '3') {


					if (CHAR2INT(depth) > CHAR2INT(lastdepth) + 1)
					{
							GEDCOMerror error;
							error.type = INV_RECORD; 	
							error.line = *line;
							return error; 	
					}
					else {
						lastdepth = depth;	
					}

				}
				//If you wanted to do depth passed 2, you could just convert it to ints and make it generic in that sense
				//Parent is reliative 
				
				
				//return buffer with 0 to parse next record
				if (depth == '0')
				{
					GEDCOMerror error;
/* Something funky is happening here */

					/*
					if (foundSubmitterRef == false)
					{
						puts("!foundSubmitterRef");
						error.type = INV_HEADER;
						error.type = -1; 
						return error;

					}
					else {

						puts("foundSubmitterRef");
					}

		*/
					if (foundEncodingRef == false)
					{
						ungetc('0', *buffer);
									
							//puts("here? asdasdasda ");


						error.type = INV_HEADER; 
						error.line = -1;
						return error; 

					}
					else if (foundSubmitterRef == false)
					{
						ungetc('0', *buffer);
									
										//puts("here? asdas");	
						error.type = INV_HEADER; 
						error.line = -1;
						return error; 

					}

					ungetc('0', *buffer);
					
					//MARK - No Error, just returning OK. 
					
					error.type = OK; 
					error.line = -1;
					return error; 
				
				}
				
			}	
			
		}
		else {
			continue;
		}
		
	}
	
	//potiential for change. 
	if (currentParent != NULL)
	{	
		free(currentParent);
	}
			
	GEDCOMerror error;
	
    error.type = OK; 
    error.line = -1;
    return error; 
	
}


//maybe pass line number
GEDCOMerror addLineToHeader(GEDCOMobject ** obj, char * token, char * substring)
{
		
	if (strcmp(token, "SOUR") == 0)
	{		
		strcpy((*obj)->header->source, substring);
	}
	else if (strcmp(token, "CHAR") == 0)
	{
		if (strcmp(substring, "ANSEL") == 0)
		{
			(*obj)->header->encoding = ANSEL;
		}
		else if (strcmp(substring, "UTF-8") == 0)
		{
			(*obj)->header->encoding = UTF8;
		}
		else if (strcmp(substring, "UNICODE") == 0)
		{
			(*obj)->header->encoding = UNICODE;
		}
		else if (strcmp(substring, "ASCII") == 0)
		{
			(*obj)->header->encoding = ASCII;
		}
	}
//???
	/*
	else if (strcmp(token, "DATE") == 0)
	{
		//maybe do this?
	}
	else if (strcmp(token, "SUBM") == 0)
	{
		 //malloc for a submitter 
//LATER
	}*/
	else {

		if (strcmp(token, "SUBM") != 0)
		{


			Field * temp = initField();
			temp->tag = (char *)calloc(1, sizeof(char) * (strlen(token) + 1));
			temp->value = (char *)calloc(1, sizeof(char) * (strlen(substring) + 1));
			strcpy(temp->tag, token);
			strcpy(temp->value, substring);
						
			insertBack(&((*obj)->header->otherFields), (void *)temp);

		}
		 
				
	}
	
	GEDCOMerror error;
	error.type = OK; 	
	error.line = -1;
	return error; 		
	//free strings here or after???
}

GEDCOMerror peekForConactination(FILE ** buffer, char ** lastLinevalue, int depth, int * line) 
{

	if (lastLinevalue == NULL)
	{
		GEDCOMerror error;
		error.type = OK; 	
		error.line = -1;
		return error; 
	}
	else if (*lastLinevalue == NULL)
	{
		GEDCOMerror error;
		error.type = OK; 	
		error.line = -1;
		return error; 
	}


		char c = '\0';
		char depthAsChar = '\0';
	
		while ((c = fgetc(*buffer)) != EOF)
		{
			//keep all data incase we have to put it back in the buffer;
			
			if (isdigit(c)) {
				//basically memsetting
				depthAsChar = '\0';
				depthAsChar = c;
				int lineDepth = CHAR2INT(c);
				
				if (lineDepth > depth)
				{
					
					//puts("could be conactination, depth is greater\n");
					
					char * token = NULL; 
					token = parseTypeToken(buffer, line);
					
					//printf("FOUND CONT OR ???: '%s'\n", token);
					
					if (strcmp(token, "CONC") == 0)
					{
						char * substring = NULL;
						substring = parseEOL(buffer, line);
						
						char * newstring = (char *)calloc(1, sizeof(char) * (strlen(*lastLinevalue) + strlen(substring) + 3));
						

						strcat(newstring, *lastLinevalue);
						strcat(newstring, " ");
						strcat(newstring, substring);
						free(*lastLinevalue);
						free(substring);
						*lastLinevalue = newstring;
						//could this be refrencing something else???
					
						*line = *line +1;
						
						continue;
					}
					else if (strcmp(token, "CONT") == 0)
					{
						char * substring = NULL; 
						substring = parseEOL(buffer, line);
						//puts(token);
						//puts(substring);
						char * newstring = (char *)calloc(1, sizeof(char) * (strlen(*lastLinevalue) + strlen(substring) + 3));
						

						strcat(newstring, *lastLinevalue);
						strcat(newstring, "\n");
						strcat(newstring, substring);
								
						free(*lastLinevalue);
						free(substring);
						
						
						*lastLinevalue = newstring;
						*line = *line + 1;
						
						continue;
					}
					else {

						//For safety, but should not happen
						if (token == NULL)
						{
							break;
						}
						
				
						//put the linedepth and token WITH SPACES back on the buffer
						//printf("WAS NOT CONC Or CONT, putting %d %s back on buffer\n", lineDepth, token);
						
						//printf("put '%c' back on buffer\n", depthAsChar);
						//printf("put ' ' back on buffer\n");
						
						fseek(*buffer, -(strlen(token) + 3), SEEK_CUR);
						/**ungetc(' ', *buffer);
							
						for (int i = strlen(token); i >= 0; i--)
						{
							if (token[i] != '\0')
							{
								ungetc(token[i], *buffer); // this line
								
							}
						}
						
						ungetc(' ', *buffer); // this line
*/
						if (!isdigit(depthAsChar))
						/**{
							ungetc(depthAsChar, *buffer); // this line
						}
						else*/ {
						
							free(token);

							GEDCOMerror error;
							error.type = INV_RECORD; 	
							error.line = *line;
							return error;
						}
						
						free(token);
						
						break;
					}
					
				}
				else {
					//printf("can't possibly be a conc, stopped reading, puttind '%d' back on buffer\n", lineDepth);
					ungetc(depthAsChar, *buffer);					
					
					break;
					
				}
				
			}
			
		}	
	
	GEDCOMerror error;
	error.type = OK; 	
	error.line = -1;
	return error; 
	
}

GEDCOMerror addLinetoParentinHeader(GEDCOMobject ** obj, char * token, char * substring, char * parent)
{
	if (strcmp(parent, "SOUR") == 0)
	{
		if (strcmp(token, "NAME") == 0)
		{
			Field * temp = initField(); 
			
			temp->tag = (char *)calloc(1, sizeof(char) * (strlen(token) + 1));
			strcpy(temp->tag, token);
			
			temp->value = (char *)calloc(1, sizeof(char) * (strlen(substring) + 1));
			strcpy(temp->value, substring);
			
			insertBack(&(*obj)->header->otherFields, temp);
		}
		if (strcmp(token, "VERS") == 0)
		{
			Field * temp = initField(); 
			
			temp->tag = (char *)calloc(1, sizeof(char) * (strlen(token) + 1));
			strcpy(temp->tag, token);
			
			temp->value = (char *)calloc(1, sizeof(char) * (strlen(substring) + 1));
			strcpy(temp->value, substring);
			
			insertBack(&(*obj)->header->otherFields, temp);
		}
		
	}	
	else if (strcmp(parent, "GEDC") == 0)
	{
		if (strcmp(token, "VERS") == 0)
		{
			(*obj)->header->gedcVersion = parseVersionNumber(substring);
		}
		else if (strcmp(token, "FORM") == 0)
		{
			Field * temp = initField(); 
			temp->tag = (char *)calloc(1, sizeof(char) * (strlen(token) + 1));
			strcpy(temp->tag, token);
			
			temp->value = (char *)calloc(1, sizeof(char) * (strlen(substring) + 1));
			strcpy(temp->value, substring);
			insertBack(&(*obj)->header->otherFields, temp);
		}
	}
	else {


		Field * temp = initField(); 
		
		temp->tag = (char *)calloc(1, sizeof(char) * (strlen(token) + 1));
		strcpy(temp->tag, token);
			
		temp->value = (char *)calloc(1, sizeof(char) * (strlen(substring) + 1));
		strcpy(temp->value, substring);
			
		insertBack(&(*obj)->header->otherFields, temp);
	}
	
	GEDCOMerror error;
	error.type = OK; 	
	error.line = -1;
	return error; 
	
}

GEDCOMerror isHeadValid(GEDCOMobject ** obj, int line)
{
	//Check to see if the head meets its minumum requirements.
	GEDCOMerror error;
	
	if ((*obj)->header == NULL) {
		error.type = INV_HEADER; 
		error.line = -1;
		return error; 
	}

	//Not null head
	else {
		if ((*obj)->header->source[0] == '\0')
		{
			error.type = INV_HEADER; 
			error.line = line;
			return error; 
		}
		if ((*obj)->header->gedcVersion == 0)
		{
			error.type = INV_HEADER; 
			error.line = line;
			return error; 
		}
   }
   
	error.type = OK; 
	error.line = -1;
	return error; 
   
}


char * parseEOL(FILE ** buffer, int * line)
{
	char attribute[256];
	memset(attribute, '\0', 255);

	bool ignore = true;
	char * temp = NULL;
	int index = 0;
	char c = '\0';

	while ((c = fgetc(*buffer)) != EOF)
	{	
		if(c == ' ' && ignore == true)
		{
			continue;
		}
		//string starts
		else {
			//never eat whitespace again.
			ignore = false;
			
			
			//build the string
			if (c != '\n' && c != '\r' && c != EOF) 
			{
				//building the string.
				attribute[index] = c; 
				index++;
				continue;
			}
			else {
				//check for another '\n' or EOR 
				
				//end the string. 
				attribute[index] = '\0';
				
				//pull end line extra off buffer
				if ((c = fgetc(*buffer)) != '\n' || c != '\r' || c != EOF)
				{
					ungetc(c, *buffer);
				}

				temp = (char *)calloc(1, sizeof(char) * (strlen(attribute) + 1)); 
				strcpy(temp, attribute);
				return temp;
			}
			
		}		
	}
	attribute[index] = '\0';
	ungetc(EOF, *buffer);
	temp = (char *)calloc(1, sizeof(char) * (strlen(attribute) + 1)); 
	strcpy(temp, attribute);
	return temp;
}



//Parses the 0 lines first token so we can tell how to future parse. Should only be used when you know what you want. Not generic
//Use cases include finding HEAD, SOUR, @ID@ 

//Should return NULL if there is nothing else left on the line.
char * parseTypeToken(FILE ** buffer, int * line)
{
	//Tokens should only be 4 characers 
	
	char * temp = NULL;
	char * token = (char *)calloc(1, sizeof(char) * 256);
	//memset(token, '\0', 255);
	int index = 0;
	
	
	char c = '\0'; 
	int ignore = 1;	

     
	 //Will grab a token from '     TOKEN\n' or 'TOKEN' or ' TOKEN EOF/\n/\r'
	 while ((c = fgetc(*buffer)) != EOF)
	 {		
		 
		  if(c == ' ')
		  {
			if (ignore == 1)
			{
				continue; 	
			}
			else {
				//end the string. 
				token[index] = '\0';
				ungetc(c, *buffer);

				//printf("found token '%s', len(%d)\n", token, (int)strlen(token));
				temp = malloc(sizeof(char) * (strlen(token) + 1)); 
				strcpy(temp, token);
				//printf("temp = '%s'", temp);
				free(token);
				
				return temp;
			}
	
		  }		
		  else if (c == '\n' || c == '\r' || c == EOF)
		  {
			  //if the end of line is the end of the string. parse here.  
			  if (ignore == 0)
			  {
				  
				  if (index == 0)
				  {
					  return NULL;
				  }
				  
					//*line = *line + 1;
					//printf("line++ = %d\n", *line); 
					ungetc(c, *buffer);
					
					token[index] = '\0';
					//printf("found token '%s', len(%d)\n", token, (int)strlen(token));
					
					//printf("temp = '%s'", temp);

					//something fucky is happening with null termiators so I give an extra character
					temp = calloc(1, sizeof(char) * (strlen(token) + 2)); 
					strcpy(temp, token);
					//printf("temp = '%s'", temp);
					free(token);
					
					return temp;
				  
			  }
			  
			  //Maybe check for one after as well. If not put back on stack.
			 
		  }
		  else
		  {
			  //this means the before whitespace has been removed. You can start parsing the word now. Therefore next time you see ' ', end the token and return it 
			  //printf("%c", c);
			  
			  token[index] = c;
			  index++; 
			  
			  ignore = 0;
		  }
		  
		 
	 }

	ungetc(EOF, *buffer);
	token[index] = '\0';
	//printf("found token '%s', len(%d)\n", token, (int)strlen(token));
	
	//printf("temp = '%s'", temp);

	//something fucky is happening with null termiators so I give an extra character
	temp = calloc(1, sizeof(char) * (strlen(token) + 2)); 
	strcpy(temp, token);
	//printf("temp = '%s'", temp);
	free(token);
	
	return temp;
				  
}


/*
 * LINKED LIST API HELPERS BELOW: 
 * 
 * 
 * THEY LOOK BAD SO I KEEP THEM DOWN HERE.
 * 
 * 
 * */
//<EVENT HELPERS>
void deleteEvent(void* toBeDeleted)
{
	if (toBeDeleted == NULL) 
	{
		//puts("Gedparser. THIS SHOULD NEVER HAPPEN OR ELSE YOU WILL BE FREEING NULL LATER GOD IS DEAD");
		return;
	}
	else 
	{
		Event * temp = (Event *)toBeDeleted;
		
		if (temp->date != NULL)
		{
			free(temp->date);
		}
		if (temp->place != NULL)
		{
			free(temp->place);
		}
	}	
}


int compareEvents(const void* first,const void* second)
{
	//1. derefrence the dates
	//2. If they both have dates
		//a. parse them out with sscanf
	//3.compare results 
	Event * event1 = (Event *)first;
	Event * event2 = (Event *)second; 
	
	if (event1 == NULL || event2 == NULL)
	{
		return -1; 
	}
	
	if (event1->date == NULL || event2->date == NULL)
	{
		
		if (event1->place != NULL || event2->place != NULL)
		{
			
			return strcmp(event1->place, event2->place);
		}
		//There is a serious issue everything in 1 of them is null
		else {
			return -100;
		}
	}
	
	Date * temp1 = initDate(); 
	Date * temp2 = initDate();
	
    parseDate(&temp1, event1->date);
    
    parseDate(&temp2, event2->date);
    
    //-1 if first ealier, 0 if same, 1 if first later
    int result = compareDate(temp1, temp2);

    free(temp1);
    free(temp2);
    
	//dummy for now.
	return result;
}

void parseFullName(Individual * person, char * token)
{
	//Take the name, parse it and shove it into person. 
	
	int length = strlen(token);
	
	char firstname[256]; 
	memset(firstname, '\0', 256);
	char lastname[256]; 
	memset(lastname, '\0', 256);
	bool stopReading = false;
	int i = 0;
	
	while (i < length)
	{
		if (token[i] == EOF || token[i] == '\n'|| token[i] == '\r')
		{
			stopReading = true;
			break;
		}
		if (token[i] != '/')
		{
			firstname[i] = token[i];
			i++;
			continue;
		}
		break;
	}
	firstname[i] = '\0';
	
	if (token[i] == '/')
	{
		i++;
	}
	//remove whitespace from end of string
	
	for (int j = strlen(firstname); j >= 0; j--)
	{
	
		if (firstname[j] != ' ' && firstname[j] != '\0')
		{
			break;
		}
		else {
			firstname[j] = '\0';
		}
	}

	int index = 0;
	if (stopReading == false)
	{
		//I starts at the character after /
		while (i < length)
		{
			if (token[i] == EOF || token[i] == '\n'|| token[i] == '\r')
			{
				stopReading = true;
				break;
			}
			if (token[i] != '/')
			{
				lastname[index] = token[i];
				i++;
				index++;
				continue;
			}
			break;
		}
	}
	else {
		//puts("missing lastname");
	}
	
	lastname[i] = '\0'; 
	
	//clean string 
	
	if (person->givenName == NULL)
	{
		person->givenName = calloc(1, sizeof(char) * (strlen(firstname) + 1));
		copyString(person->givenName, firstname);
	}
	if (person->surname == NULL)
	{
		person->surname = calloc(1, sizeof(char) * (strlen(lastname) + 1));
		copyString(person->surname, lastname);
	}

	return;
}

char * printGEDCOM(const GEDCOMobject *  obj) 
{
	if (obj == NULL)
	{
		return NULL;
	}


	char * final = calloc(1, sizeof(char) * 512);
	char * temp = calloc(1, sizeof(char) * 512);
	
	//string = concatStrings(filename, testing);

	sprintf(temp, "\n------HEADER------\n");
	final = concatStrings(final, temp);
	temp = calloc(1, sizeof(char) * 512);
	
	sprintf(temp, "Source = %s\n", obj->header->source);
	final = concatStrings(final, temp);
	temp = calloc(1, sizeof(char) * 512);
	
	printCharSet(obj->header->encoding);
	sprintf(temp, "Version = %.2lf\n", obj->header->gedcVersion);
	final = concatStrings(final, temp);
	temp = calloc(1, sizeof(char) * 512);
	

	
	ListIterator iterator = createIterator(obj->header->otherFields);
	void * elem;
	
	while ((elem = nextElement(&iterator)) != NULL) 
	{
		char * result = printField(elem); 
		sprintf(temp, "%s", result);
		free(result);
		final = concatStrings(final, temp);
		temp = calloc(1, sizeof(char) * 512);
	}
	
	
	sprintf(temp, "-----INDIVDUALS------\n");
	final = concatStrings(final, temp);
	temp = calloc(1, sizeof(char) * 512);
	
	
	ListIterator listIndivudals = createIterator(obj->individuals);
	void * person;
	
	while ((person = nextElement(&listIndivudals)) != NULL) 
	{

		/*printf("ORIG: %s\n", printIndividual(person));
		Individual * clone = deepCopyIndividual((Individual *)person);
		printf("COPY: %s\n", printIndividual((void *)clone));

*/

		char * printedPerson = printIndividual(person);
		sprintf(temp, "[NEW]%s", printedPerson);
		free(printedPerson);
		final = concatStrings(final, temp);
		temp = calloc(1, sizeof(char) * 512);
		
		ListIterator personOtherFields = createIterator(((Individual *)person)->otherFields);



		//ListIterator personOtherFields = createIterator(clone->otherFields);
		
		void * other;
	
		while ((other = nextElement(&personOtherFields)) != NULL) 
		{
			char * printedOtherFieldPerson = printField(other);
			sprintf(temp, "	->%s",printedOtherFieldPerson);
			free(printedOtherFieldPerson);
			final = concatStrings(final, temp);
			temp = calloc(1, sizeof(char) * 512);
		}
		
		ListIterator events = createIterator(((Individual *)person)->events);
		void * event;
	
		while ((event = nextElement(&events)) != NULL) 
		{
			//Event * sos = deepCopyEvent((Event *)event);


			char * printedPersonEvents = printEvent(event);
			sprintf(temp,"	->%s", printedPersonEvents);
			free(printedPersonEvents);
			final = concatStrings(final, temp);
			temp = calloc(1, sizeof(char) * 512);
			

			ListIterator personEventOtherField = createIterator(((Event *) event)->otherFields);
			void * otherEvent;
	
			while ((otherEvent = nextElement(&personEventOtherField)) != NULL) 
			{
				//Field * sos = deepCopyField((Field *) otherEvent);

				char * printedPersonEventOtherField = printField(otherEvent);
				sprintf(temp, "			->%s",printedPersonEventOtherField);
				free(printedPersonEventOtherField);
				final = concatStrings(final, temp);
				temp = calloc(1, sizeof(char) * 512);
			}
		}
						
		sprintf(temp, "\n	[Current Individuals Direct Relations, as Spouce or Child]\n\n");
		final = concatStrings(final, temp);
		temp = calloc(1, sizeof(char) * 2000);

		ListIterator theirFamilies = createIterator(((Individual *)person)->families);

		void * afamily;
		while ((afamily = nextElement(&theirFamilies)) != NULL)
		{
			//puts("Breaks here");
			char * data = printFamily(afamily);
			
			if (data != NULL)
			{
				sprintf(temp, "	FAMILIES: { \n%s	}\n\n", data);
				//exit(0);
				final = concatStrings(final, temp);
				temp = calloc(1, sizeof(char) * 2000);
				
			}
			free(data);
			
			//Look at the print datas too amke sure they can't print null
		}
		
	}
	
	
	

	
	/*
	ListIterator familyIterator = createIterator(obj->families);
	void * family;
	
	
	while ((family = nextElement(&familyIterator)) != NULL) 
	{
		printFamily(family); //RETURNS NULL RN 
	}
	*/
		
	sprintf(temp, "------SUBMITTER------\n");
	final = concatStrings(final, temp);
	temp = calloc(1, sizeof(char) * 256);
	
	
	sprintf(temp, "%s",printSubmitter(obj->submitter));
	final = concatStrings(final, temp);
	temp = calloc(1, sizeof(char) * 2000);
	
	
	//Comments can be big
	ListIterator SubmitterOtherField = createIterator(obj->submitter->otherFields);
	void * otherSub;

	while ((otherSub = nextElement(&SubmitterOtherField)) != NULL) 
	{
		sprintf(temp,"	->%s", printField(otherSub));
		final = concatStrings(final, temp);
		temp = calloc(1, sizeof(char) * 2000);
	
	}
	
	sprintf(temp, "------/TRLR/------\n");
	final = concatStrings(final, temp);

	//No need callocing temp when we are doing saving info
	
	return final; 
}

char * printSubmitter(Submitter * toBePrinted)
{
	if (toBePrinted == NULL)
	{
		return NULL;
	}	
	
	size_t needed = snprintf(NULL, 0, "Submitter: NAME:'%s' ADDRESS:'%s'\n", toBePrinted->submitterName, toBePrinted->address) + 1;
	char * buffer = (char*)malloc(needed);
	snprintf(buffer, needed,"Submitter: NAME:'%s' ADDRESS:'%s'\n", toBePrinted->submitterName, toBePrinted->address);
	
	return buffer;
	
}


char* printEvent(void* toBePrinted)
{
	if (toBePrinted == NULL)
	{
		return NULL;
	}	
	
	Event * temp; 
	temp = (Event*)toBePrinted; 
	
	
	size_t needed = snprintf(NULL, 0, "Event: type:'%s' date:'%s' place:'%s'\n", (temp->type != NULL) ? temp->type : "", (temp->date != NULL) ? temp->date : "", (temp->place != NULL) ? temp->place : "") + 1;
	char * buffer = (char*)malloc(needed);
	snprintf(buffer, needed,"Event: type:'%s' date:'%s' place:'%s'\n", (temp->type != NULL) ? temp->type : "", (temp->date != NULL) ? temp->date : "", (temp->place != NULL) ? temp->place : "");
	
	return buffer;
}


//Need to implement
void deleteIndividual(void* toBeDeleted) {
	Individual * temp = (Individual *) toBeDeleted;
	free(temp->givenName);
	free(temp->surname);
//In all deletes you will have to run their lists to be deleted
}

//removes whitespace from a string, returns a new temp one, remember to free it later
int compareIndividuals(const void* first,const void* second)
{
	Individual * temp1 = (Individual *) first;
	Individual * temp2 = (Individual *) second;
	//printf("FOUND1: '%s','%s'\n",temp1->surname, temp1->givenName);
	//printf("FOUND1: '%s','%s'\n",temp2->surname, temp2->givenName);
	





	char * temp1surname = cleanup(temp1->surname);
	char * temp1givenName = cleanup(temp1->givenName);
	
	char * temp2surname = cleanup(temp2->surname);
	char * temp2givenName = cleanup(temp2->givenName);

	if (strcmp(temp1surname, "") == 0 && strcmp(temp2surname, "") != 0)
	{
		return 1; 
	}
	else if (strcmp(temp1surname, "") != 0 && strcmp(temp2surname, "") == 0)
	{
		return -1;
	}
	
	char temp1Name[200]; 
	memset(temp1Name, '\0', 199);

	
	strcat(temp1Name,temp1surname);
	strcat(temp1Name,",");
	strcat(temp1Name,temp1givenName);
	
	//printf("CI: Temp1 = '%s'\n", temp1Name);
	
	char temp2Name[200]; 
	memset(temp2Name, '\0', 199);
	
	strcat(temp2Name,temp2surname);
	strcat(temp2Name,",");
	strcat(temp2Name,temp2givenName);
	
	//printf("CI: Temp2 = '%s'\n", temp2Name);
	//printf("compareIndividuals: '%s' and '%s'\n ", temp1Name, temp2Name);
	
	return strcmp(temp1Name, temp2Name);
}

char* printIndividual(void* toBePrinted)
{
	if (toBePrinted == NULL)
	{
		//puts("is this empty?");
		return NULL;
	}	
	
	Individual * temp; 
	temp = (Individual*)toBePrinted; 
	
	size_t needed = snprintf(NULL, 0, "Individual: surname:'%s' given:'%s'\n", temp->surname, temp->givenName) + 1;
	char * buffer = (char*)malloc(needed);
	snprintf(buffer, needed,"Individual: surname:'%s' given:'%s'\n", temp->surname, temp->givenName);
	
	return buffer;
}

void deleteFamily(void* toBeDeleted)
{
	Family * family = (Family *)toBeDeleted;
	Individual * husband = family->husband;
	Individual * wife = family->wife;
	free(husband);
	free(wife);
	
	return;
}

//compares by the number of family members. It returns -1 if first argument has fewer family members than second, 0 the number is the same, and 1 if first family has mome members than second.
int compareFamilies(const void* first,const void* second)
{
	int firstFamilyMembers = 0; 
	int secondFamilyMembers = 0;
	
	Family * firstFamily = (Family *)first;
	Family * secondFamily = (Family *)second;
	
	//First Family Count
	if (firstFamily->husband != NULL) {
		firstFamilyMembers++;
	}
	if (firstFamily->wife != NULL) {
		firstFamilyMembers++;
	}
	
	ListIterator childIterator1 = createIterator(firstFamily->children);
	
	void * child1;
	while ((child1 = nextElement(&childIterator1)) != NULL)
	{
		firstFamilyMembers++;
	}
	
	//Second Family Count 
	
	if (secondFamily->husband != NULL) {
		secondFamilyMembers++;
	}
	if (secondFamily->wife != NULL) {
		secondFamilyMembers++;
	}
	
	ListIterator childIterator2 = createIterator(secondFamily->children);
	
	void * child2;
	while ((child2 = nextElement(&childIterator2)) != NULL)
	{
		secondFamilyMembers++;
	}
	
	if (firstFamilyMembers < secondFamilyMembers) 
	{
		return -1;
	}
	else if (firstFamilyMembers == secondFamilyMembers) 
	{
		return 0;
	}
	else {
		
		return 1;
	}
}

//currently returning null rather than the object;
char* printFamily(void* toBePrinted)
{
	char * final = calloc(1, sizeof(char) * 512);
	char * temp = calloc(1, sizeof(char) * 512);
	
	Family * family = (Family *)toBePrinted; 
	
	Individual * husband;
	Individual * wife;  
	
	//printf("Printing Family: \n");
	if (family->husband != NULL)
	{
		husband = family->husband; 
		char * print = printIndividual((void*)husband);
		sprintf(temp, "	Husband: %s\n", print);
		
		free(print);
		final = concatStrings(final, temp);
		temp = calloc(1, sizeof(char) * 512);

	}
	if (family->wife != NULL)
	{
		wife = family->wife; 
		char * print = printIndividual((void*)wife);
		//puts(print);
		sprintf(temp, "	Wife: %s\n", print);
		
		free(print);
		final = concatStrings(final, temp);
		temp = calloc(1, sizeof(char) * 512);
		
	}
	
	ListIterator childIterator = createIterator(family->children);
	
	void * child;
	while ((child = nextElement(&childIterator)) != NULL)
	{
		char * print = printIndividual((void*)child);
		sprintf(temp,"	Children: %s\n", print);
		free(print);
		final = concatStrings(final, temp);
		temp = calloc(1, sizeof(char) * 512);
	}
	
	
	
	ListIterator eventIterator = createIterator(family->events);
	
	void * event;
	while ((event = nextElement(&eventIterator)) != NULL)
	{	
		char * print = printEvent((void*)event);
		sprintf(temp,"		Event: %s\n", print);
		free(print);
		final = concatStrings(final, temp);
		temp = calloc(1, sizeof(char) * 512);
		
	}
	
	
	ListIterator otherIterator = createIterator(family->otherFields);
	
	void * other;
	while ((other = nextElement(&otherIterator)) != NULL)
	{
		char * print = printField((void*)other);
		sprintf(temp,"		Other: %s\n", print);
		free(print);
		final = concatStrings(final, temp);
		temp = calloc(1, sizeof(char) * 512);
	}
	return final;
}

//<FIELD HELPERS>
void deleteField(void* toBeDeleted)
{
	if (toBeDeleted == NULL) 
	{
		return;
	}
	else 
	{
		Field * temp = (Field *)toBeDeleted;
		
		if (temp->tag != NULL)
		{
			free(temp->tag);
		}
		if (temp->value != NULL)
		{
			free(temp->value);
		}
	}
	
}

//Not tested 
int compareFields(const void* first,const void* second)
{
	Field * item1 = (Field*)first;
	char * item1Tag = item1->tag; 
	char * item1Value = item1->value;
	
	Field * item2 = (Field*)second;
	char * item2Tag = item2->tag; 
	char * item2Value = item2->value;

	if (strcmp(item1Tag, item2Tag) == 0)
	{
		if (strcmp(item1Value, item2Value) == 0)
		{
			return 0;
		}
		else {
			return strcmp(item1Value, item2Value);
		}		
	}
	else {
		return strcmp(item1Tag, item2Tag);
	}
	
}
char* printField(void* toBePrinted)
{

	if (toBePrinted == NULL)
	{
		return NULL;
	}	
	
	Field * temp; 
	temp = (Field*)toBePrinted; 
	
	size_t needed = snprintf(NULL, 0, "Field: tag:'%s' value:'%s'\n", temp->tag, temp->value) + 1;
	char * buffer = (char*)malloc(needed);
	snprintf(buffer, needed,"Field: tag:'%s' value:'%s'\n", temp->tag, temp->value);
	
	return buffer;
	
}

/* built this from example on how to dynamically save formatted text with 
 * sprintf from this stack overflow post
 * 
 * SOURCE: https://stackoverflow.com/questions/3774417/sprintf-with-automatic-memory-allocation
 */
char * printError(GEDCOMerror err)
{
	char * buffer = NULL;
	size_t needed;
	
	switch(err.type)
	{
		case OK:
			needed = snprintf(NULL, 0, "\n%s[OK]: Invoked Library executed with no errors at (line: %d)%s\n",GREEN, err.line, RESET) + 1; 
			buffer = (char*)malloc(needed);
			snprintf(buffer, needed, "\n%s[OK]: Invoked Library executed with no errors at (line: %d)%s\n",GREEN, err.line, RESET); 
			break;
		case INV_FILE: 
			needed = snprintf(NULL, 0, "\n%s[INV_FILE]: Invalid File, either not found or error at (line: %d)%s\n",RED, err.line, RESET) + 1;
			buffer = (char*)malloc(needed);
			snprintf(buffer, needed,"\n%s[INV_FILE]: Invalid File, either not found or error at (line: %d)%s\n", RED, err.line, RESET);
			break;
		case INV_GEDCOM:
			needed = snprintf(NULL, 0, "\n%s[INV_GEDCOM]: Error in the GEDCOM file at (line: %d)%s\n",RED, err.line, RESET) + 1;
			buffer = (char*)malloc(needed);
			snprintf(buffer, needed,"\n%s[INV_GEDCOM]: Error in the GEDCOM file at (line: %d)%s\n",RED, err.line, RESET);
			break;
		case INV_HEADER:
			needed = snprintf(NULL, 0, "\n%s[INV_HEADER]: The Header to the GEDCOM is invalid at (line: %d)\nIf the error line equals the end of a header, it could also mean your header is missing REQUIRED feilds to be VALID.%s\n", RED, err.line, RESET) + 1;
			buffer = (char*)malloc(needed);
			snprintf(buffer, needed,"\n%s[INV_HEADER]: The Header to the GEDCOM is invalid at (line: %d)\nIf the error line equals the end of a header, it could also mean your header is missing REQUIRED feilds to be VALID.%s\n", RED, err.line, RESET);
			break;
		case INV_RECORD:
			needed = snprintf(NULL, 0, "\n%s[INV_RECORD]: A record in this GEDCOM is invalid at (line: %d)%s\n",RED, err.line, RESET) + 1;
			buffer = (char*)malloc(needed);
			snprintf(buffer,needed, "\n%s[INV_RECORD]: A record in this GEDCOM is invalid at (line: %d)%s\n", RED, err.line, RESET);
			break;
		case OTHER_ERROR:
			needed = snprintf(NULL, 0, "\n%s[OTHER_ERROR]: an 'OTHER'. IE. 'undefined' error occured at (line: %d)%s\n", RED, err.line, RESET) + 1;
			buffer = (char*)malloc(needed);
			snprintf(buffer, needed,"\n%s[OTHER_ERROR]: an 'OTHER'. IE. 'undefined' error occured at (line: %d)%s\n", RED, err.line, RESET);
			break;
		case WRITE_ERROR:
			needed = snprintf(NULL, 0, "\n%s[WRITE_ERROR]: (line: %d)%s\n", RED, err.line, RESET) + 1;
			buffer = (char*)malloc(needed);
			snprintf(buffer, needed,"\n%s[WRITE_ERROR]: (line: %d)%s\n", RED, err.line, RESET);
			break;
		/*case WRITE_ERROR:
			needed = snprintf(NULL, 0, "\n[WRITE_ERROR]: During the writing to a new .GED, an error occured (line: %d)\n", err.line) + 1;
			buffer = (char*)malloc(needed);
			snprintf(buffer, needed,"\n[WRITE_ERROR]: During the writing to a new .GED, an error occured (line: %d)\n", err.line);
			break;*/
		default:
			printf("\n[???]: PrintError Defaulted, impossible, god is dead?\n");
			break;
	}	
	
	return buffer;
	
}





