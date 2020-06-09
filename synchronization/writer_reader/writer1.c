/*
kompilacja:
gcc -o reader1 reader1.c -lrt -pthread
wywolanie:
./writer1 k komunikat
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
        printf("otrzymano ALARM\n");
        petla = 0;
    }

    int main(int argc, char *argv[])
    {
    if (argc != 3)
    {
        printf("nieprawidlowa liczba argumentow\n");
        exit(20);
    }
    signal(SIGALRM, ALRMhandler);

    int fd, fd2;
    char *shmpath = MYFD;
    int k = atoi(argv[1]);
    char *komunikat = argv[2];

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
    

    //do stuff
    Message letter;
    //letter.PID = k+50;
    letter.PID = getpid();
    strncpy(letter.msg, komunikat,MSG_LEN);

    if (sem_wait(&memory->wrt) == -1)
    {
        perror("sem_wait");
        exit(4);
    }

    //writing is performed
    memcpy(&memory->messages[k], &letter, sizeof(letter));
    alarm(5);
    while(petla)
    {

    }

    //end of writing
    
    if (sem_post(&memory->wrt) == -1)
    {
        perror("sem_post");
        exit(5);
    }

    /* Unlink the shared memory object. Even if the peer process
    is still using the object, this is okay. The object will be
    removed only after all open references are closed. */
    //shm_unlink(shmpath);

   
    exit(EXIT_SUCCESS);
    }