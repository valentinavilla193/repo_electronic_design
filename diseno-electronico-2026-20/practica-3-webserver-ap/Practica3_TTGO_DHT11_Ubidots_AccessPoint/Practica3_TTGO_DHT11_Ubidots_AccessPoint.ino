#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <FS.h>
#include <WebServer.h>
#include <TFT_eSPI.h>
#include <Preferences.h>

// ======================================================
// CONFIGURACIÓN DEL SENSOR DHT11
// ======================================================
#define DHT_PIN 13
#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);

// ======================================================
// WIFI PREDETERMINADO
// ======================================================
// IMPORTANTE: reemplaza estos valores por los de tu propia
// red antes de subir el código. NUNCA subas tus credenciales
// reales a un repositorio público de GitHub.
const char* DEFAULT_SSID = "TU_SSID_AQUI";
const char* DEFAULT_PASSWORD = "TU_PASSWORD_AQUI";

// ======================================================
// ACCESS POINT DEL ESP32
// ======================================================
// Esta es la red que el propio ESP32 crea para poder
// configurarlo (no son tus credenciales personales, pero
// se recomienda cambiar la clave por defecto).
const char* AP_SSID = "ESP32-DHT11";
const char* AP_PASSWORD = "12345678";

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
const char* variableHum = "humedad";
const char* variableSw1 = "sw1";
const char* variableSw2 = "sw2";

// ======================================================
// OBJETOS GLOBALES
// ======================================================
WiFiClient espClient;
PubSubClient client(espClient);
TFT_eSPI tft = TFT_eSPI();
WebServer server(80);
Preferences preferences;

// ======================================================
// CREDENCIALES WIFI
// ======================================================
String wifiSSID;
String wifiPassword;

// ======================================================
// VARIABLES
// ======================================================
// Estados recibidos desde Ubidots
int sw1 = 0;
int sw2 = 0;

// Últimos valores medidos
float temperatura = 0.0;
float humedad = 0.0;

// ======================================================
// CONTROL DE TIEMPOS
// ======================================================
unsigned long tiempoAnterior = 0;
const unsigned long intervaloPublicacion = 5000;

// Reintento WiFi
unsigned long ultimoIntentoWiFi = 0;
const unsigned long intervaloWiFi = 15000;

// Reintento MQTT
unsigned long ultimoIntentoMQTT = 0;
const unsigned long intervaloMQTT = 5000;

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
void callback(
  char* topic,
  byte* payload,
  unsigned int length
) {
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

  mostrarDatos(temperatura, humedad);
}

// ======================================================
// CARGAR CONFIGURACIÓN WIFI
// ======================================================
void cargarWiFi() {
  preferences.begin("wifi", true);

  wifiSSID =
    preferences.getString(
      "ssid",
      DEFAULT_SSID
    );

  wifiPassword =
    preferences.getString(
      "password",
      DEFAULT_PASSWORD
    );

  preferences.end();

  Serial.println();
  Serial.println("WiFi cargado de memoria:");

  Serial.print("SSID: ");
  Serial.println(wifiSSID);
}

// ======================================================
// GUARDAR CONFIGURACIÓN WIFI
// ======================================================
void guardarWiFi(
  String nuevoSSID,
  String nuevaPassword
) {
  preferences.begin("wifi", false);

  preferences.putString(
    "ssid",
    nuevoSSID
  );

  preferences.putString(
    "password",
    nuevaPassword
  );

  preferences.end();

  Serial.println("Nueva configuracion WiFi guardada");
}

// ======================================================
// RESTAURAR WIFI PREDETERMINADO
// ======================================================
void restaurarWiFi() {
  preferences.begin("wifi", false);
  preferences.clear();
  preferences.end();

  Serial.println("WiFi restaurado a valores predeterminados");
}

// ======================================================
// INICIAR ACCESS POINT
// ======================================================
void iniciarAccessPoint() {
  // Permite funcionar como:
  // 1. Cliente WiFi
  // 2. Access Point
  WiFi.mode(WIFI_AP_STA);

  bool resultado =
    WiFi.softAP(
      AP_SSID,
      AP_PASSWORD
    );

  if (resultado) {
    Serial.println();
    Serial.println("==============================");
    Serial.println("ACCESS POINT INICIADO");
    Serial.println("==============================");

    Serial.print("Nombre: ");
    Serial.println(AP_SSID);

    Serial.print("Password: ");
    Serial.println(AP_PASSWORD);

    Serial.print("IP: ");
    Serial.println(WiFi.softAPIP());

    Serial.println("==============================");
  } else {
    Serial.println("Error creando Access Point");
  }
}

// ======================================================
// CONECTARSE AL WIFI
// ======================================================
void conectarWiFi() {
  Serial.println();
  Serial.println("==============================");

  Serial.print("Conectando a: ");
  Serial.println(wifiSSID);

  Serial.println("==============================");

  WiFi.begin(
    wifiSSID.c_str(),
    wifiPassword.c_str()
  );

  ultimoIntentoWiFi = millis();
}

