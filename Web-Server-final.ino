#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

// === Informations Wi-Fi ===
const char* ssid = "--------";
const char* password = "************";

// === Configuration du capteur DHT ===
#define DHTPIN 5           // Broche D1 = GPIO5
#define DHTTYPE DHT11      // Type de capteur
DHT dht(DHTPIN, DHTTYPE);

// === Variables pour stocker les mesures ===
float temperature = 0.0;
float humidity = 0.0;

// === Serveur web asynchrone sur le port 80 ===
AsyncWebServer server(80);

// === Mise à jour des valeurs chaque 10 secondes ===
unsigned long previousMillis = 0;
const long interval = 10000;

// === Page HTML à envoyer au client ===
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta charset="UTF-8"> 
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Température & Humidité</title>
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css">
  <style>
    html {
      font-family: Arial;
      display: inline-block;
      margin: 0px auto;
      text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels {
      font-size: 1.5rem;
      vertical-align: middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>🌡️ Station DHT11 - ESP8266</h2>
  <p>
  <i class="fas fa-thermometer-half fa-beat" style="color: #c70f0f;"></i>     <span class="dht-labels">Température  :</span>
     <span id="temperature">%TEMPERATURE%</span>
     <sup class="dht-labels">°C</sup>
  </p>
  <p><i class="fas fa-tint" style="color:#00add6;"></i>
     <span class="dht-labels">Humidité  :</span>
     <span id="humidity">%HUMIDITY%</span>
     <sup class="dht-labels">%</sup>
  </p>
</body>
<script>
setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000);

setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000);
</script>
</html>
)rawliteral";

// === Fonction de remplacement des balises dans le HTML ===
String processor(const String& var) {
  if (var == "TEMPERATURE") return String(temperature);
  else if (var == "HUMIDITY") return String(humidity);
  return String();
}

// === Setup ===
void setup() {
  Serial.begin(115200);
  dht.begin();

  // Connexion Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connexion au WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connecté ! IP locale : ");
  Serial.println(WiFi.localIP());

  // Routes HTTP
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor); // tuto fih send(200...) bla p kay3ti error
  });

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(temperature));
  });

  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(humidity));
  });

  // Démarrage du serveur
  server.begin();
}

// === Boucle principale ===
void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    float newTemp = dht.readTemperature();
    float newHum = dht.readHumidity();

    if (!isnan(newTemp)) {
      temperature = newTemp;
      Serial.println(temperature);
    } else {
      Serial.println("Erreur de lecture du capteur !");
    }

    if (!isnan(newHum)) {
      humidity = newHum;
      Serial.println(humidity);
    } else {
      Serial.println("Erreur de lecture du capteur !");
    }
  }
}
