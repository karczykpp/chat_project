#!/bin/bash

# 1. Pobranie IP serwera od użytkownika
echo "=========================================="
echo "   Chat Client Launcher - Docker Setup"
echo "=========================================="
read -p "Podaj adres IP serwera (np. 192.168.1.15): " SERVER_IP

if [ -z "$SERVER_IP" ]; then
  echo "Nie podano IP! Używam domyślnego 127.0.0.1 (tylko localhost)"
  SERVER_IP="127.0.0.1"
fi

# 2. Odblokowanie dostępu do X Servera (dla GUI)
echo "[INFO] Konfiguracja X11..."
xhost +local:docker

# 3. Budowanie obrazu (aby mieć pewność, że kod jest aktualny po git pull)
echo "[INFO] Budowanie obrazu Dockera..."
docker build -t chat-client .

# 4. Uruchomienie kontenera
echo "[INFO] Uruchamianie klienta..."
docker run -it --rm \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -e SERVER_IP="$SERVER_IP" \
  chat-client

# 5. Sprzątanie uprawnień (opcjonalne, dla bezpieczeństwa po zamknięciu)
# xhost -local:docker