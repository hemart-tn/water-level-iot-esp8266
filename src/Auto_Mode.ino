/* Water Level Monitoring ESP8266 + Blynk + LCD
   Versi Otomatis + Sinkronisasi Tombol Blynk
   ------------------------------------------------
   - LOW  -> Relay ON  (pompa hidup)
   - FULL -> Relay OFF (pompa mati)
   - Tombol Blynk V1 ikut berubah otomatis
   - LCD: WLevel (Low/Medium/High/Full)
           h: XXcm  Motor: ON/OFF
   - 3 LED indikator (D3,D4,D5)
   - Buzzer (D6)
   - Relay aktif-LOW (D0)
*/

#include <LiquidCrystal_I2C.h>
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

char auth[] = "S4vCWAgqU_KuzDDeAFYSyO-gEUsmu6Wo";
char ssid[] = "HiFi-13897";
char pass[] = "pssslemanale";

BlynkTimer timer;

// ---- Pin ----
#define TRIG   D7
#define ECHO   D8
#define LED3   D3     // Low
#define LED4   D4     // Medium
#define LED5   D5     // High
#define BUZZER D6     // Full buzzer
#define RELAY  D0     // Relay aktif-LOW

// ---- Parameter ----
const int MaxLevel = 15;                 // cm
const float T_LOW  = MaxLevel * 0.33f;   // 5 cm
const float T_HIGH = MaxLevel * 0.66f;   // 10 cm
const float T_FULL = MaxLevel * 0.90f;   // 13.5 cm

// ---- Variabel ----
bool relayOn = false;
bool fullAlarm = false, blinkState = false;
unsigned long lastBlink = 0;
const unsigned long BLINK_MS = 300;

int levelCm = 0;
String levelText = "---";

// ===================================================
// KONVERSI + KALIBRASI: jarak(cm) = 0.9889 * (t_us / 58.3)
int readDistanceCm() {
  digitalWrite(TRIG, LOW); delayMicroseconds(4);
  digitalWrite(TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  unsigned long t = pulseIn(ECHO, HIGH, 30000UL);
  if (t == 0) return MaxLevel + 200;

  float d_raw = t / 58.3f;          // konversi fisika
  float d_cal = 0.9889f * d_raw;    // kalibrasi: y = 0.9889 x
  return (int)(d_cal + 0.5f);       // pembulatan ke cm
}

// ===================================================
void lcdShow() {
  lcd.setCursor(0,0);
  lcd.print("WLevel ");
  lcd.print(levelText);
  lcd.print("     ");

  lcd.setCursor(0,1);
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
void setRelayAuto(bool state) {
  digitalWrite(RELAY, state ? LOW : HIGH);
  relayOn = state;
  Blynk.virtualWrite(V1, state ? 1 : 0); // sinkron tombol
}

// ===================================================
void ultrasonicTask() {
  int distance = readDistanceCm();                         // sudah terkalibrasi
  levelCm = (distance <= MaxLevel) ? (MaxLevel - distance) : 0;
  Blynk.virtualWrite(V0, levelCm);

  // Tentukan status air
  if (levelCm >= (int)T_FULL) {
    levelText = "Full  ";
    fullAlarm = true;
    setRelayAuto(false);  // OFF saat penuh
  } 
  else if (levelCm >= (int)T_HIGH) {
    levelText = "High  ";
    fullAlarm = false;
    setLEDs(HIGH, HIGH, HIGH);
    digitalWrite(BUZZER, LOW);
  } 
  else if (levelCm >= (int)T_LOW) {
    levelText = "Medium";
    fullAlarm = false;
    setLEDs(HIGH, HIGH, LOW);
    digitalWrite(BUZZER, LOW);
  } 
  else { // LOW
    levelText = "Low   ";
    fullAlarm = false;
    setLEDs(HIGH, LOW, LOW);
    digitalWrite(BUZZER, LOW);
    setRelayAuto(true);   // ON saat low
  }

  if (fullAlarm) handleFullBlink();
  lcdShow();
}

// ===================================================
BLYNK_WRITE(V1) {
  bool v = param.asInt();   // manual override, tetap sinkron
  setRelayAuto(v);
  lcdShow();
}

// ===================================================
void setup() {
  Serial.begin(9600);
  lcd.init(); lcd.backlight();

  pinMode(TRIG, OUTPUT); pinMode(ECHO, INPUT);
  pinMode(LED3, OUTPUT); pinMode(LED4, OUTPUT); pinMode(LED5, OUTPUT);
  pinMode(BUZZER, OUTPUT); pinMode(RELAY, OUTPUT);

  digitalWrite(RELAY, HIGH); relayOn = false;
  setLEDs(LOW, LOW, LOW); digitalWrite(BUZZER, LOW);

  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);

  lcd.clear();
  lcd.setCursor(0,0); lcd.print("Water Level Init");
  lcd.setCursor(0,1); lcd.print("Motor: OFF");
  delay(1000); lcd.clear();

  timer.setInterval(300L, ultrasonicTask);
}

// ===================================================
void loop() {
  Blynk.run();
  timer.run();
  handleFullBlink();
}