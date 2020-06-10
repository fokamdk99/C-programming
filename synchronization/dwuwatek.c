/*
program korzystajacy z mutexu oraz zmiennej warunku w celu synchronizacji dwoch watkow,
z ktorych kazdy wypisuje litere A lub B.
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* External Declarations	*/

pthread_mutex_t prt_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t prt_cv = PTHREAD_COND_INITIALIZER;
int prt = 0, los1, los2, koniec = 0;

void *hello_thread(void *arg)
{
    long litera2 = (long)arg;
    int litera = (int)litera2;
	while(!koniec)
    {
        los2 = (rand() %7 );
        //printf("petla2\n");
        pthread_mutex_lock(&prt_lock);
        while (prt!=1)
        {
            pthread_cond_wait(&prt_cv, &prt_lock);
        }
        printf("%c\n", litera);
        usleep(100000);
        prt = 2;
        //printf("los1: %d, los2: %d\n", los1, los2);
        if ((los1 == 6) & (los2 == 6))
        {
            koniec = 1;
        }
        pthread_cond_signal(&prt_cv);
	    pthread_mutex_unlock(&prt_lock);
    }

	pthread_exit(0);
	
}

void *world_thread(void *arg)
{
    long litera2 = (long)arg;
    int litera = (int)litera2;

    pthread_mutex_lock(&prt_lock);
	printf("%c\n", litera);
    prt = 1;
    los1 = (rand() %7 );
    
	pthread_cond_signal(&prt_cv);
	pthread_mutex_unlock(&prt_lock);

    while(!koniec)
    {
        los1 = (rand() %7 );
        //printf("petla\n");
        pthread_mutex_lock(&prt_lock);
        while (prt!=2)
        {
            pthread_cond_wait(&prt_cv, &prt_lock);
        }
        printf("%c\n", litera);
        usleep(100000);
        prt = 1;
        if ((los1 == 6) & (los2 == 6))
        {
            koniec = 1;
        }
        pthread_cond_signal(&prt_cv);
	    pthread_mutex_unlock(&prt_lock);
    }

	return (0);
}

int main(int argc, char *argv[])
{
	int n;
	pthread_attr_t attr;
	pthread_t tid;

    pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	if ((n = pthread_create(&tid, &attr, world_thread, (void *)(long)65)) > 0) {
		fprintf(stderr, "pthread_create: %s\n", strerror(n));
		exit(1);
	}

	pthread_attr_destroy(&attr);
	
	if ((n = pthread_create(&tid, NULL, hello_thread, (void *)(long)66)) > 0) {
		fprintf(stderr, "pthread_create: %s\n", strerror(n));
		exit(1);
	}

	if ((n = pthread_join(tid, NULL)) > 0) {
		fprintf(stderr, "pthread_join: %s\n", strerror(n));
		exit(1);
	}

	printf("\n");

	return (0);
}
