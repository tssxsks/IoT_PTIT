#include "DHT.h"
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid     = "IQ9";
const char* password = "Adu1122bb";

String apiKey = "MZF2PJT8URH4F1WL";
String readAPIKey = "XE213B6P3KLG954S";
const char* server = "http://api.thingspeak.com";
String channelID = "3071869";

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
}

void loop() {
  delay(2000);
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);

  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print("%  Temperature: ");
  Serial.print(t);
  Serial.print("°C ");
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String writeUrl = String(server) + "/update" + "?api_key=" + apiKey +
                 "&field1=" + String(t) +
                 "&field2=" + String(h);

    http.begin(writeUrl);
    int httpCode = http.GET();
    if (httpCode == 200) {
      Serial.println("ThingSpeak Response: " + http.getString());
    } else {
      Serial.println(String("Error code: ") + httpCode);
    }
    http.end();

    delay(20000);

    String readURL =  String(server) + "/channels/" + channelID +
                     "/fields/1/last.txt?api_key=" + readAPIKey;

    http.begin(readURL);
    httpCode = http.GET();
    if (httpCode == 200) {
      String lastTemp = http.getString();
      Serial.println("Last Temperature from ThingSpeak: " + lastTemp);
    } else {
      Serial.println(String("Error code: ") + httpCode);
    }
    http.end();

    readURL = String(server) + "/channels/" + channelID +
              "/fields/2/last.txt?api_key=" + readAPIKey;

    http.begin(readURL);
    httpCode = http.GET();
    if (httpCode == 200) {
      String lastHum = http.getString();
      Serial.println("Last Humidity from ThingSpeak: " + lastHum);
    } else {
      Serial.println(String("Error code: ") + httpCode);
    }
    http.end();
  }

  delay(20000);
}
