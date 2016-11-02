#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <fstream> 
#define FILENAME "output.txt"

/*
	Peter Edin
	CSE 2431
	Kelley
	WF 3:55 - 5:15
*/

FILE * buffer;
uint seed = 12345;
uint lineCount = 1;

struct buffer_item {
	int rand;
	int line_number;
};
struct thread_stuff {
	pthread_mutex_t mutex;
	sem_t writer;
	sem_t reader;
};

int write_item(int item) {

	try{
		buffer = fopen(FILENAME,"a+");
		fprintf(buffer, "%i\n",item);
		printf ("writer added %d\n", item);
		fclose(buffer);
		return 0;
	}
	catch(...){
		return -1;
	}
/* return 0 if successful, otherwise
    return -1 indicating an error condition */
}

int read_item(buffer_item *item) {
/* read a random line from buffer and place it in item*/
	try{
		std::ifstream ifs;
		std::string line;
		ifs.open (FILENAME);
		
		int count = -1;
		while(count != item->line_number)
		{
			getline( ifs, line );
			count += 1;
		}
		ifs.close();
		printf("reader read %s from line %d\n", line.c_str(), item->line_number);
		return 0;
	}
	catch(...){
		return -1;
	}

/* return 0 if successful, otherwise
    return -1 indicating an error condition */
}
void *writer(void *param) {
	int rand;
	thread_stuff *parm = (thread_stuff*) (param);
	while (1) {
		/* sleep for a random period of time */
		int time = rand_r(&seed) % 500;
		seed = seed + time;
		usleep(time * 1000);

		/* generate a random number, between 0-500 for readability purposes */
		rand = rand_r(&seed) % 500;
		pthread_mutex_lock(&(parm -> mutex));
		if (write_item(rand) < 0){
			printf("Error writing to file\n");	 // report error condition
		}
		pthread_mutex_unlock(&(parm -> mutex));
	}
}
void *reader(void *param) {
	thread_stuff *parm = (thread_stuff *) (param);
	buffer_item rand;
	while (1) {
		/* sleep for a random period of time */
		int time = rand_r(&seed) % 500;
		seed = seed + time;
		usleep(time * 1000);
		rand.line_number = time % lineCount;
		lineCount ++;
		sem_wait(&(parm -> reader));
		if (read_item(&rand) < 0){
			printf("Error reading from file\n"); // report error condition
		}
		sem_post(&(parm -> reader));
	}
}

int main(int argc, char*argv[]) {
/* 1. Get command line arguments argv[1], argv[2], argv[3] */
	int sleepTime;
	int writerCount;
	int readerCount;

	try{
		sleepTime = atoi(argv[1]);
		writerCount = atoi(argv[2]);
		readerCount = atoi(argv[3]);
	}
	catch(...){
		printf("Invalid commandline arguments, quitting.");
	}

	pthread_attr_t attr;
	pthread_attr_init(&attr);

	thread_stuff *info = (thread_stuff*)malloc(sizeof(thread_stuff));

/* 2. Initialize buffer, mutex, semaphores, and other global vars */

	sem_t readerSem;
	sem_init(&readerSem, 0, readerCount);

	sem_t writerSem;
	sem_init(&writerSem, 0, writerCount);

	pthread_mutex_t mutex;

	pthread_mutex_init(&mutex, NULL);

	info->mutex = mutex;
	info->reader = readerSem;
	info->writer = writerSem;

/* 3. Create writer thread(s) */
	for(int i = 0; i < writerCount; i++){
		pthread_t newWriter;
		pthread_create(&newWriter, &attr, writer, info);
	}
/* 4. Create reader thread(s) */
	for(int i = 0; i < readerCount; i++){
		pthread_t newReader;
		pthread_create(&newReader, &attr, reader, &info);
	}

/* 5. Sleep */
	usleep(sleepTime * 1000 * 1000);
/* 6. Release resources, e.g. destroy mutex and semaphores */
	pthread_mutex_destroy(&(info->mutex));

/* 7. Exit */
	return 0;
}
