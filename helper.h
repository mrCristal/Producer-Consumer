/******************************************************************
 * Header file for the helper functions. This file includes the
 * required header files, as well as the function signatures and
 * the semaphore values (which are to be changed as needed).
 ******************************************************************/


# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/ipc.h>
# include <sys/shm.h>
# include <sys/sem.h>
# include <sys/time.h>
# include <math.h>
# include <errno.h>
# include <string.h>
# include <pthread.h>
# include <ctype.h>
# include <iostream>
using namespace std;

# define SEM_KEY 0x89137747 //0x50 // Change this number as needed

union semun {
    int val;               /* used for SETVAL only */
    struct semid_ds *buf;  /* used for IPC_STAT and IPC_SET */
    ushort *array;         /* used for GETALL and SETALL */
};

struct Job
{	
	int id;
	int duration;
};

int check_arg (char *);
int sem_create (key_t, int);
int sem_init (int, int, int);
int sem_wait (int, short unsigned int);
int sem_signal (int, short unsigned int);
int sem_close (int); // remove semaphore set
int sem_timed(int id, short unsigned int num);

// int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
// arg is passed to start routine
