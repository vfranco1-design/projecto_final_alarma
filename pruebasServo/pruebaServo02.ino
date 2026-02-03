#include <ESP32Servo.h>

Servo miServo;  
int pinServo = 13;   // Pin del servo

void setup() {
  Serial.begin(115200);
  
  // Configurar el servo
  miServo.attach(pinServo);
  miServo.write(55);
  
  Serial.println("Sistema listo - DESBLOQUEADO");
}

void loop() {
  // BLOQUEAR (vertical - 90 grados)
  Serial.println("BLOQUEANDO...");
  miServo.write(150);
  delay(3000);  // Espera 3 segundos bloqueado
  
  // DESBLOQUEAR (horizontal - 0 grados)
  Serial.println("DESBLOQUEANDO...");
  miServo.write(55);
  delay(3000);  // Espera 3 segundos desbloqueado
}