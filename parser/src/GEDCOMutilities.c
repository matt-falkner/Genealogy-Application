#include "GEDCOMparser.h"
#include "GEDCOMutilities.h"

char * helloWorld(void)
{
	char * string = malloc(sizeof(char) * 256); 
	strcpy(string, "Hello World");

	return string;
}


//making this work 
Individual* findMatchInRecord(const GEDCOMobject* familyRecord, int (*compare)(const void* first, const void* second), const void* person)
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
		if (compareIndividuals(elem, person) == 0)
		{
			result = (Individual *)elem;
		}
	}
	
	return result;
}


char * getAncestorsForPersonInFile(char * filename, char * individualJson, int number) 
{

    printf("number = %d\n\n", number);

	if (filename == NULL || individualJson == NULL)
	{
		return "Error: Null input";
	}

	FILE * testBuffer = fopen(filename, "r");
	if (testBuffer == NULL)
	{
		fclose(testBuffer);
		return "Error: Could not open file.";
	}
	else {
		fclose(testBuffer);
	}

	Individual * person = JSONtoInd(individualJson);
	printf("searching desc of %s", printIndividual((void *) person));

	GEDCOMobject * object = NULL; 
	createGEDCOM(filename, &object);


	Individual * copyInRecord = findMatchInRecord(object, object->individuals.compare, person);

	//find duplicate pointer inside of this gedcom obejct 

	List Ancestors = getAncestorListN(object, copyInRecord, number);

	char * ancsListJson = gListToJSON(Ancestors);

	return ancsListJson;



}

 
char * getDescendantsForPersonInFile(char * filename, char * individualJson, int number) 
{
	printf("number = %d\n\n", number);

	if (filename == NULL || individualJson == NULL)
	{
		return "Error: Null input";
	}

	FILE * testBuffer = fopen(filename, "r");
	if (testBuffer == NULL)
	{
		fclose(testBuffer);
		return "Error: Could not open file.";
	}
	else {
		fclose(testBuffer);
	}

	Individual * person = JSONtoInd(individualJson);
	printf("searching desc of %s", printIndividual((void *) person));

	GEDCOMobject * object = NULL; 
	createGEDCOM(filename, &object);


	Individual * copyInRecord = findMatchInRecord(object, object->individuals.compare, person);

	//find duplicate pointer inside of this gedcom obejct 

	List Descendants = getDescendantListN(object, copyInRecord, number);

	char * descListJson = gListToJSON(Descendants);

	return descListJson;
}


char * GEDCOMtoJSON(GEDCOMobject * obj)
{
	char * buffer = NULL;
	size_t needed; 
	
	needed = snprintf(NULL, 0, "{\"encoding\":\"%s\",\"gedcVersion\":\"%.2lf\",\"submitterName\":\"%s\",\"submitterAddress\":\"%s\",\"numOfINDI\":\"%d\",\"numOfFamily\":\"%d\",\"source\":\"%s\"}", printCharSetJSON(obj->header->encoding), obj->header->gedcVersion, obj->submitter->submitterName, obj->submitter->address, getLength(obj->individuals), getLength(obj->families), obj->header->source) + 1; 
	buffer = (char*)malloc(needed);
	snprintf(buffer, needed,  "{\"encoding\":\"%s\",\"gedcVersion\":\"%.2lf\",\"submitterName\":\"%s\",\"submitterAddress\":\"%s\",\"numOfINDI\":\"%d\",\"numOfFamily\":\"%d\",\"source\":\"%s\"}", printCharSetJSON(obj->header->encoding), obj->header->gedcVersion,obj->submitter->submitterName, obj->submitter->address,getLength(obj->individuals), getLength(obj->families), obj->header->source);
	return buffer;
}	


char * findSex(const Individual * ind)
{

	char * string = "U";
	char * malloc_copy = malloc(sizeof(char) * (strlen(string) + 1));
	strcpy(malloc_copy, string);

	ListIterator personOtherFields = createIterator(ind->otherFields);

	void * other;

	while ((other = nextElement(&personOtherFields)) != NULL) 
	{
		Field * field = (Field * )other;

		if (field->tag != NULL)
		{

			if (strcmp(field->tag, "SEX") == 0)
			{

				if (field->value != NULL)
				{
					if (strcmp(field->value, "") == 0)
					{
						return malloc_copy;
					}

					return field->value;

				}

			}
		}

	}

	return malloc_copy;

}



char* indToJSONExtra(const Individual* ind)
{
	/*char * temp = calloc(1, sizeof(char) * 512);
	sprintf(temp, "{\"givenName\":\"%s\",\"surname\":\"%s\"}", ind->givenName, ind->surname);
	return temp;
		*/

	//printf("Found sex %s for %s\n", findSex(ind), ind->givenName);

	char * buffer = NULL;
	size_t needed; 
	
	needed = snprintf(NULL, 0, "{\"givenName\":\"%s\",\"surname\":\"%s\",\"sex\":\"%s\",\"numOfFamily\":\"%d\"}", ind->givenName, ind->surname, findSex(ind), getLength(ind->families)) + 1; 
	buffer = (char*)malloc(needed);
		
	if (buffer == NULL)
	{
		return NULL;
	}
	
	snprintf(buffer, needed, "{\"givenName\":\"%s\",\"surname\":\"%s\",\"sex\":\"%s\",\"numOfFamily\":\"%d\"}", ind->givenName, ind->surname, findSex(ind), getLength(ind->families));
	
	return buffer;
	
}


