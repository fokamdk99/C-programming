#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> //unlink
#include <sys/types.h> //umask, mkfifo
#include <sys/stat.h> //umask, mkfifo
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

#define THREAD_NR 2

struct thread_data{
    int i; //jesli 0, to read, jesli 1, to write
    int kolejka; //jesli 0, to cl1, jesli 1, to cl2
};

struct thread_data thread_data_array[2];

char *fifos[] = {"FIFO1","FIFO2", "FIFO3", "FIFO4"};

void setsignalhandler(int sig, void (*handler) (int))
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = handler;
    if (sigaction(sig, &sa, NULL) == -1)
    {
        perror("sigaction");
    }
}

void handler(int sig)
{
    fprintf(stderr, "SIGPIPE");
    return;
}

void *process(void *arg)
{
    char buf_read[30];
    int fd_read, fd_write, m, n;
    struct thread_data *data;
    data = (struct thread_data *)arg;
    int liczba = data->i;
    int kolejka = data->kolejka;
    printf("watek: czekam na otwarcie lacza, liczba to: %d,kolejka to:%d\n", liczba,kolejka);//LJO
    if (liczba == 0)
    {
        if ((fd_read = open(fifos[kolejka+2], O_RDONLY)) < 0)
        {
            perror("open2");
        }
    }
    else
    {
        if ((fd_write = open(fifos[kolejka], O_WRONLY)) < 0)
        {
            perror("open1");
        }
    }
//    fprintf(stderr, "thread reads from %s and writes to %s\n", fifos[kolejka+2], fifos[kolejka]);
    if (liczba != 0) //jesli chcemy czytac stdin //LJO
    {
		fprintf(stderr, "thread reads from %s and writes to %s\n", 
			"stdin", fifos[kolejka]);//LJO
        while(1)
        {
            while ((n = read(0, buf_read, sizeof(buf_read))) > 0) 
            {
                if ((m = write(fd_write, buf_read, n)) != n) 
                {
                    fprintf(stderr, "m=%d, n=%d\n", m, n);
                    perror("fifocl:write");
                    return (void *)3;
                } 
                else 
                {
                    fprintf(stderr, "%d bytes sent to FIFO (%s)\n",
                        m, fifos[kolejka]);
                }
            }
            if (n == 0) 
            {
                fputs("EOD\n", stderr);
                return (void *)0;
            }
            if (errno != EINTR)
                break;
            fputs("EINTR\n", stderr);
        }
    }
    else //jesli chcemy czytac to, co znajduje sie w laczu
    {
		fprintf(stderr, "thread reads from %s and writes to %s\n", 
			fifos[kolejka+2], "stdout");//LJO
         while(1)
        {
            while ((n = read(fd_read, buf_read, sizeof(buf_read))) > 0) 
            {
                fprintf(stderr, "odczytano wiadomosc fd read: %s\n", buf_read);
                if ((m = write(1, buf_read, n)) != n) 
                {
                    fprintf(stderr, "m=%d, n=%d\n", m, n);
                    perror("fifocl:write");
                    return (void *)4;
                } 
                else 
                {
                    fprintf(stderr, "%d bytes  sent to stdout \n", m);
                }
            }
            if (n == 0) 
            {
                fputs("EOD\n", stderr);
                return (void *)0;
            }
            if (errno != EINTR)
                break;
            fputs("EINTR\n", stderr);
        }
    }
    
    

    return (void *)0;
}

int main(int argc, char **argv)
{
    printf("sieimano\n");
    int retcode;
    pthread_t threads[2]; //threads[0] zajmuje sie czytaniem, threads[1] zajmuje sie pisaniem
    int kolejka = atoi(argv[1]);
    void *retval;
    printf("kolejka wynosi: %d\n", kolejka);
    setsignalhandler(SIGPIPE, handler);
    for (int i = 0; i < THREAD_NR; i++)
    {
        thread_data_array[i].i = i;
        thread_data_array[i].kolejka = kolejka;
        retcode = pthread_create(&threads[i], NULL, process, (void *) &thread_data_array[i]);
    }
    for (int i = 0; i < THREAD_NR; i++)
    {
        retcode = pthread_join(threads[i], retval);
    }
    

    return 0;
}
