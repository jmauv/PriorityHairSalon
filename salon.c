//Jonathan Mauvais
//Professor Guha
//COP 3502
//Program #6
//04/11/2022

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define strMAX 21
#define initSize 10

typedef struct customer
{
    int arrTime;
    char custName[strMAX];
    char prefStylist[strMAX];
    int points;
    int hairTime;
} customer;

typedef struct heap
{
    customer** queue;
    char stylistName[strMAX];
    int size;
    int capacity;
    long long prevCustTime;
} heap;

void init(heap* hPtr);
void findingStylist(heap* hPtr, customer* cust, int numStylists);
void insert(heap* hPtr, customer* cust);
void searchFinishedCustomer(heap* hPtr, int numStylists, long long time, int *numFinished);
void printCustomer(heap* hPtr, long long time, int* numFinished);
customer* nextUp(heap* hPtr);
void percolateUp(heap* hPtr, int index);
void percolateDown(heap* hPtr, int index);
int compare(customer* a, customer* b, char* stylist);
void swap(customer** a, customer** b);
void freeHeap(heap* hPtr, int numStylists);

int main()
{
    heap stylistLine[initSize]; //array of heaps for potential 10 stylists
    int numCustomers, numStylists, numFinished = 0, i;
    long long t = 0;

    scanf("%d%d", &numCustomers, &numStylists);

    //initialize heap each stylist
    for(int i = 0; i < numStylists; ++i)
    {
        init(&stylistLine[i]);
        scanf("%s", stylistLine[i].stylistName);
    }


    //main loop keeps looping until all customers inputted are done with their haircut
    while(numFinished != numCustomers)
    {
        //when all customers are inputted and found a line you can stop using this inner loop
        if(i != numCustomers)
            for(i = 0; i < numCustomers; ++i)
            {
                customer* cust = (customer*)malloc(sizeof(customer));

                scanf("%d%s%s%d%d", &cust->arrTime, cust->custName, cust->prefStylist,
                                    &cust->points, &cust->hairTime);

                //keeps increasing time, searching for a potential finished customer at each minute, until it matches
                //customers arrival time
                while(t != cust->arrTime)
                {
                    searchFinishedCustomer(stylistLine, numStylists, t, &numFinished);
                    t++;
                }

                //then, find the right stylist for customer and then search for a customer that could be finished
                findingStylist(stylistLine, cust, numStylists);
                searchFinishedCustomer(stylistLine, numStylists, t, &numFinished);
            }

        //after all customers are inputted and in a line you still have to
        //progress through time to get them all finished up
        t++;
        searchFinishedCustomer(stylistLine, numStylists, t, &numFinished);
    }

    //free
    freeHeap(stylistLine, numStylists);

    return 0;
}

void init(heap* hPtr)
{
    hPtr->size = -1;
    hPtr->capacity = initSize;
    hPtr->prevCustTime = -100000000;//setting this to a really big negative number to prevent potential earlier than should be removals from queue
    hPtr->queue = (customer**)malloc(sizeof(customer*) * (initSize + 1));

\
    for(int i = 0; i < initSize + 1; ++i)
        hPtr->queue[i] = NULL;
}

void findingStylist(heap* hPtr, customer* cust, int numStylists)
{
    int minLine = 100000;
    int found = 0;
    int index, index2;

    if(numStylists == 1)//only 1 stylist. no need to search
    {
        insert(&hPtr[0], cust);
        return;
    }

    for(int i = 0; i < numStylists; ++i)//if cust has preference, insert them into their stylists queue
        if(strcmp(hPtr[i].stylistName, cust->prefStylist) == 0)
        {
            insert(&hPtr[i], cust);
            found = 1;
            break;
        }

    if(found == 0)
    {   //no preference, find the queue with the smallest size
        for(int i = numStylists - 1; i >= 0; --i)
            if(hPtr[i].size <= minLine)
            {
                minLine = hPtr[i].size;
                index = i;
            }

        insert(&hPtr[index], cust);
    }
}
void insert(heap* hPtr, customer* cust)
{
    //double capacity when queue gets full
    if(hPtr->size == hPtr->capacity)
    {
        hPtr->queue = (customer**)realloc(hPtr->queue, sizeof(customer*) * (2 * hPtr->capacity + 1));
        hPtr->capacity *= 2;
    }

    hPtr->size++;
    hPtr->queue[hPtr->size] = cust;

    percolateUp(hPtr, hPtr->size);
}