char * indvsToJSON(GEDCOMobject * obj)
{
	//char * item = indvToJSON(indv)

	char * temp = calloc(1, sizeof(char) * 5000);
	char * final = calloc(1, sizeof(char) * 5000);

	if (getLength(obj->individuals) == 0)
	{
		sprintf(temp, "[]");
		return temp;
	}
	
	sprintf(temp, "[");
	final = concatStrings(final, temp);
	temp = calloc(1, sizeof(char) * 5000);

	ListIterator indvIterator = createIterator(obj->individuals);
	void * person;

	int length = getLength(obj->individuals);
	int counter = 1;

	while ((person  = nextElement(&indvIterator)) != NULL) 
	{

		char * result = indToJSONExtra((Individual *)person);
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





char * getIndividualsInFile(char * filename)
{
	FILE * file = fopen(filename, "r");

	if (file == NULL)
	{
		fclose(file);
		return "File could not be found";
	}
	else {
		fclose(file);
	 	GEDCOMobject * obj = NULL;
	 	createGEDCOM(filename, &obj);

	 	if (obj == NULL)
	 	{
	 		return "GEDCOMobject is NULL";
	 	}

	 	char * json = indvsToJSON(obj);
	 	return json;
	}

}



char * getFileDetails(char * filename)
{
	FILE * file = fopen(filename, "r");
	fclose(file);

	if (file == NULL)
	{
		return "CANNOT FIND FILE";
	}
	else
	{
		GEDCOMobject * obj = NULL;
		createGEDCOM(filename, &obj);

		ErrorCode err = validateGEDCOM(obj);

		if (err != OK)
		{
			return "ERROR IN GEDCOM FILE, NOT VALID";
		}
		else {

			char * json = GEDCOMtoJSON(obj);
			return json;
		}
	}
}

char * writeGEDCOMfromJSON(char * filename, char * json)
{
	GEDCOMobject * obj = JSONtoGEDCOM(json);
	printf("%s", printGEDCOM(obj));

	printf("filename '%s'", filename);

	GEDCOMerror error = writeGEDCOM(filename, obj);
	printf("%s", printError(error));

	return "Success";
}



List * findChildren(const Individual * person) 
{

	ListIterator theirFamilies = createIterator((person)->families);
	List * theirChildren = NULL; 
	void * afamily;

	while ((afamily = nextElement(&theirFamilies)) != NULL)
	{
		if (person == ((Family *)afamily)->husband)
		{
			theirChildren = &(((Family *)afamily)->children);
		}
		else if (person == ((Family *)afamily)->wife)
		{
			theirChildren = &(((Family *)afamily)->children);
		}
	}

	return theirChildren;

}

 
char * injectIndividualIntoFile(char * filename, char * json)
{
	printf("%s", json);

	FILE * buffer = fopen(filename, "r");
	if (buffer == NULL)
	{
		fclose(buffer);
		return "Error: Could not open file";
	}
	else {
		fclose(buffer);
		Individual * person = JSONtoInd(json);

		GEDCOMobject * object = NULL;

		createGEDCOM(filename, &object);
		addIndividual(object, person);
		writeGEDCOM(filename, object);

		return "Updated File";
		
	}

}

int customCompareIndividuals(const void * first, const void * second)
{

	if (compareIndividuals(first, second) == 0)
	{
		Individual * firstI = (Individual *)first;
		Individual * secondI = (Individual *)second;

		if (getLength(firstI->events) == getLength(secondI->events))
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




List * findListChildrenDataComparison(const Individual * person)
{

	if (getLength(person->families) == 0)
	{
		puts("families empty");
		return NULL;
	}
	else {


		ListIterator theirFamilies = createIterator((person)->families);

		List * theirChildren = NULL; 
		void * afamily;



		while ((afamily = nextElement(&theirFamilies)) != NULL)
		{
			//puts("does this happen");

			//printf("comparing %s and %s\n", printIndividual((void *)person), printIndividual((void *)((Family *)afamily)->husband));
			if (customCompareIndividuals((void *)person, (void *)((Family *)afamily)->husband) == 0)
			{
	

				if( getLength(((Family *)afamily)->children) != 0)
				{
					theirChildren = &(((Family *)afamily)->children);
					return theirChildren;
				}


				//printf("comparing %s and %s\n", printIndividual((void *)person), printIndividual((void *)((Family *)afamily)->husband));
				//puts("Person is a father");
			}
			else if (customCompareIndividuals((void *)person, (void *)((Family *)afamily)->wife) == 0)
			{

					if( getLength(((Family *)afamily)->children) != 0)
					{	
						theirChildren = &(((Family *)afamily)->children);
	
						return theirChildren;
					}
				
			}
			else {
				theirChildren = NULL;
			}
		}


		return theirChildren;

	}

}


/*
List * findChildren(const Individual * person) 
{

	if (getLength(person->families) == 0)
	{
		puts("families empty");
		return NULL;

	}

	else {

		printf("In find children with %sThey have %d FAMILIES\n", printIndividual((void *) person), getLength(person->families));
		

		ListIterator theirFamilies = createIterator((person)->families);

		List * theirChildren = NULL; 
		void * afamily;



		while ((afamily = nextElement(&theirFamilies)) != NULL)
		{
			puts("does this happen");
			if (person == ((Family *)afamily)->husband)
			{
				theirChildren = &(((Family *)afamily)->children);
				puts("Person is a father");
			}
			else if (person == ((Family *)afamily)->wife)
			{
				theirChildren = &(((Family *)afamily)->children);
				puts("Person is a mother");
			}
		}

		return theirChildren;
	}

}
*/


void  printIndividualFully( void * person)
{

	
		//printf("ORIG: %s\n", printIndividual(person));
		//Individual * clone = deepCopyIndividual((Individual *)person);
		//printf("COPY: %s\n", printIndividual((void *)clone));

		//ListIterator personOtherFields = createIterator(clone->otherFields);
		
		printf("[INDV] %s", printIndividual(person));

				ListIterator personOtherFields = createIterator(((Individual *)person)->otherFields);

		void * other;
	
		while ((other = nextElement(&personOtherFields)) != NULL) 
		{
			printf("%s",printField(other));
		}
		
		ListIterator events = createIterator(((Individual *)person)->events);
		void * event;
	
		while ((event = nextElement(&events)) != NULL) 
		{
			//Event * sos = deepCopyEvent((Event *)event);


			printf("-> %s", printEvent(event));
	
		
			

			ListIterator personEventOtherField = createIterator(((Event *) event)->otherFields);
			void * otherEvent;
	
			while ((otherEvent = nextElement(&personEventOtherField)) != NULL) 
			{
				//Field * sos = deepCopyField((Field *) otherEvent);

		
				printf("			->%s",printField(otherEvent));
			
			}
		}
						
		//sprintf(temp, "\n	[Current Individuals Direct Relations, as Spouce or Child]\n\n");
		ListIterator theirFamilies = createIterator(((Individual *)person)->families);

		void * afamily;
		while ((afamily = nextElement(&theirFamilies)) != NULL)
		{
			//puts("Breaks here");
			char * data = printFamily(afamily);
			
			if (data != NULL)
			{
				printf("	FAMILIES: { \n%s	}\n\n", data);
				//exit(0);
	
				
			}
			free(data);
			
			//Look at the print datas too amke sure they can't print null
		}
		
	

}




char * cleanup(char * original)
{
	//printf("origninal notclean = '%s'\n", original); 
	char * result = calloc(1, sizeof(char) * (strlen(original) + 1));
	
	int index = 0; 

	for (int i = 0; i < strlen(original); i++)
	{
		if (original[i] != ' ')
		{
			result[index] = original[i];
			index++;
		}
		
	}
	result[index] = '\0';	
	return result;
}


char * printCharSetJSON(CharSet item)
{
	char * value = NULL;
	
	char * temp1 = "ANSEL"; 
	char * temp2 = "UTF8"; 
	char * temp3 = "UNICODE"; 
	char * temp4 = "ASCII"; 
	
	switch(item)
	{
		case ANSEL:
			value = malloc(sizeof(char) * strlen(temp1) + 1);
			strcpy(value, temp1);
			return value;
		case UTF8:
			value = malloc(sizeof(char) * strlen(temp2) + 1);
			strcpy(value, temp2);
			return value;
		case UNICODE: 
			
			value = malloc(sizeof(char) * strlen(temp3) + 1);
			strcpy(value, temp3);
			return value;
		case ASCII:
			
			value = malloc(sizeof(char) * strlen(temp4) + 1);
			strcpy(value, temp4);
			return value;		
		default: 
			return value;
	}
	
	return value;
}


char * printCharSet(CharSet item)
{
	char * value = NULL;
	
	char * temp1 = "ANSEL\n"; 
	char * temp2 = "UTF8\n"; 
	char * temp3 = "UNICODE\n"; 
	char * temp4 = "ASCII\n"; 
	
	switch(item)
	{
		case ANSEL:
			value = malloc(sizeof(char) * strlen(temp1) + 1);
			strcpy(value, temp1);
			return value;
		case UTF8:
			value = malloc(sizeof(char) * strlen(temp2) + 1);
			strcpy(value, temp2);
			return value;
		case UNICODE: 
			
			value = malloc(sizeof(char) * strlen(temp3) + 1);
			strcpy(value, temp3);
			return value;
		case ASCII:
			
			value = malloc(sizeof(char) * strlen(temp4) + 1);
			strcpy(value, temp4);
			return value;		
		default: 
			return value;
	}
	
	return value;
}


Event * initEvent() 
{
	Event * event = calloc(1, sizeof(Event));
	event->type[0] = '\0';
	event->date = NULL;
	event->place = NULL;
	event->otherFields = initializeList(&printField, &deleteField, &compareFields);
	
	return event;
	
}


Submitter * initSubmitter()
{
	Submitter * subm = (Submitter *)calloc(1, (sizeof(Submitter) + sizeof(char)* 256));
	memset(subm->submitterName, '\0', 60);
	subm->otherFields = initializeList(&printField, &deleteField,&compareFields); 
	return subm;
}



Family * initFamily(void)
{
	Family * temp = calloc(1, sizeof(Family));
	
	if (temp == NULL) {
		//puts("Malloc Failed");
		//exit(0);
		return NULL;
	}
	temp->wife = NULL;
	temp->husband = NULL;
	
	temp->children = initializeList(&printIndividual, &deleteIndividual,&compareIndividuals); 
	temp->events = initializeList(&printEvent, &deleteEvent, &compareEvents);
	temp->otherFields = initializeList(&printField, &deleteField,&compareFields); 
	
	return temp;
}


bool customComapreEvent(const void* first,const void* second)
{
	Event * item1 = (Event*)first;
	Event * item2 = (Event*)second;
	
	
	char * item1type = item1->type;
	char * item1date = item1->date;
	char * item1place = item1->place;
	
	char * item2type = item2->type;
	char * item2date = item2->date;
	char * item2place = item2->place;
	
	if (strcmp(item1type,item2type) == 0 || (item1type == NULL && item2type == NULL))
	{
		if (strcmp(item1date,item2date) == 0 || (item1date == NULL && item2date == NULL))
		{
			if (strcmp(item1place,item2place) == 0 || (item1place == NULL && item2place == NULL))
			{
				return true;
				
			}
			else {	
				//puts("EVENT: place doesnt match");	
			}
		}
		else {
			//puts("EVENT: date doesnt match");
		}
	}
	else {
		//puts("EVENT: type doesnt match");
		
	}
	return false;
}

Individual * initIndividual(void) 
{
	Individual * temp = calloc(1, sizeof(Individual));
	
	if (temp == NULL) {
		return NULL;
	}
	
	temp->givenName = NULL;
	temp->surname = NULL;
	
	temp->families = initializeList(&printFamily, &deleteFamily,&compareFamilies); 
	temp->otherFields = initializeList(&printField, &deleteField, &compareFields);
	temp->events = initializeList(&printEvent, &deleteEvent, &compareEvents);


	return temp;
}

//Not useful beyond debugging
void printAllElements(List list)
{
	//puts("1 SEGFAULT??");

	ListIterator iterator = createIterator(list);
	
	//puts("2 SEGFAULT??");
	void * element;
	//puts("3 SEGFAULT??");
	while ((element = nextElement(&iterator)) != NULL) 
	{
		//puts("4 SEGFAULT??");
		//printf("PAL: %s", list.printData(element));
	}
	//puts("5 SEGFAULT??");//Doesn't happen here.
	
	return;
}

void copyString(char * destination, char * source)
{
	int length = strlen(source);
	int i;
	for (i = 0; i < length; i++)
	{
		destination[i] = source[i];
	}
	destination[i] = '\0';
}

Header * initHeader(void)
{
	Header * temp = calloc(1, sizeof(Header));
	memset(temp->source, '\0', 248);
	temp->gedcVersion = 0;
	temp->submitter = NULL;

	return temp;
}

/* Family Pointer Helper Functions */
void deleteFuncFamilyP(void * toBeDeleted)
{
	
	if (toBeDeleted == NULL) {
			//puts("Gedparser. THIS SHOULD NEVER HAPPEN OR ELSE YOU WILL BE FREEING NULL LATER");
			return;
	}
	else {
		Familyp * temp = (Familyp *)toBeDeleted;
		free(temp->data);
		free(temp->xref);
	}
	return;
}

char * printFuncFamilyP(void * toBePrinted)
{
	if (toBePrinted == NULL)
	{
		return NULL;
	}	
	
	Familyp * temp; 
	temp = (Familyp*)toBePrinted; 
	//print given name; 
	
	if (temp->xref == NULL)
	{
		return NULL;
	}
	return temp->xref;
}

bool customComapreFamilyP(const void* first,const void* second)
{
	Familyp * item1 = (Familyp*)first;
	char * item1string = item1->xref; 
	if (item1string == NULL)
	{
		return false;
	}
	if (second == NULL)
	{
		return false;
	} 
	
	char * item2 = (char *)second; 
	
	//If the data is matching return 0 
	if (strcmp(item1string, item2) == 0)
	{
		return true;
	}

	return false;
}
/* END Family Pointer Helper Functions */

/* Individualp Helper Functions */
void deleteFuncINDVMap(void * toBeDeleted) 
{ 
	if (toBeDeleted == NULL) {
			//puts("Gedparser. THIS SHOULD NEVER HAPPEN OR ELSE YOU WILL BE FREEING NULL LATER");
			return;
	}
	else {
		Individualp * temp = (Individualp*)toBeDeleted;
		free(temp->data);
		free(temp->xref);
	}
	
	return; 
}

char * printFuncINDVMap(void * toBePrinted) {

	if (toBePrinted == NULL)
	{
		return NULL;
	}	
	
	Individualp * temp; 
	temp = (Individualp*)toBePrinted; 
	//print given name; 
	
	if (temp->xref == NULL)
	{
		return NULL;
	}
	
	size_t needed = snprintf(NULL, 0, "Individualp: xref:'%s' given:%s\n", temp->xref, printIndividual((void *)temp->data)) + 1;
	char * buffer = (char*)malloc(needed);
	snprintf(buffer, needed,"Individualp: xref:'%s' given:%s\n", temp->xref, printIndividual((void *)temp->data));

	return buffer;
}


bool customIndividualMapCompare(const void* first,const void* second)
{
	//first will be a pointer to Individualp needs to be derefrenced 
	//second will be to a string that I am searching for
	//cast this
	Individualp * item1 = (Individualp*)first;
	char * item1string = item1->xref; 
	if (item1string == NULL)
	{
		return false;
	}
	if (second == NULL)
	{
		return false;
	} 
	
	char * item2 = (char *)second; 
	
	//If the data is matching return 0 
	if (strcmp(item1string, item2) == 0)
	{
		return true;
	}

	return false;
	
}
/*  END of Individualp  */

bool customComapreField(const void* first,const void* second)
{
	Field * item1 = (Field*)first;
	char * item1string = item1->tag; 
	if (item1string == NULL)
	{
		return false;
	}
	if (second == NULL)
	{
		return false;
	} 
	
	char * item2 = (char *)second; 
	
	//If the data is matching return 0 
	if (strcmp(item1string, item2) == 0)
	{
		return true;
	}

	return false;
}


Field * initField()
{
	Field * field = (Field*)calloc(1, sizeof(Field));
	field->tag = NULL;
	field->value = NULL;
	//field->otherFields = initializeList(&printField, &deleteField,&compareFields); 

	if (field == NULL)
	{
		//puts("calloc failed");
		return NULL;
	}
	

	/*(char*)calloc(1, sizeof(char) * strlen(tag));
	if (field->tag != NULL)
	{
		strcpy(field->tag, tag);
	}*/
	
	/*(char*)calloc(1, sizeof(char) * strlen(value));
	if (field->value != NULL)
	{ 
		strcpy(field->value, value);
	}
*/
	return field;
}
/*
Event * initEvent() {
	
	Event * event = (Event*)calloc(1, sizeof(Event));
	
	if (event == NULL)
	{
		return NULL;
	}
	
	event->type[0] = '\0';
	event->date = NULL;
	event->place = NULL;
	
	//it also has other feilds btwww
	
	return event;
	
}
*/

char * concatStrings(char * before, char * new)
{

	char * newtemp = realloc(before, sizeof(char) * (strlen(before) + strlen(new) + 3));
	strcat(newtemp, new);
	
	return newtemp;

}

char * duplicateString(char * original)
{
	char * duplicate = (char*)calloc(1, sizeof(char) * (strlen(original) + 1));
	
	for (int i = 0; i < strlen(original); i++)
	{
		duplicate[i] = original[i];
	}
	return duplicate;
}

bool inWhitelistFamily(char * token)
{
	
	if (strcmp(token, "HUSB") == 0)
	{
		return true;
	}
	else if (strcmp(token, "CHIL") == 0)
	{
		return true;
	}
	else if (strcmp(token, "WIFE") == 0)
	{
		return true;
	}
	else if (strcmp(token, "DIV") == 0)
	{
		return true;
		
	}
	else {
		return true;
	}
	//maybe add false;
	
}

bool inWhitelistSubmitter(char * token)
{
	if (strcmp(token, "NAME") == 0)
	{
		return true;
	}
	else if (strcmp(token, "ADDR") == 0)
	{
		return true;
	}
	else if (strcmp(token, "COMM") == 0)
	{
		return true;	
	}
	else if (strcmp(token, "PHON") == 0)
	{
		return true;
	}
	else {
		
		return false;
	}
	
}


bool inWhitelistHead(char * token)
{
	if (strcmp(token, "HEAD") == 0)
	{
		return true;
	}
	if (strcmp(token, "COMM") == 0)
	{
		return true;
	}
	else if (strcmp(token, "SOUR") == 0)
	{
		return true;
	}
	else if (strcmp(token, "VERS") == 0)
	{
		return true;
	}
	else if (strcmp(token, "NAME") == 0)
	{
		return true;
	}
	else if (strcmp(token, "CORP") == 0)
	{
		return true;
	}
	else if (strcmp(token, "DATA") == 0)
	{
		return true;
	}	
	else if (strcmp(token, "DATE") == 0)
	{
		return true;
	}
	else if (strcmp(token, "DEST") == 0)
	{
		return true;
	}
	else if (strcmp(token, "TIME") == 0)
	{
		return true;
	}
	else if (strcmp(token, "SUBM") == 0)
	{
		return true;
	}
	else if (strcmp(token, "SUBN") == 0)
	{
		return true;
	}
	else if (strcmp(token, "FILE") == 0)
	{
		return true;
	}
	else if (strcmp(token, "COPR") == 0)
	{
		return true;
	}
	else if (strcmp(token, "VERS") == 0)
	{
		return true;
	}
	else if (strcmp(token, "GEDC") == 0)
	{
		return true;
	}
	else if (strcmp(token, "FORM") == 0)
	{
		return true;
	}
	else if (strcmp(token, "CHAR") == 0)
	{
		return true;
	}
	else if (strcmp(token, "LANG") == 0)
	{
		return true;
	}
	else if (strcmp(token, "PLAC") == 0)
	{
		return true;
	}
	else if (strcmp(token, "NOTE") == 0)
	{
		return true;
	}
	else if (strcmp(token, "CONC") == 0)
	{
		return true;
	}
	else if (strcmp(token, "CONT") == 0)
	{
		return true;
	}
	else {
		return false;
	}
}

bool inWhitelistInvidual(char * token)
{
	if (strcmp(token, "SEX") == 0)
	{
		return true;
	}
	else if (strcmp(token, "GIVN") == 0)
	{
		return true;
	}
	else if (strcmp(token, "BURI") == 0)
	{
		return true;
	}
	else if (strcmp(token, "SURN") == 0)
	{
		return true;
	}
	else if (strcmp(token, "BIRT") == 0)
	{
		return true;
	}
	else if (strcmp(token, "DEAT") == 0)
	{
		return true;
	}
	else if (strcmp(token, "DATE") == 0)
	{
		return true;
	}
	else if (strcmp(token, "PLAC") == 0)
	{
		return true;
	}
	else if (strcmp(token, "NAME") == 0)
	{
		return true;
	}
	else if (strcmp(token, "FAMC") == 0)
	{
		return true;
	}
	else if (strcmp(token, "FAMS") == 0)
	{
		return true;
	}
	else if (strcmp(token, "RESN") == 0)
	{
		return true;
	}
	else if (strcmp(token, "REFN") == 0)
	{
		return true;
	}
	else if (strcmp(token, "CHR") == 0)
	{
		
		return true;
	}
	else if (strcmp(token, "TITL") == 0)
	{
		return true;
	}
	return false;
}
	

bool actualError(GEDCOMerror error) 
{
	switch(error.type)
	{
		case OK:
			return false;
		default: 
			return true;
	}	
} 



int compareDate(Date * temp1, Date * temp2)
{
    if (temp1->year < temp2->year)
    {   //first one is ealier
        return -1;
    }
    else if (temp1->year > temp2->year)
    {
        return 1; 
    }
    else { //the same year
        
        if (temp1->month < temp2->month)
        {   //first one is ealier
            return -1;
        }
        else if (temp1->month > temp2->month)
        {//first one is later
            return 1; 
        }
        else { //the same day
            if (temp1->day < temp2->day)
            {  
                return -1;
            }
            else if (temp1->day > temp2->day)
            {//first date is later
                return 1; 
            }
            else { //SAME!!!!
                return 0;
            }
        }
    }
}

Date * initDate(void) {
	Date * temp = calloc(1, sizeof(Date));
	temp->day = 0;
	temp->month = 0;
	temp->year = 0;
	return temp;
}


void parseDate(Date ** date, char * rawDate)
{
	int day = 0;
	int month = 0;
	int year = 0;
	
	char * rawMonth = calloc(1, sizeof(char) * 50);
	
	sscanf(rawDate, "%d %s %d", &day, rawMonth, &year);
	
	month = rawMonthParse(rawMonth);
	
	(*date)->day = day; 
	(*date)->month = month;
	(*date)->year = year;
	
}

int rawMonthParse(char * rawMonth)
{
    //printf("found %s\n", rawMonth);
	if (strcmp(rawMonth, "JAN") == 0)
	{
		return 1; 
	}
	else if (strcmp(rawMonth, "FEB") == 0)
	{
		return 2; 
	}
	else if (strcmp(rawMonth, "MAR") == 0)
	{
		return 3;
	}
	else if (strcmp(rawMonth, "APR") == 0)
	{ 
		return 4; 
	}
	else if (strcmp(rawMonth, "MAY") == 0)
	{
		return 5; 
	}
	else if (strcmp(rawMonth, "JUN") == 0)
	{
		return 6; 
	}
	else if (strcmp(rawMonth, "JUL") == 0)
	{
		return 7; 
	}
	else if (strcmp(rawMonth, "AUG") == 0)
	{
		return 8;
	}
	else if (strcmp(rawMonth, "SEP") == 0)
	{
		return 9;
	}
	else if (strcmp(rawMonth, "OCT") == 0)
	{
		return 10;
	}
	else if (strcmp(rawMonth, "NOV") == 0)
	{
		return 11;
	}
	else if (strcmp(rawMonth, "DEC") == 0)
	{
		return 12;
	}
	else {
		return -1;
		
	}
}

//Updated
float parseVersionNumber(char * string)
{
	float vers = 0.0;
	
	bool firstPeriod = false;
	char temp[250]; 
	int j = 0;
	
	for (int i = 0; i < strlen(string); i++)
	{
		//Save all numbers
		if (isdigit(string[i]))
		{
			if (firstPeriod == true)
			{
				temp[j] = string[i];
				j++;
				break;

			}

			temp[j] = string[i];
			j++;
		}
		else if (string[i] == '.')
		{
			//only record first decimal place in string
			if (firstPeriod == false)
			{
				firstPeriod = true;
				temp[j] = string[i];
				j++;
			}
			else {
				continue;
			}
		}
	}
	temp[j] = '\0';
		
	vers = (float)atof(temp);
	return vers;
	
}

List deepCopyListofLists(List original)
{

	List copy = initializeList(&printGeneration,&deleteGeneration, &compareGenerations);
//printf("getDescendantListN length %d\n", getLength(Descendants));
	ListIterator iteratorL = createIterator(original);
	void * list;
	while ((list = nextElement(&iteratorL)) != NULL)
	{
		
		List * temp = (List *)list;

		List * generationCopy = calloc(1, sizeof(List));
		*generationCopy = initializeList(&printIndividual,&deleteIndividual, &compareIndividuals);


		ListIterator iterat = createIterator(*temp);
		void * person;

		while ((person = nextElement(&iterat)) != NULL)
		{
			insertBack(generationCopy, deepCopyIndividual((Individual *) person));
		}
		insertBack(&copy, generationCopy);

	}

	return copy;
}


//returns NULL if 
List * findParents(const Individual * person)
{


	List * parents = calloc(1, sizeof(List));

	*parents = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);


	ListIterator theirFamilies = createIterator((person)->families);
	void * afamily;

	while ((afamily = nextElement(&theirFamilies)) != NULL)
	{
		if (person == ((Family *)afamily)->husband)
		{
			//not what I am looking for
			//theirChildren = &(((Family *)afamily)->children);
		}
		else if (person == ((Family *)afamily)->wife)
		{
			//theirChildren = &(((Family *)afamily)->children);
		}
		else {
			//if they are a children in the family, grab that families parents
			//puts("Found the family where they are children, therefore they have parents");
			//if the parents are not null, 
				//add them to parents list, check to see if they have already been added first

			Individual * father = ((Family *)afamily)->husband;

			if (father != NULL)
			{
			insertBack(parents, father);


			}


			Individual * mother = ((Family *)afamily)->wife;
			if (mother != NULL)
			{
				insertBack(parents, mother);


			}





		}
	}

	if (getLength(*parents) == 0)
	{
		return NULL;
	}
	else {
		return parents;
	}
}


void appendParentsToGeneration(const Individual * person, List * prevGeneration)
{
	//maybe do a null check
	List * thierParents = findParents(person);

	if (thierParents == NULL || getLength(*thierParents) == 0)
	{
		//printf("%s person had no children\n", printIndividual((void *) person));
		return;
	}
	else {

		ListIterator iterator = createIterator(*thierParents);

		void * parent;

		while ((parent = nextElement(&iterator)) != NULL) 
		{

			if (findPersonInList(prevGeneration, &compareIndividuals, parent) == false)
			{
			
				//copies?
				Individual * copy = (Individual *) parent;
				insertSorted(prevGeneration, (void *)copy);
				//insertBack(nextGeneration, tempC);
				//insertBack(nextGeneration, child);
			}
			else {

				//puts("child already in list");
			}

		}

	}
}



List * getPreviousGeneration(List * currentGeneration)
{

	List * prevGeneration = calloc(1, sizeof(List));

	*prevGeneration = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);

	ListIterator personIterator = createIterator(*currentGeneration);

	void * person;
	while ((person = nextElement(&personIterator)) != NULL)
	{
		appendParentsToGeneration((Individual *)person, prevGeneration);
	}
	if(getLength(*prevGeneration) == 0)
	{
		return NULL;
	}
	else {
		return prevGeneration;
	}
}


List * getFirstAncestors(const Individual * person)
{

	//printIndividualFully((void *)person);


	//printf("length of array %d\n", getLength(person->families));


	List * generation = calloc(1, sizeof(List));
	*generation = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);

	List * parents = findParents(person);


	//Without this null check we get a segfault
	if (parents == NULL)
	{
		return NULL;
	}

	ListIterator iterator = createIterator(*parents);


	void * parent;

	while ((parent = nextElement(&iterator)) != NULL) 
	{

		if (findPersonInList(generation, &compareIndividuals, parent) == false)
		{
			//puts("did not find child in list, adding them");
			//copy them?

			Individual * copy = (Individual *) parent;
			//printf("length of families in copy is now %d", getLength(copy->families));

			/*
			printf("---COPY---\n");
			printIndividualFully(child);
			printf("---------\n");

			*/


			insertSorted(generation, (void *)copy);
			//insertBack(generation, child);
		}
		else {

			//puts("child already in list");
		}
		//printf("Adding copy of: EVENT:(%s) to copy List\n", printField(event)); 
		//deep copy of Field
		//Event * eventCopy = deepCopyEvent((Event *)event);
		//insertBack(&(copy->events),(void *)eventCopy);
	}

	if (getLength(*generation) == 0)
	{
		return NULL;
	}
	else {

		return generation;
	}



}








List * getNextGeneration(List * currentGeneration)
{

	List * nextGeneration = calloc(1, sizeof(List));
	*nextGeneration = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);
	ListIterator personIterator = createIterator(*currentGeneration);

	//ListIterator iterat = createIterator(*temp);
	//void * person;



	void * person;
	while ((person = nextElement(&personIterator)) != NULL)
	{
		//printf("Finding children of: %s", printIndividual(person));
		appendChildrenToGeneration((Individual *)person, nextGeneration);
	}


	if (getLength(*nextGeneration) == 0)
	{
		//free(nextGeneration);
		return NULL;
	}
	else {

		//printf("found a new generation with %d memebers\n", getLength(*nextGeneration));


		return nextGeneration;
	}
}



