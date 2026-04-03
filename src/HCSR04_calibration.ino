/* HC-SR04 Distance Measurement (ESP8266)
--------------------------------------
Mengukur jarak berdasarkan durasi ECHO (µs)
Rumus: jarak (cm) = t / 58.3
Output: 1 data per detik, berhenti setelah 10 detik
*/

#define TRIG D7
#define ECHO D8

unsigned long startTime; // waktu mulai program
const unsigned long runTime = 10000; // 10 detik (ms)

void setup() {
  Serial.begin(9600);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  digitalWrite(TRIG, LOW);
  delay(1000);

  Serial.println("Mulai pengukuran jarak HC-SR04 (10 detik)...");
  startTime = millis();
}

void loop() {
  if (millis() - startTime >= runTime) {
    Serial.println("Pengukuran selesai.");
    while (true); // berhenti
  }

  // --- Trigger 10 µs ---
  digitalWrite(TRIG, LOW);
  delayMicroseconds(4);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  // --- Baca durasi echo ---
  unsigned long duration = pulseIn(ECHO, HIGH, 30000UL); // timeout 30 ms

  // --- Hitung jarak (cm) ---
  float distance = duration / 58.3; // rumus fisika: v = 343 m/s

  // --- Tampilkan hasil ---
  Serial.print("Durasi: ");
  Serial.print(duration);
  Serial.print(" us | Jarak: ");
  Serial.print(distance, 2);
  Serial.println(" cm");

  delay(1000); // 1 data per detik
}