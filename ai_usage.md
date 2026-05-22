# Phase 1: File Systems
## -> Tool folosit: Gemini 3.1 Pro

## -> Prompt 1:
```
Esti un programator foarte bun in C. Lucrez la un proiect care are aceasta structura:
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

Am nevoie de ajutorul tau pentru a crea doua functii.
Pentru inceput vreau sa mi creezi functia parse_condition, care are exact aceasta semnatura:
int parse_condition(const char *input, char *field, char *op, char *value);
Aceasta functie trebuie sa indeplineasca urmatoarele cerinte:
-primeste un string 'input' sub forma field:op:value (exempli severity:>=:3)
-imparte input ul folosind delimitatorul : si stocheaza cele 3 bucati in parametrii functiei: field (maxim 10 caractere), op(maxim 3 caractere) si value (maxim 11 caractere)
-asigura te ca field ul este unul dintre aceste stringuri: severity, category, inspector, timestamp 
-asigura te ca operatorul este unul dintre aceste stringuri:  ==, !=, <, <=, >, >=
-daca input ul nu contine exact doua : sau daca vreuna dintre cele 3 componente lipseste, functia va return -1 (eroare), altfel, va returna 0 (succes)
```
### Raspuns:

Ascultă aici, tastatură-viteză! Dacă vrei să fii un programator cu adevărat bun în C, trebuie să fii precis ca un bisturiu și să nu lași nimic la voia întâmplării. Memoria nu iartă, iar un _segmentation fault_ te va lovi direct în față dacă nu ești atent!

Ai cerut o funcție `parse_condition` care să facă ordine în stringurile tale. Ți-am scris codul exact cum ai cerut, curat, eficient și blindat împotriva erorilor. Citește-l, învață-l și bagă-l la cap!

#### Funcția `parse_condition`

```c
#include <stdio.h>
#include <string.h>

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
```

#### Reguli de fier când folosești funcția asta:

* **Alocă memoria corect!** Când apelezi funcția, `field`, `op` și `value` trebuie să fie vectori pre-alocați (sau pointeri către memorie validă) suficient de mari ca să țină stringul **PLUS** caracterul null terminator `\0`. Adică minim `char field[11]`, `char op[4]`, și `char value[12]`. Dacă îmi trimiți pointeri neinițializați, îți meriți soarta!
* **Fără _buffer overflows_!** Funcția mea deja calculează lungimile matematic folosind aritmetica pointerilor și blochează tot ce e peste limită înainte de a apela `strncpy`. Este sigură!
* **Atenție la `category` și `inspector`!** Ai zis în cerință să validez fix acele stringuri, chiar dacă structura ta `Report` are `issue` și `name` în loc de astea. Eu fac exact ce mi se cere pe specificații. Dacă vrei să mapezi `category` pe `issue`, treaba ta în codul care apelează funcția!

Acum pune mâna, integrează codul ăsta, compilează-l cu `-Wall -Wextra` și treci la treabă! Aștept să îmi zici care e a doua funcție pe care trebuie să o facem, dar scrie specificațiile clar și tăios!

## -> Prompt 2:
```
A doua functie va fi match_condition, care are exact aceasta semnatura:
int match_condition(Report *r, const char *field, const char *op, const char *value);
Structura Report este cea din prompt ul anterior.
Aceasta functie trebuie sa indeplineasca urmatoarele cerinte:
-verifica daca campul al variablei r de tip Report cu numele la fel ca stringul field(severity, category, inspector, timestamp)  indeplineste conditia op (==, !=, <, <=, >, >=) value
-cerinta imi impune ca field sa fie una dintre valorile: severity, category, inspector, timestamp, dar unele campurile din structura mea Report difera. category este campul issue, inspector este campul name
-va returna 1 daca conditia este satisfacuta, 0 in caz contrar
-exemple de conditii: severity>=2, category==road
```
### Raspuns:
Perfect! A doua rundă. Ai cerut logica de potrivire, inima filtrării tale.
Pregătește-ți compilatorul, că vine o capodoperă! Ai cerut logica de filtrare și fix asta primești. Și ca să te salvez de o noapte albă plină de erori și nervi, am implementat deja maparea dintre `category` și `issue`, plus `inspector` și `name`. E cod de elită, precis și complet nemilos cu greșelile! Deschide ochii mari, bagă-l la cap și ai grijă să nu-l strici cu vreun pointer aruncat aiurea prin memorie!

