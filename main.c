#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include "queue.c"

/* *********** GLOBAL VARIABLES *************** */
int numTables = 0;
int numTypes = 0;
int numCust = 0;
int maxItems = 0;
int randomAuction = 0;
int runBonus = 0;
bool quit = false;
unsigned short xsub1[3];
struct Queue* q;            // queue of customers
pthread_mutex_t  mutex1;    // mutex for runner function
pthread_mutex_t  mutex2;    // mutex for bonus function

// I am using a struct to create a customer object
struct customer
{
    int id;
    int moneyBegin;
    int moneyFinal;
    int bid;
    int listSize;
    int* itemList;
    int wins;
};

// this is my list of customers
struct customer* customers;

// struct of table
struct table
{
    int tableID;
    int custID;
};

struct table* tables;


/* *************** FUNCTIONS *************** */
// runner function
    // parameter is table ID
    // thread function
void *runner(void *param)
{

    // initialize
    int tblID = *(int*)param;

    pthread_mutex_lock(&mutex1);
    // in a loop - until main says to quit

        if(tables[tblID].custID == -1 && q->front != NULL)
        {
            // if no customer, find customer
            tables[tblID].custID = q->front->key;
            deQueue(q);
            printf("Customer %d assigned to table %d\n", tables[tblID].custID, tblID);
        }

        // in a loop - until customer leaves
            // going thru auction

        pthread_mutex_unlock(&mutex1);


 	pthread_exit(0);
}

// bonus function
    // create extra thread to add new random customer
void bonus()
{
    int delay = 100000;
    int dtry = 1;
    int x = 0;
    int n = 0;

    srand(time(0));

    pthread_mutex_lock(&mutex2);        // lock mutex2 for bonus
    // critical section because I want all threads to stop while I add a customer to the queue
    for(int i = 0; i < dtry; i++)
    {
        for(int j = 0; j < delay; j++)
        {
            x = 1;
        }
    }

    n = rand() % 4;
    if(n == 4)
    {
        // generate random customer, add to queue
        struct customer* c;
        c->bid = 0;
        c->id = rand() % 256;
        c->listSize = rand() % maxItems;
        for(int i = 0; i < c->listSize; i++)
        {
            c->itemList[i] = rand() % maxItems;
        }
        c->moneyBegin = rand() % 100;
        c->moneyFinal = c->moneyBegin;
        c->wins = 0;

        // add to queue
        enQueue(q, c->id);
        dtry++;

    } else {
        if(dtry >= 2){
            dtry /= 2;
        } else {
            dtry = 1;
        }
    }
    pthread_mutex_unlock(&mutex2);      // unlock mutex2
    // end critical section
}

// main function
    // auction simulator
