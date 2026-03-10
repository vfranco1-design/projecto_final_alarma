#include <Keypad.h>
#include <ESP32Servo.h>

Servo miServo;

// CONFIGURACIÓN DE PINES
const int pinServo = 13;   // Pin del servo
const int pirPin = 25;  // Pin G25 conectado al SIGNAL del PIR

// ===== CONFIGURACIÓN KEYPAD =====
const byte ROWS = 4;  // 4 filas
const byte COLS = 4;  // 4 columnas

// Pines para las filas (OUTPUT) y columnas (INPUT_PULLUP)
byte rowPins[ROWS] = {12, 14, 27, 26};  // Pines para las filas (SALIDA)
byte colPins[COLS] = {33, 32, 23, 22};  // Pines para las columnas (ENTRADA)

// Mapeo del teclado
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// Crear objeto Keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Variables para el sistema
bool sistemaActivo = true;  // Sistema activo por defecto
String inputPassword = "";  // Para almacenar la contraseña ingresada
const String PASSWORD_CORRECTA = "1234A";  // Contraseña correcta

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
 
  // Configurar pin del PIR como entrada
  pinMode(pirPin, INPUT);
 
  // CONFIGURACIÓN MANUAL DE PINES PARA EVITAR ERRORES
  // Configurar pines de FILAS como SALIDA
  for (int i = 0; i < ROWS; i++) {
    pinMode(rowPins[i], OUTPUT);
    digitalWrite(rowPins[i], HIGH);  // Poner en HIGH inicial
  }
 
  // Configurar pines de COLUMNAS como ENTRADA con PULLUP
  for (int i = 0; i < COLS; i++) {
    pinMode(colPins[i], INPUT_PULLUP);
  }
 
  // Mensajes iniciales
  Serial.println("==============================");
  Serial.println("SENSOR PIR INICIADO - ESP32");
  Serial.println("Pin PIR: G25 (Input)");
  Serial.println("Keypad configurado correctamente");
  Serial.println("Filas (Output): 12, 14, 27, 26");
  Serial.println("Columnas (Input): 33, 32, 35, 34");
  Serial.println("Sistema: ACTIVADO");
  Serial.println("==============================");
  Serial.println("Contraseña: 1234A + #");
  Serial.println("Presiona * para borrar");
  Serial.println("==============================");
 
  delay(2000);  // Tiempo para calibración del sensor
}

// ===== LOOP PRINCIPAL =====
void loop() {
  // 1. Leer el teclado
  char tecla = keypad.getKey();
 
  // Si se presionó una tecla
  if (tecla) {
    procesarTecla(tecla);
  }
 
  // 2. Solo detectar movimiento si el sistema está ACTIVADO
  if (sistemaActivo) {
    int movimiento = digitalRead(pirPin);
   
    // Si detecta movimiento (HIGH = 1)
    if (movimiento == HIGH) {
      Serial.println("¡MOVIMIENTO DETECTADO!");
      delay(1000);  // Espera 1 segundo para evitar mensajes repetidos
    }
  }
 
  delay(50);  // Pequeña pausa entre lecturas
}

// ===== FUNCIÓN PARA PROCESAR TECLAS =====
void procesarTecla(char tecla) {
  Serial.print("Tecla presionada: ");
  Serial.println(tecla);
 
  // Si presiona '#', validar contraseña
  if (tecla == '#') {
    validarContrasena();
    return;
  }
 
  // Si presiona '*', borrar entrada
  if (tecla == '*') {
    inputPassword = "";
    Serial.println("Entrada borrada");
    return;
  }
 
  // Agregar tecla a la contraseña
  inputPassword += tecla;
  Serial.print("Contraseña actual: ");
  Serial.println(inputPassword);
}

// ===== FUNCIÓN PARA VALIDAR CONTRASEÑA =====
void validarContrasena() {
  Serial.println("=== VALIDANDO CONTRASEÑA ===");
  Serial.print("Ingresada: ");
  Serial.println(inputPassword);
  Serial.print("Correcta:  ");
  Serial.println(PASSWORD_CORRECTA);
 
  // Comparar contraseñas
  if (inputPassword == PASSWORD_CORRECTA) {
    // Cambiar estado del sistema
    sistemaActivo = !sistemaActivo;
   
    if (sistemaActivo) {
      bloquearPuerta();
      Serial.println("==============================");
      Serial.println("SISTEMA ACTIVADO");
      Serial.println("==============================");
    } else {
      desbloquearPuerta();
      Serial.println("==============================");
      Serial.println("SISTEMA DESACTIVADO");
      Serial.println("SE APAGÓ");
      Serial.println("==============================");
    }
  } else {
    Serial.println("==============================");
    Serial.println("CONTRASEÑA INCORRECTA");
    Serial.println("==============================");
  }
 
  // Limpiar la contraseña ingresada
  inputPassword = "";
  Serial.println("Listo para nueva entrada...");
}

void bloquearPuerta() {
  Serial.println("BLOQUEANDO...");
  miServo.write(150);
}

void desbloquearPuerta() {
  Serial.println("DESBLOQUEANDO...");
  miServo.write(55);
}
