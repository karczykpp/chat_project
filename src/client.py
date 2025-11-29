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

    def send_request(self, command):
        username = self.entry_user.get()
        password = self.entry_pass.get()

        if not username or not password:
            messagebox.showwarning("Brak danych", "Wypełnij login i hasło!")
            return None

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
        self.handle_response(response, "LOGIN")

    def action_register(self):
        response = self.send_request("REGISTER")
        self.handle_response(response, "REGISTER")

    def handle_response(self, response, action_type):
        if not response: return

        status = response.get("status")
        message = response.get("message")

        if status == "SUCCESS":
            self.label_status.configure(text=f"Sukces: {message}", text_color="green")
            messagebox.showinfo("Sukces", message)
            
            # Tu w przyszłości otworzysz okno czatu!!!!!!!!!!!!!
            if action_type == "LOGIN":
                print("Przechodzenie do okna czatu...") 

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