void appendChildrenToGeneration(const Individual * person, List * nextGeneration)
{

	//maybe do a null check
	List * theirChildren = findChildren(person);
	

	if (theirChildren == NULL || getLength(*theirChildren) == 0)
	{
		//printf("%s person had no children\n", printIndividual((void *) person));
		return;
	}
	else {

		ListIterator iterator = createIterator(*theirChildren);

		void * child;

		while ((child = nextElement(&iterator)) != NULL) 
		{

			if (findPersonInList(nextGeneration, &compareIndividuals, child) == false)
			{
				//puts("[append] did not find child in list, adding them");
				//copy them?



				Individual * copy = (Individual *) child;
				//printf("length of families is now %d", getLength(tempC->families));

				/*
				ListIterator familyIterator1 = createIterator(tempC->families);
	
				void * fam1;

				while ((fam1 = nextElement(&familyIterator1)) != NULL) 
				{
						printf("duplicateFam: %s\n", printFamily(fam1));
				}
				*/
				insertSorted(nextGeneration, (void *)copy);
				//insertBack(nextGeneration, tempC);
				//insertBack(nextGeneration, child);
			}
			else {

				//puts("child already in list");
			}
			//printf("Adding copy of: EVENT:(%s) to copy List\n", printField(event)); 
			//deep copy of Field
			//Event * eventCopy = deepCopyEvent((Event *)event);
			//insertBack(&(copy->events),(void *)eventCopy);
		}

	}
}



