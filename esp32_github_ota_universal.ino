/*
 * ======================================================================================
 * ESP32 UNIVERSAL GITHUB OTA TEMPLATE / BOILERPLATE
 * ======================================================================================
 * Deskripsi   : Template modular untuk sistem pembaruan Over-The-Air (OTA) berbasis 
 *               GitHub Releases. Dapat diintegrasikan ke proyek apa pun (Sensor, 
 *               Relay, MQTT, WebServer, Modbus, SCADA, Smart Home, dll).
 * Pengembang  : Farel Gilang & Kontributor Komunitas
 * Lisensi     : MIT License
 * ======================================================================================
 */

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPUpdate.h>

// ======================================================================================
// 1. KONFIGURASI JARINGAN & GITHUB OTA
// ======================================================================================
const char* WIFI_SSID     = "NAMA_WIFI_ANDA";
const char* WIFI_PASSWORD = "PASSWORD_WIFI_ANDA";

// URL file binary rilis terbaru di GitHub Releases
// Format: https://github.com/<USERNAME>/<REPO>/releases/latest/download/<NAMA_FILE>.ino.bin
const char* FIRMWARE_URL  = "https://github.com/ZaxxyChill/esp32-ota/releases/latest/download/esp32_github_ota_universal.ino.bin";

// Versi firmware saat ini (Tingkatkan angka ini setiap membuat rilis baru)
const char* CURRENT_VERSION = "1.0.0";

// Interval pengecekan update otomatis (dalam milidetik)
// 60000 = 1 Menit (Untuk testing)
// 3600000 = 1 Jam (Rekomendasi untuk produksi/penggunaan jangka panjang)
const unsigned long OTA_CHECK_INTERVAL = 60000; 

// ======================================================================================
// 2. VARIABEL SISTEM
// ======================================================================================
unsigned long lastOTACheckTime = 0;

// ======================================================================================
// 3. PROTOTYPE FUNGSI
// ======================================================================================
void connectToWiFi();
void checkFirmwareUpdate();
void userSetup();
void userLoop();

// ======================================================================================
// 4. SETUP SISTEM UTAMA
// ======================================================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n==================================================");
  Serial.println("       ESP32 UNIVERSAL GITHUB OTA FRAMEWORK       ");
  Serial.printf ("       Versi Firmware: %s\n", CURRENT_VERSION);
  Serial.println("==================================================");

  // Hubungkan ke WiFi
  connectToWiFi();

  // Inisialisasi program / sensor kustom pengguna
  userSetup();

  // Lakukan pengecekan update pertama kali saat booting
  Serial.println("[OTA] Pengecekan firmware awal saat booting...");
  checkFirmwareUpdate();
}

// ======================================================================================
// 5. LOOP SISTEM UTAMA
// ======================================================================================
void loop() {
  // Jalankan logika program pengguna (Non-blocking)
  userLoop();

  // Timer pengecekan OTA otomatis tanpa menghentikan logika program utama
  if (millis() - lastOTACheckTime >= OTA_CHECK_INTERVAL) {
    lastOTACheckTime = millis();
    checkFirmwareUpdate();
  }
}

// ======================================================================================
// 6. LOGIKA PROGRAM PENGGUNA (TEMPATKAN KODE ANDA DI SINI)
// ======================================================================================

void userSetup() {
  Serial.println("[APP] Menginisialisasi komponen kustom pengguna...");
  // Masukkan inisialisasi sensor, pin relay, MQTT, OLED, dll di sini
  // Contoh:
  // pinMode(2, OUTPUT); // LED Builtin
}

void userLoop() {
  // Masukkan tugas utama alat Anda di sini (pembacaan sensor, kirim MQTT, dll)
  // CATATAN: Hindari penggunaan delay() berdurasi besar (misal: delay(5000))
  // Gunakan teknik millis() agar pengecekan OTA dan fungsi sistem tetap responsif.
}

// ======================================================================================
// 7. MESIN GITHUB OTA (ENGINE)
// ======================================================================================

void connectToWiFi() {
  Serial.printf("[WiFi] Menghubungkan ke %s ", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startAttempt = millis();
  // Timeout 15 detik untuk koneksi WiFi
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 15000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WiFi] Terhubung!");
    Serial.printf("[WiFi] IP Address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("[WiFi] Signal (RSSI): %d dBm\n", WiFi.RSSI());
  } else {
    Serial.println("\n[WiFi] Gagal terhubung (Timeout). Melanjutkan operasi offline...");
  }
}

void checkFirmwareUpdate() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[OTA] Batal cek update: WiFi tidak terhubung.");
    return;
  }

  Serial.println("[OTA] Menghubungi GitHub Releases...");

  WiFiClientSecure client;
  // Mengabaikan verifikasi sertifikat SSL root untuk mempermudah koneksi ke CDN GitHub
  client.setInsecure();

  // Wajib aktif karena GitHub me-redirect link download ke Amazon S3 CDN
  httpUpdate.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  // Eksekusi pembaruan firmware
  t_httpUpdate_return ret = httpUpdate.update(client, FIRMWARE_URL);

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("[OTA] Update Gagal! Error (%d): %s\n", 
                    httpUpdate.getLastError(), 
                    httpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("[OTA] Firmware sudah versi paling baru (Tidak ada update).");
      break;

    case HTTP_UPDATE_OK:
      Serial.println("[OTA] Firmware baru berhasil diunduh dan dipasang!");
      Serial.println("[OTA] Memulai ulang perangkat (Rebooting)...");
      ESP.restart();
      break;
  }
}
