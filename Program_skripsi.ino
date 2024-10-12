#define BLYNK_TEMPLATE_ID "TMPL66AS0FQfp"
#define BLYNK_TEMPLATE_NAME "pemantauan ketinggian air"
#define BLYNK_AUTH_TOKEN "cf7pDKI5XAheYKDIG3ZAq9JKxVhS71tm"
#include <HCSR04.h>
#include <ESP8266WiFi.h> 
#include<BlynkSimpleEsp8266.h>

#define BLYNK_PRINT Serial

BlynkTimer timer;
char auth[]= BLYNK_AUTH_TOKEN ;
char ssid[]="Rasya";
char pass[]="lupapassword";

HCSR04 hc(D5, D6); // Sensor ultrasonik
int tinggiWadah = 26; // Tinggi wadah dalam cm
int tinggiAir;
int cekpoint1 = 16;
int cekpoint2 = 18;
int cekpoint3 = 21;
unsigned long startTime = 0; // Waktu mulai
unsigned long stopTime = 0; // Waktu berhenti
bool isTiming = false; // Status apakah waktu sedang dihitung


void setup() {
  Serial.begin(9600); // Memulai Serial Monitor
  Blynk.begin(auth, ssid, pass);
}

void loop() {
  Blynk.run();
  float jarak = hc.dist(); // Mengukur jarak ke permukaan air
  tinggiAir = tinggiWadah - jarak; // Menghitung tinggi air

  Serial.print("Tinggi air: ");
  Serial.print(tinggiAir);
  Serial.println(" cm");
  Blynk.virtualWrite(V0,tinggiAir);

  if (!isTiming && tinggiAir >= cekpoint1 ) {
    // Mulai menghitung waktu saat tinggi air mencapai atau melebihi 20 cm
    startTime = millis();
    isTiming = true;
    Serial.println("Menghitung waktu mulai...");
    Blynk.logEvent("waspada",String("Status Waspada, air sudah di atas ketinggian ")+cekpoint1+(" cm yaitu : ")+tinggiAir+(" cm"));

  }

  if (isTiming && tinggiAir >= cekpoint2) {
    // Hentikan waktu saat tinggi air mencapai atau melebihi 40 cm
    stopTime = millis(); // Waktu berhenti
    unsigned long elapsedTime = stopTime - startTime; // Waktu yang berlalu
    isTiming = false; // Menghentikan penghitungan waktu

    Serial.print("Waktu berhenti. Waktu yang berlalu: ");
    Serial.print(elapsedTime / 1000.0); // Dalam detik
    Serial.println(" detik");

    // Periksa apakah elapsedTime tidak nol sebelum menghitung kecepatan
    if (elapsedTime > 0) {
      float kecepatan = (float)(cekpoint2 - cekpoint1) / (elapsedTime / 1000.0); // Kecepatan rata-rata
      Serial.print("Kecepatan air: ");
      Serial.print(kecepatan);
      Serial.println(" cm/detik");

      // Periksa apakah kecepatan tidak nol sebelum menghitung prediksi waktu
      if (kecepatan > 0) {
        float waktubanjir = (float)(cekpoint3 - cekpoint2) / kecepatan;
        Serial.print("Prediksi waktu untuk mencapai cekpoint 3: ");
        Serial.print(waktubanjir);
        Serial.println(" detik");
        Blynk.virtualWrite(V1,waktubanjir);
        Blynk.logEvent("siaga",String("Status SIAGA, air sudah di atas ketinggian ")+cekpoint2+(" cm yaitu : ")+tinggiAir+(" cm. Perkiraan waktu untuk mencapai BAHAYA= ") + waktubanjir +(" Detik"));
      } else {
        Serial.println("Tidak dapat menghitung prediksi waktu (kecepatan nol).");
      }
    } else {
      Serial.println("Waktu yang berlalu terlalu cepat, tidak dapat menghitung kecepatan.");
    }
    
  }

  if (tinggiAir >= cekpoint3 ) {
    Blynk.logEvent("bahaya",String("Status BAHAYA, air sudah di atas ketinggian ")+cekpoint3+(" cm yaitu : ")+tinggiAir+(" cm"));

  }

  if (isTiming && tinggiAir < cekpoint1) {
    // Jika tinggi air turun di bawah 20 cm, reset penghitungan
    isTiming = false; // Setel ulang status timing
    Serial.println("Penghitungan di-reset karena tinggi air di bawah 20 cm.");
  }

  delay(500); // Penundaan untuk menghindari pembacaan berulang terlalu cepat
}

BLYNK_WRITE(V2){
  tinggiWadah = param.asInt();
}
BLYNK_WRITE(V3){
  cekpoint1 = param.asInt();
}
BLYNK_WRITE(V4){
  cekpoint2 = param.asInt();
}
BLYNK_WRITE(V5){
  cekpoint3 = param.asInt();
}