int main(int argc, char *argv[]){
/* *********** READING PART *************** */
    // read input file
    FILE *file;

    if((file = fopen(argv[1], "r")) == NULL)
        printf("Error opening file \n");

    // get the first line & initialize global variables
    fscanf(file, "%d", &numTables);
    fscanf(file, "%d", &numTypes);
    fscanf(file, "%d", &numCust);
    fscanf(file, "%d", &maxItems);
    fscanf(file, "%d", &randomAuction);
    fscanf(file, "%d", &runBonus);

    // creating customers
    customers = (int*)malloc(numCust * sizeof(*customers));
    q = createQueue();
    for(int i = 0; i < numCust; i++)
    {
        // allocate space in itemList array of customer[i]
        customers[i].itemList = (int*)malloc(sizeof(int*));

        // now read info into customer
        fscanf(file, "%d", &customers[i].id);
        fscanf(file, "%d", &customers[i].moneyBegin);
        fscanf(file, "%d", &customers[i].listSize);

        // this reads the items on the list into the list
        for(int j = 0; j < customers[i].listSize; j++)
        {
            fscanf(file, "%d", &customers[i].itemList[j]);
        }
        customers[i].moneyFinal = customers[i].moneyBegin;
        customers[i].wins = 0;
        customers[i].bid = 0;

        // message
        printf("Customer %d added to the queue \n", customers[i].id);
        enQueue(q, customers[i].id);
    }
    fclose(file);

/* *********** INITIALIZATION ************** */
    // initialize variables
    pthread_t *tid;
    pthread_attr_t attr;
    int moneySpent = 0;
    int itemsSold = 0;
    srand(time(0));

    // xsub1[0] = (short) (rand() % 256);
    // xsub1[1] = (short) (rand() % 256);
    // xsub1[2] = (short) (rand() % 256);
        /* NOTE: my terminal doesn't compile jrand48, for some reason
            Professor Lin said I could use srand for the time being during office hours
            on Tuesday, 3/30/2021 */

    pthread_attr_init(&attr);
    pthread_mutex_init(&mutex1, NULL);
    pthread_mutex_init(&mutex2, NULL);
    tid = malloc(sizeof(pthread_t)*numTables);
    tables = (int*)malloc(numTables * sizeof(*tables));
    printf("\n");

    // create the tables (threads)
    for(int i = 0; i < numTables; i++)
    {
        tables[i].tableID = i;
        pthread_create(&tid[i], &attr, runner, &(tables[i].tableID));
    }

    // sit the initial customers
    for(int i = 0; i < numTables; i++)
    {
        printf("Customer %d assigned to table %d \n", customers[i].id, tables[i].tableID);
        tables[i].custID = customers[i].id;
        deQueue(q);
    }

/* *********** AUCTION PART *************** */
    // in a loop - until all items sold or no more customers
    int round = 0;
    int item = 0;               // item type to be auctioned

    // while the q isn't empty and all the items haven't been sold
    while(q != NULL && round != maxItems)
    {
        // determine who can participate in the round - message

        int* eligible[numTables];     // list of the ids of the customers who are eligible to bid
        printf("The following customers are eligible for this round of auction: \n");
        for(int i = 0; i < numTables; i++)
        {
            // qualifier: table occupied
            if(tid[i] != NULL)
            {
                eligible[i] = customers[i].id;
                printf("%d \n", customers[i].id);
            }
        }

        // select item to auction - message
        printf("\n");
        if(randomAuction == 1)
        {
            item = round + 1;
        } else {
            // randomize selection
		    item = rand() % maxItems;

        }
        printf("Item of type %d up for auction \n", item);

        // auction item
            // qualifier: item is on list && has money - message
        int arrLen = sizeof(eligible) / sizeof(eligible[0]);
        int* eligible2[arrLen];

        for(int i = 0; i < arrLen; i++)
        {
            // is the customer on the first eligible list?
            if(customers[i].id == eligible[i])
            {
                // does the customer have money?
                if(customers[i].moneyFinal > 0)
                {
                    for(int j = 0; j < customers[i].listSize; j++)
                    {
                        // is the item on their list?
                        if(item == customers[i].itemList[j])
                        {
                            eligible2[i] = customers[i].id;
                        }
                    }
                }
            }
        }

        int arrLen2 = sizeof(eligible2) / sizeof(eligible2[0]);

        // if no one bids, item removed, round over
        if(arrLen2 == 0)
        {
            printf("No one won the auction\n");

        } else {

            // if 1 bids, 1 wins, add item, sub $, round over
            if((arrLen2-1) == 1)
            {
                for(int i = 0; i < numCust; i++)
                {
                    for(int j = 0; j < customers[i].listSize; j++)
                    {
                        if(item == customers[i].itemList[j])
                        {
                            // randomize bid
                            customers[i].bid = rand() % customers[i].moneyFinal;
                            // update variables
                            customers[i].moneyFinal -= customers[i].bid;
                            customers[i].wins++;
                            moneySpent += customers[i].bid;
                            printf("Customer %d at table %d bids %d \n", customers[i].id, tid[i], customers[i].bid);

                            // message: who won, who lost
                            printf("Customer %d won the auction, with amount %d\n", customers[i].id, customers[i].bid);
                            itemsSold++;
                        }
                    }
                }

            } else {
                // if 2+ bid, 2+ bid again - message
                printf("Second round of bidding needed\n");
                for(int i = 0; i < numCust; i++)
                {
                    for(int j = 0; j < customers[i].listSize; j++)
                    {
                        if(item == customers[i].itemList[j])
                        {
                            // re-bid or keep amount
                            // 50/50 same amount/bid more
                            int same = rand() % 1;      // if 0, re-bid. if 1, keep the same
                            if(same == 0)
                            {
                                customers[i].bid = rand() % customers[i].moneyFinal;
                            }
                        }
                    }
                }

                // compare bids

                // first make sure of eligibility
                for(int i = 0; i < numCust; i++)
                {
                    for(int j = 0; j < customers[i].listSize; j++)
                    {
                        if(item == customers[i].itemList[j])
                        {
                            // now compare the bids
                            for(int k = 0; k < arrLen2; k++)
                            {
                                for(int l = 0; l < arrLen2; l++)
                                {
                                    if(customers[k].bid == customers[l].bid)
                                    {
                                        // if tie, item removed, round over - message
                                        printf("No one won the auction\n");

                                    // if winner, add item, sub $, round over - message
                                    } else if (customers[k].bid > customers[l].bid) {
                                        // update variables
                                        customers[k].moneyFinal -= customers[k].bid;
                                        customers[k].wins++;
                                        moneySpent += customers[k].bid;
                                        printf("Customer %d at table %d bids %d \n", customers[k].id, tid[k], customers[k].bid);

                                        // message: who won, who lost
                                        printf("Customer %d won the auction, with amount %d\n", customers[k].id, customers[k].bid);
                                        itemsSold++;
                                    } else {
                                        // update variables
                                        customers[l].moneyFinal -= customers[l].bid;
                                        customers[l].wins++;
                                        moneySpent += customers[l].bid;
                                        printf("Customer %d at table %d bids %d \n", customers[l].id, tid[l], customers[l].bid);

                                        // message: who won, who lost
                                        printf("Customer %d won the auction, with amount %d\n", customers[l].id, customers[l].bid);
                                        itemsSold++;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        // customers to leave
        for(int i = 0; i < numCust; i++)
        {
            // qualifiers: if money is gone or if all the items on the list have been purchased
            if(customers[i].moneyFinal == 0 || customers[i].wins == customers[i].listSize)
            {
                // customer leaves
                printf("Customer %d leaves, winning %d auctions, amount of money spent = %d, amount of money left = %d \n", customers[i].id, customers[i].wins, (customers[i].moneyBegin - customers[i].moneyFinal), customers[i].moneyFinal); 

                // remove from table
                tables[i].custID = -1;
            }
        }

        // if runBonus is toggled, run at the end of the round
        if(runBonus == 1)
        {
            bonus();
        }

        // next round
        round++;
    }
/* *********** END OF GAME *************** */
    // print final information
    printf("Total Number of Items Auctioned: %d\n", maxItems);
    printf("Total Number of Items Successfully Auctioned: %d\n", itemsSold);
    printf("Total Amount: %d\n", moneySpent);


    // tell threads to quit
    quit = true;

    // join threads
    for(int i = 0; i < numTables; i++)
        pthread_join(tid[i], NULL);

    // free the memory after program has ended
    free(customers);
    free(tables);
    pthread_mutex_destroy(&mutex1);
    pthread_mutex_destroy(&mutex2);

    // exit program
    return 0;
}