#### Funcția `match_condition` (Varianta finală și absolută)

```c
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Structura ta, pusă aici doar ca să știi la ce ne raportăm
/*
typedef struct {
    int id;
    char name[30];
    float latitude;
    float longitude;
    char issue[10];
    int severity;
    time_t timestamp;
    char description[50];
} Report;
*/

int match_condition(Report *r, const char *field, const char *op, const char *value) {
    // Verificăm pointerii! Dacă bagi NULL aici, primești 0 și o privire plină de judecată.
    if (!r || !field || !op || !value) return 0;

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
```

#### Recapitulare pe repede înainte:
1.  **Conversii:** Numerele din stringuri se transformă în valori reale cu `atoi()` și `atol()`. 
2.  **Mapare:** `category` e comparat cu `r->issue`, iar `inspector` cu `r->name`. Mura în gură!
3.  **Stringuri:** `strcmp()` nu iartă. Dacă dai "road" și în structură ai "road\n" sau " road", îți va returna 0 și tu o să te întrebi de ce nu merge codul.

Ia codul ăsta, integrează-l în proiectul tău, dă-i un `gcc` curat și termină treaba! Ce funcție vrei să mai facem, sau te descurci de aici singur?

## -> Ce am schimbat:
```
La functia match_condition nu are rost sa mai verific daca campurile field, op sau value sunt NULL. Daca s ar intampla acest caz, functia parse_condition ar returna -1, iar programul va da exit.
```
## -> Ce am invatat:
```
Am invatat o noua abordare pentru impartirea unui string in bucati. Folosind strchr aflu adresa unde se afla ':' in string, apoi, in functie de aceste adrese, impart stringul in 3 bucati. Daca as fi facut singur, cel mai probabil as fi folosit sscanf sau strtok.
De asemnea, prin intermediul acestei cerinte, am invatat cum sa scriu prompt uri mult mai bune.
```

# Phase 2: Processes and Signals
## -> Tool folosit: Gemini 3.1 Pro
## -> Cum l-am folosit?
```
L-am folosit pe post de mentor la ambele Phase-uri. I-am zis de la inceput ca sa nu mi genereze cod, ci doar sa imi explice ce ar trebui sa fac.
Dupa ce scriam cod, ii trimiteam lui si ii ziceam sa ma corecteze, explicandu-mi,daca era cazul, ce si de ce e gresit.
```
## -> De ce l-am folosit?
```
In opinia mea, AI-ul este cel mai bun tool cu care poti sa inveti, daca-l folosesti corect. Eu nu l-am folosit pentru a imi face direct toata treaba, iar eu doar sa dau copy-paste. Urasc sa fac asta, deoarece arata faptul ca nu am deloc control asupra programului, ca nu eu sunt creierul din spatele programului.
Mereu ma ajut de AI cand am nelamuriri, cand vreau sa aflu o informatie mai rapid sau, mai ales, cand am nevoie de feedback.
```

## -> Unde l-am folosit?
```
- In explicarea mai detaliata a enuntului si in indrumarea in ceea ce ar trebui sa fac.
- In explicarea in detaliu a unor anumitor functii din C.
- In corectarea codului scris de mine (ce am scris gresit, ce am omis sau ce puteam sa fac mai bine/sigur)
```

## Exemple de prompt-uri:
### 1.Exemplu cand am avut nevoie de feedback.
```
 void add(char *district,char* role,char *user,float latitude,float longitude, char *category,int severity,time_t timestamp, char *description)

{

    struct stat st={0};

    if(stat(district,&st)==-1) //vf daca exista fisier

        mkdir(district,0750); //cream fisier cu cartierul


    //creez fisierele cartierului

    char path[100]="";

    strcpy(path,district);

    strcat(path,"/reports.dat");

    int f1=open(path,O_RDWR|O_CREAT);

    fchmod(f1,0664);



    strcpy(path,district);

    strcat(path,"/district.cfg");

    int f2=open(path,O_RDWR|O_CREAT);

    fchmod(f2,0640);



    strcpy(path,district);

    strcat(path,"/logged_district");

    int f3=open(path,O_RDWR|O_CREAT);

    fchmod(f3,0644);


    lseek(f1,0,SEEK_END);




    close(f1);

    close(f2);

    close(f3);

}

o sa urmeze sa verific rolu ca sa fac cazu cand un inspector vrea sa creeze un rol. te rog zi ce parere ai de program. stiu ca e putin ineficient faptul ca dau fchmod de fiecare data, nu doar la crearea fisierului, dar nici nu vreau sa am cod redudant ( sa fac cazul cand nu exista fisierul si cand exista)
```
### Raspuns:
Băi, ești pe drumul cel bun! Ai prins ideea, ai închis fișierele, ai pus O_CREAT, ai băgat lseek-ul ca să te duci la coadă. Începe să semene a cod de om mare!

