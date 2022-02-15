#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <pthread.h>
#include <ctime>
#include <fstream>
#include <vector>
#include "queue.cpp"
#include "graph.h"
using namespace std;

// global variables
int r;                      // number of readers
int s;                      // number of writers
int n;                      // size of resource array
int toStart;                // toStart variable
int rw;                     // num readers + num writers
int* resource;              // the resource array
int* readCount;             // read count array
Queue q;                    // queue for bonus function
pthread_mutex_t rw_mutex;   // mutex for determining reading/writing
pthread_mutex_t d_mutex;    // mutex to guard deadlock algorithm
pthread_mutex_t* rc_mutex;  // mutex to guard read count array update
bool* hasLockR;              // boolean to see if reader has lock
bool* hasLockW;              // boolean to see if writer has lock

struct item {               // struct of items - keeps a counter
    int it;                 // of how many times read/written
    int count;
};
struct reader {             // reader struct
    int readerID;
    int k;
    struct item* read;
    int gID;
};
struct writer {             // writer struct
    int writerID;
    int k;
    struct item* write;
    int gID;
};
reader* readers;            // array of readers
writer* writers;            // array of writers

Graph g;                    // resource allocation graph
/* graph of size r + s + n -> total vertices
 vertices 0-(r-1) refer to readers
 vertices r-(r+s-1) refer to writers
 vertices (r+s)-(r+s+n-1) refer to resource */

/* ******************** Request_Read ***************** */

bool request_read(int reader, int indexToRead)
{
    bool canRead = false;
    cout << "REQUEST READ called by thread " << reader << " and item " << indexToRead << endl;
    // BONUS FUNCTION: QUEUEING -> add reader to back of queue
    q.enQueue(reader);

    // algorithm for single resource deadlock avoidance -> if graph has cycle, deny request
    // critical section: checking deadlock
    pthread_mutex_lock(&d_mutex);
    g.addEdge(indexToRead+rw-1, readers[reader].gID);           // edge turned to assignment
        // passing edge to graph: (resource, reader)
        // calculate graph id by adding variable rw to it

    if(g.isCyclic()){
        canRead = false;
        // remove assignment edge if request not granted
        g.removeEdge(indexToRead+rw-1);
    } else {
        canRead = true;
        // but check queue to make sure it's allowed to go ahead and read
        // see if it's at front of queue or already has lock
        if(q.front->data == reader || hasLockR[reader-1])
        {
            // critical section: updating read count
            pthread_mutex_lock(&rc_mutex[indexToRead]);
            readCount[indexToRead]+=1;
            // critical section: can read and is reading
                // if first one to read it
            if(readCount[indexToRead] == 1)
            {
                pthread_mutex_lock(&rw_mutex);
                hasLockR[reader-1] = true;
            }
        }
    }
    pthread_mutex_unlock(&d_mutex);                     // end deadlock critical section
    pthread_mutex_unlock(&rc_mutex[indexToRead]);       // end read count critical section

    // output and return
    cout << "Reader " << readers[reader].readerID << " request to read " << indexToRead << ". ";
    if(canRead)
        cout << "Granted" << endl;
    else
        cout << "Denied" << endl;
    return canRead;
}

/* ******************** Request_Write **************** */
bool request_write(int writer, int indexToWrite)
{
    bool canWrite = false;
    cout << "REQUEST WRITE called by thread " << writer << endl;
    // BONUS FUNCTION: queue writer
    q.enQueue(writer);

    // algorithm for single resource deadlock avoidance -> if graph has cycle, deny request

    // critical section: checking deadlock
    pthread_mutex_lock(&d_mutex);
    g.addEdge(indexToWrite+rw-1, writers[(writer/10)].gID+r-1);

    if(g.isCyclic()){
        canWrite = false;
        g.removeEdge(indexToWrite+rw-1);
    } else {
        canWrite = true;
        // but check to make sure it's allowed to write
        // see if it's at front of queue or already has a lock
        if(q.front->data == writer || hasLockW[(writer/10)-1])
        {
            // critical section: can read and is reading
            pthread_mutex_lock(&rw_mutex);
            hasLockW[(writer/10)-1] = true;
        }

    }
    pthread_mutex_unlock(&d_mutex);                     // end deadlock critical section

    // output and return
    cout << "Writer " << writers[(writer/10)].writerID << " request to read " << indexToWrite << ". ";
    if(canWrite)
        cout << "Granted" << endl;
    else
        cout << "Denied" << endl;
    return canWrite;
}