List * getFirstGeneration(const Individual * person)
{

	//printIndividualFully((void *)person);


	//printf("length of array %d\n", getLength(person->families));


	List * generation = calloc(1, sizeof(List));
	*generation = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);


	/* for (children in person->children)
			if (not copy child doesn't match any children in copy child list)
				duplicate the child, addToGeneration
	*/
	//puts("gets here");
	List * children = findChildren(person);


	//Without this null check we get a segfault
	if (children == NULL)
	{
		return NULL;
	}

	ListIterator iterator = createIterator(*children);


	void * child;

	while ((child = nextElement(&iterator)) != NULL) 
	{

		if (findPersonInList(generation, &compareIndividuals, child) == false)
		{
			//puts("did not find child in list, adding them");
			//copy them?

			Individual * copy = (Individual *) child;
			//printf("length of families in copy is now %d", getLength(copy->families));

			/*
			printf("---COPY---\n");
			printIndividualFully(child);
			printf("---------\n");

			*/


			insertSorted(generation, (void *)copy);
			//insertBack(generation, child);
		}
		else {

			//puts("child already in list");
		}
		//printf("Adding copy of: EVENT:(%s) to copy List\n", printField(event)); 
		//deep copy of Field
		//Event * eventCopy = deepCopyEvent((Event *)event);
		//insertBack(&(copy->events),(void *)eventCopy);
	}

	if (getLength(*generation) == 0)
	{
		return NULL;
	}
	else {

		return generation;
	}

	//If generation .count = 0; 
	//free(generation);
	//return NULL
}




