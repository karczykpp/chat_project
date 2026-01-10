import customtkinter as ctk
import socket
import json
from tkinter import messagebox
import threading
from datetime import datetime

HOST = '127.0.0.1'
PORT = 1100

ctk.set_appearance_mode("Dark") 
ctk.set_default_color_theme("blue") 

class ModernChatClient(ctk.CTk):
    def __init__(self):
        super().__init__()
        self.current_user = ""
        self.all_users = []
        self.receive_thread = None
        self.online_friends = []
        self.selected_users = []

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


        self.update_friends_ui()

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
            self.send_message_request(msg)
            
    def send_message_request(self, msg):
        if not self.current_chat_friend:
            messagebox.showwarning("Błąd", "Wybierz kogoś z listy kontaktów")
            return 
        message_text = msg
        if not message_text.strip():
            return
        
        now = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        request_data = {
            "command": "SEND_MESSAGE",
            "sender": self.current_user,
            "receiver": self.current_chat_friend,
            "content": message_text,
            "timestamp": now
        }

        msg_to_server = json.dumps(request_data) + "\n"
        self.sock.sendall(msg_to_server.encode('utf-8'))
        print(msg_to_server)
        self.entry_msg.delete(0, "end")
        # new_request = {
        #     "command": "GET_MESSAGES",
        #     "sender": self.current_user,
        #     "receiver": self.current_chat_friend
        # }
        # self.sock.sendall((json.dumps(new_request) + "\n").encode('utf-8'))

    def update_friends_ui(self):
        """Przerysowuje listę znajomych na podstawie aktualnych danych"""
        
        if not hasattr(self, 'friends_list') or not self.friends_list.winfo_exists():
            return

        for widget in self.friends_list.winfo_children():
            widget.destroy()
        
        btn_group = ctk.CTkButton(self.friends_list, text="Nowa grupa", 
                                  command=self.action_create_group,
                                  fg_color="#3498db", hover_color="#2980b9",
                                  height=32, font=("Roboto", 12, "bold"))
        btn_group.pack(pady=(5, 10), padx=10, fill="x")

        raw_all = self.all_users or ""
        raw_online = self.online_users or ""

        all_friends = [x.strip() for x in raw_all.split(",") if x.strip()]
        online_friends = [x.strip() for x in raw_online.split(",") if x.strip()]
        print("Aktualizacja UI znajomych:")
        print("Wszyscy znajomi:", all_friends)
        print("Online znajomi:", online_friends)
        self.check_vars = {} 
        for friend in all_friends:
            if friend == self.current_user:
                continue

            is_group = "(Grupa)" in friend
            clean_name = friend.replace(" (Grupa)", "")
            container = ctk.CTkFrame(self.friends_list, fg_color="transparent")
            container.pack(fill="x", pady=1, padx=5)

            if is_group:
                text_col = "#3498db" 
                display_name = f"👥 {clean_name}"
            else:
                text_col = "#2cc985" if friend in online_friends else "gray60"
                display_name = f"● {friend}" if friend in online_friends else f"○ {friend}"    

            btn = ctk.CTkButton(container, 
                                text=display_name, 
                                fg_color="transparent", 
                                text_color=text_col, 
                                anchor="w", 
                                command=lambda f=clean_name: self.send_get_message_request(f),
                                height=35,
                                hover_color=("gray80", "gray25"))
            btn.pack(side="left", fill="x", expand=True)

            if not is_group:
                var = ctk.StringVar(value="off")
                self.check_vars[clean_name] = var
                cb = ctk.CTkCheckBox(container, text="", variable=var, 
                                     onvalue="on", offvalue="off",
                                     width=24, checkbox_width=18, checkbox_height=18)
                cb.pack(side="right", padx=(0, 10))

    def send_request_logout(self):
        username = self.current_user
        command = "LOGOUT"
        request_data = {
            "command": command,
            "username": username,
        }
        try:
            msg = json.dumps(request_data) 
            self.sock.sendall(msg.encode('utf-8'))
        except Exception as e:
            messagebox.showerror("Błąd sieci", str(e))
            return None
    
    def send_request(self, command):

        username = ""
        password = ""

        if command in ["LOGIN", "REGISTER"]:
            username = self.entry_user.get()
            password = self.entry_pass.get()
            
            if username:
                self.current_user = username

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
            print(msg)
            self.sock.sendall(msg.encode('utf-8'))

        except Exception as e:
            messagebox.showerror("Błąd sieci", str(e))
            return None
    
    def listen_for_messages(self):
        while True:
            try:
                buffer = self.sock.recv(65536)
                if not buffer:
                    break
                data = buffer.decode('utf-8').strip()
                for line in data.split('\n'):
                    if not line: continue
                    response = json.loads(line)
                    print(response)
                    if isinstance(response, list):
                        self.after(0, lambda r=response: self.action_get_messages(r))
                    elif isinstance(response, dict):
                        status = response.get("status")
                        command = response.get("command")
                        print(f"Status: {status}, Command: {command}")

                        if command == "USER_ONLINE":
                            self.handle_server_message(response)

                        elif response.get("message") == "Login successful.":
                            self.after(0, lambda r=response: self.handle_login_success(r))

                        elif response.get("message") == "User registered successfully.":
                            self.after(0, self.handle_registration_success)

                        elif status == "USER_NOT_FOUND":
                            self.after(0, self.handle_user_not_found)

                        elif status == "SUCCESS":
                            pass
                        
                        else:
                             message = response.get("message", "")
                             self.after(0, lambda m=message: self.handle_error(m))


            except Exception as e:
                print("Błąd podczas odbierania wiadomości:", e)
                break

    def handle_error(self, message):
        if "UNIQUE constraint failed" in message:
             messagebox.showerror("Błąd rejestracji", "Taki użytkownik już istnieje. Wybierz inną nazwę.")
        else:
             messagebox.showerror("Błąd serwera", message)

    def handle_login_success(self, response):
        print("Przechodzenie do okna czatu...") 
        print(response)
        all_users = response.get("all_users")
        self.all_users = all_users
        online_users = response.get("online_users")
        self.online_users = online_users
        
        if not self.current_user:
             self.current_user = response.get("username", "") 
        
        self.switch_to_chat()

    def handle_registration_success(self):
        print("Zarejestrowano pomyślnie. Automatyczne logowanie...")
        self.action_login()

    def handle_user_not_found(self):
        self.label_status.configure(text="Nie znaleziono użytkownika", text_color="orange")
        if messagebox.askyesno("Błąd", "Użytkownik nie istnieje. Chcesz się zarejestrować?"):
            self.action_register()
    
    def switch_to_chat(self):
        print("Logowanie")
        if hasattr(self, 'frame') and self.frame.winfo_exists():
            self.frame.destroy()
        self.main_chat_window()

    def handle_server_message(self, message):
        print("FUNKCJA UZYTKOWNIKOW")
        if message.get("command") == "USER_ONLINE":
            username = message.get("username")
            if username:
                help_online = message.get("online_users")
                self.online_users = help_online
                print("Zaktualizowana lista online użytkowników:", self.online_users)
                self.after(0, self.update_friends_ui)

    def action_login(self):
        self.start_listening()
        self.send_request("LOGIN")

    def action_register(self):
        self.start_listening()
        response = self.send_request("REGISTER")
    
    def start_listening(self):
        if self.receive_thread is None or not self.receive_thread.is_alive():
             self.receive_thread = threading.Thread(target=self.listen_for_messages, daemon=True)
             self.receive_thread.start()

    def action_logout(self):
        print("Wylogowywanie użytkownika...")
        
        request_data = {
            "command": "LOGOUT",
            "username": self.current_user,
        }
        try:
            msg = json.dumps(request_data) + "\n"
            self.sock.sendall(msg.encode('utf-8'))
        except:
            pass

        try:
            self.sock.shutdown(socket.SHUT_RDWR)
            self.sock.close()
        except:
            pass

        if hasattr(self, 'sidebar_frame'): self.sidebar_frame.destroy()
        if hasattr(self, 'main_area'): self.main_area.destroy()
        
        self.current_user = ""
        self.all_users = []
        self.online_users = ""
    
        self.create_widgets()

        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect((HOST, PORT))
            print("Nowy socket gotowy do kolejnego logowania.")
            self.receive_thread = None
        except Exception as e:
            messagebox.showerror("Błąd", "Nie można połączyć się ponownie z serwerem.")

    def action_create_group(self):
        selected_members = []
        for friend, var in self.check_vars.items():
            if var.get() == "on":
                selected_members.append(friend)
        if len(selected_members) < 2:
            messagebox.showwarning("Grupa", "Wybierz co najmniej 2 osoby!")
            return
        
        dialog = ctk.CTkInputDialog(text="Podaj nazwę dla nowej grupy:", title="Tworzenie grupy")
        group_name = dialog.get_input()

        if group_name and group_name.strip():
            selected_members.append(self.current_user)
            
            request_data = {
                "command": "CREATE_GROUP",
                "group_name": group_name.strip(),
                "created_by": self.current_user,
                "members": selected_members
            }
            
            try:
                msg = json.dumps(request_data) + "\n"
                self.sock.sendall(msg.encode('utf-8'))
                for var in self.check_vars.values():
                    var.set("off")
                messagebox.showinfo("Sukces", f"Wysłano prośbę o utworzenie grupy: {group_name}")
            except Exception as e:
                messagebox.showerror("Błąd", f"Nie udało się wysłać prośby: {e}")

    def send_get_message_request(self, friend):
        self.current_chat_friend = friend
        sender = self.current_user
        receiver = friend
        command = "GET_MESSAGES"
        request_data = {
            "sender": sender,
            "receiver": receiver,
            "command": command
        }
        try:
            msg = json.dumps(request_data) + "\n"
            self.sock.sendall(msg.encode('utf-8'))
            print(msg)
        except Exception as e:
            messagebox.showerror("Błąd sieci", str(e))
            return None
    
    def action_get_messages(self, response):
        print("Receiver to: ", self.current_chat_friend)
        self.chat_history.configure(state="normal")
        self.chat_history.delete("1.0", "end")
        self.chat_history.insert("end", f"--- Rozmowa z {self.current_chat_friend} ---\n\n")

        print("odpowiedz to: ", response)
        if response:
            for msg in response:
                print(response, type(response))
                if msg.get("status") == "EMPTY":
                    self.chat_history.insert("end", "--- Brak historii wiadomości ---\n")
                    continue
                sender = msg.get('sender', 'Unknown')
                content = msg.get('content', '')
                time = msg.get('time', '')

                line = f"[{time}] {sender}: {content}"
                print(line)
                if sender == self.current_user:
                    display_line = f"[{time}] TY: {content}\n"
                else:
                    display_line = f"[{time}] {sender}: {content}\n"
                self.chat_history.insert("end", display_line)
        else:
            self.chat_history.insert("end", "Brak poprzednich wiadomości.\n")
        self.chat_history.configure(state="disabled")
        self.chat_history.see("end")

    def handle_response(self, response, action_type):
        if not response: return

        status = response.get("status")
        message = response.get("message")

        print("Handling response:", response)
        print("Action type:", action_type, status, message)

        if status == "SUCCESS":
            print("Operacja zakończona sukcesem.")
            messagebox.showinfo("Sukces", message)
            
            if action_type == "LOGIN":
                print("Przechodzenie do okna czatu...") 
                self.current_user = self.entry_user.get()
                self.frame.destroy()
                self.main_chat_window()

                receive_thread = threading.Thread(target=self.listen_for_messages, daemon=True)
                receive_thread.start()

        elif status == "USER_NOT_FOUND":
            self.label_status.configure(text="Nie znaleziono użytkownika", text_color="orange")
            if messagebox.askyesno("Błąd", "Użytkownik nie istnieje. Chcesz się zarejestrować?"):
                self.action_register()

        else: 
            self.label_status.configure(text=f"Błąd: {message}", text_color="red")
            self.after(0, lambda: self.handle_error(message))

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