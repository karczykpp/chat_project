#!/bin/bash

# --- SEKCJA RATUNKOWA: NAPRAWA EKRANU ---
# Jeśli system nie wie gdzie wyświetlić obraz (zmienna pusta),
# wymuszamy ekran :0 (standardowy monitor).
if [ -z "$DISPLAY" ]; then
  export DISPLAY=:0
fi
echo "[INFO] Używam ekranu: $DISPLAY"
# ----------------------------------------

# Wyłączamy buildkit (dla zgodności z SUSE w labie)
export DOCKER_BUILDKIT=0

echo "=========================================="
echo "   Chat Client Launcher - Docker Setup"
echo "=========================================="

# Pytamy o IP serwera
read -p "Podaj adres IP serwera (np. 192.168.1.15): " SERVER_IP
if [ -z "$SERVER_IP" ]; then
  echo "Nie podano IP! Używam domyślnego 127.0.0.1"
  SERVER_IP="127.0.0.1"
fi

# 1. Odblokowanie X11 (jako zwykły użytkownik, bo to jego ekran)
echo "[INFO] Konfiguracja X11..."
xhost +local:docker

# 2. Budowanie (z SUDO)
echo "[INFO] Budowanie obrazu Dockera (może zapytać o hasło)..."
sudo DOCKER_BUILDKIT=0 docker build -t chat-client .

# 3. Uruchamianie (z SUDO)
# --net=host pozwala na łatwiejszą komunikację w sieci lokalnej
# Przekazujemy na sztywno DISPLAY, który ustawiliśmy na górze
echo "[INFO] Uruchamianie klienta..."
sudo DOCKER_BUILDKIT=0 docker run -it --rm \
  --net=host \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -e SERVER_IP="$SERVER_IP" \
  chat-client
