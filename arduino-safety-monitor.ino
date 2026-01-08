/*
  Arduino-Safety-Monitor

  Behavior:
  - AIR >= 90        -> Yellow LED flashes + buzzer beeps
  - DIST < 30cm      -> Yellow LED flashes + buzzer beeps
  - BOTH triggered   -> Red LED flashes + long buzz

  OLED:
  - Top-right STATUS BOX (check / warning triangle / emergency !)
  - Stick figure (moved higher) with BOTH hands + visible thumbs:
      SAFE:   smiling face + BOTH thumbs UP (small bob animation)
      WARNING: frown face + BOTH thumbs DOWN (small bob animation)
      EMERGENCY: angry face + both hands "up" (panic) + visible thumbs (big)
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ---------------- PINS ----------------
#define DHTPIN 5
#define DHTTYPE DHT11

const int LED_R = 10;
const int LED_G = 11;
const int LED_Y = 12;

const int BUZZER = 6;

const int TRIG_PIN = 8;
const int ECHO_PIN = 7;

const int MQ_PIN = A0;

// ---------------- THRESHOLDS ----------------
const int AIR_WARN_TH  = 90;
const int PROX_WARN_CM = 30;

// ---------------- OLED ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHT dht(DHTPIN, DHTTYPE);

// ---------------- STATES ----------------
enum State { SAFE, WARN_AIR, WARN_PROX, EMERGENCY };
State state = SAFE;

// ---------------- TIMING ----------------
unsigned long lastBeepMs  = 0;
bool beepOn = false;

unsigned long lastFlashMs = 0;
bool flashOn = false;

unsigned long lastAnimMs  = 0;
bool animFlip = false;

// ---------------- HELPERS ----------------
long readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000UL);
  if (duration == 0) return -1;
  return (long)(duration * 0.0343f / 2.0f);
}

bool readDHT(float &temp, float &hum) {
  for (int i = 0; i < 3; i++) {
    hum  = dht.readHumidity();
    temp = dht.readTemperature();
    if (!isnan(hum) && !isnan(temp)) return true;
    delay(250);
  }
  return false;
}

void updateFlash(unsigned long intervalMs) {
  if (millis() - lastFlashMs >= intervalMs) {
    flashOn = !flashOn;
    lastFlashMs = millis();
  }
}

void updateAnim(unsigned long intervalMs) {
  if (millis() - lastAnimMs >= intervalMs) {
    animFlip = !animFlip;
    lastAnimMs = millis();
  }
}

void buzzerOff() {
  digitalWrite(BUZZER, LOW);
  beepOn = false;
}

void buzzerPattern(unsigned long onMs, unsigned long offMs) {
  unsigned long waitMs = beepOn ? onMs : offMs;
  if (millis() - lastBeepMs >= waitMs) {
    beepOn = !beepOn;
    digitalWrite(BUZZER, beepOn ? HIGH : LOW);
    lastBeepMs = millis();
  }
}

// ---------------- STATUS BOX (TOP-RIGHT) ----------------
void drawStatusBox() {
  int x = 104, y = 0;
  display.drawRect(x, y, 24, 16, SSD1306_WHITE);

  if (state == SAFE) {
    display.drawLine(x + 5, y + 9,  x + 9,  y + 13, SSD1306_WHITE);
    display.drawLine(x + 9, y + 13, x + 18, y + 4,  SSD1306_WHITE);
  }
  else if (state == WARN_AIR || state == WARN_PROX) {
    display.drawTriangle(x + 12, y + 3, x + 4, y + 14, x + 20, y + 14, SSD1306_WHITE);
    if (animFlip) {
      display.drawLine(x + 12, y + 6, x + 12, y + 10, SSD1306_WHITE);
      display.drawPixel(x + 12, y + 12, SSD1306_WHITE);
    }
  }
  else {
    if (animFlip) {
      display.fillRect(x + 11, y + 3, 2, 9, SSD1306_WHITE);
      display.fillRect(x + 11, y + 13, 2, 2, SSD1306_WHITE);
    }
  }
}

// ---------------- CHARACTER (MOVED HIGHER + FACE EMOTIONS + BOTH THUMBS) ----------------
void drawCharacter() {
  // moved higher so it doesn't touch bottom status text too much
  int baseX = 114;
  int baseY = 46 + (animFlip ? 1 : 0); // gentle bob

  // head
  int headX = baseX;
  int headY = baseY - 12;
  display.drawCircle(headX, headY, 3, SSD1306_WHITE);

  // eyes
  display.drawPixel(headX - 1, headY - 1, SSD1306_WHITE);
  display.drawPixel(headX + 1, headY - 1, SSD1306_WHITE);

  // mouth (smile/frown/angry)
  if (state == SAFE) {
    // smile
    display.drawPixel(headX - 1, headY + 1, SSD1306_WHITE);
    display.drawPixel(headX,     headY + 2, SSD1306_WHITE);
    display.drawPixel(headX + 1, headY + 1, SSD1306_WHITE);
  } else if (state == WARN_AIR || state == WARN_PROX) {
    // frown
    display.drawPixel(headX - 1, headY + 2, SSD1306_WHITE);
    display.drawPixel(headX,     headY + 1, SSD1306_WHITE);
    display.drawPixel(headX + 1, headY + 2, SSD1306_WHITE);
  } else {
    // angry mouth (flat) + eyebrows
    display.drawLine(headX - 1, headY + 2, headX + 1, headY + 2, SSD1306_WHITE);
    display.drawLine(headX - 2, headY - 2, headX - 1, headY - 3, SSD1306_WHITE);
    display.drawLine(headX + 2, headY - 2, headX + 1, headY - 3, SSD1306_WHITE);
  }

  // body
  display.drawLine(baseX, baseY - 8, baseX, baseY + 5, SSD1306_WHITE);

  // legs
  display.drawLine(baseX, baseY + 5, baseX - 4, baseY + 11, SSD1306_WHITE);
  display.drawLine(baseX, baseY + 5, baseX + 4, baseY + 11, SSD1306_WHITE);

  // arms + thumbs
  if (state == SAFE) {
    // BOTH thumbs UP
    // left arm up
    display.drawLine(baseX, baseY - 4, baseX - 8, baseY - 9, SSD1306_WHITE);
    display.drawLine(baseX - 8, baseY - 9, baseX - 8, baseY - 12, SSD1306_WHITE); // thumb up
    // right arm up
    display.drawLine(baseX, baseY - 4, baseX + 8, baseY - 9, SSD1306_WHITE);
    display.drawLine(baseX + 8, baseY - 9, baseX + 8, baseY - 12, SSD1306_WHITE); // thumb up
  }
  else if (state == WARN_AIR || state == WARN_PROX) {
    // BOTH thumbs DOWN
    // left arm down
    display.drawLine(baseX, baseY - 4, baseX - 8, baseY + 2, SSD1306_WHITE);
    display.drawLine(baseX - 8, baseY + 2, baseX - 8, baseY + 5, SSD1306_WHITE); // thumb down
    // right arm down
    display.drawLine(baseX, baseY - 4, baseX + 8, baseY + 2, SSD1306_WHITE);
    display.drawLine(baseX + 8, baseY + 2, baseX + 8, baseY + 5, SSD1306_WHITE); // thumb down
  }
  else {
    // EMERGENCY: both hands up waving (alternating) + big thumbs visible
    if (animFlip) {
      // arms high
      display.drawLine(baseX, baseY - 4, baseX - 9, baseY - 12, SSD1306_WHITE);
      display.drawLine(baseX, baseY - 4, baseX + 9, baseY - 12, SSD1306_WHITE);
    } else {
      // arms out
      display.drawLine(baseX, baseY - 4, baseX - 10, baseY - 6, SSD1306_WHITE);
      display.drawLine(baseX, baseY - 4, baseX + 10, baseY - 6, SSD1306_WHITE);
    }
    // bigger thumbs (little blocks)
    display.fillRect(baseX - 11, baseY - 13, 2, 2, SSD1306_WHITE);
    display.fillRect(baseX + 10, baseY - 13, 2, 2, SSD1306_WHITE);
  }
}

// ---------------- OLED ----------------
void drawOLED(float t, float h, int air, long dist, bool dhtOk) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  drawStatusBox();
  drawCharacter();

  display.setCursor(0, 0);
  if (dhtOk) {
    display.print("T:"); display.print(t, 1); display.print("C ");
    display.print("H:"); display.print(h, 0); display.print("%");
  } else {
    display.print("DHT ERROR");
  }

  display.setCursor(0, 16);
  display.print("Air: "); display.print(air);

  display.setCursor(0, 32);
  display.print("Dist: ");
  if (dist < 0) display.print("N/A");
  else { display.print(dist); display.print("cm"); }

  display.setCursor(0, 56);
  if (state == SAFE) display.print("[ SAFE ]");
  else if (state == WARN_AIR) display.print("[ WARNING ] AIR");
  else if (state == WARN_PROX) display.print("[ WARNING ] OBJECT");
  else display.print("[!! EMERGENCY !!]");

  display.display();
}

// ---------------- OLED INIT ----------------
bool initOLED() {
  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) return true;
  delay(50);
  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) return true;
  return false;
}

// ---------------- SETUP ----------------
void setup() {
  pinMode(LED_G, OUTPUT);
  pinMode(LED_Y, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  dht.begin();
  Wire.begin();

  if (!initOLED()) {
    while (true) {
      digitalWrite(LED_R, HIGH); delay(150);
      digitalWrite(LED_R, LOW);  delay(150);
    }
  }

  buzzerOff();
}

// ---------------- LOOP ----------------
void loop() {
  updateAnim(180);

  float hum = -1, temp = -1;
  bool dhtOk = readDHT(temp, hum);

  int air = analogRead(MQ_PIN);
  long dist = readDistanceCM();

  bool airWarn  = (air >= AIR_WARN_TH);
  bool proxWarn = (dist >= 0 && dist < PROX_WARN_CM);

  if (airWarn && proxWarn) state = EMERGENCY;
  else if (airWarn)        state = WARN_AIR;
  else if (proxWarn)       state = WARN_PROX;
  else                     state = SAFE;

  switch (state) {
    case SAFE:
      digitalWrite(LED_G, HIGH);
      digitalWrite(LED_Y, LOW);
      digitalWrite(LED_R, LOW);
      buzzerOff();
      break;

    case WARN_AIR:
    case WARN_PROX:
      updateFlash(250);
      digitalWrite(LED_G, LOW);
      digitalWrite(LED_R, LOW);
      digitalWrite(LED_Y, flashOn ? HIGH : LOW);
      buzzerPattern(120, 250);
      break;

    case EMERGENCY:
      updateFlash(180);
      digitalWrite(LED_G, LOW);
      digitalWrite(LED_Y, LOW);
      digitalWrite(LED_R, flashOn ? HIGH : LOW);
      buzzerPattern(800, 120);
      break;
  }

  drawOLED(temp, hum, air, dist, dhtOk);
  delay(120);
}
