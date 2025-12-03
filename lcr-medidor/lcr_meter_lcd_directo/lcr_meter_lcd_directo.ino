/*
 * MEDIDOR LCR CON ARDUINO
 * ========================
 * Versión para LCD 1602A (sin módulo I2C)
 * Botón con INPUT_PULLUP (solo conectar D2 a GND para cambiar modo)
 */

#include <LiquidCrystal.h>

// Pines LCD: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 5, 4, 3, 13);

// Pines del circuito
#define PIN_ANALOG      A0
#define PIN_CHARGE      7
#define PIN_DISCHARGE   8
#define PIN_BUTTON      2

// Resistencias de referencia
const float R_REF = 10000.0;  // 10kΩ
const float V_REF = 5.0;
const int THRESHOLD_63 = 648;

// Modos
enum Mode { MODE_R, MODE_C, MODE_L };
Mode currentMode = MODE_R;

unsigned long lastButtonPress = 0;

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  
  // Botón con resistencia pull-up interna
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  
  pinMode(PIN_CHARGE, OUTPUT);
  pinMode(PIN_DISCHARGE, OUTPUT);
  
  digitalWrite(PIN_CHARGE, LOW);
  digitalWrite(PIN_DISCHARGE, LOW);
  
  lcd.print("  MEDIDOR LCR");
  lcd.setCursor(0, 1);
  lcd.print("  Iniciando...");
  delay(1500);
  
  showMode();
}

void loop() {
  // Botón activo cuando se conecta a GND (lee LOW)
  if (digitalRead(PIN_BUTTON) == LOW) {
    if (millis() - lastButtonPress > 300) {
      lastButtonPress = millis();
      changeMode();
    }
  }
  
  switch (currentMode) {
    case MODE_R: measureResistance(); break;
    case MODE_C: measureCapacitance(); break;
    case MODE_L: measureInductance(); break;
  }
  
  delay(500);
}

void changeMode() {
  currentMode = (Mode)((currentMode + 1) % 3);
  showMode();
}

void showMode() {
  lcd.clear();
  lcd.setCursor(0, 0);
  
  switch (currentMode) {
    case MODE_R: lcd.print("Modo: RESISTENC"); break;
    case MODE_C: lcd.print("Modo: CAPACITAN"); break;
    case MODE_L: lcd.print("Modo: INDUCTANC"); break;
  }
  
  lcd.setCursor(0, 1);
  lcd.print("Midiendo...");
  delay(300);
}

void measureResistance() {
  pinMode(PIN_CHARGE, INPUT);
  pinMode(PIN_DISCHARGE, INPUT);
  
  int rawADC = analogRead(PIN_ANALOG);
  float voltage = (rawADC / 1023.0) * V_REF;
  
  lcd.setCursor(0, 1);
  
  if (rawADC < 10) {
    lcd.print("R: Sin conexion ");
    return;
  }
  
  if (rawADC > 1013) {
    lcd.print("R: Corto/0 Ohm  ");
    return;
  }
  
  float resistance = R_REF * (voltage / (V_REF - voltage));
  
  String unit;
  if (resistance >= 1000000) {
    resistance /= 1000000.0;
    unit = " MOhm";
  } else if (resistance >= 1000) {
    resistance /= 1000.0;
    unit = " kOhm";
  } else {
    unit = " Ohm";
  }
  
  lcd.print("R: ");
  lcd.print(resistance, 2);
  lcd.print(unit);
  lcd.print("    ");
}

void measureCapacitance() {
  // Descargar
  pinMode(PIN_CHARGE, INPUT);
  pinMode(PIN_DISCHARGE, OUTPUT);
  digitalWrite(PIN_DISCHARGE, LOW);
  
  while (analogRead(PIN_ANALOG) > 0) {}
  delay(50);
  
  // Cargar
  pinMode(PIN_DISCHARGE, INPUT);
  pinMode(PIN_CHARGE, OUTPUT);
  
  unsigned long startTime = micros();
  digitalWrite(PIN_CHARGE, HIGH);
  
  while (analogRead(PIN_ANALOG) < THRESHOLD_63) {
    if (micros() - startTime > 3000000) {
      lcd.setCursor(0, 1);
      lcd.print("C: Timeout      ");
      digitalWrite(PIN_CHARGE, LOW);
      return;
    }
  }
  
  unsigned long elapsedTime = micros() - startTime;
  digitalWrite(PIN_CHARGE, LOW);
  
  float capacitance = (elapsedTime / 1000000.0) / R_REF;
  
  String unit;
  if (capacitance >= 1e-3) {
    capacitance *= 1000.0;
    unit = " mF";
  } else if (capacitance >= 1e-6) {
    capacitance *= 1000000.0;
    unit = " uF";
  } else if (capacitance >= 1e-9) {
    capacitance *= 1000000000.0;
    unit = " nF";
  } else {
    capacitance *= 1000000000000.0;
    unit = " pF";
  }
  
  lcd.setCursor(0, 1);
  lcd.print("C: ");
  lcd.print(capacitance, 2);
  lcd.print(unit);
  lcd.print("    ");
  
  // Descargar
  pinMode(PIN_DISCHARGE, OUTPUT);
  digitalWrite(PIN_DISCHARGE, LOW);
  delay(50);
}

void measureInductance() {
  lcd.setCursor(0, 1);
  lcd.print("L: No disponible");
}