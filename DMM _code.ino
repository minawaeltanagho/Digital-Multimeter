#include <LiquidCrystal.h>   // Include Arduino LCD library

// LCD module connections (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

// Pin configurations and constants for voltmeter
const int analogPinVoltage = A2;
const float referenceVoltage = 5.0;
const float scaleFactor = 6.0;

// Pin configurations and constants for ammeter
const int analogPinCurrent = A0;
const float Rf = 320000; // Feedback resistance in ohms
const float Rin = 10000; // Input resistance in ohms
const float Gain = 1 + (Rf / Rin);
const float Rshunt = 17; // Shunt resistor value in ohms

// Pin configurations and constants for ohmmeter
#define CH0 12
#define CH1 11
#define CH2 10
#define CH3 9
#define CH4 8
const uint32_t res_table[5] = {100, 2000, 10000, 100000, 1000000};
byte ch_number;
uint32_t res;
char _buffer[11];

// Mode selection pins
const int modePin = A3; // Original mode pin
const int modePin2 = A4; // Additional mode pin
const int modePin3 = A5; // Another additional mode pin

void setup() {
  lcd.begin(16, 2);     // Set up the LCD's number of columns and rows
  Serial.begin(9600);   // Initialize serial communication

  // Ohmmeter pin setup
  pinMode(CH0, OUTPUT);
  pinMode(CH1, OUTPUT);
  pinMode(CH2, OUTPUT);
  pinMode(CH3, OUTPUT);
  pinMode(CH4, OUTPUT);
  ch_number = 4;
  ch_select(ch_number);

  // Mode pins setup
  pinMode(modePin, INPUT);
  pinMode(modePin2, INPUT);
  pinMode(modePin3, INPUT);
}

void loop() {
  // Read mode selection values
  bool mode2 = digitalRead(modePin2);
  bool mode3 = digitalRead(modePin3);

  if (!mode2 && !mode3) {
    voltmeterMode();
  } else if (mode2 && !mode3) {
    ammeterMode();
  } else if (!mode2 && mode3) {
    ohmmeterMode();
  } else {
    displayError();
  }
}

void voltmeterMode() {
  bool mode1 = digitalRead(modePin);
  float scaleFactor;
  if (mode1) {
  scaleFactor = 6.0;
  int sensorValue = analogRead(analogPinVoltage);
  float voltage = (sensorValue / 1023.0) * referenceVoltage * scaleFactor;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Voltage (0-30V): ");
  lcd.setCursor(2, 1);
  lcd.print("V= ");
  lcd.print(voltage, 3);
  lcd.print(" V");

  Serial.print("Measured Voltage (0-30V): ");
  Serial.println(voltage);

  delay(500);
  }else {
  scaleFactor = 1.0;
  int sensorValue = analogRead(analogPinVoltage);
  float voltage = (sensorValue / 1023.0) * referenceVoltage * scaleFactor;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Voltage (0-5V): ");
  lcd.setCursor(2, 1);
  lcd.print("V= ");
  lcd.print(voltage, 3);
  lcd.print(" V");

  Serial.print("Measured Voltage (0-5V): ");
  Serial.println(voltage);

  delay(500);
  }
}

void ammeterMode() {
  float omp_value = analogRead(analogPinCurrent);
  float Vshunt = (omp_value * (5.0 / 1023.0));
  float Vd = Vshunt / Gain;
  float current = (Vd / Rshunt) * 1000;

  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Current:");
  lcd.setCursor(2, 1);
  lcd.print("I=");
  lcd.print(current,3);
  lcd.print(" mA");

  Serial.print("Current Value: I = ");
  Serial.print(current, 3);
  Serial.println(" mA");

  delay(1000);
}

void ohmmeterMode() {
  uint16_t volt_image = analogRead(A1) + 1;

  if (volt_image >= 550 && ch_number < 4) {
    ch_number++;
    ch_select(ch_number);
    delay(50);
    return;
  }

  if (volt_image <= 90 && ch_number > 0) {
    ch_number--;
    ch_select(ch_number);
    delay(50);
    return;
  }

  if (volt_image < 900) {
    float value = (float)volt_image * res / (1023 - volt_image);
    if (value < 1000.0)
      sprintf(_buffer, "%03u.%1u Ohm", (uint16_t)value, (uint16_t)(value * 10) % 10);
    else if (value < 10000.0)
      sprintf(_buffer, "%1u.%03u kOhm", (uint16_t)(value / 1000), (uint16_t)value % 1000);
    else if (value < 100000.0)
      sprintf(_buffer, "%02u.%02u kOhm", (uint16_t)(value / 1000), (uint16_t)(value / 10) % 100);
    else if (value < 1000000.0)
      sprintf(_buffer, "%03u.%1u kOhm", (uint16_t)(value / 1000), (uint16_t)(value / 100) % 10);
    else
      sprintf(_buffer, "%1u.%03u MOhm", (uint16_t)(value / 1000000), (uint16_t)(value / 1000) % 1000);
  } else {
    sprintf(_buffer, "       ");
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Resistance:");
  lcd.setCursor(0, 4);
  lcd.print(_buffer);

  Serial.println(_buffer);

  delay(500);
}

void displayError() {
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Error: Invalid");
  lcd.setCursor(2, 1);
  lcd.print("Multiple modes");

  Serial.println("Error: Invalid Mode Selection");

  delay(1000);
}

void ch_select(byte n) {
  switch (n) {
    case 0:
      digitalWrite(CH0, LOW);
      digitalWrite(CH1, HIGH);
      digitalWrite(CH2, HIGH);
      digitalWrite(CH3, HIGH);
      digitalWrite(CH4, HIGH);
      break;
    case 1:
      digitalWrite(CH0, HIGH);
      digitalWrite(CH1, LOW);
      digitalWrite(CH2, HIGH);
      digitalWrite(CH3, HIGH);
      digitalWrite(CH4, HIGH);
      break;
    case 2:
      digitalWrite(CH0, HIGH);
      digitalWrite(CH1, HIGH);
      digitalWrite(CH2, LOW);
      digitalWrite(CH3, HIGH);
      digitalWrite(CH4, HIGH);
      break;
    case 3:
      digitalWrite(CH0, HIGH);
      digitalWrite(CH1, HIGH);
      digitalWrite(CH2, HIGH);
      digitalWrite(CH3, LOW);
      digitalWrite(CH4, HIGH);
      break;
    case 4:
      digitalWrite(CH0, HIGH);
      digitalWrite(CH1, HIGH);
      digitalWrite(CH2, HIGH);
      digitalWrite(CH3, HIGH);
      digitalWrite(CH4, LOW);
  }
  res = res_table[n];
}