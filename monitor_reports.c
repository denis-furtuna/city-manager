#include <stdio.h>

void tratare(int p)
{
    printf("ceva\n");
    exit(1);
}
int main()
{
    int pid=getpid();

    char path[100]="";
    strcpy(path,".monitor_pid");
    int f1=open(path,O_WRONLY|O_CREAT|O_APPEND|O_TRUNC,0644);
    if(f1==-1)
    {
        printf("Eroare deschidere fisier\n");
        exit(1);
    }
    fprintf(f1,"%d\n",pid);
    close(f1);

    struct sigaction sig;
    sig.sa_handler=tratare;
    sigemptyset(&sig.sa_mask);
    sig.sa_flags=0;
    sigaction(SIGUSR1,&sig,NULL);

    return 0;
}
