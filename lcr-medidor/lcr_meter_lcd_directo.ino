/*
 * MEDIDOR LCR CON ARDUINO
 * ========================
 * Versión para LCD 1602A (sin módulo I2C)
 * 
 * Proyecto escolar para medir:
 * - Resistencia (R)
 * - Capacitancia (C) 
 * - Inductancia (L)
 * 
 * Componentes necesarios:
 * - Arduino Uno/Nano
 * - LCD 1602A (conexión directa)
 * - Potenciómetro 10kΩ (contraste)
 * - Resistencias de referencia: 1kΩ, 10kΩ, 100kΩ
 * - Capacitor de referencia: 1µF
 * - Botón selector de modo
 * - Terminales de prueba
 */

#include <LiquidCrystal.h>

// ============== CONFIGURACIÓN LCD 1602A ==============
// Pines: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 5, 4, 3, 13);

// ============== CONFIGURACIÓN DE PINES ==============
#define PIN_ANALOG      A0    // Entrada analógica para medición de voltaje
#define PIN_CHARGE      7     // Pin para cargar capacitor
#define PIN_DISCHARGE   8     // Pin para descargar capacitor
#define PIN_PULSE       9     // Pin PWM para medición de inductancia
#define PIN_BUTTON      2     // Botón selector de modo

// ============== CONSTANTES DE CALIBRACIÓN ==============
// Resistencias de referencia (ajustar según los valores reales medidos)
const float R_REF_LOW   = 1000.0;    // 1kΩ para resistencias bajas
const float R_REF_MED   = 10000.0;   // 10kΩ para resistencias medias
const float R_REF_HIGH  = 100000.0;  // 100kΩ para resistencias altas

// Capacitor de referencia para medición de inductancia
const float C_REF = 1.0e-6;  // 1µF

// Voltaje de referencia del Arduino
const float V_REF = 5.0;

// Umbral para constante de tiempo (63.2% de Vmax)
const int THRESHOLD_63 = 648;  // 63.2% de 1023 (ADC de 10 bits)

// ============== VARIABLES GLOBALES ==============
enum Mode { MODE_R, MODE_C, MODE_L };
Mode currentMode = MODE_R;

unsigned long lastButtonPress = 0;
const unsigned long DEBOUNCE_TIME = 300;

// ============== SETUP ==============
void setup() {
  // Inicializar comunicación serial para debug
  Serial.begin(9600);
  
  // Inicializar LCD (16 columnas, 2 filas)
  lcd.begin(16, 2);
  lcd.clear();
  
  // Configurar pines
  pinMode(PIN_BUTTON, INPUT);
  pinMode(PIN_CHARGE, OUTPUT);
  pinMode(PIN_DISCHARGE, OUTPUT);
  pinMode(PIN_PULSE, OUTPUT);
  
  // Estado inicial
  digitalWrite(PIN_CHARGE, LOW);
  digitalWrite(PIN_DISCHARGE, LOW);
  
  // Mensaje de bienvenida
  lcd.setCursor(0, 0);
  lcd.print("  MEDIDOR LCR   ");
  lcd.setCursor(0, 1);
  lcd.print("  Iniciando...  ");
  delay(1500);
  
  showMode();
}

// ============== LOOP PRINCIPAL ==============
void loop() {
  // Verificar botón de modo
  if (digitalRead(PIN_BUTTON) == HIGH) {
    if (millis() - lastButtonPress > DEBOUNCE_TIME) {
      lastButtonPress = millis();
      changeMode();
    }
  }
  
  // Realizar medición según el modo
  switch (currentMode) {
    case MODE_R:
      measureResistance();
      break;
    case MODE_C:
      measureCapacitance();
      break;
    case MODE_L:
      measureInductance();
      break;
  }
  
  delay(500);  // Actualizar cada 500ms
}

// ============== CAMBIO DE MODO ==============
void changeMode() {
  currentMode = (Mode)((currentMode + 1) % 3);
  showMode();
}

