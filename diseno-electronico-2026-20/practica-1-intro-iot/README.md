# Práctica 1 — Intro a IoT con Ubidots

**Curso:** Diseño Electrónico 2026-20

## Objetivo

Integrar un sistema con el sensor **DHT11** para visualizar temperatura y
humedad en la pantalla TFT del TTGO T-Display, y publicar esas dos
variables en **Ubidots** vía MQTT.

## Hardware utilizado

- Placa **TTGO T-Display (ESP32)** con pantalla TFT integrada
- Sensor **DHT11** (temperatura y humedad), conectado al pin **GPIO 13**

## Librerías necesarias (Arduino IDE)

- `WiFi.h` (incluida con el core de ESP32)
- `PubSubClient` — cliente MQTT
- `DHT sensor library` (Adafruit)
- `TFT_eSPI` (autor: Bodmer)

## Funcionamiento

1. El ESP32 se conecta a la red WiFi configurada.
2. Se conecta al broker MQTT de Ubidots (`industrial.api.ubidots.com`).
3. Cada 5 segundos, lee temperatura y humedad del DHT11.
4. Muestra los valores en la pantalla TFT (o por el monitor serial si no
   hay pantalla disponible).
5. Publica ambas variables (`temperatura`, `humedad`) en Ubidots mediante
   un mensaje JSON por MQTT.

## Configuración antes de usar

Abre el archivo `.ino` y reemplaza:

| Variable | Descripción |
|---|---|
| `ssid` | Nombre de tu red WiFi |
| `password` | Contraseña de tu red WiFi |
| `UBIDOTS_TOKEN` | Tu token de la cuenta de Ubidots |

> Estos valores se dejaron como marcadores de posición (placeholders) a
> propósito
> Ver el `.gitignore` y el README principal del
> repositorio para más detalle.

## Cómo abrir el proyecto

Abre la carpeta `Practica1_TTGO_DHT11_Ubidots/` desde el Arduino IDE
(el nombre de la carpeta debe coincidir con el del archivo `.ino`, tal
como lo requiere Arduino).
