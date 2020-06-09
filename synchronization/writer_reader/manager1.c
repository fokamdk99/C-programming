//gcc -o manager1 manager1.c -lrt -pthread
    #include <stdio.h>
    #include <stdlib.h>
    #include <semaphore.h>
    #include <fcntl.h>
    #include <sys/types.h>
    #include <unistd.h>
    #include <string.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <signal.h>

    #define MSG_LEN 50 //message length
    #define MYFD "/shrdmemory" //nazwa pamieci wspoldzielonej
    #define MSG_NR 5 //liczba wiadomosci

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

    /*void ALRMhandler(int sig)
    {
    petla = 0;
    }*/

    void INThandler(int sig)
    {
        printf("koncze program\n");
        petla = 0;
        return;
    }

    int main(int argc, char *argv[])
    {
    //signal(SIGALRM, ALRMhandler);
    signal(SIGINT, INThandler);

    int fd, fd2;
    char *shmpath = MYFD;

    //create shared memory object
    //O_EXCL zwraca blad, jesli dany shm juz istnieje
    fd = shm_open(shmpath, O_CREAT | O_EXCL | O_RDWR, 0644);
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
    if (sem_init(&memory->wrt, 1, 1) == -1)
    {
        perror("sem_init1");
        exit(4);
    }
    if (sem_init(&memory->mutex, 1, 1) == -1)
    {
        perror("sem_init2");
        exit(5);
    }

    /*Message msg;
    msg.PID = 5;
    strncpy(msg.msg, "przykladowa wiadomosc2",MSG_LEN);
    memcpy(&memory->messages[0], &msg, sizeof(msg));
    if (sem_post(&memory->wrt) == -1)
    {
        perror("sem_post");
        exit(6);
    }*/
    while(petla)
    {

    }


    sem_destroy(&memory->wrt);
    sem_destroy(&memory->mutex);
    /* Unlink the shared memory object. Even if the peer process
    is still using the object, this is okay. The object will be
    removed only after all open references are closed. */
    shm_unlink(shmpath);
    printf("program zakonczony\n");

   
    exit(EXIT_SUCCESS);
    }