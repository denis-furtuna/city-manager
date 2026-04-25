# Tool folosit: Gemini 3.1 Pro

# Prompt 1:
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
```
Am nevoie de ajutorul tau pentru a crea doua functii.
Pentru inceput vreau sa mi creezi functia parse_condition, care are exact aceasta semnatura:
int parse_condition(const char *input, char *field, char *op, char *value);
Aceasta functie trebuie sa indeplineasca urmatoarele cerinte:
-primeste un string 'input' sub forma field:op:value (exempli severity:>=:3)
-imparte input ul folosind delimitatorul : si stocheaza cele 3 bucati in parametrii functiei: field (maxim 10 caractere), op(maxim 3 caractere) si value (maxim 11 caractere)
-asigura te ca field ul este unul dintre aceste stringuri: severity, category, inspector, timestamp 
-asigura te ca operatorul este unul dintre aceste stringuri:  ==, !=, <, <=, >, >=
-daca input ul nu contine exact doua : sau daca vreuna dintre cele 3 componente lipseste, functia va return -1 (eroare), altfel, va returna 0 (succes)

## Raspuns:

Ascultă aici, tastatură-viteză! Dacă vrei să fii un programator cu adevărat bun în C, trebuie să fii precis ca un bisturiu și să nu lași nimic la voia întâmplării. Memoria nu iartă, iar un _segmentation fault_ te va lovi direct în față dacă nu ești atent!

Ai cerut o funcție `parse_condition` care să facă ordine în stringurile tale. Ți-am scris codul exact cum ai cerut, curat, eficient și blindat împotriva erorilor. Citește-l, învață-l și bagă-l la cap!

### Funcția `parse_condition`

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

### Reguli de fier când folosești funcția asta:

* **Alocă memoria corect!** Când apelezi funcția, `field`, `op` și `value` trebuie să fie vectori pre-alocați (sau pointeri către memorie validă) suficient de mari ca să țină stringul **PLUS** caracterul null terminator `\0`. Adică minim `char field[11]`, `char op[4]`, și `char value[12]`. Dacă îmi trimiți pointeri neinițializați, îți meriți soarta!
* **Fără _buffer overflows_!** Funcția mea deja calculează lungimile matematic folosind aritmetica pointerilor și blochează tot ce e peste limită înainte de a apela `strncpy`. Este sigură!
* **Atenție la `category` și `inspector`!** Ai zis în cerință să validez fix acele stringuri, chiar dacă structura ta `Report` are `issue` și `name` în loc de astea. Eu fac exact ce mi se cere pe specificații. Dacă vrei să mapezi `category` pe `issue`, treaba ta în codul care apelează funcția!

Acum pune mâna, integrează codul ăsta, compilează-l cu `-Wall -Wextra` și treci la treabă! Aștept să îmi zici care e a doua funcție pe care trebuie să o facem, dar scrie specificațiile clar și tăios!

# Prompt 2:
A doua functie va fi match_condition, care are exact aceasta semnatura:
int match_condition(Report *r, const char *field, const char *op, const char *value);
Structura Report este cea din prompt ul anterior.
Aceasta functie trebuie sa indeplineasca urmatoarele cerinte:
-verifica daca campul al variablei r de tip Report cu numele la fel ca stringul field(severity, category, inspector, timestamp)  indeplineste conditia op (==, !=, <, <=, >, >=) value
-cerinta imi impune ca field sa fie una dintre valorile: severity, category, inspector, timestamp, dar unele campurile din structura mea Report difera. category este campul issue, inspector este campul name
-va returna 1 daca conditia este satisfacuta, 0 in caz contrar
-exemple de conditii: severity>=2, category==road

## Raspuns:
Perfect! A doua rundă. Ai cerut logica de potrivire, inima filtrării tale.
Pregătește-ți compilatorul, că vine o capodoperă! Ai cerut logica de filtrare și fix asta primești. Și ca să te salvez de o noapte albă plină de erori și nervi, am implementat deja maparea dintre `category` și `issue`, plus `inspector` și `name`. E cod de elită, precis și complet nemilos cu greșelile! Deschide ochii mari, bagă-l la cap și ai grijă să nu-l strici cu vreun pointer aruncat aiurea prin memorie!

### Funcția `match_condition` (Varianta finală și absolută)

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

### Recapitulare pe repede înainte:
1.  **Conversii:** Numerele din stringuri se transformă în valori reale cu `atoi()` și `atol()`. 
2.  **Mapare:** `category` e comparat cu `r->issue`, iar `inspector` cu `r->name`. Mura în gură!
3.  **Stringuri:** `strcmp()` nu iartă. Dacă dai "road" și în structură ai "road\n" sau " road", îți va returna 0 și tu o să te întrebi de ce nu merge codul.

Ia codul ăsta, integrează-l în proiectul tău, dă-i un `gcc` curat și termină treaba! Ce funcție vrei să mai facem, sau te descurci de aici singur?

# Ce am schimbat:
La functia match_condition nu are rost sa mai verific daca campurile field, op sau value sunt NULL. Daca s ar intampla acest caz, functia parse_condition ar returna -1, iar programul va da exit.

# Ce am invatat:
Am invatat o noua abordare pentru impartirea unui string in bucati. Folosind strchr aflu adresa unde se afla ':' in string, apoi, in functie de aceste adrese, impart stringul in 3 bucati. Daca as fi facut singur, cel mai probabil as fi folosit sscanf sau strtok.
De asemnea, prin intermediul acestei cerinte, am invatat cum sa scriu prompt uri mult mai bune.
