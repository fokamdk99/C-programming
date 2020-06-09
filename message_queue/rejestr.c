/*
1 utworz pamiec wspoldzielona
2 utworz dwa watki poboczne: jeden do wprowadzania nowych
uzytkownikow, drugi do nasluchiwania na ktoto

*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <mqueue.h>
#include <time.h>
#include <errno.h>

#define SHAREDMEMORY "/smemory"
#define USR_NR 128
#define NAME_LEN 256
#define MQ_NAME "/mq1"
#define MSG_NR 10

sig_atomic_t petla = 1;
static struct sigaction sa;

typedef struct
{
    int id;
    pid_t PID;
    char login[NAME_LEN];
    char imie[NAME_LEN];
    char nazwisko[NAME_LEN];
}User;

typedef struct
{
    int unique_id;
    int usr_nr;
    User users[USR_NR];
}Users;

void *new_user(void *data)
{
    int shared = (int)(long)data;
    Users *db = mmap(NULL, sizeof(Users), PROT_READ | PROT_WRITE, MAP_SHARED, shared, 0);
    if (db == MAP_FAILED)
    {
        perror("mmap");
        exit(3);
    }
    while(1)
    {
        printf("1 - utworz nowego uzytkownika\n2 - zakoncz\n");
        
        /*char *end;
        char buf[100];

        int sygnal;
        fflush(stdin);
        do 
        {
            if (!fgets(buf, sizeof buf, stdin))
                break;

            // remove \n
            buf[strlen(buf) - 1] = 0;

            sygnal = strtol(buf, &end, 10);
        } while (end != buf + strlen(buf));*/

        int sygnal;
        scanf("%d", &sygnal);


        if (sygnal == 1)
        {
            printf("Tworze nowego uzytkownika: wprowadz login:\n");
            char login[NAME_LEN], imie[NAME_LEN], nazwisko[NAME_LEN];
            scanf("%s", login);
            printf("podaj imie:\n");
            scanf("%s", imie);
            printf("podaj nazwisko:\n");
            scanf("%s", nazwisko);
            printf("\n\n");
            User *usr = malloc(sizeof(User));
            usr->id = db->unique_id;
            strncpy(usr->login, login, NAME_LEN);
            strncpy(usr->imie, imie, NAME_LEN);
            strncpy(usr->nazwisko, nazwisko, NAME_LEN);
            memcpy(&db->users[db->unique_id++], usr, sizeof(User));
            db->usr_nr++;
        }
        else
        {
            break;
        }
        
    }
    kill(0, SIGINT);
    if(munmap(db, sizeof(Users)) == -1)
    {
        perror("munmap");
    }
    printf("koncze new user\n");
    
    pthread_exit(NULL);
}

