// Adafruit Motor shield library
// copyright Adafruit Industries LLC, 2009
// this code is public domain, enjoy!
//RFID Module define
#include <SPI.h>
#include <MFRC522.h>
#include <AFMotor.h>
#include <Servo.h>

//RFID
#define SS_PIN 49
#define RST_PIN 48
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

// Connect a stepper motor with 48 steps per revolution (7.5 degree)
// to motor port #2 (M3 and M4)
AF_Stepper motor1(200, 1);    //Stepper 1
AF_Stepper motor2(200, 2);    //Stepper 2
AF_DCMotor motor(2);          //360 Servo define
Servo servo1;
Servo servo2;

//Define pin of relay and button
int buttonStatu = 0;
#define relay1 22
#define relay2 23
#define button 24
bool parkStatu;

//Define pin of IR Sensor
int IRstatu = 0;
#define IR1 31
#define IR2 33
#define IR3 35
#define IR4 37

//Define pin of LED Red
#define ledRed1 30
#define ledRed2 32
#define ledRed3 34
#define ledRed4 36

//LCD define
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  //pinMode
  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);
  pinMode(IR3, INPUT);
  pinMode(IR4, INPUT);
  pinMode(button, INPUT);

  pinMode(ledRed1, OUTPUT);
  pinMode(ledRed2, OUTPUT);
  pinMode(ledRed3, OUTPUT);
  pinMode(ledRed4, OUTPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);

  //Define pin of servo motor
  servo1.attach(9);
  servo2.attach(10);

  //Define initial position for motor
  servo1.write(0);
  servo2.write(0);

  lcd.init();               // initialize the lcd
  lcd.init();
  lcd.backlight();
  //lcd.setCursor(0, 0);
  //lcd.print("Please, insert your card");

  Serial.begin(9600);   // Initiate a serial communication
  SPI.begin();          // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  Serial.println("Approximate your card to the reader...");
  Serial.println();

  motor1.setSpeed(30);  // 10 rpm
  motor2.setSpeed(30);
}

void loop() {
  IRsensor(IR1, ledRed1);
  IRsensor(IR2, ledRed2);
  IRsensor(IR3, ledRed3);
  IRsensor(IR4, ledRed4);

  //Lcd print
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Please, insert your card");

  //parkNumberIn(cardId, ir, microStep1, microStep2, stepDelay, servoDelay)
  parkNumberIn("00 4C 3C 1A", IRsensor(IR1, ledRed1), 400, 200, 500, 3000);
  parkNumberIn("E2 E9 5D 19", IRsensor(IR2, ledRed2), 200, 400, 500, 3000);
  parkNumberIn("00 7C 4C 1A", IRsensor(IR3, ledRed3), 200, 200, 500, 3000);
  parkNumberIn("01 EA 15 26", IRsensor(IR4, ledRed4), 400, 400, 500, 3000);
}

bool IRsensor(int IrSensor, int ledRed) {
  IRstatu = digitalRead(IrSensor);
  if (IRstatu == LOW) {
    digitalWrite(ledRed, HIGH);
    return LOW;
  }
  else {
    digitalWrite(ledRed, LOW);
    return HIGH;
  }
}

void buttonDelay() {
  while (true) {
    buttonStatu = digitalRead(button);
    if (buttonStatu == LOW) {
      break;
    }
    delay(1000);
    Serial.println("Puase");
  }
  Serial.println("Escape");
}

void servoBackward(int activeTime) {
  servo1.write(0);
  servo2.write(0);
  digitalWrite(relay1, HIGH);
  delay(activeTime);
  digitalWrite(relay1, LOW);
}

void servoFoward(int activeTime) {
  servo1.write(180);
  servo2.write(180);
  digitalWrite(relay1, HIGH);
  delay(activeTime);
  digitalWrite(relay1, LOW);
}

void stepperForward(int motorStep1, int motorStep2, int delayTime) {
  motor1.step(motorStep1, FORWARD, DOUBLE);
  delay(delayTime);
  motor2.step(motorStep2, FORWARD, DOUBLE);
  delay(delayTime);
}

void stepperBackward(int motorStep1, int motorStep2, int delayTime) {
  motor1.step(motorStep1, BACKWARD, DOUBLE);
  delay(delayTime);
  motor2.step(motorStep2, BACKWARD, DOUBLE);
  delay(delayTime);
}

void parkNumberIn(String cardId, bool ir, int microStep1, int microStep2, int stepDelay, int servoDelay) {
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }
  //Show UID on serial monitor
  Serial.print("UID tag :");
  String content = "";
  byte letter;

  //print
  lcd.setCursor(0, 0);
  lcd.print("Please, insert your card");

  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();

  if (content.substring(1) == cardId) //"00 4C 3C 1A" change here the UID of the card/cards that you want to give access
  {
    //lcd print
    Serial.println("Authorized access");
    lcd.setCursor(0, 0);
    lcd.print("Authorized access");
    Serial.println();
    //parkStatu = IRsensor(IR1, ledRed1);
    parkStatu = ir;
    if (parkStatu == LOW) {
      lcd.setCursor(0, 1);
      lcd.print("Car Out");
      putCarIn(microStep1, microStep2, stepDelay, servoDelay);
    }
    else {
      lcd.setCursor(0, 1);
      lcd.print("Car In");
      putCarIn(microStep1, microStep2, stepDelay, servoDelay);
    }
    lcd.setCursor(0, 0);
    lcd.print("Authorized access");
  }

  else   {
    //lcd print
    Serial.println(" Access denied");
    lcd.setCursor(0, 0);
    lcd.print("Access denied");
  }
}

void putCarIn(int microStep1, int microStep2, int stepDelay, int servoDelay) {
  lcd.setCursor(0, 1);
  lcd.print("Put car");
  stepperForward(microStep1, microStep2, stepDelay);  // 400 microstep, half second delay time
  servoBackward(servoDelay);              // servo motor 3 seconds on
  stepperBackward(microStep1, microStep2, stepDelay);
  lcd.setCursor(0, 1);
  lcd.print("Pending");
  buttonDelay();
  lcd.setCursor(0, 1);
  lcd.print("Processing");
  stepperForward(microStep1, microStep2, stepDelay);
  servoFoward(servoDelay);
  stepperBackward(microStep1, microStep2, stepDelay);
}
