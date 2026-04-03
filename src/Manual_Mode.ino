/* Water Level Monitoring ESP8266 + Blynk + LCD
   Dengan Kalibrasi HC-SR04
   ----------------------------------------------
   - Jarak (cm) = (t_us / 58.3)  -> lalu dikalibrasi: y = a*x + b
   - Level:
       LOW    -> D3
       MEDIUM -> D3 + D4
       HIGH   -> D3 + D4 + D5
       FULL   -> D3 + D4 + D5 + BUZZER(D6) berkedip
   - Relay D0 (aktif-LOW)
*/

#include <LiquidCrystal_I2C.h>
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- Blynk / WiFi ---
char auth[] = "S4vCWAgqU_KuzDDeAFYSyO-gEUsmu6Wo";
char ssid[] = "HiFi-13897";
char pass[] = "pssslemanale";
BlynkTimer timer;

// --- Pin ---
#define TRIG   D7
#define ECHO   D8
#define LED3   D3     // LOW
#define LED4   D4     // MEDIUM/HIGH/FULL
#define LED5   D5     // HIGH/FULL
#define BUZZER D6     // buzzer
#define RELAY  D0     // aktif-LOW (LOW=ON)

// --- Kalibrasi HC-SR04 (dari regresi: y = a*x + b) ---
const float CAL_A = 0.9889f;   // slope
const float CAL_B = 0.00f;     // offset (ubah jika ada)

// --- Skala tinggi air ---
const int MaxLevel = 15;                 // cm (0..15 cm)
const float T_LOW  = MaxLevel * 0.33f;   // ≈5 cm
const float T_HIGH = MaxLevel * 0.66f;   // ≈10 cm
const float T_FULL = MaxLevel * 0.90f;   // ≈13.5 cm

bool relayOn = false;
bool fullAlarm = false, blinkState = false;
unsigned long lastBlink = 0;
const unsigned long BLINK_MS = 300;

int levelCm = 0;           // tinggi air (cm) terkalibrasi
String levelText = "---";

// ===================================================
// Baca jarak terkalibrasi (cm) dari HC-SR04
float readDistanceCmCal() {
  // trigger 10 µs
  digitalWrite(TRIG, LOW);  delayMicroseconds(4);
  digitalWrite(TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  // durasi ECHO (µs), timeout 30 ms
  unsigned long t = pulseIn(ECHO, HIGH, 30000UL);
  if (t == 0) return MaxLevel + 200;     // jika timeout

  // jarak mentah (cm) pakai fisika: t/58.3
  float d_raw = t / 58.3f;

  // terapkan kalibrasi: y = a*x + b
  float d_cal = CAL_A * d_raw + CAL_B;
  return d_cal;
}

void lcdShow() {
  // Baris 1
  lcd.setCursor(0, 0);
  lcd.print("WLevel ");
  lcd.print(levelText);
  lcd.print("     "); // bersihkan sisa

  // Baris 2
  lcd.setCursor(0, 1);
  lcd.print("h:");
  if (levelCm < 10) lcd.print('0');
  lcd.print(levelCm);
  lcd.print("cm Motor:");
  lcd.print(relayOn ? "ON " : "OFF");
  lcd.print("  ");
}

void setLEDs(bool L3, bool L4, bool L5) {
  digitalWrite(LED3, L3);
  digitalWrite(LED4, L4);
  digitalWrite(LED5, L5);
}

void handleFullBlink() {
  if (!fullAlarm) return;
  if (millis() - lastBlink >= BLINK_MS) {
    lastBlink = millis();
    blinkState = !blinkState;
    digitalWrite(LED3, blinkState);
    digitalWrite(LED4, blinkState);
    digitalWrite(LED5, blinkState);
    digitalWrite(BUZZER, blinkState);
  }
}

// ===================================================
void ultrasonicTask() {
  float distance = readDistanceCmCal();            // jarak sensor → cm (terkalibrasi)
  int height = (distance <= MaxLevel) ? (MaxLevel - (int)(distance + 0.5f)) : 0;
  if (height < 0) height = 0;
  if (height > MaxLevel) height = MaxLevel;

  levelCm = height;
  Blynk.virtualWrite(V0, levelCm);

  // Tentukan status level
  if (levelCm >= (int)T_FULL) {
    levelText = "Full  ";
    fullAlarm = true;
  } else if (levelCm >= (int)T_HIGH) {
    levelText = "High  ";
    fullAlarm = false;
    setLEDs(HIGH, HIGH, HIGH);
    digitalWrite(BUZZER, LOW);
  } else if (levelCm >= (int)T_LOW) {
    levelText = "Medium";
    fullAlarm = false;
    setLEDs(HIGH, HIGH, LOW);
    digitalWrite(BUZZER, LOW);
  } else {
    levelText = "Low   ";
    fullAlarm = false;
    setLEDs(HIGH, LOW, LOW);
    digitalWrite(BUZZER, LOW);
  }

  if (fullAlarm) handleFullBlink();
  lcdShow();
}

// ===================================================
BLYNK_WRITE(V1) {
  bool v = param.asInt();
  if (v) { digitalWrite(RELAY, LOW);  relayOn = true;  }
  else   { digitalWrite(RELAY, HIGH); relayOn = false; }
  lcdShow();
}

// ===================================================
void setup() {
  Serial.begin(9600);
  lcd.init(); lcd.backlight();

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);       // ECHO wajib via divider ke 3.3V saat wiring nyata
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  pinMode(LED5, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(RELAY, OUTPUT);

  digitalWrite(RELAY, HIGH); relayOn = false;
  setLEDs(LOW, LOW, LOW);
  digitalWrite(BUZZER, LOW);

  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);

  lcd.clear();
  lcd.setCursor(0,0); lcd.print("Water Level Init");
  lcd.setCursor(0,1); lcd.print("Motor: OFF");
  delay(1000);
  lcd.clear();

  timer.setInterval(300L, ultrasonicTask);
}

// ===================================================
void loop() {
  Blynk.run();
  timer.run();
  handleFullBlink();
}