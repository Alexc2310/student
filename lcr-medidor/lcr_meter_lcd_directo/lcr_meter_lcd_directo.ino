#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 13);

#define PIN_ANALOG      A0
#define PIN_CHARGE      7
#define PIN_DISCHARGE   8
#define PIN_BUTTON      2

const float R_REF = 10000.0;
const float V_REF = 5.0;
const int THRESHOLD_63 = 648;

enum Mode { MODE_R, MODE_C, MODE_L };
Mode currentMode = MODE_R;

unsigned long lastButtonPress = 0;

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  
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
  lcd.setCursor(0, 1);
  lcd.print("Descargando...  ");
  
  // Descargar
  pinMode(PIN_CHARGE, INPUT);
  pinMode(PIN_DISCHARGE, OUTPUT);
  digitalWrite(PIN_DISCHARGE, LOW);
  
  unsigned long dischargeStart = millis();
  while (analogRead(PIN_ANALOG) > 10) {
    if (millis() - dischargeStart > 10000) {
      lcd.setCursor(0, 1);
      lcd.print("C: Descarga lent");
      return;
    }
  }
  delay(100);
  
  lcd.setCursor(0, 1);
  lcd.print("Cargando...     ");
  
  // Cargar - D7 como INPUT para que cargue solo por el 10k
  pinMode(PIN_DISCHARGE, INPUT);
  pinMode(PIN_CHARGE, INPUT);
  
  unsigned long startTime = millis();
  
  while (analogRead(PIN_ANALOG) < THRESHOLD_63) {
    if (millis() - startTime > 30000) {
      lcd.setCursor(0, 1);
      lcd.print("C: Timeout      ");
      return;
    }
  }
  
  unsigned long elapsedTime = millis() - startTime;
  
  // Calcular: C = τ / R (tiempo en segundos)
  float capacitance = (elapsedTime / 1000.0) / R_REF;
  
  String unit;
  if (capacitance >= 1e-3) {
    capacitance *= 1000.0;
    unit = " mF";
  } else if (capacitance >= 1e-6) {
    capacitance *= 1000000.0;
    unit = " uF";
  } else {
    capacitance *= 1000000000.0;
    unit = " nF";
  }
  
  lcd.setCursor(0, 1);
  lcd.print("C: ");
  lcd.print(capacitance, 1);
  lcd.print(unit);
  lcd.print("    ");
}

void measureInductance() {
  lcd.setCursor(0, 1);
  lcd.print("L: No disponible");
}