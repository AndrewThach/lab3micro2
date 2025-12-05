//Odey
#include <Wire.h> 
#include <RTClib.h>
#include <LiquidCrystal.h>
#include <TimerOne.h>   // Kevin

// LCD RS,E,D4,D5,D6,D7 //Kevin
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

//RTC Odey
RTC_DS1307 rtc;

//MotorL293D Andrew
const int EN = 6; // switch/speed controller
const int IN1 = 2;
const int IN2 = 3;

const int SOUND = A0; 
const int BUTTON = A1; 

//Time Kevin
volatile int h = 0, m = 0, s = 0;
volatile bool updateLCD = false;

//Direction Start and Speed Andrew
bool dirCW = true;
int speedStep = 0;

void p2(int x) { //Format for time on LCD Kevin
  if (x < 10) lcd.print('0');
  lcd.print(x);
}

void timerISR() { //timer Kevin
  if (++s == 60) { //resets 0 after 60
    s = 0; 
    if (++m == 60) { //resets 0 after 60
      m = 0; 
      if (++h == 24) h = 0; //resets 0 after 24
    } 
  }
  updateLCD = true; //bool for true
}


void driveMotor() { //motor Andrew 
  int pwm[] = {0, 120, 180, 255}; //rotates in speed
  analogWrite(EN, pwm[speedStep]); //analog pin write

  digitalWrite(IN1, dirCW ? HIGH : LOW); //writes in the serial monitor
  digitalWrite(IN2, dirCW ? LOW : HIGH);
}

//LCD Kevin
void showLCD() { //lcd display
  lcd.clear();
  lcd.setCursor(0, 0); //position
  p2(h); lcd.print(":"); p2(m); lcd.print(":"); p2(s); //prints on lcd

  lcd.setCursor(0, 1); //position
  lcd.print("Dir:"); lcd.print(dirCW ? "C " : "CC "); //prints on lcd
  lcd.print("Speed:"); lcd.print(speedStep);
}

//setup for the pins
void setup() {
  Serial.begin(9600); //baud rate

  lcd.begin(16, 2); //pins

  pinMode(EN, OUTPUT); //motor pins Andrew
  pinMode(IN1, OUTPUT); //dir control 
  pinMode(IN2, OUTPUT);

  pinMode(SOUND, INPUT); //sound pins Odey
  pinMode(BUTTON, INPUT_PULLUP); //

  rtc.begin(); //rtc Odey
  DateTime now = rtc.now(); //current time
  h = now.hour(); 
  m = now.minute(); 
  s = now.second();

  
  Timer1.initialize(1000000); //interrupt 1,000,000 microseconds = 1 second
  Timer1.attachInterrupt(timerISR); //when 1 second it runs tmerISR
}

void loop() { //main loop
  static int lastB = HIGH; //button
  int b = digitalRead(BUTTON); 
  if (lastB == HIGH && b == LOW) dirCW = !dirCW; //button cc and cw
  lastB = b;

  int level = analogRead(SOUND); //sound sensor
  if (level < 40) speedStep = 0; //low
  else if (level < 60) speedStep = 1; //medium
  else if (level < 80) speedStep = 2; //high
  else speedStep = 3; //more than 80 high

  driveMotor(); //motor function

  if (updateLCD) { //lcd
    updateLCD = false; //check the flag
    showLCD();
  }

  delay(20);
}
