#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPUpdate.h>

// ==========================================
// PENGATURAN WIFI
// ==========================================
const char* ssid = "NAMA_WIFI_KAMU";
const char* password = "PASSWORD_WIFI_KAMU";

// ==========================================
// PENGATURAN GITHUB OTA
// ==========================================
// Link sudah otomatis menggunakan "/latest/"
const char* firmware_url = "https://github.com/ZaxxyChill/esp32-ota/releases/latest/download/esp32_github_ota_rgb.ino.bin";

// Versi firmware (Ubah jadi 3.0.0)
String currentVersion = "3.0.0"; 

// ==========================================
// PENGATURAN PIN RGB LED
// ==========================================
const int redPin = 25;
const int greenPin = 26;
const int bluePin = 27;

// Variabel untuk transisi warna
int hue = 0; 

// ==========================================
// PENGATURAN AUTO-UPDATE TANPA RESTART
// ==========================================
unsigned long lastCheckTime = 0;
// Interval pengecekan dalam milidetik (60000 = 1 menit)
// Jika nanti dipasang secara permanen, ubah menjadi misal 1 jam (3600000) agar ESP tidak kepanasan
const unsigned long checkInterval = 60000; 

// Deklarasi fungsi
void setColor(int red, int green, int blue);
void setHue(int h);
void checkAndUpdate();

void setup() {
  Serial.begin(115200);
  
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  Serial.println();
  Serial.print("Menghubungkan ke WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  
  int toggle = 0;
  while (WiFi.status() != WL_CONNECTED) {
    toggle = !toggle;
    if (toggle) setColor(0, 0, 50); // Biru redup
    else setColor(0, 0, 0); 
    delay(250);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi terhubung!");
  Serial.print("Versi Firmware Saat Ini: ");
  Serial.println(currentVersion);

  // Mengecek update 1x saat baru dicolok ke listrik
  delay(3000);
  checkAndUpdate();
}

void loop() {
  // Efek transisi warna halus (pelangi / rainbow fade)
  setHue(hue);
  
  hue++;
  if (hue >= 360) {
    hue = 0;
  }
  
  delay(15); 
  
  // ==========================================
  // LOGIKA OTA OTOMATIS BERKALA
  // ==========================================
  // Kode ini akan mengecek update setiap 1 menit secara diam-diam
  // tanpa menghentikan efek lampu pelangi dan tanpa perlu restart manual
  if (millis() - lastCheckTime >= checkInterval) {
    lastCheckTime = millis();
    Serial.println("Waktunya mengecek update otomatis...");
    checkAndUpdate();
  }
}

// Fungsi pembantu untuk mengatur warna LED RGB
void setColor(int red, int green, int blue) {
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}

// Fungsi transisi warna HSV
void setHue(int h) {
  float s = 1.0; 
  float v = 1.0; 
  
  float c = v * s;
  float x = c * (1.0 - abs(fmod(h / 60.0, 2) - 1.0));
  float m = v - c;
  
  float r, g, b;
  if (h >= 0 && h < 60) { r = c; g = x; b = 0; }
  else if (h >= 60 && h < 120) { r = x; g = c; b = 0; }
  else if (h >= 120 && h < 180) { r = 0; g = c; b = x; }
  else if (h >= 180 && h < 240) { r = 0; g = x; b = c; }
  else if (h >= 240 && h < 300) { r = x; g = 0; b = c; }
  else { r = c; g = 0; b = x; }
  
  int R = (r + m) * 255;
  int G = (g + m) * 255;
  int B = (b + m) * 255;
  
  setColor(R, G, B);
}

void checkAndUpdate() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure(); 

    httpUpdate.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    t_httpUpdate_return ret = httpUpdate.update(client, firmware_url);

    switch (ret) {
      case HTTP_UPDATE_OK:
        Serial.println("Update Berhasil! Memulai ulang perangkat...");
        // ESP akan me-restart dirinya otomatis jika ada file baru!
        break;
      case HTTP_UPDATE_FAILED:
      case HTTP_UPDATE_NO_UPDATES:
        // Jika tidak ada update, diam saja (supaya lampu pelangi tidak macet)
        break;
    }
  }
}
