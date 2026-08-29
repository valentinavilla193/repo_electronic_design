# Práctica 2 — Publish y Suscribe con MQTT

**Curso:** Diseño Electrónico 2026-20
**Plazo original:** 5 de agosto

## Objetivo

Extender el sistema de la Práctica 1 para que además de **publicar**
temperatura y humedad, el ESP32 se **suscriba** a dos variables de
control (`sw1` y `sw2`) definidas en Ubidots, y reaccione a los cambios
recibidos en tiempo real (representados con un círculo de color en la
pantalla TFT).

## Hardware utilizado

- Placa **TTGO T-Display (ESP32)** con pantalla TFT integrada
- Sensor **DHT11**, conectado al pin **GPIO 13**

## Librerías necesarias (Arduino IDE)

- `WiFi.h` (incluida con el core de ESP32)
- `PubSubClient` — cliente MQTT (publish/subscribe)
- `DHT sensor library` (Adafruit)
- `TFT_eSPI` (autor: Bodmer)

## Funcionamiento

1. Conexión a WiFi y al broker MQTT de Ubidots.
2. El ESP32 se **suscribe** a los topics `.../sw1/lv` y `.../sw2/lv`.
3. Cada 5 segundos publica `temperatura` y `humedad` en Ubidots.
4. Cuando llega un mensaje MQTT de `sw1` o `sw2` (por ejemplo, al mover
   un switch en el dashboard de Ubidots), el `callback` actualiza el
   estado y la pantalla cambia el color del círculo indicador:
   - Verde: `sw1 = 1` y `sw2 = 1`
   - Rojo: solo `sw1 = 1`
   - Azul: solo `sw2 = 1`
   - Gris: ambos apagados

## Configuración antes de usar

Abre el archivo `.ino` y reemplaza:

| Variable | Descripción |
|---|---|
| `ssid` | Nombre de tu red WiFi |
| `password` | Contraseña de tu red WiFi |
| `UBIDOTS_TOKEN` | Tu token de la cuenta de Ubidots |

> Estos valores se dejaron como marcadores de posición (placeholders) a
> propósito.

## Cómo abrir el proyecto

Abre la carpeta `Practica2_TTGO_DHT11_Ubidots_SW1_SW2/` desde el Arduino
IDE (el nombre de la carpeta coincide con el del `.ino`).


