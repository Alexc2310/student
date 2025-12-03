#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 13);

void setup() {
  lcd.begin(16, 2);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  digitalWrite(7, LOW);
  digitalWrite(8, LOW);
}

void loop() {
  int valor = analogRead(A0);
  
  lcd.setCursor(0, 0);
  lcd.print("A0: ");
  lcd.print(valor);
  lcd.print("    ");
  
  lcd.setCursor(0, 1);
  lcd.print("D7:OFF D8:LOW");
  
  delay(200);
}
