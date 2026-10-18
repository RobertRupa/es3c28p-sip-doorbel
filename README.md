# ES3C28P SIP Doorbell

Prototyp własnego interkomu SIP + UniFi Access dla LCDWiki ES3C28P (ESP32-S3), wydzielony z [28lcd](https://github.com/RobertRupa/28lcd).

**Status: firmware infrastrukturalny / signaling MVP, nie gotowy interkom.**
- LVGL 9.2.2 + LovyanGFX: 240x320 ILI9341, FT6336G, diagnostyka ES8311
- Wi-Fi NVS, WebGUI konfiguracyjne na porcie 80, konfiguracja hasła administratora po lokalnym USB
- SIP UDP REGISTER (bez Digest), odczyt odpowiedzi SIP
- Szablon integracji UniFi Access: ustawienia i bezpiecznie wyłączone zapytania do czasu skonfigurowania TLS/endpointów
- ESP32-H2: GPIO43/44 zarezerwowane pod UART

## Kompilacja (Ubuntu / VS Code + PlatformIO)

```bash
git clone https://github.com/RobertRupa/es3c28p-sip-doorbel.git
cd es3c28p-sip-doorbel
pio run -e es3c28p
pio run -e es3c28p -t upload --upload-port /dev/ttyACM0
pio device monitor -p /dev/ttyACM0 -b 115200
```

Przy pierwszym uruchomieniu na lokalnym monitorze USB wyślij `ADMIN mocne-haslo-do-panelu` oraz `WIFI moja-siec moje-haslo` (SSID z odstępami nie jest obecnie obsługiwane). Po ustawieniu Wi-Fi wejdź na `http://IP_ESP32/`, login `admin`. Pierwsze dane Wi-Fi wprowadzaj przez USB. Formularz WebGUI wymaga już połączenia sieciowego.

## Co jeszcze nie działa
- Logowanie SIP Digest do normalnej centrali; połączenia przychodzące i wychodzące; RTP/G.711 i audio pełnodupleksowe ES8311.
- Faktyczne API UniFi Access i otwieranie drzwi. Brak domyślnych endpointów/akcji — celowe zabezpieczenie.
- Aktualizowanie i obsługa ESP32-H2.

Nie przetestowano jeszcze builda ani sprzętu. Zobacz [architekturę](docs/ARCHITECTURE.md).