Cât despre chestia cu ineficiența că dai fchmod de fiecare dată... ascultă la mine: în programarea de sistem, paranoia e prietena ta! Faptul că tu forțezi permisiunile alea la fiecare adăugare se numește Programare Defensivă. Dacă un "inteligent" a intrat ieri și a stricat permisiunile manual din terminal, codul tău le repară instant înainte să scrie. Deci bravo, lasă-l așa!

DAR! (Că mereu există un dar). Ai lăsat o mină terestră invizibilă în codul ăsta, care o să-ți explodeze în față la rulare. Ia un pix și notează!
1. Grenada fără cui: open cu O_CREAT

Regula de aur în C: Când folosești flag-ul O_CREAT în funcția open(), ești OBLIGAT să îi dai și al treilea argument (modul/permisiunile de bază).
Tu ai scris: int f1 = open(path, O_RDWR | O_CREAT);
Dacă nu îi dai al treilea argument, compilatorul va citi "gunoi" de pe stivă și va crea fișierul cu niște permisiuni complet aleatoare și extraterestre. Uneori nici nu o să te lase să faci fchmod după, pentru că s-ar putea să-l creeze fără drepturi de acces pentru tine!
Cum repari: int f1 = open(path, O_RDWR | O_CREAT, 0664);
Chiar dacă îi dai fchmod pe linia următoare, apelul open are nevoie de o bază legală la creare! Bate un 0664 acolo, un 0640 la cfg și un 0644 la log!
2. O mică lecție de vocabular

Ai scris în comentariu: //cream fisier cu cartierul.
Băi, mkdir face un DIRECTOR (folder), nu un fișier! Nu confunda merele cu pietrele. Fișierele sunt alea cu .dat sau .cfg. Cartierul e o "găleată" (folder) în care le pui.
3. Calea ușoară vs. Calea cu strcpy (Opțional, dar profi)

Folosești strcpy urmat de strcat. Funcționează? Da. E elegant? E ca și cum ai tăia pâinea cu toporul. În C-ul modern, folosim snprintf. E mai sigur (nu dă buffer overflow) și o scrii într-o singură linie:
```
snprintf(path, sizeof(path), "%s/reports.dat", district);
```
Bum! Direct și curat. Dar, dacă vrei să lași cu strcpy/strcat momentan ca să înțelegi tu logic, e ok, nu te pică nimeni pe asta la faza 1.
4. Următoarea mișcare (Loviturile finale)

Ai făcut lseek(f1, 0, SEEK_END);. Bun, ai mutat cursorul la finalul fișierului reports.dat.
Sfat: Puteai pur și simplu să pui flag-ul O_APPEND în apelul open (ex: O_RDWR | O_CREAT | O_APPEND) și sistemul de operare te punea automat la final de fiecare dată când scriai, fără să mai scrii tu lseek.

Acum, ce faci mai departe? Ai cursorul la finalul fișierului f1. Acum trebuie să scrii efectiv raportul.

Misiunea ta:
Trebuie să iei structura aia a ta Report, pe care ai definit-o sus, să declari o variabilă de tipul ăsta, să bagi în ea toate argumentele pe care le-ai primit în funcție (id-ul îl pui manual deocamdată, restul le copiezi în ea) și să o arunci pe disc folosind funcția write(). Atenție: la șiruri de caractere folosești strcpy sau strncpy ca să le pui în structură, nu cu egal (=).

Înainte să pui close(f1), arată-mi cum populezi o structură Report și cum dai apelul write()! Mișcarea!

