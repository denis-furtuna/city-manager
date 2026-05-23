#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
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
        char string[512]="";
        while(fgets(string,512,f)!=NULL)
        {
            printf("%s",string);
        }

        fclose(f);

        if(strncmp(string,"END:",4)==0)
            printf("MONITOR si a incheiat executia\n");


    }
}

void calculate_scores(char *districte[],int contor)
{
    int pfd[contor][2];
    for(int i=0;i<contor;++i)
    {
        pipe(pfd[i]);
        int pid=fork();
        if(pid<0)
        {
            printf("Eroare la fork\n");
            exit(1);
        }
         if(pid==0)
        {

            close(pfd[i][0]);
            dup2(pfd[i][1],1);

            execl("./city_manager","city_manager","--role","manager","--user","HUB","--scorer",districte[i],NULL);
            printf("Eroare exec\n");
            exit(1);
        }
        close(pfd[i][1]);
    }

    char string[5000];
    for(int i=0;i<contor;++i)
    {
        FILE * f=fdopen(pfd[i][0],"r");
        while(fgets(string,5000,f)!=NULL)
            printf("%s",string);

        fclose(f);
    }

    for(int i=0;i<contor;++i) //cate forkuri face atatea wait uri avem
    wait(NULL);//nu ne plac zombii
}

int main()
{
    char comanda[100];
    while(1)
    {
        printf("jmek>");
        fflush(stdout);
        if(fgets(comanda,100,stdin)==NULL) break;

        comanda[strcspn(comanda,"\n")]=0;

         if(strcmp(comanda,"start_monitor")==0)
        {
            start_monitor();

        }
        else if(strstr(comanda,"calculate_scores ")==comanda) //sau strncmp
        {
            char * districte[50]; //maxim 50 districte in linia de comanda
            int contor=0;
            char sep[]=" ";
            char *p=strtok(comanda,sep);
            p=strtok(NULL, " "); //dam skip la comanda in sine
            while (p!=NULL)
            {
                if(contor==50)break;
                districte[contor++]=p;
                p=strtok(NULL," ");
            }
            calculate_scores(districte,contor);
        }
        else printf("Nu exista comanda --%s\n",comanda);
    }

    return 0;
}
