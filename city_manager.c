#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
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

void creare_symlink(char *district)
{
    char path[100]="";
    strcpy(path,district);
    strcat(path,"/reports.dat");

    char link_name[100]="";
    strcpy(link_name,"active_reports-");
    strcat(link_name,district);

    //daca exista un symlink cu acest nume il scoatem
    unlink(link_name);

    if(symlink(path,link_name)==-1)
    {
        printf("Eroare creare symlink\n");
        return;
    }
}

bool verificare_symlink(char *district)
{
    char link_name[100]="";
    strcpy(link_name,"active_reports-");
    strcat(link_name,district);

    struct stat vf1;
    if(lstat(link_name,&vf1)==0) //verificam daca exista un fisier cu numele link_name
    {
        if(S_ISLNK(vf1.st_mode)) //verificam daca e symlink
        {
            struct stat vf2;
            if(stat(link_name,&vf2)!=0) //daca cu lstat exista, iar cu stat nu exista si este si symlink => linkuit catre un fisier inexistent
            {
                printf("Warning! Este dangling link!\n");
                return 0;
            }
            else return 1;
        }
    }
    return 0;
}

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

void scrie_in_log(char *district,char *role, char *user, char *actiune)
{
    char path[100];
    strcpy(path,district);
    strcat(path,"/logged_district");

    //verificam asta acum ca sa nu creeze un inspector logged_district
    struct stat vf;
    if(stat(path,&vf)==0) //daca fisierul exista deja vf drepturile
    {
        int are_drepturi=0;
        if(strcmp(role,"manager")==0 && (vf.st_mode & S_IWUSR)) are_drepturi=1;
        else if (strcmp(role,"inspector")==0 && (vf.st_mode & S_IWGRP)) are_drepturi=1;

        if (are_drepturi==0)return ;
    }
    else if(strcmp(role,"inspector")==0) //daca nu exista fisierul si suntem si inspector => iesim din functie ca sa nu scriem/creeam logged_district
    {
        return ;
    }

    int f3=open(path,O_RDWR|O_CREAT|O_APPEND,0644);
    if(f3==-1)return ;
    fchmod(f3,0644);

    char s[200];
    time_t timestamp=time(NULL);
    char *timp_frumos=ctime(&timestamp);
    timp_frumos[strlen(timp_frumos)-1]='\0';
    snprintf(s,sizeof(s),"[%s] %s %s %s\n",timp_frumos,role,user,actiune); //creeam stringul

    write(f3,s,strlen(s));
    close(f3);
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

    close(f1);

    scrie_in_log(district,role,user,"a vizualizat un district.");

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

    //creez symlinkul in caz de nu exista
    if(verificare_symlink(district)==0)creare_symlink(district);

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
        write(f2, buf, strlen(buf)); //val de severitate
        close(f2);
    }

    scrie_in_log(district,role,user,"a adaugat un raport.");
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

    close(f1);

    scrie_in_log(district,role,user,"a sters un raport.");

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

    close(f1);

    scrie_in_log(district,role,user,"a vizualizat un raport.");
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

    close(f1);
    scrie_in_log(district,role,user,"a dat update la threshold-ul.");
}

//PARTEA CU AI

int parse_condition(const char *input, char *field, char *op, char *value) {
    // 1. Verificăm pointerii! Niciodată să nu ai încredere în ce primești!
    if (!input || !field || !op || !value) return -1;

    // 2. Căutăm exact cele două caractere ':'
    const char *first_colon = strchr(input, ':');
    if (!first_colon) return -1; // Nu e niciun ':'? Afară!

    const char *second_colon = strchr(first_colon + 1, ':');
    if (!second_colon) return -1; // Lipsește al doilea ':'? Afară!

    // Mai mult de două ':'? Eroare! Input invalid!
    if (strchr(second_colon + 1, ':') != NULL) return -1;

    // 3. Calculăm lungimile bucăților de string
    size_t len_field = first_colon - input;
    size_t len_op = second_colon - first_colon - 1;
    size_t len_value = strlen(second_colon + 1);

    // 4. Verificăm cerințele stricte de lungime și existență
    if (len_field == 0 || len_field > 10) return -1;
    if (len_op == 0 || len_op > 3) return -1;
    if (len_value == 0 || len_value > 11) return -1;

    // 5. Copiem datele în buffere (și punem terminatorul null, evident!)
    strncpy(field, input, len_field);
    field[len_field] = '\0';

    strncpy(op, first_colon + 1, len_op);
    op[len_op] = '\0';

    strncpy(value, second_colon + 1, len_value);
    value[len_value] = '\0';

    // 6. Validăm stringul 'field' - trebuie să fie EXACT astea!
    if (strcmp(field, "severity") != 0 &&
        strcmp(field, "category") != 0 &&
        strcmp(field, "inspector") != 0 &&
        strcmp(field, "timestamp") != 0) {
        return -1;
    }

    // 7. Validăm operatorul 'op' - fără invenții!
    if (strcmp(op, "==") != 0 &&
        strcmp(op, "!=") != 0 &&
        strcmp(op, "<") != 0 &&
        strcmp(op, "<=") != 0 &&
        strcmp(op, ">") != 0 &&
        strcmp(op, ">=") != 0) {
        return -1;
    }

    // Dacă a supraviețuit până aici, codul tău e bun!
    return 0;
}

