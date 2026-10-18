
// PROGRESS 2 : KONTROL MOTOR MURNI BLYNK
// Fitur: ON/OFF (V3) dan PWM Slider (V2), Display PWM (V1) & RPM (V0)

// --- KREDENSIAL BLYNK & WIFI ---
#define BLYNK_TEMPLATE_ID "ISI_DENGAN_TEMPLATE_ID_ANDA"
#define BLYNK_TEMPLATE_NAME "ISI_DENGAN_NAMA_TEMPLATE_ANDA"
#define BLYNK_AUTH_TOKEN "ISI_DENGAN_AUTH_TOKEN_DEVICE_BARU"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Masukkan SSID dan Password Wi-Fi/Tethering Anda
char ssid[] = "NAMA_WIFI_ANDA";
char pass[] = "PASSWORD_WIFI_ANDA";

// --- KONFIGURASI PIN ---
int ledPin = 2;          // Indikator LED internal ESP32
int motor1Pin1 = 27;     // Pin IN1 Driver Motor
int motor1Pin2 = 26;     // Pin IN2 Driver Motor
int enable1Pin = 12;     // Pin ENA (PWM) Driver Motor

// --- PENGATURAN PWM ---
const int freq = 30000;      // Frekuensi 30 kHz
const int resolution = 8;    // Resolusi 8-bit (0-255)
int dutyCycle = 0;           // Kecepatan awal (mati)
bool isMotorRunning = false; // Status motor

// --- FUNGSI UPDATE TAMPILAN BLYNK ---
// Memperbarui nilai Gauge dan Display di HP
void updateBlynkDisplay() {
  Blynk.virtualWrite(V1, dutyCycle); // Update Gauge PWM (0-255)
  Blynk.virtualWrite(V2, dutyCycle); // Update posisi Slider agar sinkron
  
  // Simulasi perhitungan RPM kasar berdasarkan rasio PWM (Maks 13000)
  int simulasiRPM = map(dutyCycle, 0, 255, 0, 13000);
  if(!isMotorRunning) simulasiRPM = 0;
  Blynk.virtualWrite(V0, simulasiRPM); // Update Gauge RPM
}

// --- FUNGSI HARDWARE MOTOR ---
void applyMotor() {
  if (isMotorRunning) {
    digitalWrite(motor1Pin1, HIGH);
    digitalWrite(motor1Pin2, LOW);
    ledcWrite(enable1Pin, dutyCycle);
  } else {
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, LOW);
    ledcWrite(enable1Pin, 0);
  }
  updateBlynkDisplay();
}

// --- LOGIKA BLYNK ---

// Menerima perintah dari Tombol ON/OFF di HP (Datastream V3)
BLYNK_WRITE(V3) {
  int pinValue = param.asInt(); // Baca status tombol (1 = ON, 0 = OFF)
  
  if (pinValue == 1) {
    isMotorRunning = true;
    if (dutyCycle == 0) dutyCycle = 200; // Torsi awal (Starting Torque)
    digitalWrite(ledPin, HIGH);
    Serial.println("BLYNK: Motor Dinyalakan");
  } else {
    isMotorRunning = false;
    digitalWrite(ledPin, LOW);
    Serial.println("BLYNK: Motor Dimatikan");
  }
  applyMotor();
}

// Menerima perintah dari Slider Kecepatan di HP (Datastream V2)
BLYNK_WRITE(V2) {
  dutyCycle = param.asInt(); // Baca nilai slider (0-255)
  Serial.print("BLYNK: PWM Diubah menjadi ");
  Serial.println(dutyCycle);
  
  // Jika motor sedang ON, terapkan perubahan kecepatan ke alat
  if (isMotorRunning) {
    applyMotor();
  }
}

// --- SETUP & LOOP UTAMA ---
void setup() {
  Serial.begin(115200);
  
  // Setup Pin & PWM
  pinMode(ledPin, OUTPUT);
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  ledcAttach(enable1Pin, freq, resolution);
  
  // Pastikan motor mati di awal
  applyMotor();
  
  Serial.println("Menghubungkan ke Wi-Fi dan Blynk Server...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Berhasil Terhubung!");
}

void loop() {
  Blynk.run(); // Menjaga koneksi ke server tetap hidup
}