void searchFinishedCustomer(heap* hPtr, int numStylists, long long time, int *numFinished)
{
    for(int i = 0; i < numStylists; ++i)
        //if stylist is servicing someone...
        if(hPtr[i].size > -1)
            //and if their arrival time + hair time = time & time is less than the previous customers finish time + your hair time
            //or vice versa
            if(time == (long long) hPtr[i].queue[0]->arrTime + hPtr[i].queue[0]->hairTime || time == (long long) hPtr[i].prevCustTime + hPtr[i].queue[0]->hairTime)
            {
                if(time < (long long) hPtr[i].prevCustTime + hPtr[i].queue[0]->hairTime || time < (long long) hPtr[i].queue[0]->arrTime + hPtr[i].queue[0]->hairTime) continue;

                //print output and remove from list
                printCustomer(&hPtr[i], time, numFinished);
            }
}

void printCustomer(heap* hPtr, long long time, int* numFinished)
{
    hPtr->prevCustTime = time;//the current time will be the time this customer finished. save it
    hPtr->queue[0]->points = (int)hPtr->queue[0]->points + hPtr->queue[0]->hairTime / 10;
    printf("%s %lld %d %s\n", hPtr->queue[0]->custName, time, hPtr->queue[0]->points, hPtr->stylistName);

    //if no one is waiting for the stylist after this customer, free and subtract size
    if(hPtr->size == 0)
    {
        free(hPtr->queue[0]);
        hPtr->queue[0] = NULL;
        hPtr->size--;
    }
    else //customer(s) waiting, next in line goes up
    {
        hPtr->queue[0] = nextUp(hPtr);
        //if this new customer has a hair time of zero, they need to be removed before time progresses again.
        //recursion using updated heap with new customer in front
        if(hPtr->queue[0]->hairTime == 0)
            printCustomer(hPtr, time, numFinished);
    }
    //counter for number of customers finished. increase it at the end of this function
    (*numFinished)++;
}


customer* nextUp(heap* hPtr)
{
    //next in line is index 1, where the heap actually begins. delete top of heap like normal
    customer* next = hPtr->queue[1];
    hPtr->queue[1] = hPtr->queue[hPtr->size];

    hPtr->size--;

    percolateDown(hPtr, 1);

    return next;//return customer thats got next, will move up from index 1 to 0
}

void percolateUp(heap* hPtr, int index)
{
    //if heap is size 0 or 1/index reaches 1, dont percolate
    if(index == 0 || index == 1) return;

    //parent
    int pIndex = index/2;
    if(compare(hPtr->queue[index], hPtr->queue[pIndex], hPtr->stylistName) > 0)
    {
        swap(&hPtr->queue[index], &hPtr->queue[pIndex]);
        percolateUp(hPtr, pIndex);
    }
}

void percolateDown(heap* hPtr, int index)
{
    //leaf node
    if (2*index > hPtr->size) return;

    // only left child
    if (2*index == hPtr->size)
    {
        //the last swap.
        if (compare(hPtr->queue[2*index], hPtr->queue[index], hPtr->stylistName) > 0)
            swap(&hPtr->queue[2*index], &hPtr->queue[index]);
        return;
    }

    //is left child of right child better?
    int swapIndex = compare(hPtr->queue[2*index], hPtr->queue[2*index+1], hPtr->stylistName) > 0 ? 2 * index : 2 * index + 1;

    //swap if out of order
    if (compare(hPtr->queue[swapIndex], hPtr->queue[index], hPtr->stylistName) > 0)
    {
        swap(&hPtr->queue[swapIndex], &hPtr->queue[index]);
        percolateDown(hPtr, swapIndex);
    }
}

int compare(customer* a, customer* b, char* stylist)
{
    if(a->points != b->points)
        return a->points - b->points; //returns negative if parent has more points, positive for child
    else if(strcmp(a->prefStylist, stylist) == 0 && strcmp(b->prefStylist, stylist) != 0)
        return 1; //if child has this stylist preferred and parent doesn't, return postive
    else if(strcmp(b->prefStylist, stylist) == 0 && strcmp(a->prefStylist, stylist) != 0)
        return -1; //parent has stylist preferred but not child, return negative
    else
        return strcmp(b->custName, a->custName); //else, compare names
}

void swap(customer** a, customer** b)
{
    customer* temp = *a;
    *a = *b;
    *b = temp;
}

void freeHeap(heap* hPtr, int numStylists)
{
    for(int i = 0; i < numStylists; ++i)
        free(hPtr[i].queue);
}