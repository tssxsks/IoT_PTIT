/*
  ESP-NOW Two-way Communication with DHT11 Sensor
  Adapted from Rui Santos - Random Nerd Tutorials
*/

#include <esp_now.h>
#include <WiFi.h>
#include "DHT.h"

#define DHTPIN 4       
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Địa chỉ MAC của thiết bị nhận (thay bằng MAC của ESP kia)
uint8_t broadcastAddress[] = {0x8C, 0x4B, 0x14, 0xC6, 0x14, 0x94};

// Biến dữ liệu
float temperature;
float humidity;

float incomingTemp;
float incomingHum;

String success;

// Struct gửi/nhận
typedef struct struct_message {
  float temp;
  float hum;
} struct_message;

struct_message DHTReadings;
struct_message incomingReadings;

esp_now_peer_info_t peerInfo;

// Callback khi dữ liệu được gửi
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  success = (status == ESP_NOW_SEND_SUCCESS) ? "Delivery Success" : "Delivery Fail";
}

// Callback khi dữ liệu được nhận
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  if (len == sizeof(incomingReadings)) {
    memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
    Serial.print("Bytes received: ");
    Serial.println(len);
    
    incomingTemp = incomingReadings.temp;
    incomingHum  = incomingReadings.hum;

    // Hiển thị dữ liệu ngay khi nhận được
    updateSerial();
  } else {
    Serial.println("Received data size mismatch");
  }
}

// Setup
void setup() {
  Serial.begin(9600);
  dht.begin();

  WiFi.mode(WIFI_STA);
  Serial.print("ESP32 MAC Address: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    delay(1000);
    ESP.restart();
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  // Cấu hình peer info
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    delay(1000);
    return;
  }

  Serial.println("ESP-NOW Two-way Communication Started");
}

// Loop
void loop() {
  getReadings();

  // Gán dữ liệu vào struct
  DHTReadings.temp = temperature;
  DHTReadings.hum  = humidity;

  // Gửi dữ liệu
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &DHTReadings, sizeof(DHTReadings));

  if (result == ESP_OK) {
    Serial.println("Data sent successfully");
  } else {
    Serial.println("Error sending the data");
    Serial.print("Error code: ");
    Serial.println(result);
  }

  delay(10000); // Gửi mỗi 10 giây
}

// Hàm đọc sensor
void getReadings() {
  temperature = dht.readTemperature();
  humidity    = dht.readHumidity();
  
  delay(2000);

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    temperature = 0.0;
    humidity = 0.0;
  } else {
    Serial.println("\nDATA SEND: ");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
  }
}

// In dữ liệu qua Serial
void updateSerial() {
  Serial.println("\nDATA RECEIVE: ");
  Serial.print("Temperature: ");
  Serial.print(incomingTemp);
  Serial.println(" °C");
  Serial.print("Humidity: ");
  Serial.print(incomingHum);
  Serial.println(" %");
  Serial.print("Last Send Status: ");
  Serial.println(success);
  Serial.println("==============");
}