//WORKS 
Field * deepCopyField(const Field * original)
{

	Field * copy = initField();

	if (original == NULL) 
	{
		free(copy);
		return NULL;
	}
	if (original->tag != NULL)
	{
		copy->tag = (char *)calloc(1, sizeof(char) * (strlen(original->tag) + 1));
		strcpy(copy->tag, original->tag);
			//printf("copy->type = '%s' and original->type = '%s'\n", copy->tag, original->tag);

	}
	if (original->value != NULL)
	{
		copy->value = (char *)calloc(1, sizeof(char) * (strlen(original->value) + 1));
		strcpy(copy->value, original->value);
		//printf("copy->value = '%s' and original->value = '%s'\n", copy->value, original->value);

	}


	return copy;
}





bool findPersonInList(const List * list, int (*compare)(const void* first, const void* second), const void * person)
{
	if (list == NULL || person == NULL)
	{
		return NULL;
	}
	
	ListIterator recordIterator = createIterator(*list);
	
	void * elem; 
	while ((elem = nextElement(&recordIterator)) != NULL)
	{
		if (compare(elem, person) == 0)
		{
			return true;
		}
	}

	return false;
}






//NEXT: FAMILES / INDV 
// THEY ARE CO-DEPENDANT, BUILD TOGETHER


