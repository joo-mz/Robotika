// PROGRESS 4 : FULL BLYNK + DYNAMIC TIMER + OTA
// Kontrol Jarak Jauh & Update Nirkabel

// --- KREDENSIAL BLYNK & WIFI ---
#define BLYNK_TEMPLATE_ID "ISI_DENGAN_TEMPLATE_ID_ANDA"
#define BLYNK_TEMPLATE_NAME "ISI_DENGAN_NAMA_TEMPLATE_ANDA"
#define BLYNK_AUTH_TOKEN "ISI_DENGAN_AUTH_TOKEN_DEVICE_BARU"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// Masukkan SSID dan Password Wi-Fi/Tethering Anda
char ssid[] = "NAMA_WIFI_ANDA";
char pass[] = "PASSWORD_WIFI_ANDA";

// --- PIN MOTOR ---
int motor1Pin1 = 27; 
int motor1Pin2 = 26; 
int enable1Pin = 12; 

// --- PENGATURAN PWM ---
const int freq = 30000;
const int resolution = 8;
int dutyCycle = 0;          
bool isMotorRunning = false; 

// --- VARIABEL DYNAMIC TIMER ---
int inputJam = 0;
int inputMenit = 0;
int inputDetik = 0;

unsigned long waktuMulaiTimer = 0;
unsigned long durasiTimer = 0; 
bool isTimerAktif = false;
long sisaWaktuTerakhir = -1;

unsigned long waktuTeksTerakhir = 0;
const long jedaTeks = 5000;

// Fungsi Kontrol Motor & Update Ke BLYNK

void applyMotor() {
  if (isMotorRunning) {
    digitalWrite(motor1Pin1, HIGH);
    digitalWrite(motor1Pin2, LOW);
    ledcWrite(enable1Pin, dutyCycle);
    
    Blynk.virtualWrite(V1, dutyCycle); // V1: PWM DISPLAY
    
    int simulasiRPM = map(dutyCycle, 0, 255, 0, 13000);
    Blynk.virtualWrite(V0, simulasiRPM); // V0: RPM MOTOR
    
  } else {
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, LOW);
    ledcWrite(enable1Pin, 0);
  
    Blynk.virtualWrite(V1, 0);
    Blynk.virtualWrite(V0, 0);
  }
}

// --- V3: KONTROL MANUAL ON/OFF ---
BLYNK_WRITE(V3) {
  if (param.asInt() == 1) {
    isMotorRunning = true;
    isTimerAktif = false; // Batalkan timer jika dinyalakan manual
    if (dutyCycle == 0) dutyCycle = 150; 
    Serial.println(">>> [BLYNK] Motor ON");
  } else {
    isMotorRunning = false;
    isTimerAktif = false;
    Serial.println(">>> [BLYNK] Motor OFF");
  }
  applyMotor();
}

// --- V2: SLIDER NAIK/TURUN KECEPATAN ---
BLYNK_WRITE(V2) {
  dutyCycle = param.asInt(); 
  if (isMotorRunning) applyMotor(); 
}

// --- MEMBACA SLIDER WAKTU (V4, V5, V6) ---
BLYNK_WRITE(V4) { inputJam = param.asInt(); }
BLYNK_WRITE(V5) { inputMenit = param.asInt(); }
BLYNK_WRITE(V6) { inputDetik = param.asInt(); }

// --- V7: TOMBOL TIMER START ---
BLYNK_WRITE(V7) {
  if (param.asInt() == 1) {
    // Kalkulasi total waktu ke dalam milidetik
    durasiTimer = (inputJam * 3600UL + inputMenit * 60UL + inputDetik) * 1000UL;
    
    if (durasiTimer > 0) {
      isTimerAktif = true;
      isMotorRunning = true;
      waktuMulaiTimer = millis();
      sisaWaktuTerakhir = -1;
      
      if (dutyCycle == 0) dutyCycle = 150; 
      applyMotor();
      
      Serial.println("\n>>> [TIMER] Memulai hitung mundur!");
    } else {
      // Jika tombol ditekan tapi slider waktu masih 0
      Blynk.virtualWrite(V7, 0); 
      Blynk.virtualWrite(V8, "Atur Waktu!"); 
    }
  } else {
    // Jika tombol dimatikan di tengah jalan
    isTimerAktif = false;
    isMotorRunning = false;
    applyMotor();
    Blynk.virtualWrite(V8, "BATAL");
    Serial.println(">>> [TIMER] Dibatalkan pengguna.");
  }
}

//Setup Awal

void setup() {
  Serial.begin(115200);

  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  ledcAttach(enable1Pin, freq, resolution);
  
  isMotorRunning = false;
  applyMotor();

  // 1. Konek ke WiFi
  Serial.println("\n Booting WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Koneksi WiFi Gagal! Restarting...");
    delay(3000); ESP.restart();
  }
  Serial.println("WiFi Terhubung!");
  Serial.print("IP Address ESP32: ");
  Serial.println(WiFi.localIP());

  // 2. Hubungkan ke Server Blynk
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();
  Serial.println("Berhasil Terhubung ke Blynk Cloud!");
  Blynk.virtualWrite(V8, "00:00:00"); 

  // 3. Konfigurasi OTA
  ArduinoOTA.setHostname("ESP32-Jonathan-FinalUTS"); 
  ArduinoOTA.setPassword("admin123");                
  
  ArduinoOTA.onStart([]() { Serial.println("\n MENERIMA UPDATE OTA..."); });
  ArduinoOTA.onEnd([]() { Serial.println("\n OTA SUKSES! RESTARTING..."); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.begin();
  Serial.println("\n--- BLYNK + TIMER + OTA SIAP 100% ---");
}

//Loop Utama

void loop() {
  Blynk.run();         // Jaga koneksi HP <--> ESP32
  ArduinoOTA.handle(); // Jaga pintu nirkabel untuk update kodingan

  // Indikator Siaga di Serial
  if (!isTimerAktif && (millis() - waktuTeksTerakhir >= jedaTeks)) {
    waktuTeksTerakhir = millis(); 
    Serial.println("[INFO] Sistem Standby... OTA dan Blynk Aktif.");
  }

  // --- LOGIKA HITUNG MUNDUR FORMAT JAM:MENIT:DETIK ---
  if (isTimerAktif) {
    unsigned long waktuBerjalan = millis() - waktuMulaiTimer;
    
    if (waktuBerjalan < durasiTimer) {
      long sisaTotalDetik = (durasiTimer - waktuBerjalan) / 1000;
      
      if (sisaTotalDetik != sisaWaktuTerakhir) {
        int h = sisaTotalDetik / 3600;
        int m = (sisaTotalDetik % 3600) / 60;
        int s = sisaTotalDetik % 60;
        
        char teksWaktu[10];
        sprintf(teksWaktu, "%02d:%02d:%02d", h, m, s);
        
        Blynk.virtualWrite(V8, teksWaktu); 
        sisaWaktuTerakhir = sisaTotalDetik;
      }
    } else {
      // Waktu Habis
      isTimerAktif = false;
      isMotorRunning = false;
      applyMotor();
      
      Blynk.virtualWrite(V7, 0);         
      Blynk.virtualWrite(V8, "SELESAI"); 
      Serial.println("\n>>> [TIMER] Waktu habis, Motor otomatis MATI.");
    }
  }
}