/* ******************** Release_Read ***************** */
void release_read(int reader, int indexToRead)
{
    // done reading item
    // critical section: updating readCount
    pthread_mutex_lock(&rc_mutex[indexToRead]);
    readCount[indexToRead]-=1;
    // check queue
    if(q.front->data == reader)
    {
        // if first one to read it
        if(readCount[indexToRead] == 0)
        {
            g.removeEdge(indexToRead+rw-1);
            pthread_mutex_unlock(&rw_mutex);
            hasLockR[reader] = false;             // reset hasLock
        }
        // remove from queue
        q.deQueue();
    }

    pthread_mutex_unlock(&rc_mutex[indexToRead]);

    // output
    cout << "Reader " << readers[reader].readerID << " released " << indexToRead << endl;
}


/* ******************** Release_Write **************** */
void release_write(int writer, int indexToWrite)
{
    // done reading item
    // check queue
    if(q.front->data == writer)
    {
        g.removeEdge(indexToWrite+rw-1);
        pthread_mutex_unlock(&rw_mutex);
        hasLockW[writer/10] = false;                 // reset hasLock
        // remove from queue
        q.deQueue();
    }

    // output
    cout << "Writer " << writers[(writer/10)].writerID << " released " << indexToWrite << endl;
}

/* ******************** mySleep ********************** */
// based on provided code
void mySleep()
{
    int s, ms;
    struct timespec t0, t1;

    s = rand() % 3;
    ms = rand()  % 1000;
    if (s == 0 && ms < 500)
	    ms += 500;
    t0.tv_sec = s;
    t0.tv_nsec = ms * 1000000;
    nanosleep(&t0, &t1);

}

/* ******************** Reader Thread **************** */
void *rthread(void *param){
    // initialize
    int id = *((int*) param);
    // wait for toStart = 1
    while(toStart != 1)
    {

    }

    while(true)
    {
        // randomly select item to read
        int index = rand() % readers[id].k;

        // call request read
        bool request = request_read(id, readers[id].read[index].it);

        //if request granted
        if(request)
        {
            // print
            cout << "Reader " << readers[id].readerID << ": the value of item " << readers[id].read[index].it << " is " << resource[index] << endl;
            // also add to count
            readers[id].read[index].count++;
        }

        // mySleep()
        mySleep();

        // random number between 0-2
        int x = rand() % 2;
        // if num == 0
        if(x == 0)
        {
            // call release_read
            release_read(id, index);
        }

        // if every item has been read at least once
        int readOnce = 0;
        for(int i = 0; i < readers[id].k; i++)
        {
            if(readers[id].read[i].count >= 1)
            {
                readOnce++;
            }
        }
        if(readOnce == readers[id].k)
        {
            // random num between 0-4
            int y = rand() % 4;
            // if num == 0
            if(y == 0)
            {
                // exit loop
                break;
            }
        }
    }

    // call mySleep()
    mySleep();

    // post-processing

    // print
    cout << "Reader " << readers[id].readerID << " finished." << endl;

    // exit
    pthread_exit(0);
}



/* ******************** Writer Thread **************** */
void *wthread(void *param){
    // initialize
    int id = *((int*) param);
    // wait for toStart = 1
    while(toStart != 1)
    {

    }

    while(true)
    {
        // randomly select item to write
        int index = rand() % writers[id/10].k;

        // call request write
        bool request = request_write(id, writers[id/10].write[index].it);

        //if request granted
        if(request)
        {
            // change value = id of writer + random num btwn 0-10
            int num = rand() % 10;
            resource[index] = writers[id/10].writerID + num;
            // print
            cout << "Writer " << writers[id/10].writerID << ": the value of item " << writers[id/10].write[index].it << " is updated to " << resource[index] << endl;
            // also add to count
            writers[id/10].write[index].count++;

            // mySleep()
            mySleep();

            // random number between 0-2
            int x = rand() % 2;
            // if num == 0
            if(x == 0)
            {
                // call release_write
                release_write(id, index);
            }
        } else {
            // call mySleep()
            mySleep();
        }

        // if every item has been written at least once
        int writtenOnce = 0;
        for(int i = 0; i < writers[id/10].k; i++)
        {
            if(writers[id/10].write[i].count >= 1)
            {
                writtenOnce++;
            }
        }
        if(writtenOnce == writers[id/10].k)
        {
            // random num between 0-5
            int y = rand() % 5;
            // if num == 0
            if(y == 0)
            {
                // release_write for every item not released
                release_write(id, index);
                // exit loop
                break;
            }
        }
    }

    // call mySleep()
    mySleep();

    // post-processing

    // print
    cout << "Writer " << writers[id/10].writerID << " finished." << endl;

    // exit
    pthread_exit(0);
}

