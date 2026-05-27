## Pentru instrucțiuni complete de utilizare din terminal, detalii despre comenzi și regulile de securitate, consultă manualul în format PDF:

👉 **[Manual de Utilizare (PDF)](manual_de_utilizare.pdf)**


# 🖥️ City Hub - Interfață Grafică de Control (GUI) & Arhitectură Distribuită

Panou de control centralizat dezvoltat în Python pentru monitorizarea asincronă și coordonarea sistemului distribuit de gestiune a rapoartelor din districte. Aplicația decuplează complet execuția proceselor grele de backend (scrise în C) de interfața cu utilizatorul, comunicând securizat printr-un tunel criptat SSH.

---

## 🏗️ Vizualizare Arhitecturală: Modelul "Scatter-Gather" (MapReduce Handcrafted)

Sistemul nu efectuează o simplă concatenare oarbă a fișierelor. Pentru comanda `calculate_scores`, Hub-ul în C și această interfață grafică implementează o paradigmă clasică de calcul paralel:

1. **Faza MAP (Scatter / Împrăștiere):** La declanșarea calculului din GUI, Hub-ul central spawnează procese lucrătoare independente (`fork()`) pentru fiecare district selectat. Acestea rulează în paralel (`execl("./city_manager", ...)`) și scanează local fișierele binare `reports.dat`.
2. **Canalele IPC (Redirecționare):** Prin manipularea tabelei de descriptori (`dup2(pfd[1], 1)`), ieșirea standard (`stdout`) a fiecărui lucrător este deturnată direct într-o conductă RAM (pipe) legată de Hub. Toți copiii urlă datele în paralel prin stațiile lor radio.
3. **Faza REDUCE (Gather / Agregare Centralizată):** Hub-ul părinte acționează ca un General în buncăr. Colectează asincron liniile de text formatat (`fgets`), le parsează chirurgical (`sscanf`) folosind seturi de caractere (`%29[^,]`) pentru a izola virgulele și cumulează matematic scorurile inspectorilor comuni (`ins[index].scor += scor`). Rezultatul final este un raport unic, consolidat și curățat de duplicate.

---

## 🚀 Caracteristici Principale
* **Conexiune Criptată SSH (Paramiko):** Execuție de la distanță direct pe kernel-ul Linux, eliminând necesitatea expunerii unor porturi TCP vulnerabile în rețea.
* **Interfață Asincronă (Non-Blocking UI):** Toate interogările grele pe disc și monitorizarea log-urilor rulează pe fire de execuție secundare (`threading`), garantând că aplicația rămâne fluidă și nu îngheață.
* **Procesare Paralelă Coordonată:** GUI-ul formatează dinamic listele native Python prin îmbinare (`" ".join(districte)`) într-un șir curat, facilitând parsarea prin tokenizare (`strtok`) în backend-ul din C.
* **Consolă Terminal Integrată:** Captură directă și randare live a fluxurilor de eroare și output din sistem pentru un debugging chirurgical.

---

## 📦 Cerințe de Sistem și Dependențe

Aplicația necesită Python 3.8+ instalat pe mașina gazdă (Windows/macOS) și următoarele pachete externe:

```bash
pip install paramiko python-dotenv
```

*Dacă interfața utilizează un toolkit vizual avansat pentru randare profesională:*
```bash
pip install customtkinter
```

---

## ⚙️ Configurarea Mediului de Lucru (.env)

Înainte de prima lansare, creează un fișier numit `.env` în directorul rădăcină al componentei Python pentru calibrarea tunelului de comunicație:

```env
# Configurație Conexiune SSH
SSH_HOST=192.168.x.x          # IP-ul serverului Linux (extras via 'hostname -I')
SSH_PORT=22                   # Portul standard OpenSSH
SSH_USER=student              # Utilizatorul tău de pe mașina virtuală Linux
SSH_PASS=parola_ta_secreta    # Parola contului de Linux

# Căi de Sistem pe Server
PROJECT_PATH=/home/student/proiect_cti/backend
```

⚠️ **NOTĂ DE SECURITATE:** Fișierul `.env` conține credențiale administrative în text clar. Adăugați-l obligatoriu în `.gitignore` pentru a preveni publicarea accidentală a parolelor pe repozitorii publice de GitHub!

---

## 🛠️ Ghid de Utilizare și Operațiuni Tactice

### 1. 📝 Management Binar de Rapoarte (Operațiuni CRUD)
* **Adăugare Raport (`add`):** Transmite structura fixed-size pe server. ID-urile sunt calculate algoritmic în mod optim în timp constant interogând dimensiunea Inode-ului prin `fstat`, eliminând complet parcurgerea liniară a fișierului.
* **Ștergere Raport (`remove_report`):** Execută mutarea structurilor din spate prin glisarea pointerilor de disc (`lseek`) direct în binar și restrânge fișierul fizic prin `ftruncate`, eliminând riscul de memory leaks.
* **Filtrare Avansată (`filter`):** Extrage date bazate pe criterii temporale, comparând matematic timestamp-urile întregi (*UNIX Epoch*) stocate pe 8 bytes.

### 2. 👁️ Monitorizare în Fundal (`start_monitor`)
* Lansează o arhitectură de tip **Double-Fork** pe server pentru a izola procesul de monitorizare și a-l transforma într-un Daemon imun la închiderea terminalului (adoptat automat de procesul `init`/`systemd` PID 1).
* Fluxul de date este capturat în timp real din pipe-uri și trimis către consola GUI fără a genera procese Zombie pe partiție.

### 3. 📊 Calcul Agregat de Sarcini (`calculate_scores`)
* Selectează districtele dorite din interfață. Aplicația le convertește automat într-un format digerabil pentru sistemul de operare și declanșează procesarea distribuită explicată în secțiunea de arhitectură.
