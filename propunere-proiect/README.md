# City Manager - Sistem Distribuit (Client-Server)

Acest proiect reprezintă o arhitectură hibridă pentru gestionarea și monitorizarea infrastructurii urbane, bazată pe un client grafic (Windows/Python) și un motor de execuție de înaltă performanță (Linux/C).

## Structură Proiect

- `src/Client/`: Interfața grafică și logica de comunicare SSH.
- `src/Server/`: Daemon-ul de monitorizare și motorul de procesare date (C).
- `Propunere_Prezentare.pdf`: Documentația tehnică a proiectului.

## Cerințe Sistem

- **Server (Linux):** GCC (pentru compilare C).
- **Client (Windows):** Python 3.10+, `pip`, `customtkinter`, `paramiko`, `python-dotenv`.

## Instrucțiuni de Instalare și Utilizare

### 1. Configurare Server

1. Navigați în `src/Server/`.
2. Asigurați-vă că fișierele sursă `.c` sunt prezente.
3. Rulați scriptul de compilare:

```bash
chmod +x compile_all.sh
./compile_all.sh
```

Acest script va genera executabilele `monitor`, `city_manager` și `city_hub`.

### 2. Configurare Client

1. Navigați în `src/Client/`.
2. Instalați dependențele:

```bash
pip install -r requirements.txt
```

3. Completați credențialele SSH:

```env
SSH_HOST=ip_server
SSH_USER=utilizator
SSH_PASS=parola
```

4. Lansați interfața:

```bash
python main.py
```

## Notă Tehnică

Sistemul utilizează un tunel SSH pentru securitate și sincronizare I/O bazată pe timpi de așteptare pentru a garanta integritatea datelor la scriere.

