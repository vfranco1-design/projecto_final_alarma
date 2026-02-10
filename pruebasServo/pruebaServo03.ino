#include <ESP32Servo.h>

Servo miServo;  
int pinServo = 13;
int pinPIR = 22;     // Sensor de movimiento

void setup() {
  Serial.begin(115200);
  
  miServo.attach(pinServo);
  pinMode(pinPIR, INPUT);
  
  // Empezar desbloqueado
  miServo.write(55);
  
  Serial.println("Sistema listo");
  Serial.println("Calibrando sensor PIR (30 segundos)...");
  delay(30000);  // El PIR necesita calibrarse
  Serial.println("✓ PIR calibrado. Detectando movimiento...");
}

void loop() {
  int movimiento = digitalRead(pinPIR);
  
  if(movimiento == HIGH) {
    // MOVIMIENTO DETECTADO → BLOQUEAR
    Serial.println("🚨 MOVIMIENTO DETECTADO - BLOQUEANDO");
    miServo.write(150);
    delay(3000);  // Mantiene bloqueado 3 segundos
    
    // Volver a desbloquear
    Serial.println("DESBLOQUEANDO");
    miServo.write(55);
    delay(1000);
  }
  
  delay(100);
}