#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 13);

void setup() {
  lcd.begin(16, 2);
  pinMode(7, INPUT);    // D7 como entrada (no interfiere)
  pinMode(8, INPUT);    // D8 como entrada (no interfiere)
}

void loop() {
  int valor = analogRead(A0);
  
  lcd.setCursor(0, 0);
  lcd.print("A0: ");
  lcd.print(valor);
  lcd.print("    ");
  
  lcd.setCursor(0, 1);
  lcd.print("Carga por 10k");
  
  delay(100);
}