/* ******************** Main ************************* */
int main(int argc, char *argv[]) {
    // initialization
    srand(time(0));

    /* NOTE ABOUT JRAND48 - Professor Lin said it was fine to
        not use it in office hours on 4/19/21 */

    // read input file
    ifstream filename;
    filename.open(argv[1]);

        // get first line and assign variables
    filename >> r >> s >> n;
    resource = (int*)calloc(n, sizeof(int*));
    g = Graph(r+s+n);
    rw = r+s;

        // get the readers
    readers = (reader*)malloc(r * sizeof(*readers));
    for(int i = 0; i < r; i++)
    {
        filename >> readers[i].readerID >> readers[i].k;                    // get readerID, length of reader list
        cout << "id: " << readers[i].readerID << endl;
        readers[i].read = (item*)malloc(readers[i].k * sizeof(item*));      // allocate reader list
        readers[i].gID = i;                                                 // create graphID for reader (0 to r-1)
        for(int j = 0; j < readers[i].k; j++)
        {
            filename >> readers[i].read[j].it;                              // get next item for reader list
            g.addEdge(readers[i].gID, readers[i].read[j].it+rw-1);          // add claim edge
            readers[i].read[j].count = 0;
            cout << readers[i].read[j].it << endl;
        }
    }

        // get the writers
    writers = (writer*)malloc(s * sizeof(*writers));
    for(int i = 0; i < s; i++)
    {
        filename >> writers[i].writerID >> writers[i].k;                    // get writerID, length of writer list
        cout << "id: " << writers[i].writerID << endl;
        writers[i].write = (item*)malloc(writers[i].k * sizeof(item*));     // allocate writer list
        writers[i].gID = i+r;                                               // create graphID for writer (r to r+s-1)
        for(int j = 0; j < writers[i].k; j++)
        {
            filename >> writers[i].write[j].it;                             // get next item for writer list
            g.addEdge(writers[i].gID+r-1, writers[i].write[j].it+rw-1);     // add claim edge
            writers[i].write[j].count = 0;
            cout << writers[i].write[j].it << endl;
        }
    }

    // pre-processing
    pthread_t *rtid;
    pthread_t *wtid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    rtid = (pthread_t*)malloc(sizeof(pthread_t) * r);
    wtid = (pthread_t*)malloc(sizeof(pthread_t) * s);
    readCount = (int*)calloc(n, sizeof(int*));
    rc_mutex = (pthread_mutex_t*)malloc(n*sizeof(pthread_mutex_t*));
    pthread_mutex_init(&rw_mutex, NULL);
    pthread_mutex_init(&d_mutex, NULL);

    // boolean arrays init
    hasLockR = (bool*)calloc(r, sizeof(bool*));
    for(int i = 0; i < r; i++)
    {
        hasLockR[i] = false;
    }
    hasLockW = (bool*)calloc(r, sizeof(bool*));
    for(int i = 0; i < s; i++)
    {
        hasLockW[i] = false;
    }

    // initializing array of mutexes for read count
    for(int i = 0; i < n; i++)
    {
        pthread_mutex_init(&(rc_mutex[i]), NULL);
    }

    // set toStart
    toStart = 0;

    // create threads
        // readers
    for(int i = 0; i < r; i++)
    {
        pthread_create(&(rtid[i]), &attr, rthread, &i);
    }
        // writers
    for(int i = 0; i < s; i++)
    {
        int x = i*10;
        pthread_create(&(wtid[i]), &attr, wthread, &x);
    }

    // call mySleep
    mySleep();

    // set toStart
    toStart = 1;

    // wait for threads

    // print done
    cout << "All Done!" << endl;

    // exit
        // readers
    for(int i = 0; i < r; i++)
        pthread_join(rtid[i], NULL);

        // writers
    for(int i = 0; i < r; i++)
        pthread_join(wtid[i], NULL);

    free(readers);
    free(writers);
    free(resource);
    pthread_mutex_destroy(&rw_mutex);
    pthread_mutex_destroy(&d_mutex);
    for(int i = 0; i < n; i++)
    {
        pthread_mutex_destroy(&(rc_mutex[i]));
    }

    return 0;
}