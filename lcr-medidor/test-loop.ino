#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 13);
int contador = 0;

void setup() {
  lcd.begin(16, 2);
  pinMode(2, INPUT_PULLUP);
}

void loop() {
  lcd.setCursor(0, 0);
  lcd.print("Pin D2: ");
  
  if (digitalRead(2) == LOW) {
    lcd.print("GND ");
    contador++;
  } else {
    lcd.print("HIGH");
  }
  
  lcd.setCursor(0, 1);
  lcd.print("Cont: ");
  lcd.print(contador);
  lcd.print("   ");
  
  delay(200);
}