// ======================================================
// CREAR PAGINA WEB
// ======================================================
String crearPaginaWeb() {
  String pagina = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport"
content="width=device-width, initial-scale=1">
<title>ESP32 DHT11</title>

<style>
body {
  font-family: Arial;
  background: #eeeeee;
  text-align: center;
  margin: 0;
  padding: 20px;
}

.contenedor {
  background: white;
  max-width: 450px;
  margin: auto;
  padding: 25px;
  border-radius: 15px;
  box-shadow:
    0px 2px 10px
    rgba(0,0,0,0.2);
}

h1 {
  color: #333333;
}

h2 {
  color: #555555;
}

input {
  width: 90%;
  padding: 12px;
  margin: 8px;
  border:
    1px solid #cccccc;
  border-radius: 8px;
}

button {
  width: 95%;
  padding: 12px;
  margin-top: 10px;
  border: none;
  border-radius: 8px;
  background: #2196F3;
  color: white;
  font-size: 16px;
}

button:hover {
  background: #1976D2;
}

.botonRojo {
  background: #f44336;
}

.estado {
  background: #eeeeee;
  padding: 10px;
  border-radius: 10px;
  margin-bottom: 20px;
}
</style>
</head>

<body>

<div class="contenedor">

<h1>ESP32 DHT11</h1>

<div class="estado">

<h2>Estado WiFi</h2>
)rawliteral";

  if (WiFi.status() == WL_CONNECTED) {
    pagina += "<p><b>Conectado a:</b> ";
    pagina += WiFi.SSID();
    pagina += "</p>";

    pagina += "<p><b>IP local:</b> ";
    pagina += WiFi.localIP().toString();
    pagina += "</p>";
  } else {
    pagina +=
      "<p><b>No conectado al router</b></p>";

    pagina += "<p>Intentando conectar a: ";
    pagina += wifiSSID;
    pagina += "</p>";
  }

  pagina += R"rawliteral(
</div>

<h2>Configurar WiFi</h2>

<form action="/guardar" method="POST">

<input
type="text"
name="ssid"
placeholder="Nombre de la red WiFi"
required>

<br>

<input
type="password"
name="password"
placeholder="Contraseña">

<br>

<button type="submit">
Guardar WiFi
</button>

</form>

<br>

<form action="/restaurar" method="POST">

<button
class="botonRojo"
type="submit">

Restaurar WiFi predeterminado

</button>

</form>

</div>

</body>
</html>
)rawliteral";

  return pagina;
}

// ======================================================
// CONFIGURAR SERVIDOR WEB
// ======================================================
void configurarServidorWeb() {
  // Página principal
  server.on(
    "/",
    HTTP_GET,
    []() {
      server.send(
        200,
        "text/html",
        crearPaginaWeb()
      );
    }
  );

  // ----------------------------------------------------
  // GUARDAR NUEVA RED
  // ----------------------------------------------------
  server.on(
    "/guardar",
    HTTP_POST,
    []() {
      String nuevoSSID =
        server.arg("ssid");

      String nuevaPassword =
        server.arg("password");

      if (nuevoSSID.length() == 0) {
        server.send(
          400,
          "text/plain",
          "SSID invalido"
        );

        return;
      }

      guardarWiFi(
        nuevoSSID,
        nuevaPassword
      );

      server.send(
        200,
        "text/html",
        R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport"
content="width=device-width, initial-scale=1">
</head>

<body style="
font-family:Arial;
text-align:center;
padding:30px;
">

<h2>WiFi guardado correctamente</h2>

<p>El ESP32 se reiniciara.</p>

<p>
Vuelva a conectarse al Access Point si es necesario.
</p>

</body>
</html>
)rawliteral"
      );

      delay(1500);

      ESP.restart();
    }
  );

  // ----------------------------------------------------
  // RESTAURAR WIFI
  // ----------------------------------------------------
  server.on(
    "/restaurar",
    HTTP_POST,
    []() {
      restaurarWiFi();

      server.send(
        200,
        "text/html",
        R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport"
content="width=device-width, initial-scale=1">
</head>

<body style="
font-family:Arial;
text-align:center;
padding:30px;
">

<h2>WiFi restaurado</h2>

<p>
Se usara nuevamente la red predeterminada.
</p>

<p>
Consulte DEFAULT_SSID en el codigo.
</p>

<p>
El ESP32 se reiniciara.
</p>

</body>
</html>
)rawliteral"
      );

      delay(1500);

      ESP.restart();
    }
  );

  // Iniciar servidor
  server.begin();

  Serial.println();

  Serial.println("Servidor web iniciado");

  Serial.println(
    "Configuracion disponible en:"
  );

  Serial.println(
    "http://192.168.4.1"
  );
}

