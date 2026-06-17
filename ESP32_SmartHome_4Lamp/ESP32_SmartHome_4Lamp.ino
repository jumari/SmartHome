#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

// ================= KONFIGURASI WIFI =================
// Mode Station: Masukkan SSID & Password Wifi Rumah Anda
const char* ssid_STA = "R.53_LAB LG";
const char* password_STA = "ayosinau";

// Mode Access Point: ESP32 akan memancarkan Wifi sendiri jika Wifi Rumah tidak terhubung
const char* ssid_AP = "ESP32-SmartHome";
const char* password_AP = "12345678"; // minimal 8 karakter

WebServer server(80);

// ================= PIN MAPPING RELAY / LAMPU =================
const int RELAY_1 = 2;  // Lampu Teras (MENGGUNAKAN LED INTERNAL PIN 2)
const int RELAY_2 = 19; // Lampu Ruang Tamu
const int RELAY_3 = 21; // Lampu Kamar
const int RELAY_4 = 22; // Lampu Dapur

// Menyimpan status lampu (false = mati, true = menyala)
bool state_L1 = false;
bool state_L2 = false;
bool state_L3 = false;
bool state_L4 = false;

// ================= SOURCE CODE HTML DASHBOARD =================
// Dashboard modern dengan desain glassmorphism & responsive
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Smart Home Dashboard</title>
<link href="https://fonts.googleapis.com/css2?family=Outfit:wght@300;400;600;700&display=swap" rel="stylesheet">
<style>
  :root {
    --bg-gradient: linear-gradient(135deg, #0f172a 0%, #1e1b4b 100%);
    --card-bg: rgba(30, 41, 59, 0.7);
    --card-border: rgba(255, 255, 255, 0.08);
    --text-primary: #f8fafc;
    --text-secondary: #94a3b8;
    --color-on: #38bdf8;
    --color-on-glow: rgba(56, 189, 248, 0.4);
    --color-off: #475569;
  }
  * {
    box-sizing: border-box;
    margin: 0;
    padding: 0;
    font-family: 'Outfit', sans-serif;
  }
  body {
    min-height: 100vh;
    background: var(--bg-gradient);
    color: var(--text-primary);
    display: flex;
    justify-content: center;
    align-items: center;
    padding: 20px;
  }
  .container {
    width: 100%;
    max-width: 600px;
    background: var(--card-bg);
    backdrop-filter: blur(16px);
    -webkit-backdrop-filter: blur(16px);
    border: 1px solid var(--card-border);
    border-radius: 24px;
    padding: 30px;
    box-shadow: 0 20px 40px rgba(0,0,0,0.4);
  }
  header {
    text-align: center;
    margin-bottom: 30px;
  }
  header h1 {
    font-size: 28px;
    font-weight: 700;
    background: linear-gradient(to right, #38bdf8, #818cf8);
    -webkit-background-clip: text;
    -webkit-text-fill-color: transparent;
    margin-bottom: 5px;
  }
  header p {
    color: var(--text-secondary);
    font-size: 14px;
  }
  .grid {
    display: grid;
    grid-template-columns: repeat(2, 1fr);
    gap: 20px;
  }
  @media(max-width: 480px) {
    .grid {
      grid-template-columns: 1fr;
    }
  }
  .card {
    background: rgba(15, 23, 42, 0.4);
    border: 1px solid var(--card-border);
    border-radius: 20px;
    padding: 20px;
    display: flex;
    flex-direction: column;
    align-items: center;
    transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
    position: relative;
    overflow: hidden;
  }
  .card:hover {
    transform: translateY(-5px);
    border-color: rgba(255, 255, 255, 0.15);
  }
  .card.active {
    box-shadow: 0 10px 25px var(--color-on-glow);
    border-color: var(--color-on);
  }
  .icon-wrapper {
    width: 60px;
    height: 60px;
    border-radius: 50%;
    background: rgba(255, 255, 255, 0.05);
    display: flex;
    justify-content: center;
    align-items: center;
    margin-bottom: 15px;
    transition: all 0.3s ease;
    color: var(--text-secondary);
  }
  .card.active .icon-wrapper {
    background: rgba(56, 189, 248, 0.15);
    color: var(--color-on);
  }
  .card-title {
    font-size: 16px;
    font-weight: 600;
    margin-bottom: 5px;
  }
  .card-status {
    font-size: 12px;
    color: var(--text-secondary);
    margin-bottom: 15px;
    text-transform: uppercase;
    letter-spacing: 1px;
  }
  .card.active .card-status {
    color: var(--color-on);
    font-weight: 600;
  }
  .switch {
    position: relative;
    display: inline-block;
    width: 60px;
    height: 30px;
  }
  .switch input {
    opacity: 0;
    width: 0;
    height: 0;
  }
  .slider {
    position: absolute;
    cursor: pointer;
    top: 0;
    left: 0;
    right: 0;
    bottom: 0;
    background-color: var(--color-off);
    transition: .4s;
    border-radius: 34px;
  }
  .slider:before {
    position: absolute;
    content: "";
    height: 22px;
    width: 22px;
    left: 4px;
    bottom: 4px;
    background-color: white;
    transition: .4s;
    border-radius: 50%;
  }
  input:checked + .slider {
    background-color: var(--color-on);
  }
  input:checked + .slider:before {
    transform: translateX(30px);
  }
</style>
</head>
<body>
<div class="container">
  <header>
    <h1>Smart Home IoT</h1>
    <p>ESP32 Dashboard Kendali 4 Lampu</p>
  </header>
  <div class="grid">
    <!-- Lampu 1 -->
    <div class="card" id="card-1">
      <div class="icon-wrapper">
        <svg xmlns="http://www.w3.org/2000/svg" width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M15 14c.2-1 .7-1.7 1.5-2.5 1-.9 1.5-2.2 1.5-3.5A5 5 0 0 0 8 8c0 1 .3 2.2 1.5 3.5.7.7 1.3 1.5 1.5 2.5"/><path d="M9 18h6"/><path d="M10 22h4"/></svg>
      </div>
      <div class="card-title">Lampu Teras</div>
      <div class="card-status" id="status-1">Mati</div>
      <label class="switch">
        <input type="checkbox" id="switch-1" onchange="toggleLight(1)">
        <span class="slider"></span>
      </label>
    </div>
    <!-- Lampu 2 -->
    <div class="card" id="card-2">
      <div class="icon-wrapper">
        <svg xmlns="http://www.w3.org/2000/svg" width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M15 14c.2-1 .7-1.7 1.5-2.5 1-.9 1.5-2.2 1.5-3.5A5 5 0 0 0 8 8c0 1 .3 2.2 1.5 3.5.7.7 1.3 1.5 1.5 2.5"/><path d="M9 18h6"/><path d="M10 22h4"/></svg>
      </div>
      <div class="card-title">Lampu R. Tamu</div>
      <div class="card-status" id="status-2">Mati</div>
      <label class="switch">
        <input type="checkbox" id="switch-2" onchange="toggleLight(2)">
        <span class="slider"></span>
      </label>
    </div>
    <!-- Lampu 3 -->
    <div class="card" id="card-3">
      <div class="icon-wrapper">
        <svg xmlns="http://www.w3.org/2000/svg" width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M15 14c.2-1 .7-1.7 1.5-2.5 1-.9 1.5-2.2 1.5-3.5A5 5 0 0 0 8 8c0 1 .3 2.2 1.5 3.5.7.7 1.3 1.5 1.5 2.5"/><path d="M9 18h6"/><path d="M10 22h4"/></svg>
      </div>
      <div class="card-title">Lampu Kamar</div>
      <div class="card-status" id="status-3">Mati</div>
      <label class="switch">
        <input type="checkbox" id="switch-3" onchange="toggleLight(3)">
        <span class="slider"></span>
      </label>
    </div>
    <!-- Lampu 4 -->
    <div class="card" id="card-4">
      <div class="icon-wrapper">
        <svg xmlns="http://www.w3.org/2000/svg" width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M15 14c.2-1 .7-1.7 1.5-2.5 1-.9 1.5-2.2 1.5-3.5A5 5 0 0 0 8 8c0 1 .3 2.2 1.5 3.5.7.7 1.3 1.5 1.5 2.5"/><path d="M9 18h6"/><path d="M10 22h4"/></svg>
      </div>
      <div class="card-title">Lampu Dapur</div>
      <div class="card-status" id="status-4">Mati</div>
      <label class="switch">
        <input type="checkbox" id="switch-4" onchange="toggleLight(4)">
        <span class="slider"></span>
      </label>
    </div>
  </div>
</div>
<script>
  function updateUI(id, state) {
    const card = document.getElementById('card-' + id);
    const status = document.getElementById('status-' + id);
    const sw = document.getElementById('switch-' + id);
    sw.checked = state;
    if (state) {
      card.classList.add('active');
      status.innerText = 'Menyala';
    } else {
      card.classList.remove('active');
      status.innerText = 'Mati';
    }
  }

  function getStates() {
    fetch('/api/states')
      .then(res => res.json())
      .then(data => {
        updateUI(1, data.l1);
        updateUI(2, data.l2);
        updateUI(3, data.l3);
        updateUI(4, data.l4);
      })
      .catch(err => console.error(err));
  }

  function toggleLight(id) {
    const sw = document.getElementById('switch-' + id);
    const state = sw.checked ? 1 : 0;
    fetch(`/api/toggle?id=${id}&state=${state}`)
      .then(res => res.json())
      .then(data => {
        updateUI(id, data.state);
      })
      .catch(err => {
        console.error(err);
        // kembalikan state switch jika gagal
        sw.checked = !sw.checked;
      });
  }

  // Load status pertama kali
  getStates();
  // Sinkronisasi status berkala setiap 5 detik
  setInterval(getStates, 5000);
</script>
</body>
</html>
)rawliteral";

// ================= HANDLERS WEB SERVER =================
void handleRoot() {
  server.send(200, "text/html", index_html);
}

void handleGetStates() {
  String json = "{";
  json += "\"l1\":" + String(state_L1 ? "true" : "false") + ",";
  json += "\"l2\":" + String(state_L2 ? "true" : "false") + ",";
  json += "\"l3\":" + String(state_L3 ? "true" : "false") + ",";
  json += "\"l4\":" + String(state_L4 ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

void handleToggle() {
  if (server.hasArg("id") && server.hasArg("state")) {
    int id = server.arg("id").toInt();
    bool state = server.arg("state").toInt() == 1;

    switch (id) {
      case 1:
        state_L1 = state;
        digitalWrite(RELAY_1, state_L1 ? HIGH : LOW);
        break;
      case 2:
        state_L2 = state;
        digitalWrite(RELAY_2, state_L2 ? HIGH : LOW);
        break;
      case 3:
        state_L3 = state;
        digitalWrite(RELAY_3, state_L3 ? HIGH : LOW);
        break;
      case 4:
        state_L4 = state;
        digitalWrite(RELAY_4, state_L4 ? HIGH : LOW);
        break;
    }
    
    Serial.printf("Lampu %d diubah menjadi: %s\n", id, state ? "MENYALA" : "MATI");
    
    String json = "{\"success\":true,\"state\":" + String(state ? "true" : "false") + "}";
    server.send(200, "application/json", json);
  } else {
    server.send(400, "application/json", "{\"error\":\"Missing arguments\"}");
  }
}

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

  Serial.println("\n--- Memulai Smart Home ESP32 ---");

  // Mencoba menyambung ke Wifi rumah (Mode Station)
  WiFi.begin(ssid_STA, password_STA);
  Serial.print("Menghubungkan ke Wifi Rumah");
  
  int attempts = 0;
  // Menunggu koneksi maksimal 10 detik (20 * 500ms)
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    // Berhasil terhubung ke Wifi Rumah
    Serial.println("\nBerhasil terhubung!");
    Serial.print("IP Address ESP32 Anda: ");
    Serial.println(WiFi.localIP());
  } else {
    // Gagal terhubung, jalankan Mode Access Point (AP)
    Serial.println("\nGagal terhubung ke Wifi rumah.");
    Serial.println("Memulai Mode Access Point (Pancarkan Wifi Sendiri)...");
    
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid_AP, password_AP);
    
    Serial.print("Nama Wifi (SSID): ");
    Serial.println(ssid_AP);
    Serial.print("IP Address Web Server: ");
    Serial.println(WiFi.softAPIP());
  }

  // Inisialisasi mDNS responder untuk http://skagataPintar.local
  if (MDNS.begin("skagataPintar")) {
    Serial.println("mDNS responder started: http://skagataPintar.local");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }

  // Routing Web Server
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/states", HTTP_GET, handleGetStates);
  server.on("/api/toggle", HTTP_GET, handleToggle);

  server.begin();
  Serial.println("Web Server siap dijalankan!");
}

void loop() {
  server.handleClient();
  delay(2); // Menjaga stabilitas ESP32
}
