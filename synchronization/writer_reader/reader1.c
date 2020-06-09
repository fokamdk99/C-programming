/*
kompilacja:
gcc -o reader1 reader1.c -lrt -pthread
wywolanie: ./reader1 k
*/

    #include <stdio.h>
    #include <stdlib.h>
    #include <semaphore.h>
    #include <fcntl.h>
    #include <sys/types.h>
    #include <unistd.h>
    #include <string.h>
    #include <sys/mman.h>
    #include <signal.h>

    #define MSG_LEN 50
    #define MYFD "/shrdmemory"
    #define MSG_NR 5

    sig_atomic_t petla = 1;

    typedef struct{
    int PID;
    char msg[MSG_LEN];
    }Message;

    typedef struct{
    sem_t mutex;
    sem_t wrt;
    int readcount;
    Message messages[MSG_NR];
    }Shared;

    void ALRMhandler(int sig)
    {
        printf("otrzymano ALRM\n");
        petla = 0;
    }

    int main(int argc, char *argv[])
    {
    if (argc != 2)
    {
        printf("nieprawidlowa liczba argumentow\n");
        exit(20);
    }
    signal(SIGALRM, ALRMhandler);

    int fd, fd2;
    char *shmpath = MYFD;
    int k = atoi(argv[1]);
    char zeruj[MSG_LEN] = "wyzerowana wiadomosc";

    //open shared memory
    fd = shm_open(shmpath, O_RDWR, 0644);
    if (fd == -1)
    {
        perror("shm_open");
        exit(1);
    }
    //set size of the shared memory object to the size of structure
    if (ftruncate(fd, sizeof(Shared)) == -1)
    {
        perror("ftruncate");
        exit(2);
    }

    //map the object into the process's address space
    Shared *memory = mmap(NULL, sizeof(Shared), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (memory == MAP_FAILED)
    {
        perror("mmap");
        exit(3);
    }

    if (sem_wait(&memory->mutex) == -1)
    {
        perror("sem_wait1");
        exit(4);
    }

    memory->readcount++;
    
    if (memory->readcount == 1)
    {
        //jesli jest to pierwszy czytelnik, to czekaj az ewentualny
        //pisarz zakonczy dzialanie
        if (sem_wait(&memory->wrt) == -1)
        {
            perror("sem_wait2");
            exit(5);
        }
    }
    
    if (sem_post(&memory->mutex) == -1)
        {
            perror("sem_post1");
            exit(6);
        }

    //reading is performed
    printf("msg pid: %d, msg: %s\n", memory->messages[k].PID, memory->messages[k].msg);
    alarm(5);
    while(petla)
    {

    }
    //end of reading

    if (sem_wait(&memory->mutex) == -1)
    {
        perror("sem_wait3");
        exit(7);
    }

    memory->readcount--;
    
    memcpy(&memory->messages[k].msg, zeruj, sizeof(zeruj));
    if (memory->readcount == 0)
    {
        if (sem_post(&memory->wrt) == -1)
        {
            perror("sem_post2");
            exit(8);
        }
    }
    if (sem_post(&memory->mutex) == -1)
    {
        perror("sem_post3");
        exit(9);
    }

    /* Unlink the shared memory object. Even if the peer process
    is still using the object, this is okay. The object will be
    removed only after all open references are closed. */
    //shm_unlink(shmpath);

   
    exit(EXIT_SUCCESS);
    }