void showMode() {
  lcd.clear();
  lcd.setCursor(0, 0);
  
  switch (currentMode) {
    case MODE_R:
      lcd.print("Modo: RESISTENC");
      break;
    case MODE_C:
      lcd.print("Modo: CAPACITAN");
      break;
    case MODE_L:
      lcd.print("Modo: INDUCTANC");
      break;
  }
  
  lcd.setCursor(0, 1);
  lcd.print("Midiendo...");
  delay(500);
}

// ============== MEDICIÓN DE RESISTENCIA ==============
/*
 * Principio: Divisor de voltaje
 * 
 *     5V ----[R_REF]----+----[Rx]---- GND
 *                       |
 *                      A0
 * 
 * Fórmula: Rx = R_REF * (Vout / (Vin - Vout))
 */
void measureResistance() {
  // Configurar pines para medición de resistencia
  pinMode(PIN_CHARGE, INPUT);
  pinMode(PIN_DISCHARGE, INPUT);
  
  // Leer voltaje
  int rawADC = analogRead(PIN_ANALOG);
  float voltage = (rawADC / 1023.0) * V_REF;
  
  // Calcular resistencia
  float resistance;
  String unit;
  
  if (rawADC < 10) {
    // Circuito abierto o resistencia muy alta
    lcd.setCursor(0, 1);
    lcd.print("R: Sin conexion ");
    return;
  }
  
  if (rawADC > 1013) {
    // Cortocircuito o resistencia muy baja
    lcd.setCursor(0, 1);
    lcd.print("R: Corto/0 Ohm  ");
    return;
  }
  
  // Seleccionar resistencia de referencia según el rango
  float refResistance;
  if (rawADC < 100) {
    refResistance = R_REF_HIGH;  // Resistencias altas
  } else if (rawADC < 500) {
    refResistance = R_REF_MED;   // Resistencias medias
  } else {
    refResistance = R_REF_LOW;   // Resistencias bajas
  }
  
  // Calcular Rx
  resistance = refResistance * (voltage / (V_REF - voltage));
  
  // Formatear salida con unidades apropiadas
  if (resistance >= 1000000) {
    resistance = resistance / 1000000.0;
    unit = " MOhm";
  } else if (resistance >= 1000) {
    resistance = resistance / 1000.0;
    unit = " kOhm";
  } else {
    unit = " Ohm";
  }
  
  // Mostrar resultado
  lcd.setCursor(0, 1);
  lcd.print("R: ");
  lcd.print(resistance, 2);
  lcd.print(unit);
  lcd.print("     ");  // Limpiar caracteres residuales
  
  // Debug serial
  Serial.print("Resistencia: ");
  Serial.print(resistance);
  Serial.println(unit);
}

// ============== MEDICIÓN DE CAPACITANCIA ==============
/*
 * Principio: Constante de tiempo RC
 * 
 * Se carga el capacitor a través de una resistencia conocida
 * y se mide el tiempo hasta alcanzar el 63.2% del voltaje máximo.
 * 
 * τ = R * C
 * C = τ / R
 */
