#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
void tratare(int p)
{
    write(1,"A fost adaugat un nou report\n",29);
    //printf("A fost adaugat un nou report\n"); nu folisim printf din cauza ca functia e asincrona si se paote bugui programul - deadlock
}
void tratare_moarte(int p)
{
    unlink(".monitor_pid");
    write(1,"END: Gata cu programul, gata cu .monitor_pid\n",40);
    //printf("Gata cu programul, gata cu .monitor_pid\n");
    exit(0);//0 - succes, 1- eroare
}
int main()
{
    int pid=getpid();

    char path[100]="";
    strcpy(path,".monitor_pid");

    struct stat vf;
    if(lstat(".monitor_pid",&vf)==0)
    {
        printf("Exisata deja un monitor deschis!\n");
        exit(1);
    }
    int f1=open(path,O_WRONLY|O_CREAT|O_TRUNC,0644);
    if(f1==-1)
    {
        printf("Eroare deschidere fisier\n");
        exit(1);
    }
    char buf[10];
    sprintf(buf,"%d\n",pid);
    write(f1,buf,strlen(buf));
    close(f1);

    struct sigaction sig;
    sig.sa_handler=tratare;
    sigemptyset(&sig.sa_mask);
    sig.sa_flags=0;
    sigaction(SIGUSR1,&sig,NULL);

    struct sigaction sig_end;
    sig_end.sa_handler=tratare_moarte;
    sigemptyset(&sig_end.sa_mask); //in caz de vin alte semnale, nu le blocam
    sig_end.sa_flags=0; //n avem reguli speciale precum SA_SIGINFO
    sigaction(SIGINT,&sig_end,NULL);
    while(1)
    {
        pause();//asteptam un semnal
    }

    return 0;
}
