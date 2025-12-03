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
    case MODE_L: lcd.print("Modo: INDUCTANC"); b