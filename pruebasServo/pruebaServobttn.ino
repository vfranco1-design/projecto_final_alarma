#include <ESP32Servo.h>

Servo miServo;  
int pinServo = 13;
int pinBoton = 22;  // Pin donde conectas el botón

bool bloqueado = false;

void setup() {
  Serial.begin(115200);
  
  // Configurar el servo
  miServo.attach(pinServo);
  
  // Configurar el botón
  pinMode(pinBoton, INPUT_PULLUP);  // INPUT_PULLUP es importante
  
  // Empezar desbloqueado
  miServo.write(60);
  bloqueado = false;
  
  Serial.println("Sistema listo - DESBLOQUEADO");
  Serial.println("Presiona el botón para bloquear/desbloquear");
}

void loop() {
  // Leer el estado del botón
  int estadoBoton = digitalRead(pinBoton);
  
  // Si el botón está presionado (LOW porque usamos INPUT_PULLUP)
  if(estadoBoton == LOW) {
    delay(50);  // Anti-rebote
    
    // Verificar que sigue presionado
    if(digitalRead(pinBoton) == LOW) {
      
      if(!bloqueado) {
        // BLOQUEAR
        Serial.println("BLOQUEANDO...");
        miServo.write(160);
        bloqueado = true;
      } else {
        // DESBLOQUEAR
        Serial.println("DESBLOQUEANDO...");
        miServo.write(60);
        bloqueado = false;
      }
      
      // Esperar a que suelte el botón
      while(digitalRead(pinBoton) == LOW) {
        delay(10);
      }
      delay(200);  // Pausa extra para evitar rebotes
    }
  }
}