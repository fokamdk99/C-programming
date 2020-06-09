/*
1 otworz msgqueue
2 wyslij zapytanie do rejestru
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

#define SHAREDMEMORY "/smemory"
#define USR_NR 128
#define NAME_LEN 256
#define MQ_NAME "/mq1"
#define MSG_NR 10

sig_atomic_t petla = 1;
static struct sigaction sa;

void INThandler(int sig)
{
    printf("otrzymano SIGINT\n");
    petla = 0;
    return;
}

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

void ALRMhandler(int sig)
{
    printf("odebrano alarm\n");
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
    signal(SIGALRM, ALRMhandler);
    struct mq_attr attr;
    unsigned int pri;
    int ret;
    int zakoncz = 0;
    mqd_t mqueue;
    attr.mq_maxmsg = MSG_NR;
    attr.mq_msgsize = sizeof(User);
    if((mqueue = mq_open(MQ_NAME, O_RDWR, 0644, &attr)) == -1)
    {
        perror("mq_open");
        exit(1);
    }
    char login[NAME_LEN];
    pid_t PID = getpid();
    printf("Podaj login:\n");
    alarm(15);
    scanf("%s", login);
    printf("podany login to: %s\n", login);
    User *ktoto = malloc(sizeof(User));
    //ktoto->PID = getpid();
    ktoto->PID = PID;
    strncpy(ktoto->login, login, NAME_LEN);
    if (mq_send(mqueue, (const char *)ktoto, sizeof(User), 1) == -1)
    {
        perror("mq_send");
    }

    while(!zakoncz)
    {
        if((ret=mq_receive(mqueue, (char *)ktoto, sizeof(User), &pri)) == -1)
        {
            perror("mq_receive");
            pthread_exit((void*)13);
        }
        printf("wlasny PID - %d, otrzymany PID - %d\n", PID, ktoto->PID);
        if(PID == ktoto->PID)
        {
            printf("odebrano info zwrotne: imie - %s, nazwisko - %s\n", ktoto->imie, ktoto->nazwisko);
            zakoncz = 1;
        }
    }

    if (mq_close(mqueue) == -1)
    {
        perror("mq_close");
    }
    
    return 0;
}