int match_condition(Report *r, const char *field, const char *op, const char *value) {

    // --- Cazul 1: SEVERITY (Întreg) ---
    if (strcmp(field, "severity") == 0) {
        int val = atoi(value);

        if (strcmp(op, "==") == 0) return r->severity == val;
        if (strcmp(op, "!=") == 0) return r->severity != val;
        if (strcmp(op, "<") == 0)  return r->severity < val;
        if (strcmp(op, "<=") == 0) return r->severity <= val;
        if (strcmp(op, ">") == 0)  return r->severity > val;
        if (strcmp(op, ">=") == 0) return r->severity >= val;
    }
    // --- Cazul 2: TIMESTAMP (Timp / Întreg lung) ---
    else if (strcmp(field, "timestamp") == 0) {
        time_t val = (time_t)atol(value);

        if (strcmp(op, "==") == 0) return r->timestamp == val;
        if (strcmp(op, "!=") == 0) return r->timestamp != val;
        if (strcmp(op, "<") == 0)  return r->timestamp < val;
        if (strcmp(op, "<=") == 0) return r->timestamp <= val;
        if (strcmp(op, ">") == 0)  return r->timestamp > val;
        if (strcmp(op, ">=") == 0) return r->timestamp >= val;
    }
    // --- Cazul 3: CATEGORY (String -> mapat brutal pe 'issue') ---
    else if (strcmp(field, "category") == 0) {
        int cmp = strcmp(r->issue, value);

        if (strcmp(op, "==") == 0) return cmp == 0;
        if (strcmp(op, "!=") == 0) return cmp != 0;
        if (strcmp(op, "<") == 0)  return cmp < 0;
        if (strcmp(op, "<=") == 0) return cmp <= 0;
        if (strcmp(op, ">") == 0)  return cmp > 0;
        if (strcmp(op, ">=") == 0) return cmp >= 0;
    }
    // --- Cazul 4: INSPECTOR (String -> mapat brutal pe 'name') ---
    else if (strcmp(field, "inspector") == 0) {
        int cmp = strcmp(r->name, value);

        if (strcmp(op, "==") == 0) return cmp == 0;
        if (strcmp(op, "!=") == 0) return cmp != 0;
        if (strcmp(op, "<") == 0)  return cmp < 0;
        if (strcmp(op, "<=") == 0) return cmp <= 0;
        if (strcmp(op, ">") == 0)  return cmp > 0;
        if (strcmp(op, ">=") == 0) return cmp >= 0;
    }

    // Dacă ajungi aici, înseamnă că `parse_condition` ți-a dat o mizerie de field.
    return 0;
}

void filter(char *district,char *role, char *user,int argc,char *argv[])
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
    char field[11],op[4],value[12];

    while(read(f1,&r,sizeof(Report))==sizeof(Report))
    {
        bool ok=true;
        for(int i=7;i<argc && ok;++i)
        {
            if(parse_condition(argv[i],field,op,value)==-1)
            {
                printf("Formatul conditiei: %s , nu e bun!!\n",argv[i]);
                exit(1);
            }
            if(match_condition(&r,field,op,value)==0)ok=false;
        }
        if(ok)
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
    }
    close(f1);

    scrie_in_log(district,role,user,"a utilizat comanda filter.");

}

void rm_district(char *district, char* role, char *user)
{
    if(strcmp(role,"manager")!=0)return ;

    char path[100]="";
    strcpy(path,"active_reports-");
    strcat(path,district);

    pid_t pid=fork();
    if(pid<0)
    {
        return ;
    }
    if(pid==0)
    {
        execlp("rm","rm","-rf",district,path,NULL);
        exit(1);
    }
    int st;
    wait(&st);
    scrie_in_log(district,role,user,"a sters un district");

}

int main(int argc,char *argv[])
{
    if(argc<7)
    {
        printf("Clar prea putine argumente\n");
        exit(1);
    }
    char *role = argv[2];
    char *user = argv[4];
    if(strlen(user)>30)
    {
        printf("Numele userului depaseste valoarea maxima de 30 caractere");
        exit(1);
    }

    char *comanda = argv[5];

    char *district = argv[6];
    if(strlen(district)>90)
    {
        printf("Numele districtului depaseste valoarea maxima de 90 caractere");
        exit(1);
    }


    if(strcmp(comanda,"--add")==0)
    {
        if(argc<12)
        {
            printf("Prea putine argumente pentru add\n");
            exit(1);
        }

        char *category = argv[9];
        if(strlen(category)>10)
        {
            printf("Numele categoriei depaseste valoarea maxima de 10 caractere");
            exit(1);
        }

        char *description = argv[11];
        if(strlen(description)>50)
        {
            printf("Lungimea descrierii depaseste valoarea maxima de 50 caractere");
            exit(1);
        }

        float latitude = atof(argv[7]);
        float longitude = atof(argv[8]);
        int severity = atoi(argv[10]);
        add(district,role,user,latitude,longitude,category,severity,description);

    }
    else if(strcmp(comanda,"--list")==0)
    {
        list(district,role,user);
    }
    else if(strcmp(comanda,"--remove_report")==0)
    {
        if(argc<8)
        {
            printf("Prea putine argumente pentru remove_report\n");
            exit(1);
        }
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
        int value=atoi(argv[7]);
        update_threshold(district,role,user,value);
    }
    else if(strcmp(comanda,"--filter")==0)
    {
        if(argc<8)
        {
            if(argc==7)printf("Posibil ai uitat sa introduci conditia. Daca nu ai nicio conditie, poti utiliza functia list.\n");
            else printf("Prea putine argumente pentru filter\n");
            exit(1);
        }
        filter(district,role,user,argc,argv);

    }
    else if(strcmp(comanda,"--check_link")==0)
    {
        if(verificare_symlink(district)==1)printf("Nu e dangling link!\n");
        else printf("Acest link doar link bun nu e!\n");
    }
    else if(strcmp(comanda,"--remove_district")==0)
    {
        rm_district(district,role,user);
    }
    else
    {
        printf("Operatie neidentificata: '%s'\n",comanda);
        exit(1);
    }
}
