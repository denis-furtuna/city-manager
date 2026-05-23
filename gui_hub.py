import threading
import customtkinter as ctk
import paramiko

# Setările vizuale
ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("blue")

# Coordonatele serverului Linux
HOST_IP = "192.168.1.135"
USER = "debian"
PASS = "debian"
PROJECT_PATH = "/home/debian/SO/proiect"


def execute_remote_command(command):
    """Execută o comandă unică prin SSH și returnează rezultatul."""
    try:
        client = paramiko.SSHClient()
        client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        client.connect(HOST_IP, username=USER, password=PASS, timeout=5)

        stdin, stdout, stderr = client.exec_command(command)
        output = stdout.read().decode('utf-8')
        errors = stderr.read().decode('utf-8')
        client.close()

        return errors if errors else output
    except Exception as e:
        return f"Eroare rețea: {str(e)}"


class CommandHub(ctk.CTk):
    def __init__(self):
        super().__init__()

        self.title("City Manager - Control Panel")
        self.geometry("1300x850")  # Am mărit puțin lățimea ca să încapă 3 terminale frumos

        self.grid_rowconfigure(0, weight=1)
        self.grid_columnconfigure(1, weight=1)

        self.active_districts = []
        self.checkbox_vars = []

        # --- PANOU LATERAL ---
        self.sidebar_frame = ctk.CTkFrame(self, width=220, corner_radius=0)
        self.sidebar_frame.grid(row=0, column=0, sticky="nsew")
        self.sidebar_frame.grid_rowconfigure(5, weight=1)

        self.logo_label = ctk.CTkLabel(self.sidebar_frame, text="CITY HUB\nDashboard",
                                       font=ctk.CTkFont(size=22, weight="bold"))
        self.logo_label.grid(row=0, column=0, padx=20, pady=(20, 30))

        self.btn_add = ctk.CTkButton(self.sidebar_frame, text="Adăugare Raport", command=self.show_add_frame)
        self.btn_add.grid(row=1, column=0, padx=20, pady=10)

        self.btn_list = ctk.CTkButton(self.sidebar_frame, text="Baza de Date", command=self.show_list_frame)
        self.btn_list.grid(row=2, column=0, padx=20, pady=10)

        self.btn_scores = ctk.CTkButton(self.sidebar_frame, text="Performanță Inspectori",
                                        command=self.show_scores_frame)
        self.btn_scores.grid(row=3, column=0, padx=20, pady=10)

        self.status_label = ctk.CTkLabel(self.sidebar_frame, text="Monitor: OFFLINE", text_color="red",
                                         font=ctk.CTkFont(weight="bold"))
        self.status_label.grid(row=6, column=0, padx=20, pady=5)

        self.btn_start_monitor = ctk.CTkButton(self.sidebar_frame, text="Pornire Monitor", command=self.start_monitor,
                                               fg_color="#2e7d32", hover_color="#1b5e20")
        self.btn_start_monitor.grid(row=7, column=0, padx=20, pady=5)

        self.btn_stop_monitor = ctk.CTkButton(self.sidebar_frame, text="Oprire Monitor", command=self.stop_monitor,
                                              fg_color="#b71c1c", hover_color="#7f0000", state="disabled")
        self.btn_stop_monitor.grid(row=8, column=0, padx=20, pady=(5, 20))

        # --- ZONA DREAPTĂ (Workspace sus + Terminale jos) ---
        self.right_panel = ctk.CTkFrame(self, fg_color="transparent")
        self.right_panel.grid(row=0, column=1, padx=20, pady=20, sticky="nsew")
        self.right_panel.grid_rowconfigure(0, weight=3)  # Zona formularelor
        self.right_panel.grid_rowconfigure(1, weight=2)  # Zona terminalelor
        self.right_panel.grid_columnconfigure(0, weight=1)

        # Workspace
        self.workspace_frame = ctk.CTkFrame(self.right_panel, corner_radius=10)
        self.workspace_frame.grid(row=0, column=0, sticky="nsew", pady=(0, 10))

        self.main_label = ctk.CTkLabel(self.workspace_frame, text="Sistem pregătit. Selectați o acțiune din meniu.",
                                       font=ctk.CTkFont(size=18))
        self.main_label.pack(expand=True)

        # Split Terminal Frame (Acum cu 3 coloane!)
        self.console_container = ctk.CTkFrame(self.right_panel, fg_color="transparent")
        self.console_container.grid(row=1, column=0, sticky="nsew")
        self.console_container.grid_columnconfigure(0, weight=1)
        self.console_container.grid_columnconfigure(1, weight=1)
        self.console_container.grid_columnconfigure(2, weight=1)
        self.console_container.grid_rowconfigure(1, weight=1)

        # Terminal 1: Monitor Live (Stânga - Verde)
        self.lbl_mon = ctk.CTkLabel(self.console_container, text="[ monitor log ]", font=ctk.CTkFont(weight="bold"))
        self.lbl_mon.grid(row=0, column=0, padx=(0, 5), pady=0, sticky="w")
        self.monitor_console = ctk.CTkTextbox(self.console_container, font=ctk.CTkFont(family="Consolas", size=12),
                                              text_color="#00ff00", fg_color="#121212")
        self.monitor_console.grid(row=1, column=0, padx=(0, 5), sticky="nsew")

        # Terminal 2: City Manager (Mijloc - Galben)
        self.lbl_mgr = ctk.CTkLabel(self.console_container, text="[ city_manager ]", font=ctk.CTkFont(weight="bold"))
        self.lbl_mgr.grid(row=0, column=1, padx=5, pady=0, sticky="w")
        self.manager_console = ctk.CTkTextbox(self.console_container, font=ctk.CTkFont(family="Consolas", size=12),
                                              text_color="#ffff00", fg_color="#121212")
        self.manager_console.grid(row=1, column=1, padx=5, sticky="nsew")

        # Terminal 3: City Hub (Dreapta - Cyan)
        self.lbl_hub = ctk.CTkLabel(self.console_container, text="[ city_hub ]", font=ctk.CTkFont(weight="bold"))
        self.lbl_hub.grid(row=0, column=2, padx=(5, 0), pady=0, sticky="w")
        self.hub_console = ctk.CTkTextbox(self.console_container, font=ctk.CTkFont(family="Consolas", size=12),
                                          text_color="#00ffff", fg_color="#121212")
        self.hub_console.grid(row=1, column=2, padx=(5, 0), sticky="nsew")

        self.stop_wiretap = False

        # Scanăm districtele la pornire
        self._run_async(self._async_scan_districts)

    def _run_async(self, target, *args):
        threading.Thread(target=target, args=args, daemon=True).start()

    def append_to_console(self, target_console, text):
        """Scrie textul în terminalul specificat (monitor, manager sau hub)."""
        if target_console == "monitor":
            console = self.monitor_console
        elif target_console == "manager":
            console = self.manager_console
        else:
            console = self.hub_console

        console.insert("end", text + "\n")
        console.see("end")

    def _async_scan_districts(self):
        cmd = f"cd {PROJECT_PATH} && for d in */; do if [ -f \"${{d}}reports.dat\" ]; then echo \"${{d%/}}\"; fi; done"
        raspuns = execute_remote_command(cmd)
        self.active_districts = [d.strip() for d in raspuns.strip().split('\n') if d.strip() and "Eroare" not in d]

    def _render_checkboxes(self, parent_frame):
        self.checkbox_vars = []
        for widget in parent_frame.winfo_children(): widget.destroy()

        if not self.active_districts:
            ctk.CTkLabel(parent_frame, text="Niciun district găsit. Adăugați un raport.").pack(pady=10)
            return

        for dist in self.active_districts:
            var = ctk.StringVar(value="")
            chk = ctk.CTkCheckBox(parent_frame, text=dist, variable=var, onvalue=dist, offvalue="")
            chk.pack(pady=2, anchor="w", padx=20)
            self.checkbox_vars.append(var)

    # --- LOGICA MONITOR ---
    def start_monitor(self):
        self.status_label.configure(text="Monitor: SE CONECTEAZĂ...", text_color="yellow")
        self.btn_start_monitor.configure(state="disabled")
        self._run_async(self._async_start_monitor)

    def _async_start_monitor(self):
        try:
            client = paramiko.SSHClient()
            client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
            client.connect(HOST_IP, username=USER, password=PASS, timeout=5)
            cmd = f"cd {PROJECT_PATH} && nohup ./monitor </dev/null > monitor.log 2>&1 &"
            client.exec_command(cmd)
            client.close()
        except Exception as e:
            self.after(0, self.append_to_console, "monitor", f"[Eroare]: {e}")

        self.stop_wiretap = False
        self._run_async(self._wiretap_monitor_log)
        self.after(0, self._update_gui_after_start)

    def _wiretap_monitor_log(self):
        try:
            client = paramiko.SSHClient()
            client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
            client.connect(HOST_IP, username=USER, password=PASS, timeout=5)
            self.monitor_ssh_client = client
            stdin, stdout, stderr = client.exec_command(f"tail -f {PROJECT_PATH}/monitor.log")

            for line in iter(stdout.readline, ""):
                if self.stop_wiretap: break
                if line.strip(): self.after(0, self.append_to_console, "monitor", line.strip())
            client.close()
        except Exception:
            pass

    def _update_gui_after_start(self):
        self.status_label.configure(text="Monitor: ONLINE", text_color="#4caf50")
        self.btn_stop_monitor.configure(state="normal")
        self.append_to_console("monitor", "[Info] Procesul monitor a fost pornit.")

    def stop_monitor(self):
        self.status_label.configure(text="Monitor: SE OPREȘTE...", text_color="yellow")
        self.btn_stop_monitor.configure(state="disabled")
        self._run_async(self._async_stop_monitor)

    def _async_stop_monitor(self):
        cmd = f"cd {PROJECT_PATH} && kill -2 $(cat .monitor_pid)"
        execute_remote_command(cmd)

        self.stop_wiretap = True
        if hasattr(self, 'monitor_ssh_client'): self.monitor_ssh_client.close()
        self.after(0, self._update_gui_after_stop)

    def _update_gui_after_stop(self):
        self.status_label.configure(text="Monitor: OFFLINE", text_color="red")
        self.btn_start_monitor.configure(state="normal")
        self.append_to_console("monitor", "[Info] Procesul monitor a fost oprit.")

    # --- LOGICA ADĂUGARE RAPOARTE ---
    def show_add_frame(self):
        for widget in self.workspace_frame.winfo_children(): widget.destroy()

        title = ctk.CTkLabel(self.workspace_frame, text="Adăugare Raport", font=ctk.CTkFont(size=20, weight="bold"))
        title.pack(pady=(15, 5))

        self.opt_role = ctk.CTkOptionMenu(self.workspace_frame, values=["inspector", "manager"])
        self.opt_role.pack(pady=5, padx=50, fill="x")

        self.entry_user = ctk.CTkEntry(self.workspace_frame, placeholder_text="Nume (ex: Denis)")
        self.entry_user.pack(pady=5, padx=50, fill="x")

        self.entry_district = ctk.CTkEntry(self.workspace_frame, placeholder_text="District (ex: centru)")
        self.entry_district.pack(pady=5, padx=50, fill="x")

        self.entry_lat = ctk.CTkEntry(self.workspace_frame, placeholder_text="Latitudine")
        self.entry_lat.pack(pady=5, padx=50, fill="x")

        self.entry_lon = ctk.CTkEntry(self.workspace_frame, placeholder_text="Longitudine")
        self.entry_lon.pack(pady=5, padx=50, fill="x")

        self.opt_category = ctk.CTkOptionMenu(self.workspace_frame, values=["road", "lighting", "flooding", "waste"])
        self.opt_category.pack(pady=5, padx=50, fill="x")

        self.opt_severity = ctk.CTkOptionMenu(self.workspace_frame, values=["1", "2", "3"])
        self.opt_severity.pack(pady=5, padx=50, fill="x")

        self.entry_desc = ctk.CTkEntry(self.workspace_frame, placeholder_text="Descriere incident")
        self.entry_desc.pack(pady=5, padx=50, fill="x")

        self.btn_submit = ctk.CTkButton(self.workspace_frame, text="Trimite Raport", command=self.submit_report)
        self.btn_submit.pack(pady=10)

    def submit_report(self):
        role, user, dist, lat, lon = self.opt_role.get(), self.entry_user.get(), self.entry_district.get(), self.entry_lat.get(), self.entry_lon.get()
        cat, sev, desc = self.opt_category.get(), self.opt_severity.get(), self.entry_desc.get()

        if not all([user, dist, lat, lon, desc]):
            self.append_to_console("manager", "[Eroare] Completați toate câmpurile formularului.")
            return

        self.btn_submit.configure(state="disabled")
        self.append_to_console("manager", f"> Exercițiu comandă: adăugare raport în {dist}...")
        self._run_async(self._async_submit_report, role, user, dist, lat, lon, cat, sev, desc)

    def _async_submit_report(self, role, user, dist, lat, lon, cat, sev, desc):
        cmd = f"cd {PROJECT_PATH} && ./city_manager --role {role} --user {user} --add {dist} {lat} {lon} {cat} {sev} '{desc}'"
        raspuns = execute_remote_command(cmd)

        self.after(0, lambda: self.btn_submit.configure(state="normal"))
        self.after(0, self.append_to_console, "manager", f"{raspuns.strip()}\n")
        self._async_scan_districts()

    # --- LOGICA VIZUALIZARE BAZĂ DE DATE (--list) ---
    def show_list_frame(self):
        for widget in self.workspace_frame.winfo_children(): widget.destroy()

        title = ctk.CTkLabel(self.workspace_frame, text="Vizualizare Rapoarte",
                             font=ctk.CTkFont(size=20, weight="bold"))
        title.pack(pady=(15, 5))

        self.districts_frame_list = ctk.CTkScrollableFrame(self.workspace_frame, height=120,
                                                           label_text="Selectați Districtele")
        self.districts_frame_list.pack(pady=5, padx=50, fill="x")
        self._render_checkboxes(self.districts_frame_list)

        self.btn_fetch_list = ctk.CTkButton(self.workspace_frame, text="Interoghează Baza", command=self.fetch_list)
        self.btn_fetch_list.pack(pady=10)

    def fetch_list(self):
        selected = [var.get() for var in self.checkbox_vars if var.get() != ""]
        if not selected:
            self.append_to_console("manager", "[Atenție] Selectați cel puțin un district.")
            return

        self.btn_fetch_list.configure(state="disabled")
        self.append_to_console("manager", f"> Se procesează lista pentru: {', '.join(selected)}")
        self._run_async(self._async_fetch_list, selected)

    def _async_fetch_list(self, selected_districts):
        for dist in selected_districts:
            cmd = f"cd {PROJECT_PATH} && ./city_manager --role manager --user GUI_Hub --list {dist}"
            raspuns = execute_remote_command(cmd)
            self.after(0, self.append_to_console, "manager", f"[{dist}]:\n{raspuns.strip()}\n")

        self.after(0, lambda: self.btn_fetch_list.configure(state="normal"))

    # --- LOGICA SCORURI INSPECTORI (city_hub REPL) ---
    def show_scores_frame(self):
        for widget in self.workspace_frame.winfo_children(): widget.destroy()

        title = ctk.CTkLabel(self.workspace_frame, text="Calcul Performanță", font=ctk.CTkFont(size=20, weight="bold"))
        title.pack(pady=(15, 5))

        self.districts_frame_scores = ctk.CTkScrollableFrame(self.workspace_frame, height=120,
                                                             label_text="Selectați Districtele")
        self.districts_frame_scores.pack(pady=5, padx=50, fill="x")
        self._render_checkboxes(self.districts_frame_scores)

        self.btn_fetch_scores = ctk.CTkButton(self.workspace_frame, text="Calculează Scoruri (City Hub)",
                                              command=self.fetch_scores)
        self.btn_fetch_scores.pack(pady=10)

    def fetch_scores(self):
        selected = [var.get() for var in self.checkbox_vars if var.get() != ""]
        if not selected:
            self.append_to_console("hub", "[Atenție] Selectați cel puțin un district.")
            return

        self.btn_fetch_scores.configure(state="disabled")
        dist_args = " ".join(selected)
        self.append_to_console("hub", f"> Se calculează scorurile pentru: {dist_args}...")
        self._run_async(self._async_fetch_scores, dist_args)

    def _async_fetch_scores(self, dist_args):
        cmd = f"cd {PROJECT_PATH} && echo 'calculate_scores {dist_args}' | ./city_hub --role manager --user GUI_Hub"
        raspuns = execute_remote_command(cmd)

        self.after(0, lambda: self.btn_fetch_scores.configure(state="normal"))
        self.after(0, self.append_to_console, "hub", f"{raspuns.strip()}\n")


if __name__ == "__main__":
    app = CommandHub()
    app.mainloop()
