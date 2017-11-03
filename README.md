# JulianaNFC
Koppelvlak voor Alexia. Leest NFC-kaarten en gooit ze op de WebSocket.

## Gebruik
1. Pak de laatste versie van [releases](https://github.com/Inter-Actief/JulianaNFC_C/releases) en pak uit.
    1. **Wil je SSL?** - Zorg voor een `juliana.crt` en `juliana.key` 
2. Start met `JulianaNFC.exe`
3. Je kan verbinden met de websocket op localhost:3000 (ws:// of wss://, afhankelijk van SSL)
4. Plug de kaartlezer in
5. Scan een [kaart](#kaartlezers), hij wordt over de websocket gegooid

## Kaartlezers
JulianaNFC is ontwikkeld voor en getest met de [ACR122U](https://www.acs.com.hk/en/products/3/acr122u-usb-nfc-reader/) van ACS. Deze is over het algemeen goed verkrijgbaar. Zorg er ook voor dat de PC/SC Drivers geïnstalleerd zijn.

## Protocol
Tijdens het verbinden moet je `nfc` als WebSocket protocol meegeven.

```javascript
new WebSocket("ws://localhost:3000", "nfc");
```

De data die je ontvangt bij een scan is in de vorm van JSON.

```javascript
{
  "type": "iso-a",
  "uid": "aa:bb:cc:dd",
  "atqa": "aa:bb",      // alleen bij iso-a
  "sak": "aa",          // alleen bij iso-a
}
```

## Compileren
JulianaNFC is geschreven in pure C en Windows API. De meegeleverde projectfiles zijn voor Visual Studio 2017. Sterke aanrader is om daarnaast [vcpkg](https://github.com/Microsoft/vcpkg) te gebruiken. JulianaNFC is afhankelijk van [libwebsockets](https://libwebsockets.org/).

Gebruik om de dependencies te installeren `vcpkg install libwebsockets:x86-windows libwebsockets:x64-windows`.

Daarna kun je de solution openen in builden, dat zou dan verder probleemloos moeten verlopen.

## Vragen?
Contact mij (Cas Ebbers) of de WWW-commissie van [Inter-Actief](https://www.inter-actief.net).