//WORKS 
Event * deepCopyEvent(const Event * original)
{
	Event * copy = initEvent();
	
	if (original == NULL)
	{
		free(copy);
		return NULL;
	}
	if (original->type[0] != '\0')
	{
		//They have to be this length
		if (strlen(original->type) == 4)
		{
			for (int i = 0; i <= 4; i++)
			{
				copy->type[i] = original->type[i];
			}
			copy->type[4] = '\0';
		}
	}

	//printf("copy->type = '%s' and original->type = '%s'\n", copy->type, original->type);

	if (original->date != NULL)
	{
		copy->date = (char *)calloc(1, sizeof(char) * (strlen(original->date) + 1));
		strcpy(copy->date, original->date);
	}

	//printf("copy->type = '%s' and original->type = '%s'\n", copy->date, original->date);

	if (original->place != NULL)
	{
		copy->place = (char *)calloc(1, sizeof(char) * (strlen(original->place) + 1));
		strcpy(copy->place, original->place);
	}

	//printf("copy->place = '%s' and original->place = '%s'\n", copy->place, original->place);


	ListIterator iterator = createIterator(original->otherFields);
	void * elem;

	while ((elem = nextElement(&iterator)) != NULL) 
	{
		//printf("Adding copy of: (%s) to copy List\n", printField(elem)); 
		//deep copy of Field
		Field * currFieldCopy = deepCopyField((Field *)elem);
		insertBack(&(copy->otherFields),(void *)currFieldCopy);
	}

	return copy; 
}


Individual * deepCopyIndividual(const Individual * original)
{
	Individual * copy = initIndividual();

	if (original == NULL)
	{
		free(copy);
		return NULL; 
	}

	if (original->givenName != NULL)
	{
		copy->givenName = (char *)calloc(1, (sizeof(char) * strlen(original->givenName) + 1));
		strcpy(copy->givenName, original->givenName);
	}


	if (original->surname != NULL) {
		copy->surname = (char *)calloc(1, (sizeof(char) * strlen(original->surname) + 1));
		strcpy(copy->surname, original->surname);
	}	


	//Deep Event Fields Copying
	ListIterator Eventiterator = createIterator(original->events);
	void * event;

	while ((event = nextElement(&Eventiterator)) != NULL) 
	{
		//printf("Adding copy of: EVENT:(%s) to copy List\n", printField(event)); 
		//deep copy of Field
		Event * eventCopy = deepCopyEvent((Event *)event);
		insertBack(&(copy->events),(void *)eventCopy);
	}

	//Deep Other Fields Copying 
	ListIterator iterator = createIterator(original->otherFields);
	void * otherField;

	while ((otherField = nextElement(&iterator)) != NULL) 
	{
		//printf("Adding copy of: OTHERFEILD(%s) to copy List\n", printField(otherField)); 
		//deep copy of Field
		Field * fieldCopy = deepCopyField((Field *)otherField);
		insertBack(&(copy->otherFields),(void *)fieldCopy);
	}


	//ListIterator familyIterator = createIterator(original->families);
	//void * fam;
	
/*
	while ((fam = nextElement(&familyIterator)) != NULL) 
	{
		printf("originalFams: %s\n", printFamily(fam));
	}
*/


	//List * tempFam = duplicateList(original->families);
/*	

typedef struct listHead{
    Node* head;
    Node* tail;
    int length;
    void (*deleteData)(void* toBeDeleted);
    int (*compare)(const void* first,const void* second);
    char* (*printData)(void* toBePrinted);
} List;

*/
	copy->families.head = original->families.head; 
	copy->families.tail = original->families.tail; 
	copy->families.length = original->families.length;
	//ListIterator familyIterator1 = createIterator(copy->families);
	
	//void * fam1;

/*
	while ((fam1 = nextElement(&familyIterator1)) != NULL) 
	{
		printf("duplicateFam: %s\n", printFamily(fam1));
	}
	*/

	//copy list of family
	return copy; 
}














bool comparePointersBool(const void * first, const void * second) 
{
	Individual * one = ((Individualp *)first)->data; 
	Individual * two = (Individual *)second; 
	
	if (one == two) {
		return true; 	
	}
	return false; 
}

bool comparePointersFamilyPBool(const void * first, const void * second) 
{

	Family * one = ((Familyp *)first)->data; 
	Family * two = (Family *)second;
	
	//printf("comparing %s %s\n", printFamily(((void *)one)), printFamily(((void *) two)));

	if (one == two)
	{
		return true;
	}

	return false; 
}

int comparePointers(const void * first, const void * second) 
{
	Individualp * one = (Individualp *)first; 
	Individualp * two = (Individualp *)second; 
	
	if (one->data == two->data) {
		return 0; 	
	}
	return 1; 
}


int comparePointersFamilyP(const void * first, const void * second) 
{
	/*
	Familyp * one = (Familyp *)first; 
	Familyp * two = (Familyp *)second; 
	
	
	if (one->data == two->data) {
		return 0; 	
	}
	return 1; 
	*/
	return 0;
}



/* How to know if its a family S or C 
 * 
 * Check the families 
 * 
 * If family->husband == person || family->wife == person
 * 		It's family Spouce 
 * 
 * ELSE 
 * 		FAMILY C 
 * 
 * Write to file accordingly 
 * 
 */




 
 
 
