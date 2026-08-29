# Diseño Electrónico 2026-20

Repositorio del curso **Diseño Electrónico 2026-20**, con las prácticas
desarrolladas a lo largo del semestre usando un **TTGO T-Display (ESP32)**,
un sensor **DHT11** y la plataforma **Ubidots**.

**Estudiante:** Valen

## Contenido

| Carpeta | Práctica | Descripción |
|---|---|---|
| [`practica-1-intro-iot/`](practica-1-intro-iot/) | Práctica 1 — Intro IoT (Ubidots) | Lectura de DHT11, visualización en pantalla TFT y publicación en Ubidots. |
| [`practica-2-mqtt-publish-subscribe/`](practica-2-mqtt-publish-subscribe/) | Práctica 2 — Publish y Subscribe con MQTT | Publica temperatura/humedad y se suscribe a variables de control (`sw1`, `sw2`). |
| [`practica-3-webserver-ap/`](practica-3-webserver-ap/) | Práctica 3 — Webserver + Access Point en modo STA | Servidor web propio del ESP32 para configurar el WiFi, funcionando como AP y STA a la vez. |

La **Práctica 4 — Intro a GIT** es precisamente la creación y publicación
de este repositorio: cada práctica anterior quedó organizada en su propia
carpeta, con su propio código y su propio `README.md`.

Cada carpeta de práctica incluye su propio `README.md` con el objetivo,
el hardware/librerías usadas y las instrucciones de configuración.

## ⚠️ Sobre las credenciales (WiFi / Ubidots)

El código de cada práctica **no incluye credenciales reales**. Los
campos de SSID, password y token de Ubidots se dejaron como marcadores
de posición (por ejemplo `TU_SSID_AQUI`, `COLOCA_AQUI_TU_TOKEN`) a
propósito, porque este repositorio se sube a un GitHub **público**: subir
tu WiFi o tu token de Ubidots en texto plano los expondría a cualquiera
que vea el repositorio.

Antes de compilar y subir el código a tu placa, edita el `.ino`
correspondiente y coloca tus propios valores **solo en tu copia local**
(no los subas de vuelta al repositorio).

## Cómo subir este repositorio a GitHub

1. Crea un repositorio nuevo y vacío en GitHub (sin README, sin
   `.gitignore`, sin licencia — para evitar conflictos).
2. Desde esta carpeta (`diseno-electronico-2026-20/`), en una terminal:

   ```bash
   git init
   git add .
   git commit -m "Práctica 4: organización del repositorio del curso"
   git branch -M main
   git remote add origin https://github.com/TU_USUARIO/TU_REPOSITORIO.git
   git push -u origin main
   ```

3. Verifica en GitHub que las tres carpetas de prácticas aparezcan
   correctamente.

## Estructura del repositorio

```
diseno-electronico-2026-20/
├── README.md
├── .gitignore
├── practica-1-intro-iot/
│   ├── README.md
│   └── Practica1_TTGO_DHT11_Ubidots/
│       └── Practica1_TTGO_DHT11_Ubidots.ino
├── practica-2-mqtt-publish-subscribe/
│   ├── README.md
│   ├── evidencias/
│   │   └── README.md
│   └── Practica2_TTGO_DHT11_Ubidots_SW1_SW2/
│       └── Practica2_TTGO_DHT11_Ubidots_SW1_SW2.ino
└── practica-3-webserver-ap/
    ├── README.md
    └── Practica3_TTGO_DHT11_Ubidots_AccessPoint/
        └── Practica3_TTGO_DHT11_Ubidots_AccessPoint.ino
```
