#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
void start_monitor()
{
    int pfd[2];
    int hub_mon=fork();

    if(hub_mon<0)
	{
		printf("Eroare la fork\n");
		exit(1);
	}
    if(hub_mon==0)
    {

        pipe(pfd);
        int monitor=fork();
        if(monitor<0)
        {
            printf("Eroare la fork\n");
            exit(1);
        }
        if(monitor==0)
        {

            close(pfd[0]);
            dup2(pfd[1],1);

            execl("./monitor","monitor",NULL);
            printf("Eroare exec\n");
            exit(1);
        }
        close(pfd[1]);
        FILE * f=fdopen(pfd[0],"r");
        char string[512];
        while(fgets(string,512,f)!=NULL)
            printf("%s",string);

        fclose(f);
        if(strncmp(string,"END:",4)==0)printf("MONITOR si a incheiat executia\n");
        exit(0);
    }
}

int main()
{
    char comanda[20];
    while(1)
    {
        printf("jmek>");
        fflush(stdout);
        if(fgets(comanda,20,stdin)==NULL) break;

        comanda[strcspn(comanda,"\n")]=0;

         if(strcmp(comanda,"start_monitor")==0)
        {
            start_monitor();

        }
        else if(strcmp(comanda,"calculate_scores")==0)
        {

        }
    }

    return 0;
}
