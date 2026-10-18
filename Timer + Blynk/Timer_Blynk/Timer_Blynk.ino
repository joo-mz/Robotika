// PROGRESS 3 : KONTROL MOTOR + DYNAMIC TIMER + BLYNK
// Fitur: ON/OFF, PWM, Hitung Mundur Kustom via Aplikasi Blynk

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

//  DOKUMENTASI VIRTUAL PIN PADA BLYNK
// V0 : Gauge   - Simulasi Putaran Mesin / RPM (0 - 13000)
// V1 : Gauge   - Tampilan Nilai Kecepatan PWM (0 - 255)
// V2 : Slider  - Pengatur Kecepatan Motor PWM (0 - 255)
// V3 : Button  - Sakelar Motor ON/OFF (Mode Switch, 0/1)
// V4 : Slider  - Input Jam (0 - 24)
// V5 : Slider  - Input Menit (0 - 59)
// V6 : Slider  - Input Detik (0 - 59)
// V7 : Button  - Tombol Start Timer (Mode Switch, 0/1)
// V8 : Display - Teks Hitung Mundur / Countdown (String)

// --- KONFIGURASI PIN ---
int motor1Pin1 = 27;    // Pin IN1 Driver Motor
int motor1Pin2 = 26;    // Pin IN2 Driver Motor
int enable1Pin = 12;    // Pin ENA (PWM) Driver Motor

// --- PENGATURAN PWM ---
const int freq = 30000;      // Frekuensi PWM
const int resolution = 8;    // Resolusi 8-bit (0-255)
int dutyCycle = 0;           // Kecepatan awal
bool isMotorRunning = false; // Status motor

// --- VARIABEL DYNAMIC TIMER ---
int inputJam = 0;
int inputMenit = 0;
int inputDetik = 0;

unsigned long waktuMulaiTimer = 0; // Mencatat waktu saat timer dimulai
unsigned long durasiTimer = 0;     // Total durasi timer dalam milidetik
bool isTimerAktif = false;         // Status timer berjalan
long sisaWaktuTerakhir = -1;       // Menyimpan detik terakhir (mencegah spam Blynk)

// Fungsi Kontrol Motor & Update Layar Blynk

// Menerapkan nilai PWM ke hardware & update antarmuka Blynk
void applyMotor() {
  if (isMotorRunning) {
    digitalWrite(motor1Pin1, HIGH);
    digitalWrite(motor1Pin2, LOW);
    ledcWrite(enable1Pin, dutyCycle);
    
    Blynk.virtualWrite(V1, dutyCycle); // Update gauge PWM
    
    int simulasiRPM = map(dutyCycle, 0, 255, 0, 13000);
    Blynk.virtualWrite(V0, simulasiRPM); // Update gauge RPM
    
  } else {
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, LOW);
    ledcWrite(enable1Pin, 0);
  
    Blynk.virtualWrite(V1, 0);
    Blynk.virtualWrite(V0, 0);
  }
}

// Menerima perintah dari Tombol ON/OFF Manual (V3)
BLYNK_WRITE(V3) {
  if (param.asInt() == 1) {
    isMotorRunning = true;
    isTimerAktif = false; // Batalkan timer jika tombol manual ditekan
    if (dutyCycle == 0) dutyCycle = 150; 
    Serial.println(">>> [BLYNK] Motor ON ");
  } else {
    isMotorRunning = false;
    isTimerAktif = false; // Batalkan timer jika dimatikan manual
    Serial.println(">>> [BLYNK] Motor OFF ");
  }
  applyMotor();
}

// Menerima perintah dari Slider Kecepatan (V2)
BLYNK_WRITE(V2) {
  dutyCycle = param.asInt(); 
  Serial.print(">>> [BLYNK] PWM Diubah menjadi: ");
  Serial.println(dutyCycle);
  if (isMotorRunning) applyMotor(); 
}

// Membaca nilai dari Slider Waktu (V4, V5, V6)
BLYNK_WRITE(V4) { inputJam = param.asInt(); }
BLYNK_WRITE(V5) { inputMenit = param.asInt(); }
BLYNK_WRITE(V6) { inputDetik = param.asInt(); }

// Menerima perintah dari Tombol Start Timer (V7)
BLYNK_WRITE(V7) {
  if (param.asInt() == 1) {
    // Kalkulasi total waktu dari slider ke dalam milidetik
    durasiTimer = (inputJam * 3600UL + inputMenit * 60UL + inputDetik) * 1000UL;
    
    if (durasiTimer > 0) {
      isTimerAktif = true;
      isMotorRunning = true;
      waktuMulaiTimer = millis();
      sisaWaktuTerakhir = -1; // Reset agar langsung update layar
      
      if (dutyCycle == 0) dutyCycle = 150; 
      applyMotor();
      
      Serial.println("\n>>> [TIMER] Memulai hitung mundur dari aplikasi!");
    } else {
      // Jika tombol start ditekan tapi semua slider waktu masih 0
      Blynk.virtualWrite(V7, 0);             // Kembalikan tombol ke OFF
      Blynk.virtualWrite(V8, "Atur Waktu!"); // Tampilkan pesan peringatan
    }
  } else {
    // Jika tombol start dimatikan manual di tengah jalan
    isTimerAktif = false;
    isMotorRunning = false;
    applyMotor();
    Blynk.virtualWrite(V8, "BATAL");
    Serial.println(">>> [TIMER] Dibatalkan oleh pengguna.");
  }
}

// Setup Awal

void setup() {
  Serial.begin(115200);

  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  ledcAttach(enable1Pin, freq, resolution);
  
  isMotorRunning = false;
  applyMotor();

  Serial.println("\n Menghubungkan ke WiFi dan Blynk... "); 
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  Serial.println(">>> Berhasil Terhubung ke Blynk Cloud!");
  Blynk.virtualWrite(V8, "00:00:00"); // Reset layar timer di HP saat baru menyala
}

// Loop Utama

void loop() {
  Blynk.run(); // Menjaga koneksi dengan server Blynk

  // --- LOGIKA HITUNG MUNDUR FORMAT JAM:MENIT:DETIK ---
  if (isTimerAktif) {
    unsigned long waktuBerjalan = millis() - waktuMulaiTimer;
    
    if (waktuBerjalan < durasiTimer) {
      long sisaTotalDetik = (durasiTimer - waktuBerjalan) / 1000;
      
      // Update layar HP hanya saat detiknya berubah agar aplikasi tidak lag
      if (sisaTotalDetik != sisaWaktuTerakhir) {
        int h = sisaTotalDetik / 3600;
        int m = (sisaTotalDetik % 3600) / 60;
        int s = sisaTotalDetik % 60;
        
        // Format teks menjadi 00:00:00
        char teksWaktu[10];
        sprintf(teksWaktu, "%02d:%02d:%02d", h, m, s);
        
        Blynk.virtualWrite(V8, teksWaktu); 
        
        // --- Mencetak Teks Hitung Mundur ke Serial Monitor ---
        Serial.print(">>> [TIMER] Sisa Waktu: ");
        Serial.println(teksWaktu);
        
        sisaWaktuTerakhir = sisaTotalDetik;
      }
    } else {
      // Waktu Habis
      isTimerAktif = false;
      isMotorRunning = false;
      applyMotor();
      
      Blynk.virtualWrite(V7, 0);         // Matikan tombol Start di HP
      Blynk.virtualWrite(V8, "SELESAI"); // Tampilkan teks selesai
      Serial.println("\n>>> [TIMER] Waktu habis! Motor otomatis MATI.");
    }
  }
}

