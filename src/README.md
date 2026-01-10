# Projekt Czat TCP

Prosta aplikacja czatu typu klient-serwer napisana w C++ (serwer) i Python (klient).

## Wymagania

### Serwer (Linux/Unix)
- Kompilator GCC z obsługą C++17
- Biblioteka SQLite3 (`libsqlite3-dev`)
- `pthread` (standardowo w systemie)

### Klient
- Python 3.x
- Biblioteka `customtkinter`

## Instalacja i Uruchomienie

### 1. Serwer
Należy wejść do katalogu z kodem źródłowym i skompilować serwer poleceniem `make`:

```bash
cd src
make
```

Uruchomienie serwera:
```bash
./server
```
Serwer nasłuchuje na porcie **1100**. Baza danych `chat_database.db` zostanie utworzona automatycznie przy pierwszym uruchomieniu.

### 2. Klient
Wymaga zainstalowania zależności:
```bash
pip install -r requirements.txt
```

Uruchomienie klienta:
```bash
python3 client.py
```

## Funkcje
- Rejestracja i logowanie użytkowników.
- Lista dostępnych użytkowników (status online/offline).
- Wysyłanie wiadomości prywatnych.
- Tworzenie grup i czaty grupowe.
- Historia wiadomości przechowywana w bazie SQLite.
