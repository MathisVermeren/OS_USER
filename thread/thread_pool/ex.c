// 1 PRODUCTEUR donne des ordres de travail n°53, n°75, n°107
// N CONSOMMATEURS, LES WORKERS consomment les ordres de travail

// on a besoin d'une structure de données entre le producteur et les consommateurs.
//	FIFO, first in, first out
//		tableau avec une certaine profondeur
//	on peut écrire on peut rajouter un élément à la fifo si fifo pas pleine
//	on lire la fifo si celle-ci n'est pas vide

//	producteur		fifo		N consommateurs
//		est ce que je peux écrire
//					est-ce qu'il y a des données disponibles dans la fifo

//	fifo est un tampon, un buffer

#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t buffer_lock_mutex         = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  buffer_not_full_cond      = PTHREAD_COND_INITIALIZER;
pthread_cond_t  buffer_not_empty_cond     = PTHREAD_COND_INITIALIZER;

#define NUM_WORKERS 10

pthread_t thread_producer;
pthread_t thread_consumer[NUM_WORKERS];

typedef struct
{
        int * buffer;
        int   buf_in;
        int   buf_out;
        int   buffer_max;
}bounded_buffer_t;

bounded_buffer_t bb;

void init_buffer(int buffer_max)
{
        bb.buffer = (int*)malloc(sizeof(int)*buffer_max);
        bb.buffer_max =buffer_max;
}

int buffer_add(int item)
{
	/*
        if (bb.buf_in == INT_MAX) 
		bb.buf_in = 0;
	*/

        if ((bb.buf_in - bb.buf_out) >= 0 && abs(bb.buf_in - bb.buf_out) < bb.buffer_max)
        {
                bb.buffer[ bb.buf_in % bb.buffer_max ] = item;
                bb.buf_in ++;
                return 1;
        }
        return 0;
}

int buffer_get()
{
	/*
        if (bb.buf_out == INT_MAX) 
		bb.buf_out = 0;
	*/

        if ((bb.buf_in - bb.buf_out) > 0 && abs(bb.buf_in - bb.buf_out) < bb.buffer_max)
        {
                int val = bb.buffer[ bb.buf_out % bb.buffer_max ];
                bb.buf_out++;
                return val;
        }
        return -1;
}

int is_buffer_full()
{
        if (abs(bb.buf_out - bb.buf_in) == bb.buffer_max-1)
                return 1;
        return 0;
}

int is_buffer_empty()
{
        if (bb.buf_in == bb.buf_out)
                return 1;
        return 0;
}

void destroy_buffer()
{
        free(bb.buffer);
}

static void *producer()
{
	int workOrder=0;

	while (1)
	{
		if (pthread_mutex_lock(&buffer_lock_mutex)!=0)
                        printf("there is an error in pthread_mutex_lock in producer()\n");

		while(is_buffer_full())
		{
			if (pthread_cond_wait(&buffer_not_full_cond,&buffer_lock_mutex)!=0)
                                printf("there is an error in pthread_cond_wait in producer()\n");
                }

		//printf("producer adds workOrder=%d\n",workOrder);
                buffer_add(workOrder);
		workOrder++;

		if (pthread_cond_broadcast(&buffer_not_empty_cond)!=0)
			printf("there is an error in pthread_cond_broadcast in producer()\n");
		if (pthread_mutex_unlock(&buffer_lock_mutex)!=0)
			printf("there is an error in pthread_mutek_unlock in producer()\n");
	}
}

/* Fonction pour tirer un nombre au sort entre 0 et max. */
static int get_random (int max)
{
   double val;

   val = (double) max * rand ();
   val = val / (RAND_MAX + 1.0);

   return ((int) val);
}

void performWorkOrder(int nw,int wo,int pw)
{
	printf("%d performing workOrder=%d performedWork=%d\n",nw,wo,pw);
	sleep(get_random(10));
}

static void *consumer(void *data)
{
	int numWorker=(int)(long)data;
	int performedWork=0;

	while(1)
	{
		if (pthread_mutex_lock(&buffer_lock_mutex)!=0)
                        printf("there is an error in pthread_mutex_lock in consumer()\n");

		while (is_buffer_empty())
		{
			if (pthread_cond_wait(&buffer_not_empty_cond,&buffer_lock_mutex)!=0)
                                printf("there is an error in pthread_cond_wait in consumer()\n");
		}

                int workOrder = buffer_get();
		//printf("consumer workOrder=%d\n",workOrder);

		if (pthread_cond_signal(&buffer_not_full_cond)!=0)
			printf("there is an error in pthread_cond_signal in consumer()\n");

		if (pthread_mutex_unlock(&buffer_lock_mutex)!=0)
			printf("there is an error in pthread_mutek_unlock in consumer()\n");

		performWorkOrder(numWorker,workOrder,performedWork);
		performedWork++;
        }
}

int main(int argc, char **argv)
{
	init_buffer(5);

	int ret = pthread_create (&thread_producer, NULL, producer, NULL);
	for (int i=0;i<NUM_WORKERS;i++)
		ret = pthread_create (&thread_consumer[i], NULL, consumer, (void*)(long)i); 

	pthread_join(thread_producer,NULL);
	for (int i=0;i<NUM_WORKERS;i++)
		pthread_join(thread_consumer[i],NULL);

	return 0;
}

