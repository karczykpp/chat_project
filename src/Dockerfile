FROM python:3.10-slim

# Instalacja zależności systemowych dla GUI (Tkinter)
RUN apt-get update && apt-get install -y \
    python3-tk \
    x11-apps \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Kopiowanie kodu
COPY client.py .

# Instalacja bibliotek Pythona
RUN pip install customtkinter

# Start aplikacji
CMD ["python", "client.py"]