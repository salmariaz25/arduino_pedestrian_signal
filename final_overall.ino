#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

#define SS_PIN 10
#define RST_PIN 9
#define SERVO_PIN 3
#define ULTRASONIC_TRIGGER 6
#define ULTRASONIC_ECHO 7
#include <Wire.h>  
#include <LiquidCrystal_I2C.h>  

LiquidCrystal_I2C lcd(0x27, 16, 2);  // select LCD at i2c address 0x27
String data;

MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo myServo;
int red = 5; 
int green = 4;
int buzzer = 2;
int carred = A0; 
int cargreen = 8;
int caryellow = A1;
int pos = 0;
bool rfidActive = false;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  myServo.attach(SERVO_PIN);
  Serial.println("RFID and Ultrasonic Servo Control Ready");
  myServo.write(0);
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(carred, OUTPUT);
  pinMode(cargreen, OUTPUT);
  pinMode(caryellow, OUTPUT);
  digitalWrite(red, HIGH);
  digitalWrite(cargreen, HIGH);
  digitalWrite(caryellow, LOW);
  digitalWrite(carred, LOW);
  lcd.init();  // initializing LCD
  lcd.backlight();  // turn on the backlight
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Stop!");
  lcd.setCursor(0, 1);
  lcd.print("Do not cross!");
}

bool checkRFIDCard() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return false;
  }
  
  if (!mfrc522.PICC_ReadCardSerial()) {
    return false;
  }
  
  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  
  Serial.println("Scanned UID: " + content);
  
  if (content.substring(1) == "53 49 BE 0E") {
    return true;
  }
  
  return false;
}

long readUltrasonicDistance(int triggerPin, int echoPin) {
  pinMode(triggerPin, OUTPUT);
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  pinMode(echoPin, INPUT);
  return pulseIn(echoPin, HIGH);
}

void loop() {
  digitalWrite(caryellow, LOW);
  digitalWrite(carred, LOW);
  // RFID Priority Logic
  if (checkRFIDCard() || rfidActive) {
    rfidActive = true;
    for (int i = 0; i < 3; i++) {
      lcd.clear();
      lcd.print("Please wait...");
      digitalWrite(cargreen, HIGH);
      delay(500);
      digitalWrite(cargreen, LOW);
      delay(500);
      }
    digitalWrite(caryellow, HIGH);
    delay(1000);
    digitalWrite(caryellow, LOW);
    delay(500);
    digitalWrite(carred, HIGH);
        
    // Move servo to 120 degrees
    for (pos = 0; pos <= 120; pos += 1) { 
      digitalWrite(red, LOW);
      digitalWrite(green, HIGH);
      tone(buzzer, 1000, 100);
      myServo.write(pos);             
      delay(15);                       
    }
    
    // Wait for 15 seconds for RFID
    unsigned long servoStartTime = millis();
    while (millis() - servoStartTime < 15000) {
      lcd.clear();
      lcd.print("You can cross :)");
      // Only RFID can reset timer
      if (checkRFIDCard()) {
        servoStartTime = millis();
      }
      if (millis() - servoStartTime >= 10000) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Almost time to");
        lcd.setCursor(0, 1);
        lcd.print("stop crossing :(");
        for (int i = 0; i < 10; i++) {
          digitalWrite(red, LOW);
          tone(buzzer, 1000, 100);
          digitalWrite(green, HIGH);
          delay(250);
          digitalWrite(green, LOW);
          delay(250);
      }
    }    
    delay(100);
  }  
    // Return servo to 0 degrees
    for (pos = 120; pos >= 0; pos -= 1) {
      myServo.write(pos);
      digitalWrite(green, LOW);
      digitalWrite(red, HIGH);
      delay(15);                                     
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Stop!");
    lcd.setCursor(0, 1);
    lcd.print("Do not cross!");
    digitalWrite(cargreen, HIGH);
    rfidActive = false;
  }

  // Ultrasonic Sensor Logic (only if RFID is not active)
  if (!rfidActive) {
    long cm = 0.01723 * readUltrasonicDistance(ULTRASONIC_TRIGGER, ULTRASONIC_ECHO);
    
    if (cm < 5) {
      Serial.print(cm);
      Serial.println("cm - Ultrasonic detection");
      lcd.clear();
      lcd.print("Please wait...");
      digitalWrite(caryellow, LOW);
      digitalWrite(carred, LOW);
      for (int i = 0; i < 7; i++) {
      digitalWrite(cargreen, HIGH);
      delay(500);
      digitalWrite(cargreen, LOW);
      delay(500);
      }
      digitalWrite(caryellow, HIGH);
      delay(2500);
      digitalWrite(caryellow, LOW);
      delay(500);
      digitalWrite(carred, HIGH);
      lcd.clear();
      lcd.print("You can cross :)");
      
      // Move servo to 120 degrees
      for (pos = 0; pos <= 120; pos += 1) { 
        digitalWrite(red, LOW);
        digitalWrite(green, HIGH);
        tone(buzzer, 1000, 100);
        myServo.write(pos);             
        delay(15);                       
      }
      
      // Wait for 10 seconds for Ultrasonic (timer cannot be reset)
      unsigned long servoStartTime = millis();
      while (millis() - servoStartTime < 10000) {

        if (checkRFIDCard()) {
        servoStartTime = millis();
      }
        if (millis() - servoStartTime >= 5000) {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Almost time to");
          lcd.setCursor(0, 1);
          lcd.print("stop crossing :(");
          for (int i = 0; i < 10; i++) {
            digitalWrite(red, LOW);
            tone(buzzer, 1000, 100);
            digitalWrite(green, HIGH);
            delay(250);
            digitalWrite(green, LOW);
            delay(250);
        }
      }
        delay(100);
    }
      
      // Return servo to 0 degrees
      for (pos = 120; pos >= 0; pos -= 1) { 
        digitalWrite(green, LOW);
        myServo.write(pos);
        digitalWrite(red, HIGH);
        delay(15);                                     
      }
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Stop!");
      lcd.setCursor(0, 1);
      lcd.print("Do not cross!");
      digitalWrite(cargreen, HIGH);
    }
  }
}

