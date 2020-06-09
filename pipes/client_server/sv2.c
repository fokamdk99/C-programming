/*
wywolanie: ./sv2
potem ./cl2 0
na koniec ./cl2 1
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> //unlink
#include <sys/types.h> //umask, mkfifo
#include <sys/stat.h> //umask, mkfifo
#include <fcntl.h>
#include <errno.h>
/*
#define FIFO1 "FIFO1" //cl1 write, sv read
#define FIFO2 "FIFO2" //cl1 read, sv write
#define FIFO3 "FIFO3" //cl2 write, sv read
#define FIFO4 "FIFO4" //cl2 read, sv write
*/
#define BUFF_NR 2 //2 bufory, z ktorych kazdy otrzymuje wiadomosc od sasiedniego cl i przesyla do niego informacje
#define PIPE_NR 4 //ilosc laczy
char *fifos[] = {"FIFO1","FIFO2", "FIFO3", "FIFO4"};

void *process(void *arg) //liczba to 0 lub 1
{
    int fd_read, fd_write, m, n;
    char buf_read[50], buf_write[50];
    long liczba = (long)arg;
    printf("otrzymalem liczbe: %ld\n", liczba);

    if (unlink(fifos[liczba]) == 0) /* removal of FIFO (if pre-existed) */
    {	
		fprintf(stderr, "Existing %s removed\n", fifos[liczba]);
	}
    if (unlink(fifos[PIPE_NR -liczba-1]) == 0) /* removal of FIFO (if pre-existed) */
    {	
		fprintf(stderr, "Existing %s removed\n", fifos[PIPE_NR -liczba-1]);
	}
    umask(0);
    if(mkfifo(fifos[liczba], 0666))
    {
        perror("mkfifo1");
    }
    if (mkfifo(fifos[PIPE_NR -liczba-1], 0666))
    {
        perror("mkfifo2");
    }
    if ((fd_read = open(fifos[liczba], O_RDONLY)) < 0) //FIFO1 lub FIFO2
    {
        perror("open1");
    }
    if ((fd_write = open(fifos[PIPE_NR -liczba-1], O_WRONLY)) < 0) //FIFO4 lub FIFO3
    {
        perror("open2");
    }
    setbuf(stderr, NULL);
    fprintf(stderr, "thread reads from %s and writes to %s\n", fifos[liczba], fifos[PIPE_NR-liczba-1]);
    fprintf(stderr, "FIFO read opened, capacity = %ld\n", fpathconf(fd_read, _PC_PIPE_BUF));
    fprintf(stderr, "FIFO write opened, capacity = %ld\n", fpathconf(fd_write, _PC_PIPE_BUF));
    while(1)
    {
        struct stat sb_read;
        if ((fstat(fd_read, &sb_read) == 0))
        {
            fprintf(stderr, "%lu bytes found in FIFO read; mtime:%s", 
            (unsigned long)sb_read.st_size,ctime(&sb_read.st_mtime));//LJO
        }
        n = read(fd_read,buf_read, sizeof(buf_read));
        fprintf(stderr, "%d bytes read from %s\n", n, fifos[liczba]);
        if (n > 0)
        {
            if ((m = write(fd_write, buf_read, n)) != n)
            {
                fprintf(stderr, "m=%d, n=%d\n", m, n);
                return (void *)2;
            }
            fprintf(stderr, "%lu bytes written to %s\n", (unsigned long)sb_read.st_size, fifos[PIPE_NR-liczba-1]);
        }
        else if (n == 0)
        {
            fputs("EOD\n", stderr);
            return 0;
        }
        else
        {
            if(errno == EINTR)
            {
                fputs("EINTR\n", stderr);
                continue;
            }
            perror("read");
            break;
        }
    }
    
    return (void *)liczba;
}

int main()
{
    for (int i = 0; i < BUFF_NR; i++)
    {
        if(unlink(fifos[i]) == 0)
        {
            fprintf(stderr, "existing %s removed\n", fifos[i]);
        }
    }

    umask(0);
    for (int i = 0; i < BUFF_NR; i++)
    {
        if (mkfifo(fifos[i], 0666))
        {
            perror("mkfifo");
        }
    }

    pthread_t threads[BUFF_NR];
    int retcode;
    void *retval;

    for (int i = 0; i < BUFF_NR; i++)
    {
        //int licz = 2*i;
        retcode = pthread_create(&threads[i], NULL, process, (void *)(long)i);
        if (retcode != 0)
        {
            fprintf(stderr, "pthread_create error %d, %d\n", i, retcode);
        }
    }

    for (int i = 0; i < BUFF_NR; i++)
    {
        retcode = pthread_join(threads[i], &retval);
        if (retcode != 0)
        {
            fprintf(stderr, "pthread_join error %d, %d", i, retcode);
        }
    }

    printf("udalo sie zakonczyc program\n");
    
    return 0;
}
