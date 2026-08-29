# Práctica 3 — Integración Webserver + Access Point en modo STA

**Curso:** Diseño Electrónico 2026-20
**Plazo original:** 24 de agosto

## Objetivo

Integrar un **servidor web** embebido en el ESP32 que funcione al mismo
tiempo como **Access Point (AP)** y como **cliente WiFi (STA)**, de modo
que se pueda configurar el SSID/password de la red desde una página web
servida por el propio ESP32, guardando la configuración en memoria no
volátil (`Preferences`).

## Hardware utilizado

- Placa **TTGO T-Display (ESP32)** con pantalla TFT integrada
- Sensor **DHT11**, conectado al pin **GPIO 13**

## Librerías necesarias (Arduino IDE / VS Code + PlatformIO)

- `WiFi.h`, `WebServer.h`, `Preferences.h`, `FS.h` (incluidas con el core
  de ESP32)
- `PubSubClient` — cliente MQTT
- `DHT sensor library` (Adafruit)
- `TFT_eSPI` (autor: Bodmer)

Como se indicó en el enunciado de esta práctica, las librerías de Ubidots
y de manejo de pantalla **no necesitan agregarse como librerías externas
"de Ubidots"**; la de pantalla se busca en el gestor de librerías como
`TFT_eSPI` (autor Bodmer).

## Funcionamiento

1. Al iniciar, el ESP32 carga de memoria (`Preferences`) el último SSID
   y password guardados (o usa los valores por defecto).
2. Levanta simultáneamente:
   - Un **Access Point** propio (`ESP32-DHT11`) para poder configurarlo
     sin depender de que ya esté conectado a una red.
   - Una conexión como **cliente (STA)** hacia el router configurado.
3. Sirve una página web (`http://192.168.4.1` en modo AP) con:
   - Estado actual de la conexión WiFi.
   - Un formulario para guardar una nueva red WiFi.
   - Un botón para restaurar la red WiFi predeterminada.
4. Publica temperatura y humedad en Ubidots y se suscribe a `sw1`/`sw2`,
   igual que en la Práctica 2.

## Configuración antes de usar

Abre el archivo `.ino` y reemplaza:

| Variable | Descripción |
|---|---|
| `DEFAULT_SSID` | Nombre de tu red WiFi por defecto |
| `DEFAULT_PASSWORD` | Contraseña de tu red WiFi por defecto |
| `UBIDOTS_TOKEN` | Tu token de la cuenta de Ubidots |

> `AP_SSID` / `AP_PASSWORD` son las credenciales del **Access Point que
> crea el propio ESP32** (no tus credenciales personales), pero se
> recomienda cambiar la contraseña por defecto (`12345678`) por una
> propia si vas a usar este montaje fuera de un entorno de laboratorio.

## Cómo abrir el proyecto

Abre la carpeta `Practica3_TTGO_DHT11_Ubidots_AccessPoint/` desde el
Arduino IDE, o ábrela como carpeta de proyecto en VS Code si se trabaja
con la extensión de Arduino / PlatformIO.