GEDCOMerror writeHEADER(FILE ** buffer, const GEDCOMobject * obj)
{
	//Submitter Iteration number.
	GEDCOMerror error; 
	
	Header * header = obj->header;

	if (header == NULL)
	{
		error.type = WRITE_ERROR; 
		error.line = 0;
		return error;	
	}
	
	fprintf(*buffer, "0 HEAD\n");
	fprintf(*buffer, "1 SOUR %s\n", header->source);
	
	bool foundGEDC = false;

	ListIterator iterator = createIterator(header->otherFields);
	void * elem;
	while ((elem = nextElement(&iterator)) != NULL) 
	{
		char * tag = printFieldTag(elem); 
		char * value = printFieldValue(elem);
		
		//Only nessisary for GEDC
		if (strcmp(tag, "GEDC") == 0)
		{
			fprintf(*buffer, "1 %s %s\n", tag, value);
			fprintf(*buffer, "2 VERS %.2lf\n", header->gedcVersion);
			foundGEDC = true;
			free(tag);
			free(value);
			continue;
		}
		//If there is going to be a CONT, update the string to contain it	
		searchAndReplaceCONT(&value, headDepthEvaluator(tag));
		//\n in this is up in the air
		fprintf(*buffer, "%d %s %s\n", headDepthEvaluator(tag), tag, value);
		
		free(tag);
		free(value);
	}

	if (foundGEDC == false)
	{
		fprintf(*buffer, "1 GEDC\n");
		fprintf(*buffer, "2 VERS %.2lf\n", header->gedcVersion);
	}

	
	char * temp = printCharSet(header->encoding);
	fprintf(*buffer, "1 CHAR %s", temp);
	free(temp);

	//IF the submitter exists
	/* I don't save the submitter tag, assume and generate */
	fprintf(*buffer, "1 SUBM @S01@\n");

	error.type = OK;
	error.line = -1; 
	return error;
}

GEDCOMerror writeSUBMITTER(FILE ** buffer, const GEDCOMobject * obj)
{
	GEDCOMerror error; 
	Submitter * subm = obj->submitter; 
	
	if (subm == NULL)
	{
		error.type = WRITE_ERROR;
		error.line = -1; 
		return error;
	}
	else {
	
		//convert and print address
		char * temp = (char *)calloc(1, sizeof(char) * (strlen(subm->address) + 1));
		strcpy(temp, subm->address);
		
		searchAndReplaceCONT(&temp,1);
		
		fprintf(*buffer,"0 @S01@ SUBM\n1 NAME %s\n", subm->submitterName);
 	

 // WHY did this happen 
		if (strcmp(subm->address, "") != 0)
		{
			fprintf(*buffer,"1 ADDR %s\n", temp);
		}
	

		free(temp);
		
		error.type = OK;
		error.line = -1;
		return error;
	}	
}

GEDCOMerror writeINDIVIDUAL(FILE ** buffer, const GEDCOMobject * obj, List * individualPointersMap, List * familyPointersMap)
{
	GEDCOMerror error; 
	int counter = 1; 
	int familyCounter = 1;
	
	
	/* Saving algorithm */
	ListIterator listIndivudals = createIterator(obj->individuals);
	void * person;
	
	while ((person = nextElement(&listIndivudals)) != NULL) 
	{
		/*
			CHECK INDIVIDUALS
		*/
		void * result = findElement(*individualPointersMap, &comparePointersBool, person); 
		
		if (result == NULL)
		{
			char * ref = generateXREF(counter, 'I');
			counter++;
				
			Individualp * temp = (Individualp *)calloc(1, sizeof(Individualp));
			temp->data = person; 
			temp->xref = ref;	
			
			insertBack(individualPointersMap, (void *) temp);
					
			/*
				Print Person to GEDCOM FILE 
			*/
			fprintf(*buffer, "0 %s INDI\n", ref);
			fprintf(*buffer, "%s", printIndividualToGEDCOM((Individual *) person));

			printIndOtherToGEDCOM((Individual *) person,*buffer);



				
			/*
			 * 	CHECK FAMILIES AND GENERATE THEM
			 */
			ListIterator theirFamilies = createIterator(((Individual *)person)->families);

			void * afamily;
			while ((afamily = nextElement(&theirFamilies)) != NULL)
			{
				/*
					Before doing hash map shit, print out their family relations
				*/
				/* Super greasy void pointer comparison boolean function, don't use it other than this case!!!  */
				void * result = findElement(*familyPointersMap, &comparePointersFamilyPBool, afamily); 
				
				if (result == NULL)
				{
					char * ref = generateXREF(familyCounter, 'F');
					familyCounter++;
						
					Familyp * temp = (Familyp *)calloc(1, sizeof(Familyp));
					temp->data = afamily; 
					temp->xref = ref;	
					
					insertBack(familyPointersMap, (void *) temp);
					
				}

			}
			/*	NOW THAT YOU'VE GENERATED THE HASHMAP FOR THAT PERSONS FAMILIES
			 * 
			 * 	SEARCH FOR THEIR XREFS AGAIN AND PRINT THEM BASED ON IF CHILD OR SPOUCE
			 */ 
			 			 
			ListIterator theirFamilies2 = createIterator(((Individual *)person)->families);

			void * afamily2;
			while ((afamily2 = nextElement(&theirFamilies2)) != NULL)
			{
				/*
					Before doing hash map shit, print out their family relations
				*/
				/* Super greasy void pointer comparison boolean function, don't use it other than this case!!!  */
				void * result = findElement(*familyPointersMap, &comparePointersFamilyPBool, afamily2); 
				
				if (result != NULL)
				{
					Familyp * famp = (Familyp *)result;
					Family * fam = (Family *)famp->data; 
					
					//Printing their relation to said families to the GEDCOM 
					if (fam->husband == (Individual *)person || fam->wife == (Individual *)person)
					{
						fprintf(*buffer, "1 FAMS %s\n", famp->xref); 
					}
					else {
						fprintf(*buffer, "1 FAMC %s\n", famp->xref); 
					}
					
				}
			}
			
		}
		else {
			puts("Interesting, already incountered");
		}
	}

	error.type = OK;
	error.line = -1; 
	return error;
}

