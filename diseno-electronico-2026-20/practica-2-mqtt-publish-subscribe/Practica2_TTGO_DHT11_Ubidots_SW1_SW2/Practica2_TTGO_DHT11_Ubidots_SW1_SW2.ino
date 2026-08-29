#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <TFT_eSPI.h>

// ======================================================
// CONFIGURACIÓN DEL SENSOR DHT11
// ======================================================
#define DHT_PIN 13
#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);

// ======================================================
// CONFIGURACIÓN WIFI
// ======================================================
// IMPORTANTE: reemplaza estos valores por los de tu propia
// red antes de subir el código. NUNCA subas tus credenciales
// reales a un repositorio público de GitHub.
const char* ssid = "TU_SSID_AQUI";
const char* password = "TU_PASSWORD_AQUI";

// ======================================================
// CONFIGURACIÓN UBIDOTS
// ======================================================
// IMPORTANTE: reemplaza por tu propio token de Ubidots.
// Nunca subas tu token real a un repositorio público.
const char* UBIDOTS_TOKEN =
  "COLOCA_AQUI_TU_TOKEN";

const char* mqttServer = "industrial.api.ubidots.com";
const int mqttPort = 1883;

const char* deviceLabel = "sensor-temperatura-humedad";

const char* variableTemp = "temperatura";
const char* variableHum  = "humedad";
const char* variableSw1  = "sw1";
const char* variableSw2  = "sw2";

// ======================================================
// OBJETOS GLOBALES
// ======================================================
WiFiClient espClient;
PubSubClient client(espClient);
TFT_eSPI tft = TFT_eSPI();

// Estados recibidos desde Ubidots
int sw1 = 0;
int sw2 = 0;

// Últimos valores medidos
float temperatura = 0.0;
float humedad = 0.0;

// Control del tiempo de publicación
unsigned long tiempoAnterior = 0;
const unsigned long intervaloPublicacion = 5000;

// ======================================================
// MOSTRAR DATOS EN LA PANTALLA
// ======================================================
void mostrarDatos(float temp, float hum) {
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  tft.setCursor(10, 10);
  tft.print("Temp: ");
  tft.print(temp, 1);
  tft.println(" C");

  tft.setCursor(10, 40);
  tft.print("Humedad: ");
  tft.print(hum, 1);
  tft.println(" %");

  uint16_t color;

  if (sw1 == 1 && sw2 == 1) {
    color = TFT_GREEN;
  } else if (sw1 == 1) {
    color = TFT_RED;
  } else if (sw2 == 1) {
    color = TFT_BLUE;
  } else {
    color = TFT_DARKGREY;
  }

  tft.fillCircle(160, 100, 20, color);

  tft.setTextSize(1);

  tft.setCursor(130, 125);
  tft.print("SW1: ");
  tft.print(sw1);

  tft.setCursor(130, 140);
  tft.print("SW2: ");
  tft.print(sw2);
}

// ======================================================
// MOSTRAR ERROR DEL DHT11
// ======================================================
void mostrarErrorDHT11() {
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setTextSize(2);

  tft.setCursor(10, 20);
  tft.println("Error DHT11");

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);

  tft.setCursor(10, 60);
  tft.println("Revise el sensor");

  tft.setCursor(10, 75);
  tft.println("Senal: GPIO 13");
}

// ======================================================
// CALLBACK MQTT
// ======================================================
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";

  for (unsigned int i = 0; i < length; i++) {
    message += static_cast<char>(payload[i]);
  }

  String topicRecibido = String(topic);
  int value = message.toInt();

  Serial.println("\nMensaje MQTT recibido");

  Serial.print("Topic: ");
  Serial.println(topicRecibido);

  Serial.print("Valor: ");
  Serial.println(value);

  if (topicRecibido.endsWith("/sw1/lv")) {
    sw1 = value;
    Serial.println("SW1 actualizado");
  } else if (topicRecibido.endsWith("/sw2/lv")) {
    sw2 = value;
    Serial.println("SW2 actualizado");
  }

  // Actualizar inmediatamente el círculo de la pantalla
  mostrarDatos(temperatura, humedad);
}

