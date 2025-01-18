#include <Keypad.h>
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(11, 12);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

const byte ROWS = 4;
const byte COLS = 4;
char keypressed;

char keys[ROWS][COLS] = 
{
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
byte rowPins[ROWS] = { 9, 8, 7, 6 };
byte colPins[COLS] = { 5, 4, 3, 2 };
byte servoMotorPin = 10;
byte speakerPin = 13;
byte greenPin = 16;
byte redPin = 15;

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
Servo myservo;

const int fingerIDForPythonSignal[] = {1, 2, 3, 4}; 
const int PersonA = 1;
const int PersonB = 2;
const int PersonC = 3;
const int PersonD = 4;
const int PersonAPassword = 2205;
const int PersonBPassword = 506; 
const int PersonCPassword = 1612;
const int PersonDPassword = 3107;

bool signalReceived = false; // Flag to indicate if a signal has been received from Python
bool accessGranted = false; // Flag to indicate if access has been granted
bool fingerprintVerified = false; // Flag to indicate if the fingerprint has been verified
int receivedCode; // Variable to store the received code
String input = ""; // String to store keypad input
String in = ""; // String to display keypad input on LCD

void setup()
{
  Serial.begin(9600);
  finger.begin(57600);
  myservo.attach(servoMotorPin);
  pinMode(speakerPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  myservo.write(0);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("WELCOME");
  delay(1000);
  lcd.setCursor(0, 0);
  lcd.print("WAITING FOR SIGNAL"); // Display message indicating waiting for signal from Python
  lcd.setCursor(0, 1);
  lcd.print("TO START");
}

void loop() 
{
  if (!signalReceived) {
    while (!signalReceived) {
      if (Serial.available() > 0) {
        receivedCode = Serial.parseInt();
        if (receivedCode >= 1 && receivedCode <= 4) {
          signalReceived = true;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("SIGNAL RECEIVED");
          delay(1000);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("PLACE FINGER");
          delay(3000);
          break;
        }
      }
    }
  }

  if (signalReceived && !fingerprintVerified) {
    if (getFingerprint()) {
      bool matchFound = false;
      for (int i = 0; i < sizeof(fingerIDForPythonSignal) / sizeof(fingerIDForPythonSignal[0]); i++) {
        if (finger.fingerID == fingerIDForPythonSignal[i]) {
          matchFound = true;
          break;
        }
      }
      if (matchFound) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("FINGERPRINT");
        lcd.setCursor(0, 1);
        lcd.print("VERIFIED");
        digitalWrite(greenPin, HIGH);
        delay(2000);
        digitalWrite(greenPin, LOW);
        fingerprintVerified = true;
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ACCESS DENIED");
        digitalWrite(redPin, HIGH);
        tone(speakerPin, 1000);
        delay(3000);
        noTone(speakerPin);
        digitalWrite(redPin, LOW);
        resetCode();
      }
    }
  }

  if (fingerprintVerified && !accessGranted) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ENTER PASSWORD");
    while (keypressed != '#' && !accessGranted) 
    {
      keypressed = keypad.getKey();
      if (keypressed) 
      {
        input += keypressed;
        in += keypressed;
        lcd.setCursor(0, 1);
        lcd.print(in);
        if (input.length() == 4) {
          int enteredPassword = input.toInt();
          switch (receivedCode) {
            case PersonA:
              if (enteredPassword == PersonAPassword) {
                grantAccess();
              } else {
                denyAccess();
              }
              break;
            case PersonB:
              if (enteredPassword == PersonBPassword) {
                grantAccess();
              } else {
                denyAccess();
              }
              break;
            case PersonC:
              if (enteredPassword == PersonCPassword) {
                grantAccess();
              } else {
                denyAccess();
              }
              break;
            case PersonD:
              if (enteredPassword == PersonDPassword) {
                grantAccess();
              } else {
                denyAccess();
              }
              break;
          }
        }
      }
    }
  }
}

void grantAccess() {
  accessGranted = true;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ACCESS GRANTED");
  digitalWrite(greenPin, HIGH);
  delay(1000);
  myservo.write(70);
  delay(3000);
  myservo.write(0);
  digitalWrite(greenPin, LOW);
  resetCode();
}

void denyAccess() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WRONG PASSWORD");
  digitalWrite(redPin, HIGH);
  tone(speakerPin, 1000);
  delay(3000);
  noTone(speakerPin);
  digitalWrite(redPin, LOW);
  resetCode();
}

void resetCode() {
  input = "";
  in = "";
  keypressed = 0;
  signalReceived = false;
  accessGranted = false;
  fingerprintVerified = false;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WELCOME");
  delay(1000);
  lcd.setCursor(0, 0);
  lcd.print("WAITING FOR SIGNAL");
  lcd.setCursor(0, 1);
  lcd.print("TO START");
}

bool getFingerprint() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PLACE FINGER");
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) 
  {
    lcd.setCursor(0, 1);
    lcd.print("NO FINGERPRINT");
    delay(2000);
    return false;
  }
  p = finger.image2Tz();
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK && finger.confidence >= 85) 
  {
    return true;
  } 
  else 
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ACCESS DENIED");
    digitalWrite(redPin, HIGH);
    tone(speakerPin, 1000);
    delay(3000);
    noTone(speakerPin);
    digitalWrite(redPin, LOW);
    resetCode();
    return false;
  }
}
