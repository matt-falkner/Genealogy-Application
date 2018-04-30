#include "LinkedListAPI.h"
#include "assert.h"



/*
	!!!!!!REFRENCE!!!!!!!: 
	* 	Since I last took 2750 last winter, I never had a Data Structures that covered ADT. Others used their own from 2520, but 
	*   as the prof said, im using his to 'level the playing feild' 
	*   
	* 	As a result, I heavily used The prof's example LinkedList.c to create mine to focus on the parser.
	*   I am not claiming it as my own work. Although, It has been modified from it's orginal version to work here.
	* 
	* 	No point rebuilding the wheel. I asked some TA's and they said it was cool as long as I refrenced it here.
	* 
	* Just paranoid about academic misconduct lol

*/
enum ERROR 
{
    malloc_fail, 
    missing_function_pointers, 
    unable_insertBack,
    empty_list,
}; 

void failure(enum ERROR e); 

//Returns the generic pointer to the data at the front of the list. 



/** Function that searches for an element in the list using a comparator function.
 * If an element is found, a pointer to the data of that element is returned
 * Returns NULL if the element is not found.
 *@pre List exists and is valid.  Comparator function has been provided.
 *@post List remains unchanged.
 *@return The data associated with the list element that matches the search criteria.  If element is not found, return NULL.
 *@param list - a list sruct
 *@param customCompare - a pointer to comparator fuction for customizing the search
 *@param searchRecord - a pointer to search data, which contains seach criteria
 *Note: while the arguments of compare() and searchRecord are all void, it is assumed that records they point to are
 *      all of the same type - just like arguments to the compare() function in the List struct
 **/
void* findElement(List list, bool (*customCompare)(const void* first,const void* second), const void* searchRecord)
{
	ListIterator iterator = createIterator(list);
	
	void * elem;
	while ((elem = nextElement(&iterator)) != NULL){
		//char * temp = list.printData(elem);
		
		//printf("FELEMENT: item  %s\n", temp);
		if (customCompare(elem, searchRecord) == true)
		{
			//printf("FELEMENT: FOUND-> item  %s\n", temp);
			//free(temp);
			return elem;
		}
		//free(temp);
	}
	
	return NULL;
}

void * getFromFront(List list)
{
    if (list.head == NULL)
    {
		return NULL;
	}
	
	return list.head->data;
}

void * getFromBack(List list)
{
    if (list.tail == NULL){
		return NULL;
	}

	return list.tail->data;
}

/* the basics */ 
Node* initializeNode(void* data) 
{
    Node * temporary = malloc(sizeof(Node)); 
    //End the program if the malloc fails. 
    if (temporary == NULL) { failure(malloc_fail);}

    temporary->data = data; 
    temporary->next = NULL;
    temporary->previous = NULL;

    return temporary;
}

void clearList(List* list){
	
    if (list == NULL){
		return;
	}
	
	if (list->head == NULL && list->tail == NULL){
		return;
	}
	
	Node* tmp;
	
	while (list->head != NULL){
		list->deleteData(list->head->data);
		tmp = list->head;
		list->head = list->head->next;
		free(tmp);
	}
	
	list->head = NULL;
	list->tail = NULL;
}

void failure(enum ERROR e){ 
   /*
    switch (e)
    {
        case malloc_fail: 
            //printf("Malloc returned NULL. Failed to Allocate memory. Exitting...\n");
            exit(1);
        case missing_function_pointers:
            //printf("One or More Function pointers to 'InitalizeList' are NULL when they should not be. Exiting program, because if I don't this shit gunna burn.\n");
            exit(1);
        case unable_insertBack: 
            printf("Failed to append data because either the list or the void data was null. Not my fault. Your fault. Continuing regardless\n");
            return; 
        case empty_list:
            printf("List provided is NULL, cannot complete action\n");
            return;  
        default: 
            printf("This should not happen. Continuing anyways");
            return; 
    }
    */
    return;
}

List initializeList(char* (*printFunction)(void* toBePrinted),void (*deleteFunction)(void* toBeDeleted),int (*compareDataFunction)(const void* first,const void* second)) 
{
    if (printFunction == NULL || deleteFunction == NULL || compareDataFunction == NULL)
    {
        failure(missing_function_pointers);
    }

    List temporary; 

    assert(printFunction != NULL);
    assert(deleteFunction != NULL);
    assert(compareDataFunction != NULL);

    temporary.length = 0; 
    temporary.head = NULL; 
    temporary.tail = NULL; 
    temporary.deleteData = deleteFunction; 
    temporary.compare = compareDataFunction;
    temporary.printData = printFunction; 

    return temporary;
}