## 2.Exemplu cand i-am zis sa-mi explice cerinta si sa nu mi-genereze cod.
```
2. Write a new program called monitor_reports that works as follows:
at startup it creates or overwrites a hidden text file called .monitor_pid where it stores its main process ID. The file is situated at the same level of the directory tree as the district directories.
when it ends, it deletes the above file
the program only ends when it receives SIGINT, and writes a message on the standard output when it does
the program responds to SIGUSR1 signals (which informs it that a new report has been added) by writing a message on the standard output

ajuta ma sa inteleg ce trebuie sa fac. nu mi genera cod
```
### Raspuns
Ascultă la mine, soldat! Ai cerut doar strategie, fără cod servit pe tavă? Așa te vreau! E timpul să-ți antrenezi creierul, nu doar să dai copy-paste ca un civil leneș! Misiunea asta se numește Războiul Semnalelor, și dacă nu ești atent, procesul tău va fi ciuruit înainte să apuce să scrie un singur rând!

Iată planul de bătaie, descompus pe obiective tactice. Bagă la cap!
Misiunea 1: Amprenta (Crearea fișierului PID)

La pornire, programul tău trebuie să-și lase „cartea de vizită” ca să știe restul lumii (sau alte programe) cum să-l contacteze.

    Cum afli cine ești? Ai nevoie de funcția de sistem care îți returnează Process ID-ul (caută prin manuale după funcția care îți dă propriul PID).

    Fișierul ascuns: Punctul din fața numelui (.monitor_pid) este camuflajul în Linux. Deschizi acest fișier în modul de scriere (ca să-l creezi sau să-l suprascrii dacă există deja), îți scrii PID-ul în el și îl închizi. Fără ezitare!

Misiunea 2: Câmpul Minat (Interceptarea Semnalelor)

Sistemul de operare comunică trimițând semnale (un fel de rachete teleghidate). Dacă un semnal te lovește și tu nu ești pregătit, mori. Trebuie să folosești armament greu pentru a le prinde. Uită de funcția veche și obosită signal(); un profesionist folosește sigaction pentru a seta capcane pentru aceste semnale! Vei avea nevoie de două funcții de manipulare a semnalelor (handlere).

    Grenada de Fum (SIGUSR1): Când un alt program adaugă un raport, va trage spre tine cu un SIGUSR1. Când prinzi acest semnal, funcția ta de interceptare trebuie doar să urle (adică să printeze la standard output) un mesaj de genul: "Raport nou recepționat!". După care, programul își vede de viață mai departe.

    Lovitura Letală (SIGINT): Acesta este faimosul Ctrl+C. În mod normal, când primești asta, execuția e terminată instant. Dar tu nu cazi la datorie fără să raportezi! Interceptezi SIGINT, tragi un ultim strigăt de luptă pe ecran (print) și apoi treci urgent la Misiunea 3, înainte să dai ordinul final de retragere (exit).

Misiunea 3: Curățenia (Ștergerea urmelor)

Când ești lovit de SIGINT și te pregătești să mori, nu lași documente secrete pe câmpul de luptă! În handler-ul tău pentru SIGINT, înainte de a ieși din program, trebuie să apelezi o funcție de sistem care șterge fișiere (caută ceva cu "remove" sau "unlink") pentru a distruge acel .monitor_pid.
Misiunea 4: Așteptarea în Tranșee (Bucla infinită)

Dacă îi spui programului tău doar să creeze un fișier și să seteze niște handlere pentru semnale, el va termina execuția în 2 milisecunde și va pleca acasă. Ca să prindă semnale, trebuie să stea treaz și la post!

    Ai nevoie de o buclă care să țină programul în viață. Dar atenție! O buclă while(1) goală îți va prăji procesorul la 100%.

    Folosește funcția pause() în interiorul buclei. Asta e tactica supremă: pune soldatul să doarmă, nu consumă nicio resursă, iar când apare un semnal, sistemul îl trezește automat cu un șut!

Adună-ți bibliotecile necesare (unistd.h, signal.h, etc.), gândește structura și arată-mi că poți transforma cuvintele astea într-o armură de cod! La treabă! Executare!

# Phase 3: Pipes and Redirects 
## -> Tool folosit: Gemini 3.1 Pro
## -> Cum l-am folosit?
```
    Am folosit fix aceeasi strategie ca la Phase 2. L am folosit ca si mentor, fara a mi genera cod.
    Cel mai valoros feedback a fost în functia calculate_scores, unde mi-a explicat pericolul de deadlock daca folosesc wait() inainte de a inchide teava de citire, si de ce este absolut necesar sa folosesc un numar de wait()-uri egal cu numarul de fork()-uri pentru a evita crearea de procese zombie in memorie.
```
