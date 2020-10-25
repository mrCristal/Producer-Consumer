/******************************************************************
 * The Main program with the two functions. A simple
 * example of creating and using a thread is provided.
 ******************************************************************/

#include "helper.h"
#include <deque>

void *producer (void *id);
void *consumer (void *id);

deque<Job> queue; // shared object

int JOBS_TO_CONSUME; // to be accessed by consumers only and decremented
// num 1 is for mutex, 2 is for full buffer (prod) and 3 is for empty buffer (cons)
int semsetID, mutex=0, space=1, items=2;

int PROD_ID=0;

int main (int argc, char **argv)
{
	int qSize = check_arg(argv[1]);
	int itemPerProd = check_arg(argv[2]);
	int numProds = check_arg(argv[3]);
	int numCons = check_arg(argv[4]);

	if ((qSize==0) || (itemPerProd==0) || (numProds==0) || (numCons==0))
	{
		cerr << "Invalid parameters specified. Please try again (with no 0s)"<<endl;
		return 0;
	}

	JOBS_TO_CONSUME = numProds * itemPerProd;

	semsetID = sem_create(SEM_KEY, 3); //only 3 semaphores for now
	if (semsetID == -1)
	{
		cerr<<"Could not generate semaphores. Porgramme cancelled."<<endl;
		return 0;
	}
	else {cerr << "Semaphores generated"<<endl;}

	int resultInit = sem_init(semsetID, mutex, 1);
	if (resultInit == -1)
	{
		sem_close(semsetID);
		cerr << "Could not initiate mutex semaphore. Programme cancelled"<<endl;
		return 0;
	}
	else {cerr << "Mutex semaphore initiated"<<endl;}
	
	resultInit = sem_init(semsetID, space, qSize);
	if (resultInit == -1) 
	{
		sem_close(semsetID);
		cerr << "Could not initiate space semaphore. Programme cancelled"<<endl;
		return 0;
	}
	else {cerr << "Space semaphores initiated"<<endl;}
	
	resultInit = sem_init(semsetID, items, 0);
	if (resultInit == -1)
	{
		sem_close(semsetID);
		cerr << "Could not initiate items semaphore. Programme cancelled"<<endl;
		return 0;
	}
	else {cerr << "Items semaphore initiated"<<endl;}
	
	// semaphores are initialised at this point
	
	pthread_t prod_array[numProds];
	
	for (int count = 0; count<numProds;count++)
	{	
		cerr<<"Creating new producer"<<endl;
		if(pthread_create(&prod_array[count], NULL, producer, (void *) &itemPerProd)!=0)
		{
			cerr<<"Failed to create producer thread. Programme cancelled"<<endl;
			sem_close(semsetID);
			return 0;
		}
	}

	int ids[numCons];
	pthread_t cons_array[numCons];
	for (int count = 0; count < numCons; count++)
	{	
		cerr<<"Creating new consumer"<<endl;
		ids[count] = count+1;
		if (pthread_create(&cons_array[count], NULL, consumer, (void*) &ids[count])!=0)
		{
			cerr<<"Failed to create consumer thread. Programme cancelled"<<endl;
			sem_close(semsetID);
			return 0;
		}
	}  
	
	for (int count=0; count<numProds;count++)
	{
		pthread_join(prod_array[count],NULL);
	}
	
	for (int count=0; count<numCons;count++)
	{
		pthread_join(cons_array[count],NULL);
	}
	
	sem_close(semsetID);
	
	cerr <<"Programme finished"<< endl;

	return 0;
}

void *producer (void *parameter) // parameter will be the number of jobs
{
	PROD_ID+=1;
	Job job;
	
	int *param = (int *) parameter;
	// create param nr of jobs
	int job_nr = 0, prod_id = PROD_ID;
	
	while(job_nr<*param)
	{
	
		if (sem_timed(semsetID, space)==EAGAIN)
		{// quitting if no spaces appear after 20 seconds
			cerr<<"Queue is full"<<endl;
			pthread_exit(0);
		} else { // assuming 0 here, although the returned value could also be an error
		
		if (sem_wait(semsetID, mutex)!=0)
		{// if there is an error
			sem_close(semsetID);
			cerr<<"Mutex (on down) semaphore failure in provider. Programme terminated"<<endl;
			exit(0);
		}
		//Produce job
		int id = job_nr+1, duration = rand()%10+1; // duration for each job between 1-10 seconds
		job.id = id;
		job.duration=duration;
		queue.push_front(job); // add to queue from front
		
		if (sem_signal(semsetID, mutex)!=0) // release access
		{// if there is an error
			sem_close(semsetID);
			cerr<<"Mutex (on up) semaphore failure in provider. Programme terminated"<<endl;
			exit(0);
		}
		if (sem_signal(semsetID, items)!=0) // increment item count
		{// if there is an error
			sem_close(semsetID);
			cerr<<"Items (on up) semaphore failure in provider. Programme terminated"<<endl;
			exit(0);
		}
		
		sleep(rand()%5+1); // produce every 1 - 5 seconds
		job_nr++; // increment job count
		cerr<<"Producer("<<prod_id<<") JOB ID "<<id<<" duration "<<duration<<endl;
		}
	} 
	
	cerr<<"Producer "<<prod_id<<" has no more jobs to generate"<<endl;
	pthread_exit(0);
}

void *consumer (void *id) 
{
	Job job;
	
	int* consumer_id = (int*) id;

	while (JOBS_TO_CONSUME>0) 
	{	
		
		if (sem_timed(semsetID, items)==EAGAIN)
		{// quitting if no spaces appear after 20 seconds
			cerr<<"Queue is empty"<<endl;
			pthread_exit(0);
		} else {
		
			if(sem_wait(semsetID, mutex)!=0) // check access
			{
				sem_close(semsetID);
				cerr<<"Items (on down) semaphore failure in consumer. Programme terminated"<<endl;
				exit(0);
			}		
			job = queue.back();
			queue.pop_back();
			JOBS_TO_CONSUME--;
			if (sem_signal(semsetID, mutex)!=0)
			{
				sem_close(semsetID);
				cerr<<"Mutex (on up) semaphore failure in consumer. Programme terminated"<<endl;
				exit(0);
			}		
			if(sem_signal(semsetID, space)!=0)
			{
				sem_close(semsetID);
				cerr<<"Space (on up) semaphore failure in consumer. Programme terminated"<<endl;
				exit(0);
			}		
			cerr<< "Consumer("<<*consumer_id<<"): JOB ID "<<job.id
				<<" executing sleep duration "<<job.duration<<endl; 
			sleep(job.duration); 
			cerr<<"JOB ID "<<job.id<<" completed"<<endl;
		}
	}
	
	cerr<<"No more jobs left"<<endl;
	pthread_exit (0);
}
