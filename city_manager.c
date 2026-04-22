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

void list(char *district,char *role,char *user)
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

     ////logged_districtint
    if(strcmp(role,"manager")==0)
    {
        strcpy(path,district);
        strcat(path,"/logged_district");
        int f3=open(path,O_RDWR|O_CREAT|O_APPEND,0644);
        fchmod(f3,0644);
        char s[100];
        time_t timestamp=time(NULL);
        char *timp_frumos=ctime(&timestamp);
        timp_frumos[strlen(timp_frumos)-1]='\0';
        snprintf(s,sizeof(s),"[%s] %s %s a vizualizat districtul %s.\n",timp_frumos,role,user,district); //creeam stringul
        write(f3,s,strlen(s));
        close(f3);
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
            printf("Rolul %s nu are permisiuni de scriere in reports.dat!\n", role);
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

    if(severity>3)severity=3;
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
    if(strcmp(role,"manager")==0)
    {
        strcpy(path,district);
        strcat(path,"/logged_district");
        int f3=open(path,O_RDWR|O_CREAT|O_APPEND,0644);
        fchmod(f3,0644);
        char s[100];
        time_t timestamp=time(NULL);
        char *timp_frumos=ctime(&timestamp);
        timp_frumos[strlen(timp_frumos)-1]='\0';
        snprintf(s,sizeof(s),"[%s] %s %s a adaugat un raport.\n",timp_frumos,role,user); //creeam stringul
        write(f3,s,strlen(s));
        close(f3);
    }

}

void remove_report(char *district,char *role,char *user,int id)
{
    if(strcmp(role,"manager")!=0)return ;

    Report temp;
    off_t write_pos;
    off_t read_pos;
    char path[100]="";
    strcpy(path,district);
    strcat(path,"/reports.dat");
    struct stat vf;
    if(stat(path,&vf)!=0)return ;

    int are_drepturi=0;
    if(strcmp(role,"manager")==0 && (vf.st_mode & S_IRUSR) && (vf.st_mode & S_IWUSR)) are_drepturi=1;


    if (are_drepturi==0)
    {
        printf("Rolul %s nu are permisiuni de citire/scriere in reports.dat!\n", role);
        exit(1);
    }

    int f1=open(path,O_RDWR);
    bool ok=false;
    while(read(f1,&temp,sizeof(Report))==sizeof(Report))
    {
        if(!ok && temp.id==id)
        {
            ok=true;
            write_pos=lseek(f1,0,SEEK_CUR)-sizeof(Report);
            read_pos=write_pos+sizeof(Report);
        }
        else if(ok)
        {
            temp.id--;
            lseek(f1,write_pos,SEEK_SET);
            write(f1,&temp,sizeof(Report));

            write_pos+=sizeof(Report);
            read_pos+=sizeof(Report);

            lseek(f1,read_pos,SEEK_SET);

        }
    }
    if(ok)ftruncate(f1,vf.st_size-sizeof(Report));

     ////logged_districtint
    if(strcmp(role,"manager")==0)
    {
        strcpy(path,district);
        strcat(path,"/logged_district");
        int f3=open(path,O_RDWR|O_CREAT|O_APPEND,0644);
        fchmod(f3,0644);
        char s[100];
        time_t timestamp=time(NULL);
        char *timp_frumos=ctime(&timestamp);
        timp_frumos[strlen(timp_frumos)-1]='\0';
        snprintf(s,sizeof(s),"[%s] %s %s a sters un raport.\n",timp_frumos,role,user); //creeam stringul
        write(f3,s,strlen(s));
        close(f3);
    }

    close(f1);
}