// ======================================================
// CONEXIÓN WIFI
// ======================================================
void setupWiFi() {
  Serial.print("Conectando a WiFi");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println("WiFi conectado");

  Serial.print("Direccion IP: ");
  Serial.println(WiFi.localIP());
}

// ======================================================
// CONEXIÓN MQTT
// ======================================================
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando a Ubidots MQTT...");

    // ID único basado en la dirección MAC del ESP32
    String clientId = "ESP32-";
    clientId += String(
      static_cast<uint32_t>(ESP.getEfuseMac()),
      HEX
    );

    if (
      client.connect(
        clientId.c_str(),
        UBIDOTS_TOKEN,
        ""
      )
    ) {
      Serial.println(" conectado");

      String topicSw1 =
        String("/v1.6/devices/") +
        deviceLabel +
        "/" +
        variableSw1 +
        "/lv";

      String topicSw2 =
        String("/v1.6/devices/") +
        deviceLabel +
        "/" +
        variableSw2 +
        "/lv";

      client.subscribe(topicSw1.c_str());
      client.subscribe(topicSw2.c_str());

      Serial.print("Suscrito a: ");
      Serial.println(topicSw1);

      Serial.print("Suscrito a: ");
      Serial.println(topicSw2);

    } else {
      Serial.print("Error MQTT, codigo: ");
      Serial.println(client.state());

      Serial.println("Nuevo intento en 5 segundos");
      delay(5000);
    }
  }
}

// ======================================================
// PUBLICAR DATOS EN UBIDOTS
// ======================================================
void publicarDatos(float temp, float hum) {
  String topic =
    String("/v1.6/devices/") +
    deviceLabel;

  String payload = "{";

  payload += "\"";
  payload += variableTemp;
  payload += "\":";
  payload += String(temp, 1);

  payload += ",";

  payload += "\"";
  payload += variableHum;
  payload += "\":";
  payload += String(hum, 1);

  payload += "}";

  bool enviado = client.publish(
    topic.c_str(),
    payload.c_str()
  );

  Serial.println("\nPublicacion MQTT");

  Serial.print("Topic: ");
  Serial.println(topic);

  Serial.print("Payload: ");
  Serial.println(payload);

  if (enviado) {
    Serial.println("Datos enviados correctamente");
  } else {
    Serial.println("No se pudieron enviar los datos");
  }
}

// ======================================================
// SETUP
// ======================================================
void setup() {
  Serial.begin(115200);
  delay(500);

  // Iniciar el sensor DHT11
  dht.begin();

  // Iniciar pantalla TTGO T-Display
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  tft.setCursor(10, 10);
  tft.println("Iniciando...");

  // Conectar WiFi
  setupWiFi();

  // Configurar MQTT
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  client.setBufferSize(512);
}

// ======================================================
// LOOP
// ======================================================
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    setupWiFi();
  }

  if (!client.connected()) {
    reconnectMQTT();
  }

  client.loop();

  unsigned long tiempoActual = millis();

  if (
    tiempoActual - tiempoAnterior >=
    intervaloPublicacion
  ) {
    tiempoAnterior = tiempoActual;

    // Leer el sensor DHT11
    humedad = dht.readHumidity();
    temperatura = dht.readTemperature();

    // Verificar que las lecturas sean válidas
    if (
      isnan(temperatura) ||
      isnan(humedad)
    ) {
      Serial.println("\nError al leer el sensor DHT11");
      mostrarErrorDHT11();
      return;
    }

    Serial.println("\nLectura del DHT11");

    Serial.print("Temperatura: ");
    Serial.print(temperatura, 1);
    Serial.println(" C");

    Serial.print("Humedad: ");
    Serial.print(humedad, 1);
    Serial.println(" %");

    publicarDatos(temperatura, humedad);
    mostrarDatos(temperatura, humedad);
  }
}
