// PROGRESS 1 : KONTROL MOTOR DASAR / Menggunakan Kabel
// Fitur: ON/OFF, Naik/Turun PWM via Serial Monitor

// --- KONFIGURASI PIN ---
int ledPin = 2;         // Indikator LED internal ESP32
int motor1Pin1 = 27;    // Pin IN1 Driver Motor
int motor1Pin2 = 26;    // Pin IN2 Driver Motor
int enable1Pin = 12;    // Pin ENA (PWM) Driver Motor

// --- PENGATURAN PWM ---
const int freq = 30000;      // Frekuensi PWM
const int resolution = 8;    // Resolusi 8-bit (0-255)
int dutyCycle = 200;         // Nilai kecepatan awal
const int DUTY_STEP = 10;    // Jarak kenaikan/penurunan kecepatan
const int DUTY_MIN  = 0;
const int DUTY_MAX  = 255;

bool isMotorRunning = false; // Status motor (menyala/mati)
String inputString = "";     // Penampung teks dari Serial Monitor

//Fungsi Kontrol Motor 

// Menerapkan nilai PWM ke hardware jika motor dalam status menyala
void applyMotor() {
  if (isMotorRunning) {
    digitalWrite(motor1Pin1, HIGH);
    digitalWrite(motor1Pin2, LOW);
    ledcWrite(enable1Pin, dutyCycle);
  }
}

// Menyalakan motor
void MotorOn() {
  isMotorRunning = true;
  if (dutyCycle == 0) dutyCycle = 200; // Beri nilai awal jika kecepatan 0
  applyMotor();
  Serial.print(">>> Motor ON, Kecepatan = "); Serial.println(dutyCycle);
  digitalWrite(ledPin, HIGH); 
}

// Mematikan motor
void MotorOff() {
  isMotorRunning = false;
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  ledcWrite(enable1Pin, 0); // Matikan sinyal PWM
  Serial.println(">>> Motor OFF");
  digitalWrite(ledPin, LOW); 
}

// Menambah kecepatan sebesar DUTY_STEP
void naikkanKecepatan() {
  dutyCycle += DUTY_STEP;
  if (dutyCycle > DUTY_MAX) dutyCycle = DUTY_MAX; // Batasi maksimal 255
  applyMotor();
  if (isMotorRunning) {
    Serial.print(">>> Kecepatan NAIK = "); Serial.println(dutyCycle);
  }
}

// Mengurangi kecepatan sebesar DUTY_STEP
void turunkanKecepatan() {
  dutyCycle -= DUTY_STEP;
  if (dutyCycle < DUTY_MIN) dutyCycle = DUTY_MIN; // Batasi minimal 0
  applyMotor();
  if (isMotorRunning) {
    Serial.print(">>> Kecepatan TURUN = "); Serial.println(dutyCycle);
  }
}

// Mengatur kecepatan ke nilai angka yang diketik manual
void setDuty(int val) {
  if (val < DUTY_MIN) val = DUTY_MIN;
  if (val > DUTY_MAX) val = DUTY_MAX;
  dutyCycle = val;
  applyMotor();
  if (isMotorRunning) {
    Serial.print(">>> Set Kecepatan = "); Serial.println(dutyCycle);
  }
}

// Setup Awal

void setup() {
  Serial.begin(115200);
  
  // Inisialisasi pin sebagai OUTPUT
  pinMode(ledPin, OUTPUT);
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  
  // Konfigurasi pin PWM ESP32
  ledcAttach(enable1Pin, freq, resolution);
  
  MotorOff(); // Pastikan motor mati saat baru menyala
  
  Serial.println("\n--- Sistem Kontrol Motor Siap ---");
  Serial.println("Ketik: '1' (ON), '0' (OFF), '+' (Naik), '-' (Turun), 'PWM:150' (Set Nilai)");
}

void loop() {
  // Membaca perintah dari pengguna melalui Serial Monitor
  while (Serial.available() > 0) {
    char c = Serial.read();
    
    // Jika menemukan karakter Enter (selesai mengetik)
    if (c == '\n' || c == '\r') {
      inputString.trim();
      
      // Mengecek apakah perintah berupa teks set nilai PWM (contoh: PWM:150)
      if (inputString.startsWith("PWM:")) {
        int val = inputString.substring(4).toInt();
        setDuty(val);
      } 
      // Mengecek apakah perintah berupa 1 karakter kontrol (1, 0, +, -)
      else if (inputString.length() > 0) {
        char cmd = inputString.charAt(0);
        if (cmd == '1')      MotorOn();
        else if (cmd == '0') MotorOff();
        else if (cmd == '+') naikkanKecepatan();
        else if (cmd == '-') turunkanKecepatan();
      }
      
      inputString = ""; // Kosongkan variabel untuk perintah berikutnya
    } else {
      inputString += c; // Rangkai karakter yang diketik menjadi kata
    }
  }
}
