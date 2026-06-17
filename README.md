# Skagata Pintar Smart Home

## Struktur file

- `index.html` dan `status.json` adalah file web GitHub Pages.
- `ESP32_SmartHome_4Lamp/` adalah program ESP32 hybrid: web lokal + sinkron GitHub.
- `ESP32_SmartHome_GitHub/` adalah program ESP32 yang hanya membaca status dari GitHub.
- `Roda_Dua_PS4_Bluepad32/` dan `Roda_Empat_PS4_Bluepad32/` adalah program robot PS4.

Folder program ESP32/robot tetap masuk `.gitignore`, jadi GitHub Pages hanya perlu memuat file web.

## Alur integrasi lampu

1. GitHub Pages mengubah `status.json` lewat GitHub API.
2. ESP32 membaca `status.json` dari GitHub secara berkala dan mengubah relay.
3. Jika `github_token` di `ESP32_SmartHome_4Lamp.ino` diisi, kontrol lokal ESP32 juga mengirim status baru ke GitHub.

Token tidak ditulis di `index.html`. Saat membuka web GitHub Pages, isi token di kolom yang tersedia; token disimpan di browser perangkat itu saja.

