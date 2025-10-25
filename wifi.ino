#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

#define DHTPIN D4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "LR Wifi";
const char* password = "Lumbanraja";

IPAddress local_IP(192, 168, 100, 122);
IPAddress gateway(192, 168, 100, 1);
IPAddress subnet(255, 255, 255, 0);

ESP8266WebServer server(80);

void handleData() {
  float suhu = dht.readTemperature();
  float kelembapan = dht.readHumidity();

  if (isnan(suhu) || isnan(kelembapan)) {
    // kirim error JSON agar client tahu
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(500, "application/json", "{\"error\":\"Gagal membaca sensor\"}");
    Serial.println("Error: gagal membaca DHT");
    return;
  }

  // tambahkan header CORS supaya bisa diakses dari html di host lain
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

  String json = "{";
  json += "\"suhu\":" + String(suhu, 1) + ",";
  json += "\"kelembapan\":" + String(kelembapan, 1);
  json += "}";

  server.send(200, "application/json", json);

}

void setup() {
  Serial.begin(115200);
  dht.begin();

  // set static IP (opsional) — kalau network-mu beda, ubah sesuai subnet router
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);

  Serial.print("Menghubungkan ke WiFi");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
    if (millis() - start > 20000) { // timeout 20s
      Serial.println("\nGagal konek WiFi (timeout). Periksa SSID/password.");
      break;
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ Terhubung ke WiFi!");
    Serial.print("IP NodeMCU: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n⚠️ Tidak terhubung ke WiFi — perbaiki dulu.");
  }

  server.on("/data", handleData);

  server.begin();
  Serial.println("🌐 Server berjalan (port 80)");
}

void loop() {
  server.handleClient();
}