void view(char *district,char* role,char *user,int id)
{
    Report r;
    char path[100]="";
    strcpy(path,district);
    strcat(path,"/reports.dat");
    struct stat vf;
    if(stat(path,&vf)!=0)
    {
        printf("Nu exista districtul %s\n",district);
        return ;
    }

    int are_drepturi=0;
    if(strcmp(role,"manager")==0 && (vf.st_mode & S_IRUSR)) are_drepturi=1;
    else if (strcmp(role,"inspector")==0 && (vf.st_mode & S_IRGRP)) are_drepturi=1;

    if (are_drepturi==0)
    {
        printf("Rolul %s nu are permisiuni de citire in reports.dat!\n", role);
        exit(1);
    }

    int f1=open(path,O_RDONLY);
    while(read(f1,&r,sizeof(Report))==sizeof(Report))
    {
        if(r.id==id)
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
            close(f1);
            return ;
        }
    }
    printf("Nu exista raport cu id-ul %d\n",id);

     ////logged_districtint
    if(strcmp(role,"manager")==0)
    {
        strcpy(path,district);
        strcat(path,"/logged_district");
        int f3=open(path,O_RDWR|O_CREAT|O_APPEND,0644);
        fchmod(f3,0644);
        char s[100];
        time_t timestamp=time(NULL);
        char *timp_frumos=ctime(&timestamp);
        timp_frumos[strlen(timp_frumos)-1]='\0';
        snprintf(s,sizeof(s),"[%s] %s %s a vizualizat districtul %s cu id-ul %d.\n",timp_frumos,role,user,district,id); //creeam stringul
        write(f3,s,strlen(s));
        close(f3);
    }

    close(f1);
}

void update_threshold(char *district,char *role,char *user,int value)
{
    if(strcmp(role,"manager")!=0)return ;
    char path[100]="";
    strcpy(path,district);
    strcat(path,"/district.cfg");
    struct stat vf;
    if(stat(path,&vf)!=0)
    {
        printf("Nu exista fisierul de configurare/districtul %s\n",district);
        return ;
    }

    if((vf.st_mode & 0777) != 0640)
    {
        printf("Fisierul nu are permisiunile bune\n");
        exit(1);
    }

    int f1=open(path,O_WRONLY | O_TRUNC);
    if(f1==-1)
    {
        printf("Eroare deschidere fisier");
        return;
    }

    char buf[50];
    snprintf(buf,sizeof(buf),"%d\n",value);
    write(f1,buf,strlen(buf));

     ////logged_districtint
    if(strcmp(role,"manager")==0)
    {
        strcpy(path,district);
        strcat(path,"/logged_district");
        int f3=open(path,O_RDWR|O_CREAT|O_APPEND,0644);
        fchmod(f3,0644);
        char s[100];
        time_t timestamp=time(NULL);
        char *timp_frumos=ctime(&timestamp);
        timp_frumos[strlen(timp_frumos)-1]='\0';
        snprintf(s,sizeof(s),"[%s] %s %s a dat update la threshold-ul.\n",timp_frumos,role,user); //creeam stringul
        write(f3,s,strlen(s));
        close(f3);
    }

    close(f1);
}

int main(int argc,char *argv[])
{
    if(argc<6)
    {
        printf("Clar prea putine argumente\n");
        exit(1);
    }
    char *role = argv[2];
    char *user = argv[4];
    char *comanda = argv[5];
    if(strcmp(comanda,"--add")==0)
    {
        if(argc<12)
        {
            printf("Prea putine argumente pentru add\n");
            exit(1);
        }
        char *district = argv[6];
        char *category = argv[9];
        char *description = argv[11];
        float latitude = atof(argv[7]);
        float longitude = atof(argv[8]);
        int severity = atoi(argv[10]);
        add(district,role,user,latitude,longitude,category,severity,description);

    }
    else if(strcmp(comanda,"--list")==0)
    {
        if(argc<7)
        {
            printf("Prea putine argumente pentru list\n");
            exit(1);
        }
        char *district = argv[6];
        list(district,role,user);
    }
    else if(strcmp(comanda,"--remove_report")==0)
    {
        if(argc<8)
        {
            printf("Prea putine argumente pentru remove_report\n");
            exit(1);
        }
        char *district = argv[6];
        int id=atoi(argv[7]);
        remove_report(district,role,user,id);
    }
    else if(strcmp(comanda,"--view")==0)
    {
        if(argc<8)
        {
            printf("Prea putine argumente pentru view\n");
            exit(1);
        }
        char *district=argv[6];
        int id=atoi(argv[7]);
        view(district,role,user,id);
    }
    else if(strcmp(comanda,"--update_threshold")==0)
    {
        if(argc<8)
        {
            printf("Prea putine argumente pentru update_threshold\n");
            exit(1);
        }
        char *district=argv[6];
        int value=atoi(argv[7]);
        update_threshold(district,role,user,value);

    }
    else
    {
        printf("Operatie neidentificata\n");
        exit(1);
    }
}