GEDCOMerror writeFAMILIES(FILE ** buffer, const GEDCOMobject * obj, List * individualPointersMap, List * familyPointersMap)
{
	
	GEDCOMerror error;
	/*
			For families in list of families 
			
				search hashmap for family, get it's xref, else there's an issue don't print gedcom error 
				
					print 0 xref FAM 
						
						now, you have a husband an wife, if they are not null search the individualPointers map for them 
						
						If they are there, print 1 HUSB/WIFE XREF 
				
						now for children in childrenList
						
							search individual pointers for them 
							
								if returns inidivualP 
										
										print 1 CHIL xref 
	 */
	 
	ListIterator listFamily = createIterator(obj->families);
	void * family;
	
	while ((family = nextElement(&listFamily)) != NULL) 
	{
	
		void * result = findElement(*familyPointersMap, &comparePointersFamilyPBool, family); 
		
		if (result != NULL)
		{
			Familyp * famp = (Familyp *)result;
			Family * fam = (Family *)famp->data; 
			
			fprintf(*buffer, "0 %s FAM\n", famp->xref);
			
			Individual * husband = fam->husband; 
			Individual * wife = fam->wife; 
			
			if (husband != NULL) {
				void * result = findElement(*individualPointersMap, &comparePointersBool, husband); 
				
				if (result != NULL)
				{
					Individualp * indip = (Individualp *)result;
						//Individual * indi = (Individual *)indip->data; 
			
						fprintf(*buffer, "1 HUSB %s\n", indip->xref);
			
					
				}
				else {
					printf("GED ERROR");
					
				}
				
				

			}
			if (wife != NULL) {
				
				void * result = findElement(*individualPointersMap, &comparePointersBool, wife); 

				if (result != NULL) {
				Individualp * indip = (Individualp *)result;
				//Individual * indi = (Individual *)indip->data; 
			
				fprintf(*buffer, "1 WIFE %s\n", indip->xref);
				
						
				}
				else {
					printf("GED ERROR");
					
				}
				
			}
			
			//Search and print children
			
			ListIterator childIterator = createIterator(fam->children);

			void * child;
	
			while ((child = nextElement(&childIterator)) != NULL) 
			{
				
				void * result = findElement(*individualPointersMap, &comparePointersBool, child); 

				if (result != NULL)
				{
					Individualp * indip = (Individualp *)result;
					//Individual * indi = (Individual *)indip->data; 
			
					fprintf(*buffer, "1 CHIL %s\n", indip->xref);
				
					
				}
				else {
					
					//printf("GED ERROR");
				}
			
			}
			printFamOtherToGEDCOM((Family *)family, *buffer);
			
		}
		else {
			
			//puts("There is an issue couldn't find family");
		}
	
	 
	}
	 
	 
	 error.type = OK; 
	 error.line = -1;
	 return error;
	 
	
}





/* 
You give it a string, if there are newlines, replace 

Could be a bad idea, giving it a try though..


There could be a off by 1 error in here
*/
void searchAndReplaceCONT(char ** string, int currentDepth)
{
	char possibleUpdate[1000]; 
	memset(possibleUpdate, '\0', 1000);
	int index = 0; 
	int length = strlen(*string);
	char c = '\0';
	int i = 0;


	int chars = 0;

	for (i = 0; i < length; i++)
	{
		c = (*string)[i]; 
		chars++;

		if (c != '\n')
		{
			possibleUpdate[index++] = (*string)[i]; 
		}
		else {
			size_t needed = snprintf(NULL, 0, "\n%d CONT ", currentDepth + 1) + 1;
			char * buffer = (char*)malloc(needed);
			snprintf(buffer, needed,"\n%d CONT ", currentDepth + 1);
			index += strlen(buffer); 
			strcat(possibleUpdate, buffer);
			free(buffer);
		}	
		
		/*
		if (chars > 250)
		{
			chars = 0;
			
		}
		*/
		
	}
	possibleUpdate[index] = '\0';

	if (i  == index) {
		//puts("No changes...");
		return;
	}
	else {
		//Am I leaking memory if i do this?
		//puts("Imposing changes..");
		*string = realloc(*string, sizeof(char) * strlen(possibleUpdate) + 1);
		strcpy(*string, possibleUpdate); 
		return;
	}

}


/* Simply evaluates what depth a given head TAG should represent
 * 
 */
int headDepthEvaluator(char * tag)
{
	//IE. 
	if (strcmp(tag, "VERS") == 0)
	{
		return 2; 
	}
	else if (strcmp(tag, "FORM") == 0)
	{
		return 2; 
	}
	else if (strcmp(tag, "NAME") == 0)
	{
		return 2; 
	}
	else {
		return 1; 
	}

}

char * printFieldTag(void * toBePrinted)
{
	if (toBePrinted == NULL)
	{
		return NULL;
	}	
	
	Field * temp; 
	temp = (Field*)toBePrinted; 
	
	size_t needed = snprintf(NULL, 0, "%s", temp->tag) + 1;
	char * buffer = (char*)malloc(needed);
	snprintf(buffer, needed,"%s", temp->tag);
	
	return buffer;
}

char * printFieldValue(void * toBePrinted)
{
	if (toBePrinted == NULL)
	{
		return NULL;
	}	
	
	Field * temp; 
	temp = (Field*)toBePrinted; 
	
	size_t needed = snprintf(NULL, 0, "%s", temp->value) + 1;
	char * buffer = (char*)malloc(needed);
	snprintf(buffer, needed,"%s", temp->value);
	
	return buffer;
}

void printFamOtherToGEDCOM(Family * family, FILE * buffer)
{

	if (buffer == NULL || family == NULL){
		return;
	}

	ListIterator events = createIterator((family)->events);
	void * evn;

	while ((evn = nextElement(&events)) != NULL) 
	{
	
		Event * event = (Event *)evn;
    
		if (event->type[0] != '\0')
		{
			fprintf(buffer,"1 %s\n", event->type);
			
			if (event->date != NULL)
			{
				if (strlen(event->date) != 0)
				{
					fprintf(buffer,"2 DATE %s\n", event->date);

				}
			}
			if (event->place != NULL)
			{
				if (strlen(event->place) != 0)
				{
					fprintf(buffer,"2 PLAC %s\n", event->place);
				}
			}
		}	
	}


	return;
}

void printIndOtherToGEDCOM(Individual * temp, FILE * buffer)
{
	if (buffer == NULL || temp == NULL){
		return;
	}

	if (temp->givenName != NULL)
	{
		if (strlen(temp->givenName) != 0)
		{
			fprintf(buffer,"2 GIVN %s\n", temp->givenName);
		}
	}
	if (temp->surname != NULL)
	{
		if (strlen(temp->surname) != 0)
		{
			fprintf(buffer,"2 SURN %s\n", temp->surname);
		}
	}

	//Print other fields too


	ListIterator otherFields = createIterator((temp)->otherFields);
	void * field;

	while ((field = nextElement(&otherFields)) != NULL) 
	{

		Field * otherField = (Field *)field;

		if (otherField->tag != NULL && otherField->value != NULL)
		{

			fprintf(buffer,"1 %s %s\n", otherField->tag, otherField->value);
		}
	}



	ListIterator events = createIterator((temp)->events);
	void * evn;

	while ((evn = nextElement(&events)) != NULL) 
	{
	
		Event * event = (Event *)evn;
    
		if (event->type[0] != '\0')
		{
			fprintf(buffer,"1 %s\n", event->type);
			
			if (event->date != NULL)
			{
				if (strlen(event->date) != 0)
				{
					fprintf(buffer,"2 DATE %s\n", event->date);

				}
			}
			if (event->place != NULL)
			{
				if (strlen(event->place) != 0)
				{
					fprintf(buffer,"2 PLAC %s\n", event->place);
				}
			}
		}	
	}

//		fprintf(*buffer,"0 @S01@ SUBM\n1 NAME %s\n1 ADDR %s\n", subm->submitterName, temp);


	return;
}


char * printIndividualToGEDCOM(Individual * temp)
{
	char * buffer = NULL;
	if (temp->givenName && temp->surname)
	{
		size_t needed = snprintf(NULL, 0, "1 NAME %s /%s/\n", temp->givenName, temp->surname) + 1;
		buffer = (char*)malloc(needed);
		snprintf(buffer, needed,"1 NAME %s /%s/\n ", temp->givenName, temp->surname);	
	}
	else {
		size_t needed = snprintf(NULL, 0, "MISSING DATA\n") + 1;
	    buffer = (char*)malloc(needed);
		snprintf(buffer, needed,"MISSING DATA\n");	
	}
	
	return buffer; 
}

char * generateXREF(int index, char type)
{	
	size_t needed = snprintf(NULL, 0, "@%c%d@", type, index) + 1;
	char * buffer = (char*)malloc(needed);
	snprintf(buffer, needed,"@%c%d@", type, index);
	return buffer;
}







