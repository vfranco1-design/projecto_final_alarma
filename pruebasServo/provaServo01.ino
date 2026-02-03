#include <ESP32Servo.h>

Servo miServo;  
int pinServo = 13;   // Pin del servo
int pinBuzzer = 22;  // Pin del buzzer (puedes cambiar a otro GPIO)

void setup() {
  Serial.begin(115200);
  
  // Configurar el servo
  miServo.attach(pinServo);
  
  // Configurar el buzzer
  pinMode(pinBuzzer, OUTPUT);
  
  Serial.println("Servo y Buzzer listos");
}

void loop() {
  // Mover a 0 grados + pitido corto
  Serial.println("Moviendo a 0°");
  miServo.write(0);
  tone(pinBuzzer, 4000, 200);
  delay(1000);
  
  // Mover a 90 grados + pitido medio
  Serial.println("Moviendo a 90°");
  miServo.write(90);
  tone(pinBuzzer, 4000, 200);
  delay(1000);
  
  // Mover a 180 grados + pitido agudo
  Serial.println("Moviendo a 180°");
  miServo.write(180);
  tone(pinBuzzer, 4000, 200);
  delay(1000);
}