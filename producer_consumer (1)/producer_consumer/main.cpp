//
// Example from: http://www.amparo.net/ce155/sem-ex.c
//
// Adapted using some code from Downey's book on semaphores
//
// Compilation:
//
//       g++ main.cpp -lpthread -o main -lm
// or 
//      make
//

#include <unistd.h>     /* Symbolic Constants */
#include <sys/types.h>  /* Primitive System Data Types */
#include <errno.h>      /* Errors */
#include <stdio.h>      /* Input/Output */
#include <stdlib.h>     /* General Utilities */
#include <pthread.h>    /* POSIX Threads */
#include <string.h>     /* String handling */
#include <semaphore.h>  /* Semaphore */
#include <iostream>



using namespace std;



//decides what mode to use based on number thingy
int changer = 0;

/*
 This wrapper class for semaphore.h functions is from:
 http://stackoverflow.com/questions/2899604/using-sem-t-in-a-qt-project
 */
class Semaphore {
public:
    // Constructor
    Semaphore(int initialValue)
    {
        sem_init(&mSemaphore, 0, initialValue);
    }
    // Destructor
    ~Semaphore()
    {
        sem_destroy(&mSemaphore); /* destroy semaphore */
    }
    
    // wait
    void wait()
    {
        sem_wait(&mSemaphore);
    }
    // signal
    void signal()
    {
        sem_post(&mSemaphore);
    }
    
    
private:
    sem_t mSemaphore;
};


// lightswitch class imported from python into c++
class Lightswitch {
public:
    Lightswitch() : counter(0), mutex(1) {}

    // Called by readers
    void lock(Semaphore &room) {
        mutex.wait();
        counter++;
        if (counter == 1) {
            room.wait();       // first reader locks out writers
        }
        mutex.signal();
    }

    // Called by readers
    void unlock(Semaphore &room) {
        mutex.wait();
        counter--;
        if (counter == 0) {
            room.signal();     // last reader lets writers back in
        }
        mutex.signal();
    }

private:
    int counter;
    Semaphore mutex;
};












/* global vars */
const int bufferSize = 5;
const int numConsumers = 5; 
const int numProducers = 5; 

/* semaphores are declared global so they can be accessed
 in main() and in thread routine. */
Semaphore Mutex(1);
Semaphore Spaces(bufferSize);
Semaphore Items(0);             

//empty rooms
Semaphore roomEmpty(1);     // Writers lock this
Semaphore turnstile(1);     // waiting room
Lightswitch readSwitch;     // Allows multiple readers in the room


//for queston2 
Lightswitch writeSwitch = Lightswitch ();
Semaphore noReaders = Semaphore (1);
Semaphore noWriters = Semaphore (1);

// dining philosphers 1
Semaphore footman(4);  // limits amount of philosphers at the table to be only 4
Semaphore forks[5] = {Semaphore(1), Semaphore(1), Semaphore(1), Semaphore(1), Semaphore(1)};



//from da book
int left(int i) {
    return i;            // philosopher i uses fork i on the left
}

int right(int i) {
    return (i + 1) % 5;  // fork to the right
}

void get_forks(int i) {
    footman.wait();
    forks[right(i)].wait();
    forks[left(i)].wait();
}

void put_forks(int i) {
    forks[right(i)].signal();
    forks[left(i)].signal();
    footman.signal();
}




/*
    Producer function 
*/
// also the writer lol
void *Producer ( void *threadID )
{
    // Thread number 
    int x = (long)threadID;

    while( 1 )
    {

      //Commented out code is orginal
        //sleep(3); // Slow the thread down a bit so we can see what is going on
        //Spaces.wait();


      // took awnser directly from the book
  if(changer ==1) {
          sleep(3);
    turnstile.wait();
        roomEmpty.wait();
                printf("Writer %d wrting \n", x);
                fflush(stdout);
        roomEmpty.signal();
    turnstile.signal();
            //Mutex.signal();
            //Items.signal();
  
  //writer priortiy
  } else if (changer == 2){

    // need the sleep or it goes crazy
    sleep(3);

  //straight from the book
   writeSwitch.lock ( noReaders );
   noWriters.wait ();
                printf("Writer %d wrting \n", x);
                fflush(stdout);
 noWriters.signal ();
 writeSwitch.unlock( noReaders );


    
    
  } else if (changer == 3) {

    int id = (long)threadID - 1;  // philosopher index 0–4

    get_forks(id);
    printf("Philosopher %d eating\n", id + 1);
    fflush(stdout);
    sleep(2);

    put_forks(id);
    printf("Philosopher %d thinking\n", id + 1);
    fflush(stdout);
    sleep(2);
    
}  else if (changer == 4) {
      //this one is a little weird cause it allows a bunch of philosphers
    // but from what i get even billies get the right  fork and oddy fellows get the left fork
 



    
    sleep(2);       
    int id = x - 1; // philosophers numbered 0-4
    int left = id;
    int right = (id + 1) % 5;

    printf("Philosopher %d thinking\n", id + 1);
    fflush(stdout);

    // if even
    if (id % 2 == 0) {  
        forks[right].wait();
        forks[left].wait();
      // if odd
    } else {        
        forks[left].wait();
        forks[right].wait();
    }

    printf("Philosopher %d eating\n", x);
    fflush(stdout);

    forks[left].signal();
    forks[right].signal();
    
    sleep(2);       


    
}


      
    }

}

/*
    Consumer function 
*/
void *Consumer ( void *threadID )
{
    // Thread number 
    int x = (long)threadID;
    
    while( 1 )
    {
      //og code commented
        //Items.wait();
       // Mutex.wait();
  if(changer ==1) {
  turnstile.wait();
  turnstile.signal();
  
  readSwitch.lock(roomEmpty);
              printf("Reader %d reading \n", x);
              fflush(stdout);
  
  readSwitch.unlock(roomEmpty);
        //Mutex.signal();
        //Spaces.signal();
        sleep(5);   // Slow the thread down a bit so we can see what is going on
    //writer prioty
  } else if (changer ==2){

  noReaders.wait ();
   readSwitch .lock ( noWriters );
  noReaders.signal ();

              printf("Reader %d reading \n", x);
              fflush(stdout);

   readSwitch.unlock ( noWriters );
  sleep(5);
    } 


      
  }

}


int main(int argc, char **argv )
{
    changer = atoi(argv[1]);   // read from command line

    pthread_t producerThread[ numProducers ];
    pthread_t consumerThread[ numConsumers ];

    // Create the producers 
    for( long p = 0; p < numProducers; p++ )
    {
        int rc = pthread_create ( &producerThread[ p ], NULL, 
                                  Producer, (void *) (p+1) );
        if (rc) {
            printf("ERROR creating producer thread # %d; \
                    return code from pthread_create() is %d\n", p, rc);
            exit(-1);
        }
    }

  if(changer < 3){
    // Create the consumers 
    for( long c = 0; c < numConsumers; c++ )
    {
        int rc = pthread_create ( &consumerThread[ c ], NULL, 
                                  Consumer, (void *) (c+1) );
        if (rc) {
            printf("ERROR creating consumer thread # %d; \
                    return code from pthread_create() is %d\n", c, rc);
            exit(-1);
        }
    }
    }

    printf("Main: program completed. Exiting.\n");


    // To allow other threads to continue execution, the main thread 
    // should terminate by calling pthread_exit() rather than exit(3). 
    pthread_exit(NULL); 


} /* main() */