void *new_message(void *data)
{
    struct timespec req_sleep_time, rem;
    int shared = (int)(long)data;
    Users *db = mmap(NULL, sizeof(Users), PROT_READ | PROT_WRITE, MAP_SHARED, shared, 0);
    if (db == MAP_FAILED)
    {
        perror("mmap");
        pthread_exit((void*)11);
    }
    printf("new message thread\n");

    struct mq_attr attr;
    unsigned int pri;
    int ret;
    int found = 0;
    mqd_t mqueue;
    attr.mq_maxmsg = MSG_NR;
    attr.mq_msgsize = sizeof(User);
    if((mqueue = mq_open(MQ_NAME, O_RDWR | O_CREAT, 0644, &attr)) == -1)
    {
        perror("mq_open");
        pthread_exit((void*)12);
    }
    User *ktoto = malloc(sizeof(User));
    while(petla)
    {
        if((ret=mq_receive(mqueue, (char *)ktoto, sizeof(User), &pri)) == -1)
        {
            perror("mq_receive");
            pthread_exit((void*)13);
        }
        if (ktoto->PID < 0)
        {
            printf("przerywam new message\n");
            break;
        }
        printf("otrzymana wiadomosc: PID - %d, login - %s\n", ktoto->PID, ktoto->login);
        for (int i = 0; i < USR_NR; i++)
        {
            if(strcmp(db->users[i].login, ktoto->login) == 0)
            {
                found = 1;
                printf("znaleziono\n");
                strncpy(ktoto->imie, db->users[i].imie, NAME_LEN);
                strncpy(ktoto->nazwisko, db->users[i].nazwisko, NAME_LEN);
                req_sleep_time.tv_sec = (rand() % 3);
                req_sleep_time.tv_nsec = 0;
                printf("spie przez %ld sekund\n", req_sleep_time.tv_sec);
                while (nanosleep(&req_sleep_time, &rem) == -1 && errno == EINTR)
                {
                    req_sleep_time.tv_sec = rem.tv_sec;
                    req_sleep_time.tv_nsec = rem.tv_nsec;
                }
                if (mq_send(mqueue, (const char *)ktoto, sizeof(User), 1) == -1)
                {
                    perror("mq_send");
                }
                break;
            }
        }
        if (found == 0)
        {
            printf("nie znaleziono\n");
        }

    }
    if (mq_close(mqueue) == -1)
    {
        perror("mq_close");
    }
    if(munmap(db, sizeof(Users)) == -1)
    {
        perror("munmap");
    }
    printf("koncze nowa wiadomosc\n");
    pthread_exit(NULL);
}

void INThandler(int sig)
{
    printf("otrzymano SIGINT\n");
    //petla = 0;
    struct mq_attr attr;
    unsigned int pri;
    int ret;
    mqd_t mqueue;
    attr.mq_maxmsg = MSG_NR;
    attr.mq_msgsize = sizeof(User);
    if((mqueue = mq_open(MQ_NAME, O_RDWR | O_CREAT, 0644, &attr)) == -1)
    {
        perror("mq_open");
        pthread_exit((void*)12);
    }
    User *ktoto = malloc(sizeof(User));
    ktoto->PID = -1;
    if (mq_send(mqueue, (const char *)ktoto, sizeof(User), 10) == -1)
    {
        perror("mq_send");
    }
    if (mq_close(mqueue) == -1)
    {
        perror("mq_close");
    }

    return;
}

int main(int argc, char **argv)
{
    sa.sa_handler = INThandler;
    sigaction(SIGINT, &sa, NULL);
    int sm, check = 0;

    if ((sm = shm_open(SHAREDMEMORY, O_RDWR, 0664)) == -1)
    {
        check = 1;
    }


    if(check == 1)
    {
        if ((sm = shm_open(SHAREDMEMORY, O_RDWR | O_CREAT, 0664)) == -1)
        {
            perror("shm_open");
            exit(1);
        }
    }
    

    if(argc == 2)
    {
        if (strcmp("-", argv[1]) == 0)
        {
            printf("usuwam pamiec wspoldzielona\n");

            //if(close(sm) == -1)
            if(shm_unlink(SHAREDMEMORY) == -1)
            {
                perror("shm_unlink");
            }
        }
        exit(EXIT_SUCCESS);
    }

    if (ftruncate(sm, sizeof(Users)) == -1)
    {
        perror("truncate");
        exit(2);
    }

    Users *database = mmap(NULL, sizeof(Users), PROT_READ | PROT_WRITE, MAP_SHARED, sm, 0);
    if (database == MAP_FAILED)
    {
        perror("mmap");
        exit(3);
    }

    if(check == 1)
    {
        database->unique_id = 0;
        database->usr_nr = 0;
    }
    
    
    pthread_t p1, p2;
    void *retval;
    if (pthread_create(&p1, NULL, new_user, (void *)(long)sm) != 0 ||
    pthread_create(&p2, NULL, new_message, (void *)(long)sm) != 0)
    {
        perror("pthread_create");
        exit(4);
    }

    pthread_join(p1, &retval);
    pthread_join(p2, &retval);
    //pthread_cancel(p1);
    if(munmap(database, sizeof(Users)) == -1)
    {
        perror("munmap");
    }
    printf("koncze program\n");



    return 0;
}