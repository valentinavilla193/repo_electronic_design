#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <TFT_eSPI.h>

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
const char* UBIDOTS_TOKEN = "COLOCA_AQUI_TU_TOKEN";
const char* DEVICE_LABEL = "sensor-temperatura-humedad";

const char* VARIABLE_TEMP = "temperatura";
const char* VARIABLE_HUM = "humedad";

const char* mqttServer = "industrial.api.ubidots.com";
const int mqttPort = 1883;

// ======================================================
// CONFIGURACIÓN DEL SENSOR DHT
// ======================================================
#define DHTPIN 13
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// ======================================================
// OBJETOS
// ======================================================
WiFiClient espClient;
PubSubClient client(espClient);
TFT_eSPI tft = TFT_eSPI();

// ======================================================
// CONEXIÓN WIFI
// ======================================================
void connectWiFi() {
  Serial.print("Conectando a WiFi");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println("WiFi conectado");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

// ======================================================
// CONEXIÓN MQTT CON UBIDOTS
// ======================================================
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando a Ubidots por MQTT...");

    String clientId = "ESP32Client-";
    clientId += String((uint32_t)ESP.getEfuseMac(), HEX);

    if (client.connect(
          clientId.c_str(),
          UBIDOTS_TOKEN,
          UBIDOTS_TOKEN
        )) {
      Serial.println(" conectado");
    } else {
      Serial.print(" fallo, código=");
      Serial.print(client.state());
      Serial.println(". Reintentando en 5 segundos.");
      delay(5000);
    }
  }
}

// ======================================================
// MOSTRAR DATOS EN LA PANTALLA
// ======================================================
void mostrarEnPantalla(float temperatura, float humedad) {
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  tft.setCursor(10, 25);
  tft.print("Temperatura:");

  tft.setCursor(10, 50);
  tft.print(temperatura, 1);
  tft.print(" C");

  tft.setCursor(10, 85);
  tft.print("Humedad:");

  tft.setCursor(10, 110);
  tft.print(humedad, 1);
  tft.print(" %");
}

// ======================================================
// MOSTRAR ERROR EN LA PANTALLA
// ======================================================
void mostrarErrorDHT() {
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setTextSize(2);

  tft.setCursor(10, 40);
  tft.println("Error al leer");

  tft.setCursor(10, 70);
  tft.println("sensor DHT");
}

// ======================================================
// SETUP
// ======================================================
void setup() {
  Serial.begin(115200);

  // Iniciar sensor DHT
  dht.begin();

  // Iniciar pantalla TTGO T-Display
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  tft.setCursor(10, 40);
  tft.println("Iniciando...");

  // Conectar a WiFi
  connectWiFi();

  // Configurar servidor MQTT
  client.setServer(mqttServer, mqttPort);

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(10, 40);
  tft.println("Sistema listo");

  delay(1500);
}

// ======================================================
// LOOP
// ======================================================
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!client.connected()) {
    reconnectMQTT();
  }

  client.loop();

  // Leer sensor DHT
  float humedad = dht.readHumidity();
  float temperatura = dht.readTemperature();

  // Verificar que la lectura sea válida
  if (isnan(temperatura) || isnan(humedad)) {
    Serial.println("Error al leer el sensor DHT");

    mostrarErrorDHT();

    delay(2000);
    return;
  }

  // Crear topic de Ubidots
  String topic = String("/v1.6/devices/") + DEVICE_LABEL;

  // Crear mensaje JSON
  String json = "{";
  json += "\"";
  json += VARIABLE_TEMP;
  json += "\":";
  json += String(temperatura, 2);
  json += ",";
  json += "\"";
  json += VARIABLE_HUM;
  json += "\":";
  json += String(humedad, 2);
  json += "}";

  // Mostrar datos en monitor serial
  Serial.println("--------------------------------");
  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" °C");

  Serial.print("Humedad: ");
  Serial.print(humedad);
  Serial.println(" %");

  Serial.print("Topic: ");
  Serial.println(topic);

  Serial.print("JSON: ");
  Serial.println(json);

  // Enviar datos a Ubidots
  bool publicado = client.publish(topic.c_str(), json.c_str());

  if (publicado) {
    Serial.println("Datos enviados correctamente");
  } else {
    Serial.println("Error al enviar los datos");
  }

  // Mostrar datos en la pantalla
  mostrarEnPantalla(temperatura, humedad);

  // Esperar 5 segundos
  delay(5000);
}
