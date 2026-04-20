#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
typedef struct
{
    int id;
    char name[30];
    float latitude;
    float longitude;
    char issue[10];
    int severity;
    time_t timestamp;
    char description[50];
}Report;

void bit_to_symbol(mode_t x)
{
    if(x & S_IRUSR)printf("r");
    else printf("-");
    if(x & S_IWUSR)printf("w");
    else printf("-");
    if(x & S_IXUSR)printf("x");
    else printf("-");

    if(x & S_IRGRP)printf("r");
    else printf("-");
    if(x & S_IWGRP)printf("w");
    else printf("-");
    if(x & S_IXGRP)printf("x");
    else printf("-");

    if(x & S_IROTH)printf("r");
    else printf("-");
    if(x & S_IWOTH)printf("w");
    else printf("-");
    if(x & S_IXOTH)printf("x");
    else printf("-");

    printf(" | ");
}

void list(char *district,char *role)
{
    char path[100]="";
    strcpy(path,district);
    strcat(path,"/reports.dat");
    struct stat vf;
    if(stat(path,&vf)!=0)
    {
        printf("Nu exista acest cartier!\n");
        exit(1);
    }

    int are_drepturi=0;
    if(strcmp(role,"manager")==0 && (vf.st_mode & S_IRUSR)) are_drepturi=1;
    else if (strcmp(role,"inspector")==0 && (vf.st_mode & S_IRGRP)) are_drepturi=1;

    if (are_drepturi==0)
    {
        printf("ACCES RESPINS: Rolul %s nu are permisiuni de citire in reports.dat!\n", role);
        exit(1);
    }

    printf("INFO: ");
    bit_to_symbol(vf.st_mode);
    printf("Dimensiunea: %ld | ",vf.st_size);
    printf("Ultima modificare: %s",ctime(&vf.st_mtime));

    int f1=open(path,O_RDONLY);
    Report r;
    while(read(f1,&r,sizeof(Report))==sizeof(Report))
    {
        printf("---------------\n");
        printf("ID: %d\n",r.id);
        printf("Name: %s\n",r.name);
        printf("Latitude: %f\n",r.latitude);
        printf("Longitude: %f\n",r.longitude);
        printf("Issue: %s\n",r.issue);
        printf("Severity: %d\n",r.severity);
        printf("Timestamp: %s",ctime(&r.timestamp));
        printf("Description: %s\n",r.description);
    }
    close(f1);

}

void add(char *district,char* role,char *user,float latitude,float longitude, char *category,int severity, char *description)
{
    ////verificam permisiunile
    char path[100]="";
    strcpy(path,district);
    strcat(path,"/reports.dat");
    struct stat vf;
    if(stat(path,&vf)==0)//stat returneaza 0 daca fisierul exista
    {
        int are_drepturi=0;
        if(strcmp(role,"manager")==0 && (vf.st_mode & S_IWUSR)) are_drepturi=1;
        else if (strcmp(role,"inspector")==0 && (vf.st_mode & S_IWGRP)) are_drepturi=1;

        if (are_drepturi==0)
        {
            printf("ACCES RESPINS: Rolul %s nu are permisiuni de scriere in reports.dat!\n", role);
            exit(1); // am facut asta aici deoarece mentioneaza in enunt sa vf inainte de orice operatie ca avem permisiuni.
        }
    }
    bool ok=false; // ma folosesc de acest ok pentru cazurile cand fisierul abia ce s a creat
    struct stat st={0};
    if(stat(district,&st)==-1) //vf daca exista fisier
    {
        mkdir(district,0750); //creeam fisier cu cartierul
        ok=true;
    }

    //////creez fisierele cartierului + scriere
    ////reports.dat
    //path e deja creat
    int f1=open(path,O_RDWR|O_CREAT|O_APPEND,0664); //o_APPEND e echivalentul lui lseek(f1,0,SEEK_END);
    fchmod(f1,0664); //chiar daca e putin ineficient ca tot dau fchmod chiar daca l am setat deja cand l am creat, poate fi si o metoda de preventie in caz de cineva umbla la permisiuni

    //creeam o variabila de tip raport si apoi ii dam write pe disc
    Report r;
    struct stat file_info;
    fstat(f1, &file_info);
    int dimensiune_fisier = file_info.st_size;
    int next_id = (dimensiune_fisier / sizeof(Report)) + 1; //generez automat id urile
    r.id=next_id;

    strncpy(r.name,user,sizeof(r.name)-1);//folosim strncpy ca sa nu dea pe dinafara
    r.name[sizeof(r.name)-1]='\0'; // ne asiguram ca se termina bine

    r.latitude=latitude;
    r.longitude=longitude;

    strncpy(r.issue,category,sizeof(r.issue)-1);
    r.issue[sizeof(r.issue)-1]='\0';

    r.severity=severity;
    r.timestamp=time(NULL); //citim timpul actual => o sa fie un nr dubios

    strncpy(r.description,description,sizeof(r.description)-1);
    r.description[sizeof(r.description)-1]='\0';

    write(f1,&r,sizeof(Report)); //scriem tot in f1
    close(f1);


    ////district.cfg
    strcpy(path,district);
    strcat(path,"/district.cfg");
    if(ok)
    {
        int f2=open(path,O_RDWR|O_CREAT,0640);
        fchmod(f2,0640);
        char buf[] = "2\n";
        write(f2, buf, strlen(buf)); //severity default value
        close(f2);
    }


    ////logged_districtint
    strcpy(path,district);
    strcat(path,"/logged_district");
    if(strcmp(role,"manager")==0)
    {
        int f3=open(path,O_RDWR|O_CREAT|O_APPEND,0644);
        fchmod(f3,0644);
        char s[100];
        char *timp_frumos=ctime(&r.timestamp);
        timp_frumos[strlen(timp_frumos)-1]='\0';
        snprintf(s,sizeof(s),"[%s] %s %s a adaugat un raport.\n",timp_frumos,role,user); //creeam stringul
        write(f3,s,strlen(s));
        close(f3);
    }








}

int main(int argc,char *argv[])
{
    if(argc<6)
    {
        printf("Clar prea putine argumente\n");
        exit(1);
    }
    char *role = argv[3];
    char *user = argv[5];
    char *comanda = argv[6];
    if(strcmp(comanda,"--add")==0)
    {
        if(argc<12)
        {
            printf("Prea putine argumente pt add\n");
            exit(1);
        }
        char *district = argv[7];
        char *category = argv[10];
        char *description = argv[12];
        float latitude = atof(argv[8]);
        float longitude = atof(argv[9]);
        int severity = atoi(argv[11]);
        add(district,role,user,latitude,longitude,category,severity,description);
        //void add(char *district,char* role,char *user,float latitude,float longitude, char *category,int severity, char *description)
        //./p --role manager --user dog --add deva 1.3 5.6 copac 2 "rip brr brr patapim"
    }
}
