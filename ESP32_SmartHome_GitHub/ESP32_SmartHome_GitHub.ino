#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>

// ================= KONFIGURASI WIFI =================
const char* ssid = "R.53_LAB LG";
const char* password = "ayosinau";

// ================= KONFIGURASI GITHUB =================
// URL raw dari file status.json di repository GitHub Anda
// Pastikan username, nama repository, dan nama branch (main/master) sudah benar
const char* github_url = "https://raw.githubusercontent.com/jumari/SmartHome/main/status.json";

// ================= PIN MAPPING RELAY / LAMPU =================
const int RELAY_1 = 2;   // Lampu Teras (LED Internal)
const int RELAY_2 = 19;  // Lampu Ruang Tamu
const int RELAY_3 = 21;  // Lampu Kamar
const int RELAY_4 = 22;  // Lampu Dapur

unsigned long lastTime = 0;
const unsigned long timerDelay = 5000; // Cek perubahan di GitHub setiap 5 detik (5000 ms)

void setup() {
  Serial.begin(115200);

  // Set pin relay sebagai OUTPUT
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(RELAY_3, OUTPUT);
  pinMode(RELAY_4, OUTPUT);

  // Default: Matikan semua relay di awal
  digitalWrite(RELAY_1, LOW);
  digitalWrite(RELAY_2, LOW);
  digitalWrite(RELAY_3, LOW);
  digitalWrite(RELAY_4, LOW);

  Serial.println("\n--- Memulai Smart Home ESP32 via GitHub Control ---");

  // Menghubungkan ke WiFi
  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan ke WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Terhubung!");
  Serial.print("IP Address ESP32: ");
  Serial.println(WiFi.localIP());

  // Inisialisasi mDNS responder untuk http://skagataPintar.local
  if (MDNS.begin("skagataPintar")) {
    Serial.println("mDNS responder started: http://skagataPintar.local");
  }
}

void loop() {
  // Melakukan request secara berkala tanpa memblokir program (non-blocking)
  if ((millis() - lastTime) > timerDelay) {
    if (WiFi.status() == WL_CONNECTED) {
      WiFiClientSecure client;
      client.setInsecure(); // Mengabaikan verifikasi SSL certificate agar request HTTPS lebih stabil

      HTTPClient http;
      http.begin(client, github_url);
      
      int httpResponseCode = http.GET();
      
      if (httpResponseCode > 0) {
        if (httpResponseCode == HTTP_CODE_OK) {
          String payload = http.getString();
          Serial.println("\n--- Data Baru Diterima dari GitHub ---");
          Serial.println(payload);
          
          // Parsing data JSON
          JsonDocument doc;
          DeserializationError error = deserializeJson(doc, payload);
          
          if (!error) {
            // Membaca status masing-masing lampu (default: false jika data tidak ditemukan)
            bool l1 = doc["lampu1"] | false;
            bool l2 = doc["lampu2"] | false;
            bool l3 = doc["lampu3"] | false;
            bool l4 = doc["lampu4"] | false;
            
            // Mengubah kondisi pin fisik relay sesuai status dari GitHub
            digitalWrite(RELAY_1, l1 ? HIGH : LOW);
            digitalWrite(RELAY_2, l2 ? HIGH : LOW);
            digitalWrite(RELAY_3, l3 ? HIGH : LOW);
            digitalWrite(RELAY_4, l4 ? HIGH : LOW);
            
            Serial.printf("Status Lampu saat ini -> Teras (L1): %s | R. Tamu (L2): %s | Kamar (L3): %s | Dapur (L4): %s\n",
                          l1 ? "NYALA" : "MATI", 
                          l2 ? "NYALA" : "MATI", 
                          l3 ? "NYALA" : "MATI", 
                          l4 ? "NYALA" : "MATI");
          } else {
            Serial.print("Gagal parsing JSON: ");
            Serial.println(error.f_str());
          }
        } else {
          Serial.printf("Gagal mengambil data. HTTP Code: %d\n", httpResponseCode);
        }
      } else {
        Serial.printf("Error pada koneksi HTTP: %s\n", http.errorToString(httpResponseCode).c_str());
      }
      http.end();
    } else {
      Serial.println("Koneksi WiFi terputus, mencoba menyambung kembali...");
    }
    lastTime = millis();
  }
}
