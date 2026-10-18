<div align="center">
  <h1>ESP32 Motor Controller</h1>
  <p><i>IoT Wireless Control (Blynk + OTA) & Local Serial GUI (Python)</i></p>
  
  <img src="https://img.shields.io/badge/Hardware-ESP32-blue?style=flat-square&logo=espressif" alt="ESP32" />
  <img src="https://img.shields.io/badge/Language-C++-00599C?style=flat-square&logo=c%2B%2B" alt="C++" />
  <img src="https://img.shields.io/badge/GUI-Python_Tkinter-3776AB?style=flat-square&logo=python" alt="Python" />
  <img src="https://img.shields.io/badge/IoT_Platform-Blynk-17B777?style=flat-square" alt="Blynk" />
</div>

<br />

## Deskripsi Proyek

Repositori ini mendokumentasikan pengembangan sistem kendali aktuator (Motor DC dan L298N Motor Driver) menggunakan mikrokontroler ESP32. Proyek ini disusun secara progresif
mencakup transisi dari kendali perangkat keras dasar hingga sistem otomasi nirkabel berbasis Internet of Things (IoT).

Sistem ini mendukung dua mode operasi utama:
1. **Local Control:** Pengendalian via antarmuka grafis (GUI) Python menggunakan protokol Serial USB.
2. **Wireless Control:** Pengendalian jarak jauh via aplikasi Blynk dengan algoritma hitung mundur asinkron dan kapabilitas Over-The-Air (OTA) firmware update.

---

## Struktur Repositori

Pembelajaran dan pengujian disusun secara hierarkis ke dalam direktori berikut:

* **`/Setup`** — Konfigurasi awal mikrokontroler, uji konektivitas Serial, dan kalibrasi pin I/O.
* **`/Basic`** — Kendali putaran motor tingkat rendah (ON/OFF) dan Modulasi Lebar Pulsa (PWM) statis berbasis C++.
* **`/Basic + Blynk`** — Migrasi kontrol ke platform IoT nirkabel menggunakan pustaka `BlynkSimpleEsp32`.
* **`/Timer + Blynk`** — Implementasi otomasi aktuator menggunakan algoritma dynamic countdown timer asinkron (berbasis fungsi `millis()`) tanpa memblokir konektivitas jaringan.
* **`/Timer + Blynk + OTA`** — Integrasi sistem Over-The-Air (OTA). Memungkinkan flashing firmware secara nirkabel via Local Area Network (Wi-Fi) dengan sistem keamanan otentikasi.
* **`/Python`** — Pembangunan antarmuka GUI Desktop menggunakan `tkinter` dan `pyserial` untuk kendali aktuator secara real-time via komunikasi kabel.

---

## Spesifikasi Perangkat

### Perangkat Keras
* DOIT ESP32 DEVKIT V1
* Modul Driver Motor L298N
* Aktuator Motor DC 5V
* Kabel Jumper & Breadboard

### Perangkat Lunak
* **IDE:** Arduino IDE 2.x, Visual Studio Code
* **Library C++:** `<WiFi.h>`, `<BlynkSimpleEsp32.h>`, `<ArduinoOTA.h>`
* **Library Python:** `tkinter`, `pyserial`

---

<div align="center">
  <p><b>Dikembangkan oleh:</b></p>
  <p>Jonathan Teguh<br>
  <i>Program Studi Informatika — UPN "Veteran" Jawa Timur</i></p>
</div>
