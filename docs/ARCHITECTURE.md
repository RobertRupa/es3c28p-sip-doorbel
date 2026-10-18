# Architektura i granice pierwszej wersji

Baza sprzętowa: [RobertRupa/28lcd](https://github.com/RobertRupa/28lcd), LCDWiki ES3C28P 2.8-inch ESP32-S3.
Piny i sterownik wyświetlacza pochodzą z projektu źródłowego.

Moduły:
- LVGL 9 + LovyanGFX: ekran statusu i diagnostyki.
- HTTP WebGUI: konfiguracja Wi-Fi, SIP i UniFi; wymaga hasła nadanego lokalnie po USB (komenda `ADMIN bardzo-mocne-haslo`).
- SIP UDP: transmisja prób REGISTER i odbiór odpowiedzi; bez Digest i bez sesji głosowej.
- UniFi Access: struktura ustawień, bez odpytywania lub otwierania zamka przed poprawnym skonfigurowaniem weryfikacji TLS i zgodnych endpointów API.
- ESP32-H2: piny S3 GPIO44 TX -> H2 RX oraz S3 GPIO43 RX <- H2 TX; wspólna masa i logika 3.3V. Brak aktualnego firmware H2.

## Roadmap
1. Skonfigurować certyfikat CA i bezpieczne przechowywanie tokenu UniFi Access oraz dopasować endpointy do wersji Access.
2. Dodać SIP Digest (401/407), parser transakcji SIP i obsługę INVITE/ACK/BYE/CANCEL.
3. Uruchomić jednoczesne I2S RX/TX ES8311 8 kHz, G.711 A/μ-law, RTP z jitter buffer.
4. Dodać UI połączeń i obsługę DTMF oraz kontrolę autoryzacji polecenia otwarcia.
5. Opracować binarny protokół UART H2 + aktualizacje koprocesora z hosta.

## Ostrzeżenia
Nie jest to jeszcze gotowy domofon ani klient SIP audio. Panel HTTP Basic nie szyfruje danych — w testach izoluj VLAN zarządzający; w produkcji użyj HTTPS/reverse proxy i mocniejszego schematu sesji. Hasła w NVS nie są automatycznie szyfrowane bez flash/NVS encryption. Nie umieszczaj sekretów w Git.