void measureCapacitance() {
  // Descargar el capacitor completamente
  pinMode(PIN_CHARGE, INPUT);
  pinMode(PIN_DISCHARGE, OUTPUT);
  digitalWrite(PIN_DISCHARGE, LOW);
  
  // Esperar descarga completa
  while (analogRead(PIN_ANALOG) > 0) {
    // Esperar
  }
  delay(100);
  
  // Preparar para cargar
  pinMode(PIN_DISCHARGE, INPUT);
  pinMode(PIN_CHARGE, OUTPUT);
  
  // Iniciar carga y medir tiempo
  unsigned long startTime = micros();
  digitalWrite(PIN_CHARGE, HIGH);
  
  // Esperar hasta alcanzar 63.2% de Vmax
  while (analogRead(PIN_ANALOG) < THRESHOLD_63) {
    // Timeout de 5 segundos para capacitores grandes
    if (micros() - startTime > 5000000) {
      lcd.setCursor(0, 1);
      lcd.print("C: Timeout      ");
      digitalWrite(PIN_CHARGE, LOW);
      return;
    }
  }
  
  unsigned long elapsedTime = micros() - startTime;
  
  // Detener carga
  digitalWrite(PIN_CHARGE, LOW);
  
  // Calcular capacitancia: C = τ / R
  float tau = elapsedTime / 1000000.0;  // Convertir a segundos
  float capacitance = tau / R_REF_MED;  // Usar resistencia de 10k
  
  // Formatear salida
  String unit;
  if (capacitance >= 1e-3) {
    capacitance = capacitance * 1000.0;
    unit = " mF";
  } else if (capacitance >= 1e-6) {
    capacitance = capacitance * 1000000.0;
    unit = " uF";
  } else if (capacitance >= 1e-9) {
    capacitance = capacitance * 1000000000.0;
    unit = " nF";
  } else {
    capacitance = capacitance * 1000000000000.0;
    unit = " pF";
  }
  
  // Mostrar resultado
  lcd.setCursor(0, 1);
  lcd.print("C: ");
  lcd.print(capacitance, 2);
  lcd.print(unit);
  lcd.print("     ");
  
  // Debug serial
  Serial.print("Capacitancia: ");
  Serial.print(capacitance);
  Serial.println(unit);
  
  // Descargar el capacitor
  pinMode(PIN_DISCHARGE, OUTPUT);
  digitalWrite(PIN_DISCHARGE, LOW);
  delay(100);
}

// ============== MEDICIÓN DE INDUCTANCIA ==============
/*
 * Principio: Circuito oscilador LC
 * 
 * Se crea un circuito resonante LC con un capacitor conocido.
 * La frecuencia de resonancia es: f = 1 / (2π√(LC))
 * Despejando: L = 1 / (4π²f²C)
 * 
 * NOTA: Este método requiere una implementación más elaborada
 * con comparadores o usando el Timer del Arduino.
 * Esta es una versión simplificada.
 */
void measureInductance() {
  // Generar pulsos y medir respuesta
  unsigned long pulseCount = 0;
  unsigned long startTime = millis();
  
  // Contar pulsos durante 100ms
  while (millis() - startTime < 100) {
    // Generar pulso
    digitalWrite(PIN_PULSE, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_PULSE, LOW);
    delayMicroseconds(10);
    
    // Detectar cruce por cero (simplificado)
    if (analogRead(PIN_ANALOG) > 512) {
      pulseCount++;
    }
  }
  
  // Calcular frecuencia
  float frequency = pulseCount * 10.0;  // Hz (ajustado por período de medición)
  
  if (frequency < 10) {
    lcd.setCursor(0, 1);
    lcd.print("L: Sin conexion ");
    return;
  }
  
  // Calcular inductancia: L = 1 / (4π²f²C)
  float pi_sq = PI * PI;
  float inductance = 1.0 / (4.0 * pi_sq * frequency * frequency * C_REF);
  
  // Formatear salida
  String unit;
  if (inductance >= 1.0) {
    unit = " H";
  } else if (inductance >= 1e-3) {
    inductance = inductance * 1000.0;
    unit = " mH";
  } else {
    inductance = inductance * 1000000.0;
    unit = " uH";
  }
  
  // Mostrar resultado
  lcd.setCursor(0, 1);
  lcd.print("L: ");
  lcd.print(inductance, 2);
  lcd.print(unit);
  lcd.print("     ");
  
  // Debug serial
  Serial.print("Inductancia: ");
  Serial.print(inductance);
  Serial.println(unit);
}

// ============== FUNCIONES AUXILIARES ==============

// Función para auto-calibración (opcional)
void autoCalibrate() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calibrando...");
  
  // Medir offset del ADC
  int offset = 0;
  for (int i = 0; i < 100; i++) {
    offset += analogRead(PIN_ANALOG);
  }
  offset = offset / 100;
  
  lcd.setCursor(0, 1);
  lcd.print("Offset: ");
  lcd.print(offset);
  
  delay(2000);
}

// Función para mostrar información de debug
void showDebugInfo() {
  Serial.println("=== DEBUG INFO ===");
  Serial.print("Modo actual: ");
  Serial.println(currentMode);
  Serial.print("ADC raw: ");
  Serial.println(analogRead(PIN_ANALOG));
  Serial.println("==================");
}
