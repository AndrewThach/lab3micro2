#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal.h>

// LCD RS,E,D4,D5,D6,D7
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// RTC
RTC_DS1307 rtc;

// Motor (L293D)
const int EN = 6;
const int IN1 = 2;
const int IN2 = 3;

// Inputs
const int SOUND = A0;
const int BUTTON = A1;

// Time (updated by interrupt)
volatile int h = 0, m = 0, s = 0;
volatile bool updateLCD = false;

// Direction + speed
bool dirCW = true;
int speedStep = 0;  // 0–3

// ---------- PRINT TWO DIGITS ----------
void p2(int x) {
  if (x < 10) lcd.print('0');
  lcd.print(x);
}

// ---------- TIMER1: 1 SEC ----------
ISR(TIMER1_COMPA_vect) {
  if (++s == 60) { s = 0; if (++m == 60) { m = 0; if (++h == 24) h = 0; } }
  updateLCD = true;
}

// ---------- TIMER SETUP ----------
void setupTimer() {
  noInterrupts();
  TCCR1A = 0; TCCR1B = 0; TCNT1 = 0;
  OCR1A = 15624;        // 1 second
  TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);
  interrupts();
}

// ---------- UPDATE MOTOR ----------
void driveMotor() {
  int pwm[] = {0, 120, 180, 255};
  analogWrite(EN, pwm[speedStep]);

  digitalWrite(IN1, dirCW ? HIGH : LOW);
  digitalWrite(IN2, dirCW ? LOW : HIGH);
}

// ---------- LCD ----------
void showLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  p2(h); lcd.print(":"); p2(m); lcd.print(":"); p2(s);

  lcd.setCursor(0, 1);
  lcd.print("Dir:"); lcd.print(dirCW ? "C " : "CC ");
  lcd.print("Speed:"); lcd.print(speedStep);
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(9600);

  lcd.begin(16, 2);

  pinMode(EN, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(SOUND, INPUT);
  pinMode(BUTTON, INPUT_PULLUP);

  rtc.begin();
  DateTime now = rtc.now();
  h = now.hour(); m = now.minute(); s = now.second();

  setupTimer();
}

// ---------- LOOP ----------
void loop() {

  // Button toggles direction
  static int lastB = HIGH;
  int b = digitalRead(BUTTON);
  if (lastB == HIGH && b == LOW) dirCW = !dirCW;
  lastB = b;

  // Sound → speed step
  int level = analogRead(SOUND);
  if (level < 40) speedStep = 0;
  else if (level < 60) speedStep = 1;
  else if (level < 80) speedStep = 2;
  else speedStep = 3;

  driveMotor();

  if (updateLCD) {
    updateLCD = false;
    showLCD();
  }

  delay(20);
}
