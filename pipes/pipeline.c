#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "fcntl.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#define FILE1 "pipeline_out.txt"

int main(int argc, char** argv)
{
    int pipefd[2];
    int childpid,childpid2;
    int fd;
    //char* cmd[3]={"ls",NULL,NULL};
    //char* cmd2[3]={"grep",".c",NULL};

    char **args;
    int przedzial = 0;
    args = malloc(argc * sizeof(char *));

    for (int i = 1; i < argc; i++)
    {
        args[i] = argv[i];
    }

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(args[i], "+") == 0)
        {
            przedzial = i;
        }
    }
    printf("przedzial to: %d\n", przedzial);
    char **komenda1;
    char **komenda2;
    int komenda1_len, komenda2_len;
    komenda1_len = przedzial-1+2;//przedzial-1 to liczba argumentow podanych, + 2 dla NULL, NULL
    komenda2_len = argc-3-przedzial+1;//argc-3-przedzial to liczba argumentow podanych, +1 dla NULL
    komenda1 = malloc(komenda1_len*sizeof(char*)); 
    komenda2 = malloc(komenda2_len*sizeof(char*)); 

    for (int i = 0; i < komenda1_len-2; i++)
    {
        komenda1[i] = args[i+1];
        //printf("komenda1[%d] to %s\n", i, komenda1[i]);
    }
    for (int i = 0; i < komenda2_len-1; i++)
    {
        komenda2[i] = args[i+przedzial+1];
        //printf("komenda2[%d] to %s\n", i, komenda2[i]);
    }

    komenda1[komenda1_len-2] = NULL;
    komenda1[komenda1_len-1] = NULL;
    komenda2[komenda2_len-1] = NULL;

    for (int i = 0; i < komenda1_len; i++)
    {
        if (komenda1[i] == NULL)
        {
            printf("komenda1[%d] to NULL\n", i);
        }
        else
        {
            printf("komenda1[%d] to %s\n", i, komenda1[i]);    
        }
        
        
    }
    for (int i = 0; i < komenda2_len; i++)
    {
        if (komenda2[i] == NULL)
        {
            printf("komenda2[%d] to NULL\n", i);
        }
        else
        {
            printf("komenda2[%d] to %s\n", i, komenda2[i]);    
        }
        
        
    }


    
    pipe(pipefd);
    if(childpid=fork())
    {
    //parent
    }
    else
    {  
    //child
    //write
    dup2(pipefd[1],STDOUT_FILENO);
    close(pipefd[0]);
    //execvp("ls", cmd);
    execvp(komenda1[0], komenda1);
    }
    if(childpid2=fork())
    {
    }
    else
    {
    close(pipefd[1]);
    dup2(pipefd[0],STDIN_FILENO);
    if ((fd = open(FILE1, O_WRONLY | O_CREAT, 0644)) < 0)
		{
			perror("open");
		}
	dup2(fd, STDOUT_FILENO);
    //execvp("grep",cmd2);
    execvp(komenda2[0], komenda2);
    }
    close(pipefd[0]);
    close(pipefd[1]);
    close(fd);
    printf("koncze\n");
    return 0;
}