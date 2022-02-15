Lauren Horton
CS 7343

Readme for Program 3

To compile and run code:
    I was using command prompt to run the following commands:
    g++ *.cpp               // compiles
    a.exe input.txt         // executes

The way I implement the bonus function for Program 3:
    I have a queue, q, that keeps track of what reader threads and writer threads are requesting to read.
    As the thread calls its specific request function, I push it to the queue. Just before allowing the thread to read or write,
    I check to see if that thread is already at the front of the queue or already has a lock before allowing it to proceed. If it
    already has a lock, it will be stored in either the hasLockR boolean array or the hasLockW array, which correspond to reader
    threads and writer threads respectively.
    Then I remove that thread from the front of the queue.

    This solves the issue of ensuring a writer won't be forever blocked by readers, but won't allow new readers to read yet.
    By checking if something already has a lock, it allows those who are reading/writing to continue doing so, and they are
    dequeued after releasing the lock.