ListIterator createIterator(List list){
    ListIterator iter;
 
    iter.current = list.head;
    
    return iter;
}

void* nextElement(ListIterator* iter){
    Node* tmp = iter->current;
    
    if (tmp != NULL){
        iter->current = iter->current->next;
        return tmp->data;
    }else{
        return NULL;
    }
}

char * toString(List list)
{
	ListIterator iter = createIterator(list);
	char* str;
		
	str = (char*)malloc(sizeof(char));
	strcpy(str, "");
	
	void* elem;
	while( (elem = nextElement(&iter)) != NULL){
		char* currDescr = list.printData(elem);
		int newLen = strlen(str)+50+strlen(currDescr);
		str = (char*)realloc(str, newLen);
		strcat(str, "\n");
		strcat(str, currDescr);
		
		free(currDescr);
	}
	
	return str;
}

void insertSorted(List *list, void *toBeAdded)
{
	if (list == NULL || toBeAdded == NULL)
    {
		return;
	}
	
	if (list->head == NULL){
		insertBack(list, toBeAdded);
		return;
	}
	
	if (list->compare(toBeAdded, list->head->data) <= 0)
    {
		insertFront(list, toBeAdded);
		return;
	}
	
	if (list->compare(toBeAdded, list->tail->data) > 0)
    {
		insertBack(list, toBeAdded);
		return;
	}
	
	Node* currNode = list->head;
	
	while (currNode != NULL)
    {
		if (list->compare(toBeAdded, currNode->data) <= 0)
        {
		
			char* currDescr = list->printData(currNode->data); 
			char* newDescr = list->printData(toBeAdded); 
		
			//printf("Inserting %s before %s\n", newDescr, currDescr);

			free(currDescr);
			free(newDescr);
		
			Node* newNode = initializeNode(toBeAdded);
			newNode->next = currNode;
			newNode->previous = currNode->previous;
			currNode->previous->next = newNode;
			currNode->previous = newNode;

            //printf("list->length = %d. Adding 1", list->length);
            list->length = list->length+1; 
		
			return;
		}
	
		currNode = currNode->next;
	}
	
	return;
}

void * deleteDataFromList(List * list, void * toBeDeleted)
{
	if (list == NULL || toBeDeleted == NULL)
    {
		return NULL;
	}
	
	Node* tmp = list->head;
	
	while(tmp != NULL)
	{
		if (list->compare(toBeDeleted, tmp->data) == 0)
		{
			
			//Unlink the node
			Node* delNode = tmp;
			
			if (tmp->previous != NULL)
            {
				tmp->previous->next = delNode->next;
			}
            else
            {
				list->head = delNode->next;
			}
			
			if (tmp->next != NULL)
            {
				tmp->next->previous = delNode->previous;
			}
            else
            {
				list->tail = delNode->previous;
			}
			
			void * data = delNode->data;
			free(delNode);
			
            //printf("list->length = %d. Removing 1", list->length);
            list->length = list->length - 1; 

			return data;
			
		}
        else
        {
			tmp = tmp->next;
		}
	}


	
	return NULL;
}

void insertFront(List* list, void* toBeAdded) 
{
    if (list == NULL || toBeAdded == NULL){
		return;
	}
	
	Node* newNode = initializeNode(toBeAdded);
	
    if (list->head == NULL && list->tail == NULL){
        list->head = newNode;
        list->tail = list->head;
    }else{
		newNode->next = list->head;
        list->head->previous = newNode;
    	list->head = newNode;
    }
    list->length = list->length+1; 
}

int getLength(List list)
{
    int length  =   0;  
    if (list.head == NULL)
    {
        failure(empty_list);
        //Length is effectively NONE. 
        return length;
    }

    Node * temporary = list.head; 
    
   
    while (temporary != NULL)
    {
        length++;
        temporary = temporary->next; 
    }

    return length;
}

void insertBack(List * list, void * toBeAdded)
{
    if (list == NULL || toBeAdded == NULL)
    {
        failure(unable_insertBack); 
        return;
    }

    Node * temporary = initializeNode(toBeAdded);

    //if list is empty. make it the head. 
    if (list->head == NULL && list->tail == NULL)
    {
        list->head = temporary;
        list->tail = list->head;
    }
    else 
    {
        temporary->previous = list->tail; 
        list->tail->next = temporary; 
        list->tail = temporary; 
    }

    //printf("list->length = %d. Adding 1", list->length);
    list->length = list->length+1; 
}
