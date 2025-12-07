#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// =====================
// LCD CONFIG
// =====================
// Common I2C addresses: 0x27 or 0x3F
LiquidCrystal_I2C lcd(0x27, 16, 2);

// =====================
// PIN CONFIG
// =====================
const int moisturePin   = A0;  // Analog moisture sensor
const int waterPumpPin  = 2;   // Relay for water pump (ACTIVE LOW)
const int fertilizerPin = 3;   // Relay for fertilizer pump (ACTIVE LOW)

// =====================
// BEHAVIOR CONFIG (editable at runtime)
// =====================
int DRY_THRESHOLD_PERCENT = 50;            // Show DRY at/under this (%)
unsigned long waterDurationMs     = 5000;  // WATER command duration
unsigned long fertilizeDurationMs = 3000;  // FERTILIZE command duration

// Moisture mapping (tune to your sensor/calibration)
// "750" ~ dry, "350" ~ wet (adjust to your readings)
const int MOISTURE_DRY_RAW = 750;
const int MOISTURE_WET_RAW = 350;

// =====================
// STATE
// =====================
String scheduledDate = ""; // YYYY-MM-DD
String scheduledTime = ""; // HH:MM (24h)

// =====================
// HELPERS
// =====================
int moisturePercentFromRaw(int raw) {
  int pct = map(raw, MOISTURE_DRY_RAW, MOISTURE_WET_RAW, 0, 100);
  return constrain(pct, 0, 100);
}

void lcdPrintLine(uint8_t col, uint8_t row, const String &text) {
  lcd.setCursor(col, row);
  lcd.print(text);
  int len = text.length();
  for (int i = len; i < 16 - col; i++) lcd.print(' ');
}

void pumpsOff() {
  digitalWrite(waterPumpPin, HIGH);     // OFF for active LOW
  digitalWrite(fertilizerPin, HIGH);    // OFF for active LOW
}

void activateWaterPump(unsigned long ms) {
  Serial.println("WATER:ON");
  digitalWrite(waterPumpPin, LOW);   // ON
  delay(ms);
  digitalWrite(waterPumpPin, HIGH);  // OFF
  Serial.println("WATER:OFF");
}

void activateFertilizerPump(unsigned long ms) {
  Serial.println("FERTILIZE:ON");
  digitalWrite(fertilizerPin, LOW);   // ON
  delay(ms);
  digitalWrite(fertilizerPin, HIGH);  // OFF
  Serial.println("FERTILIZE:OFF");
}

String toUpperCopy(String s) {
  s.trim();
  for (size_t i = 0; i < s.length(); i++) s[i] = toupper(s[i]);
  return s;
}

void handleCommand(String cmd) {
  // Accept both lower/upper case; keep original for parameter parsing
  String upper = toUpperCopy(cmd);

  Serial.print("CMD:"); Serial.println(cmd);

  if (upper == "WATER") {
    activateWaterPump(waterDurationMs);
    return;
  }

  if (upper == "FERTILIZE") {
    activateFertilizerPump(fertilizeDurationMs);
    return;
  }

  if (upper == "PUMPS:OFF") {
    pumpsOff();
    Serial.println("OK:PUMPS_OFF");
    return;
  }

  if (upper.startsWith("SCHEDULE:")) {
    // Format: SCHEDULE:YYYY-MM-DD:HH:MM
    int p1 = cmd.indexOf(':');
    int p2 = cmd.indexOf(':', p1 + 1);
    if (p1 > 0 && p2 > 0) {
      scheduledDate = cmd.substring(p1 + 1, p2);
      scheduledTime = cmd.substring(p2 + 1);
      Serial.print("SCHEDULED:");
      Serial.print(scheduledDate);
      Serial.print(" ");
      Serial.println(scheduledTime);
    } else {
      Serial.println("ERR:BAD_SCHEDULE");
    }
    return;
  }

  if (upper == "STATUS?" || upper == "STATUS") {
    int raw = analogRead(moisturePin);
    int pct = moisturePercentFromRaw(raw);
    Serial.print("STATUS: raw="); Serial.print(raw);
    Serial.print(" percent="); Serial.print(pct);
    Serial.print(" thresh="); Serial.println(DRY_THRESHOLD_PERCENT);
    return;
  }

  // Optional runtime settings
  // SET:THRESHOLD:NN (0-100)
  if (upper.startsWith("SET:THRESHOLD:")) {
    int sep = cmd.lastIndexOf(':');
    int val = cmd.substring(sep + 1).toInt();
    val = constrain(val, 0, 100);
    DRY_THRESHOLD_PERCENT = val;
    Serial.print("OK:THRESHOLD=");
    Serial.println(DRY_THRESHOLD_PERCENT);
    return;
  }

  // SET:WATER_MS:NNNN
  if (upper.startsWith("SET:WATER_MS:")) {
    int sep = cmd.lastIndexOf(':');
    unsigned long val = cmd.substring(sep + 1).toInt();
    waterDurationMs = max<unsigned long>(val, 0);
    Serial.print("OK:WATER_MS=");
    Serial.println(waterDurationMs);
    return;
  }

  // SET:FERT_MS:NNNN
  if (upper.startsWith("SET:FERT_MS:")) {
    int sep = cmd.lastIndexOf(':');
    unsigned long val = cmd.substring(sep + 1).toInt();
    fertilizeDurationMs = max<unsigned long>(val, 0);
    Serial.print("OK:FERT_MS=");
    Serial.println(fertilizeDurationMs);
    return;
  }

  if (upper == "GET:CONFIG") {
    Serial.print("CONFIG:THRESH="); Serial.print(DRY_THRESHOLD_PERCENT);
    Serial.print(" WATER_MS="); Serial.print(waterDurationMs);
    Serial.print(" FERT_MS="); Serial.println(fertilizeDurationMs);
    return;
  }

  Serial.println("ERR:UNKNOWN_CMD");
}

void setup() {
  Serial.begin(9600);

  pinMode(moisturePin, INPUT);
  pinMode(waterPumpPin, OUTPUT);
  pinMode(fertilizerPin, OUTPUT);
  pumpsOff();

  // LCD init
  lcd.begin(16, 2);
  lcd.backlight();
  lcdPrintLine(0, 0, "Soil Moisture:");
  lcdPrintLine(0, 1, "Value: ---%   ");

  // Blink to show ready
  for (int i = 0; i < 2; i++) {
    digitalWrite(waterPumpPin, LOW); delay(120);
    digitalWrite(waterPumpPin, HIGH); delay(120);
  }

  Serial.println("READY");
}

void loop() {
  // Read moisture
  int raw = analogRead(moisturePin);
  int pct = moisturePercentFromRaw(raw);

  // Debug
  Serial.print("Raw: "); Serial.print(raw);
  Serial.print(" => "); Serial.print(pct); Serial.println('%');

  // LCD percentage
  lcdPrintLine(0, 1, String("Value: ") + pct + "%");

  // DRY/WET only (no automatic pump action)
  if (pct <= DRY_THRESHOLD_PERCENT) {
    lcd.setCursor(10, 1); lcd.print(" Dry  ");
  } else {
    lcd.setCursor(10, 1); lcd.print(" Wet  ");
  }

  // Handle incoming commands
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.length() > 0) handleCommand(cmd);
  }

  delay(800);
}