// ======================================================
// CONEXIÓN MQTT
// ======================================================
void reconnectMQTT() {
  if (client.connected()) {
    return;
  }

  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  Serial.print(
    "Conectando a Ubidots MQTT..."
  );

  // ID único
  String clientId = "ESP32-";

  clientId += String(
    static_cast<uint32_t>(
      ESP.getEfuseMac()
    ),
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

    client.subscribe(
      topicSw1.c_str()
    );

    client.subscribe(
      topicSw2.c_str()
    );

    Serial.print("Suscrito a: ");
    Serial.println(topicSw1);

    Serial.print("Suscrito a: ");
    Serial.println(topicSw2);
  } else {
    Serial.print(
      "Error MQTT, codigo: "
    );

    Serial.println(
      client.state()
    );
  }
}

// ======================================================
// PUBLICAR DATOS EN UBIDOTS
// ======================================================
void publicarDatos(
  float temp,
  float hum
) {
  if (!client.connected()) {
    return;
  }

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

  bool enviado =
    client.publish(
      topic.c_str(),
      payload.c_str()
    );

  Serial.println();

  Serial.println("Publicacion MQTT");

  Serial.print("Topic: ");
  Serial.println(topic);

  Serial.print("Payload: ");
  Serial.println(payload);

  if (enviado) {
    Serial.println(
      "Datos enviados correctamente"
    );
  } else {
    Serial.println(
      "No se pudieron enviar los datos"
    );
  }
}

// ======================================================
// SETUP
// ======================================================
void setup() {
  Serial.begin(115200);

  delay(500);

  // ----------------------------------------------------
  // SENSOR
  // ----------------------------------------------------
  dht.begin();

  // ----------------------------------------------------
  // PANTALLA
  // ----------------------------------------------------
  tft.init();

  tft.setRotation(1);

  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(
    TFT_WHITE,
    TFT_BLACK
  );

  tft.setTextSize(2);

  tft.setCursor(10, 10);

  tft.println(
    "Iniciando..."
  );

  // ----------------------------------------------------
  // CARGAR WIFI GUARDADO
  // ----------------------------------------------------
  cargarWiFi();

  // ----------------------------------------------------
  // CREAR ACCESS POINT
  // ----------------------------------------------------
  iniciarAccessPoint();

  // ----------------------------------------------------
  // INICIAR CONEXIÓN AL ROUTER
  // ----------------------------------------------------
  conectarWiFi();

  // ----------------------------------------------------
  // SERVIDOR WEB
  // ----------------------------------------------------
  configurarServidorWeb();

  // ----------------------------------------------------
  // MQTT
  // ----------------------------------------------------
  client.setServer(
    mqttServer,
    mqttPort
  );

  client.setCallback(
    callback
  );

  client.setBufferSize(
    512
  );
}

// ======================================================
// LOOP
// ======================================================
void loop() {
  // ====================================================
  // SERVIDOR WEB DEL ACCESS POINT
  // ====================================================
  server.handleClient();

  // ====================================================
  // CONTROL DEL WIFI
  // ====================================================
  if (
    WiFi.status() != WL_CONNECTED
  ) {
    if (
      millis() - ultimoIntentoWiFi >=
      intervaloWiFi
    ) {
      Serial.println();

      Serial.println(
        "WiFi desconectado"
      );

      Serial.println(
        "Intentando nuevamente..."
      );

      WiFi.disconnect();

      delay(100);

      conectarWiFi();
    }
  } else {
    // ==================================================
    // MQTT
    // ==================================================
    if (!client.connected()) {
      if (
        millis() - ultimoIntentoMQTT >=
        intervaloMQTT
      ) {
        ultimoIntentoMQTT = millis();

        reconnectMQTT();
      }
    } else {
      client.loop();
    }
  }

  // ====================================================
  // LECTURA DEL SENSOR
  // ====================================================
  unsigned long tiempoActual =
    millis();

  if (
    tiempoActual - tiempoAnterior >=
    intervaloPublicacion
  ) {
    tiempoAnterior =
      tiempoActual;

    // Leer DHT11
    humedad =
      dht.readHumidity();

    temperatura =
      dht.readTemperature();

    // --------------------------------------------------
    // COMPROBAR SENSOR
    // --------------------------------------------------
    if (
      isnan(temperatura) ||
      isnan(humedad)
    ) {
      Serial.println();

      Serial.println(
        "Error al leer el sensor DHT11"
      );

      mostrarErrorDHT11();

      return;
    }

    // --------------------------------------------------
    // MOSTRAR EN SERIAL
    // --------------------------------------------------
    Serial.println();

    Serial.println(
      "Lectura del DHT11"
    );

    Serial.print(
      "Temperatura: "
    );

    Serial.print(
      temperatura,
      1
    );

    Serial.println(" C");

    Serial.print(
      "Humedad: "
    );

    Serial.print(
      humedad,
      1
    );

    Serial.println(" %");

    // --------------------------------------------------
    // ENVIAR A UBIDOTS
    // --------------------------------------------------
    publicarDatos(
      temperatura,
      humedad
    );

    // --------------------------------------------------
    // MOSTRAR EN PANTALLA
    // --------------------------------------------------
    mostrarDatos(
      temperatura,
      humedad
    );
  }
}
