#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t mutex01         = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond_ready01      = PTHREAD_COND_INITIALIZER;
pthread_cond_t  cond_avail01     = PTHREAD_COND_INITIALIZER;

pthread_mutex_t mutex12         = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond_ready12      = PTHREAD_COND_INITIALIZER;
pthread_cond_t  cond_avail12     = PTHREAD_COND_INITIALIZER;

pthread_mutex_t mutex23         = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond_ready23      = PTHREAD_COND_INITIALIZER;
pthread_cond_t  cond_avail23     = PTHREAD_COND_INITIALIZER;

int used01;
int used12;
int used23;

pthread_t thread[4];

static void *thread0()
{
	while (1)
	{
		if (pthread_mutex_lock(&mutex01)!=0)
                      printf("there is an error in pthread_mutex_lock in thread0()\n");

		// On attend que l'étage 1 soit prêt
		while (used01==1)
		{
			if (pthread_cond_wait(&cond_ready01,&mutex01)!=0)
				printf("there is an error in pthread_cond_wait in thread0()\n");
		}

		// On envoie à l'étage 1
		printf("produire thread0\n");
		used01=1;

		if (pthread_cond_signal(&cond_avail01)!=0)
			printf("there is an error in pthread_cond_signal in thread0()\n");

		if (pthread_mutex_unlock(&mutex01)!=0)
                      printf("there is an error in pthread_mutex_unlock in thread0()\n");

	}
}

static void *thread1()
{
	while (1)
	{
		// Partie 1, on cherche à récupérer une donnée depuis l'étape précédent

		if (pthread_mutex_lock(&mutex01)!=0)
                      printf("there is an error in pthread_mutex_lock in thread1()\n");

		// On attend que l'étage 0 nous envoie une donnée 
		while (used01==0)
		{
			if (pthread_cond_wait(&cond_avail01,&mutex01)!=0)
				printf("there is an error in pthread_cond_wait in thread1()\n");
		}

		// On lit la donnée éventuellement
		used01=0;

		if (pthread_cond_signal(&cond_ready01)!=0)
			printf("there is an error in pthread_cond_signal in thread1()\n");

		if (pthread_mutex_unlock(&mutex01)!=0)
                      printf("there is an error in pthread_mutex_unlock in thread1()\n");

		// Partie 2, on cherche à propager

		if (pthread_mutex_lock(&mutex12)!=0)
                      printf("there is an error in pthread_mutex_lock in thread1()\n");

		// On attend que l'étage 2 soit prêt
		while (used12==1)
		{
			if (pthread_cond_wait(&cond_ready12,&mutex12)!=0)
				printf("there is an error in pthread_cond_wait in thread1()\n");
		}

		// On envoie à l'étage 2
		printf("produire thread1\n");
		used12=1;

		if (pthread_cond_signal(&cond_avail12)!=0)
			printf("there is an error in pthread_cond_signal in thread1()\n");

		if (pthread_mutex_unlock(&mutex12)!=0)
                      printf("there is an error in pthread_mutex_unlock in thread1()\n");
	}
}

static void *thread2()
{
	while (1)
	{
		// Partie 1, on cherche à récupérer une donnée depuis l'étape précédent 1

		if (pthread_mutex_lock(&mutex12)!=0)
                      printf("there is an error in pthread_mutex_lock in thread2()\n");

		// On attend que l'étage 1 nous envoie une donnée 
		while (used12==0)
		{
			if (pthread_cond_wait(&cond_avail12,&mutex12)!=0)
				printf("there is an error in pthread_cond_wait in thread2()\n");
		}

		// On lit la donnée éventuellement
		used12=0;

		if (pthread_cond_signal(&cond_ready12)!=0)
			printf("there is an error in pthread_cond_signal in thread2()\n");

		if (pthread_mutex_unlock(&mutex12)!=0)
                      printf("there is an error in pthread_mutex_unlock in thread2()\n");

		// Partie 2, on cherche à propager

		if (pthread_mutex_lock(&mutex23)!=0)
                      printf("there is an error in pthread_mutex_lock in thread1()\n");

		// On attend que l'étage 3 soit prêt
		while (used23==1)
		{
			if (pthread_cond_wait(&cond_ready23,&mutex23)!=0)
				printf("there is an error in pthread_cond_wait in thread2()\n");
		}

		// On envoie à l'étage 3
		printf("produire thread2\n");
		used23=1;

		if (pthread_cond_signal(&cond_avail23)!=0)
			printf("there is an error in pthread_cond_signal in thread2()\n");

		if (pthread_mutex_unlock(&mutex23)!=0)
                      printf("there is an error in pthread_mutex_unlock in thread2()\n");
	}
}

static void *thread3()
{
	while (1)
	{
		// Partie 1, on cherche à récupérer une donnée depuis l'étape précédent 2

		if (pthread_mutex_lock(&mutex23)!=0)
                      printf("there is an error in pthread_mutex_lock in thread3()\n");

		// On attend que l'étage 2 nous envoie une donnée 
		while (used23==0)
		{
			if (pthread_cond_wait(&cond_avail23,&mutex23)!=0)
				printf("there is an error in pthread_cond_wait in thread3()\n");
		}

		// On lit la donnée éventuellement
		printf("afficher thread3\n");
		used23=0;

		if (pthread_cond_signal(&cond_ready23)!=0)
			printf("there is an error in pthread_cond_signal in thread3()\n");

		if (pthread_mutex_unlock(&mutex23)!=0)
                      printf("there is an error in pthread_mutex_unlock in thread3()\n");
	}
}

int main(int argc, char **argv)
{

	used01=0;
	used12=0;
	used23=0;
	int ret = pthread_create (&thread[0], NULL, thread0, NULL);
	ret = pthread_create (&thread[1], NULL, thread1, NULL);
	ret = pthread_create (&thread[2], NULL, thread2, NULL);
	ret = pthread_create (&thread[3], NULL, thread3, NULL);

	for (int i=0;i<4;i++)
		pthread_join(thread[i],NULL);

	return 0;
}

