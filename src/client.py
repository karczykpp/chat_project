import customtkinter as ctk
import socket
import json
from tkinter import messagebox

HOST = '127.0.0.1'
PORT = 1104 

ctk.set_appearance_mode("Dark") 
ctk.set_default_color_theme("blue") 

class ModernChatClient(ctk.CTk):
    def __init__(self):
        super().__init__()
        self.current_user = ""
        self.all_users = []
        self.online_users = []

        self.title("Komunikator - Klient")
        self.geometry("900x500")
        self.resizable(False, False)

        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect((HOST, PORT))
        except Exception as e:
            messagebox.showerror("Błąd krytyczny", f"Nie można połączyć z serwerem!\n{e}")
            self.destroy()
            return

        self.create_widgets()

    def create_widgets(self):
        self.frame = ctk.CTkFrame(self)
        self.frame.pack(pady=20, padx=20, fill="both", expand=True)

        self.label_title = ctk.CTkLabel(self.frame, text="Witaj w CzatApp", font=("Roboto", 24, "bold"))
        self.label_title.pack(pady=30)

        self.entry_user = ctk.CTkEntry(self.frame, placeholder_text="Nazwa użytkownika", width=250, height=40)
        self.entry_user.pack(pady=10)

        self.entry_pass = ctk.CTkEntry(self.frame, placeholder_text="Hasło", show="*", width=250, height=40)
        self.entry_pass.pack(pady=10)

        self.btn_login = ctk.CTkButton(self.frame, text="Zaloguj się", command=self.action_login, width=250, height=40)
        self.btn_login.pack(pady=(20, 10)) 

        self.btn_register = ctk.CTkButton(self.frame, text="Utwórz konto", command=self.action_register, 
                                          width=250, height=40, fg_color="transparent", border_width=2, text_color=("gray10", "#DCE4EE"))
        self.btn_register.pack(pady=10)

        self.label_status = ctk.CTkLabel(self.frame, text="Połączono z serwerem 127.0.0.1", text_color="gray")
        self.label_status.pack(side="bottom", pady=10)

    def main_chat_window(self):
        self.grid_columnconfigure(0, weight=0)
        self.grid_columnconfigure(1, weight=1)
        self.grid_rowconfigure(0, weight=1)

        self.sidebar_frame = ctk.CTkFrame(self, width=200, corner_radius=0)
        self.sidebar_frame.grid(row=0, column=0, sticky="nsew")
        
        self.logo_label = ctk.CTkLabel(self.sidebar_frame, text="CzatApp \nKontakty", font=ctk.CTkFont(size=20, weight="bold"))
        self.logo_label.grid(row=0, column=0, padx=20, pady=(20, 10))

        self.friends_list = ctk.CTkScrollableFrame(self.sidebar_frame, label_text="Dostępni")
        self.friends_list.grid(row=1, column=0, padx=20, pady=10, sticky="nsew")
        
        self.sidebar_frame.grid_rowconfigure(1, weight=1)

        print("Wszyscy uzytkownicy: ", self.all_users)
        print("Online uzytkownicy: ", self.online_users)
        friends = self.all_users.split(",")
        for friend in friends:
            btn = ctk.CTkButton(self.friends_list, text=friend, fg_color="transparent", border_width=1, text_color=("gray10", "#DCE4EE"))
            btn.pack(pady=5, padx=5, fill="x")

        self.user_info_label = ctk.CTkLabel(self.sidebar_frame, text=f"{self.current_user}", anchor="w")
        self.user_info_label.grid(row=2, column=0, padx=20, pady=(10, 0), sticky="ew")

        self.btn_logout = ctk.CTkButton(self.sidebar_frame, text="Wyloguj", command=self.action_logout, fg_color="#d63031", hover_color="#ff7675")
        self.btn_logout.grid(row=3, column=0, padx=20, pady=20)


        self.main_area = ctk.CTkFrame(self, corner_radius=0, fg_color="transparent")
        self.main_area.grid(row=0, column=1, sticky="nsew")
        
        self.main_area.grid_rowconfigure(0, weight=1)
        self.main_area.grid_rowconfigure(1, weight=0) 
        self.main_area.grid_columnconfigure(0, weight=1)

        self.chat_history = ctk.CTkTextbox(self.main_area, width=250)
        self.chat_history.grid(row=0, column=0, padx=20, pady=(20, 10), sticky="nsew")
        self.chat_history.insert("0.0", "Witaj na czacie!\nTutaj pojawią się wiadomości.\n\n")
        self.chat_history.configure(state="disabled") 

        self.entry_frame = ctk.CTkFrame(self.main_area, fg_color="transparent")
        self.entry_frame.grid(row=1, column=0, padx=20, pady=20, sticky="ew")

        self.entry_msg = ctk.CTkEntry(self.entry_frame, placeholder_text="Napisz wiadomość...")
        self.entry_msg.pack(side="left", fill="x", expand=True, padx=(0, 10))
        
        self.entry_msg.bind("<Return>", lambda event: self.send_message_gui())

        self.btn_send = ctk.CTkButton(self.entry_frame, text="Wyślij ➤", width=80, command=self.send_message_gui)
        self.btn_send.pack(side="right")
    
    def send_message_gui(self):
        msg = self.entry_msg.get()
        if msg:
            self.chat_history.configure(state="normal")
            self.chat_history.insert("end", f"Ja: {msg}\n")
            self.chat_history.configure(state="disabled")
            self.chat_history.see("end")
            self.entry_msg.delete(0, "end")
            # TUTAJ W PRZYSZŁOŚCI WYŚLESZ JSON DO SERWERA C++

    def send_request(self, command):

        username = ""
        password = ""

        if command in ["LOGIN", "REGISTER"]:
            username = self.entry_user.get()
            password = self.entry_pass.get()

            if not username or not password:
                messagebox.showwarning("Brak danych", "Wypełnij login i hasło!")
                return None
        else:
            username = self.current_user
            password = ""

        request_data = {
            "command": command,
            "username": username,
            "password": password
        }

        try:
            msg = json.dumps(request_data)
            self.sock.sendall(msg.encode('utf-8'))

            buffer = self.sock.recv(2048)
            if not buffer:
                return None
            
            response = json.loads(buffer.decode('utf-8'))
            return response

        except Exception as e:
            messagebox.showerror("Błąd sieci", str(e))
            return None

    def action_login(self):
        response = self.send_request("LOGIN")
        print(response)
        all_users = response.get("all_users", [])
        self.all_users = all_users
        print(self.all_users, "XXXX")
        online_users = response.get("online_users", [])
        self.online_users = online_users
        self.handle_response(response, "LOGIN")

    def action_register(self):
        response = self.send_request("REGISTER")
        self.handle_response(response, "REGISTER")

    def action_logout(self):
        print("Wylogowywanie użytkownika...")
        response = self.send_request("LOGOUT")
        print(response)
        self.handle_response(response, "LOGOUT")

    def handle_response(self, response, action_type):
        if not response: return

        status = response.get("status")
        message = response.get("message")

        print("Handling response:", response)
        print("Action type:", action_type, status, message)

        if status == "SUCCESS":
            print("Operacja zakończona sukcesem.")
            messagebox.showinfo("Sukces", message)
            
            # Tu w przyszłości otworzysz okno czatu!!!!!!!!!!!!!

            if action_type == "LOGIN":
                print("Przechodzenie do okna czatu...") 
                self.current_user = self.entry_user.get()
                self.frame.destroy()
                self.main_chat_window()
            elif action_type == "LOGOUT":
                print("Wylogowywanie...")
                if hasattr(self, 'sidebar_frame'): self.sidebar_frame.destroy()
                if hasattr(self, 'main_area'): self.main_area.destroy()

                self.grid_columnconfigure(0, weight=0)
                self.grid_columnconfigure(1, weight=0)
                self.grid_rowconfigure(0, weight=0)

                self.current_user = ""

                self.create_widgets()

        elif status == "USER_NOT_FOUND":
            self.label_status.configure(text="Nie znaleziono użytkownika", text_color="orange")
            if messagebox.askyesno("Błąd", "Użytkownik nie istnieje. Chcesz się zarejestrować?"):
                self.action_register()

        else: 
            self.label_status.configure(text=f"Błąd: {message}", text_color="red")
            messagebox.showerror("Błąd serwera", message)

    def on_closing(self):
        try:
            self.sock.close()
        except:
            pass
        self.destroy()

if __name__ == "__main__":
    app = ModernChatClient()
    app.protocol("WM_DELETE_WINDOW", app.on_closing)
    app.mainloop()