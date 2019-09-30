/*
 * ESP8266 - Webserver Configuration
 */
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#ifndef STASSID
#define STASSID "SSID"
#define STAPSK  "PASSWORD"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

void handleRoot() {
  server.send(200, "text/plain", "hello from esp8266!");
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

/*
 * BME280 Configuration
 */
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;
float temperature;
float pressure;
float altitude;
float humidity;
String comodo = "meuQuarto";

void readBME() {
  temperature = bme.readTemperature();
  pressure = bme.readPressure() / 100.0F;
  altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  humidity = bme.readHumidity();
}

String metrics(){
  readBME();
  String reply = "";
  reply += "# HELP bm280_temperature Reads Celsius temperature from BME280 sensor\n";
  reply += "# TYPE bm280_temperature gauge\n";
  reply += "bm280_temperature{comodo=\"" + comodo + "\"} "+ String(temperature)+ "\n";
  reply += "# HELP bm280_humidity Reads humidity percentile from BME280 sensor\n";
  reply += "# TYPE bm280_humidity gauge\n";
  reply += "bm280_humidity{comodo=\"" + comodo + "\"} "+ String(humidity)+ "\n";
  reply += "# HELP bm280_pressure Reads atmospheric pressure from BME280 sensor\n";
  reply += "# TYPE bm280_pressure gauge\n";
  reply += "bm280_pressure{comodo=\"" + comodo + "\"} "+ String(pressure)+ "\n";

  return reply;
}



void setup(void) {
  Serial.begin(115200);
  pinMode(3, OUTPUT);
  digitalWrite(3, HIGH);
  Wire.begin(0, 2);
  Wire.setClock(100000);
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin(comodo)) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/metrics", []() {
    server.send(200, "text/plain", metrics());
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}
