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

void SIGINThandler(int sig)
{
    //printf("otrzymano SIGINT\n");
    petla = 0;
    return;
}

int main(int argc, char **argv)
{
    signal(SIGINT, SIGINThandler);
    int sm;
    if ((sm = shm_open(SHAREDMEMORY, O_RDONLY, 0664)) == -1)
    {
        perror("shm_open");
        exit(1);
    }

    /*if (ftruncate(sm, sizeof(Users)) == -1)
    {
        perror("truncate");
        exit(2);
    }*/

    Users *db = mmap(NULL, sizeof(Users), PROT_READ, MAP_SHARED, sm, 0);
    if (db == MAP_FAILED)
    {
        perror("mmap");
        exit(3);
    }
    int sygnal;
    while(1)
    {
        printf("1 - uzyskaj aktualny stan pamieci\n2 - zakoncz\n");
        scanf("%d", &sygnal);
        if (sygnal == 1)
        {
            printf("usr_nr wynosi: %d\n", db->usr_nr);
            for (int i = 0; i < db->usr_nr; i++)
            {
                printf("login: %s, imie: %s, nazwisko: %s\n", db->users[i].login, db->users[i].imie, db->users[i].nazwisko);
            }
        }
        else
        {
            break;
        }
    }
    if(munmap(db, sizeof(Users)) == -1)
    {
        perror("munmap");
    }
    printf("koncze monitor\n");
    return 0;
}