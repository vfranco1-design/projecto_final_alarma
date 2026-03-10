#include <Keypad.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <WebServer.h>
#include <time.h>

Servo miServo;

// CONFIGURACIÓN DE PINES
const int pinServo = 13;   // Pin del servo
const int pirPin = 25;  // Pin G25 conectado al SIGNAL del PIR

// Credenciales de conexión WiFi 
const char* ssid     = "Wewewewe";
const char* password = "GoAfkPls";

// Inicialización de variables para logs
WebServer server(80);
int pressCount = 0;
String ipAddress = "0.0.0.0";
bool timeOk = false;
String logs[5];
int logCount = 0;

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

  WiFi.scanNetworks();
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts++ < 40) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("ERROR: No se pudo conectar al WiFi.");
    while(1) { delay(1000); Serial.println("Reinicia la placa..."); }
  }

  ipAddress = WiFi.localIP().toString();
  Serial.println("WiFi OK | IP: " + ipAddress);

  configTime(3600, 3600, "pool.ntp.org");
  struct tm t;
  for (int i = 0; i < 10 && !timeOk; i++) {
    delay(500);
    if (getLocalTime(&t)) timeOk = true;
  }
  Serial.println(timeOk ? "Hora sincronizada" : "Sin hora NTP");

  addLog("Sistema iniciado");
  addLog("IP: " + ipAddress);

  server.on("/", []() {
    String html = "<!DOCTYPE html><html><head>"
      "<meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
      "<title>ESP32 Boton</title><style>"
      "body{font-family:Arial;padding:20px}"
      ".green{background:#4CAF50;color:#fff;padding:15px;border-radius:5px;text-align:center;font-size:20px}"
      ".blue{background:#3498db;color:#fff;padding:10px;border-radius:5px;text-align:center;margin:10px 0}"
      ".count{font-size:24px;color:#2196F3;text-align:center;margin:20px}"
      ".box{border:2px solid #ddd;padding:15px;margin-top:20px;border-radius:5px}"
      ".item{padding:5px 0;border-bottom:1px solid #eee}"
      ".ts{color:#666;font-size:.9em;font-family:monospace}"
      "</style></head><body>"
      "<h1>ESP32 Button Logger</h1>"
      "<div class='green'>IP: " + ipAddress + "</div>"
      "<div class='blue'>Hora: <strong>" + getTime() + "</strong></div>"
      "<h2 class='count'>Registros: " + String(pressCount) + "</h2>"
      "<p style='text-align:center'><a href='/'>Actualizar</a> | <a href='/clear'>Borrar Logs</a></p>"
      "<div class='box'><h3>Historial:</h3>";

    if (!logCount) {
      html += "<p>Presiona el boton para ver logs aqui...</p>";
    } else {
      for (int i = 0; i < logCount; i++) {
        int sep = logs[i].indexOf("] ");
        html += "<div class='item'><div class='ts'>" + logs[i].substring(0, sep+1) + "</div>"
                "<div>" + logs[i].substring(sep+2) + "</div></div>";
      }
    }
    html += "</div></body></html>";
    server.send(200, "text/html", html);
  });

  server.on("/clear", []() {
    for (int i = 0; i < 5; i++) logs[i] = "";
    logCount = 0;
    pressCount = 0;
    addLog("Logs borrados");
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  });

  server.begin();
  addLog("Servidor web listo");
 
  // Configurar pin del PIR como entrada
  pinMode(pirPin, INPUT);

  //Inicializar servo
  miServo.attach(pinServo);
 
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
  server.handleClient();

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
      procesarLog("sensor");
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
      procesarLog("sistemaOn");
      Serial.println("==============================");
      Serial.println("SISTEMA ACTIVADO");
      Serial.println("==============================");
    } else {
      desbloquearPuerta();
      procesarLog("sistemaOff");
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

String getTime() {
  if (!timeOk) return "--:--:--";
  struct tm t;
  if (!getLocalTime(&t)) return "??:??:??";
  char buf[9];
  strftime(buf, sizeof(buf), "%H:%M:%S", &t);
  return String(buf);
}

void addLog(String msg) {
  String entry = "[" + getTime() + "] " + msg;
  Serial.println(entry);
  if (logCount >= 5) {
    for (int i = 0; i < 4; i++) logs[i] = logs[i+1];
    logs[4] = entry;
  } else {
    logs[logCount++] = entry;
  }
}

void procesarLog(String tipo) {
  static int sensorCount = 0;
  static int sistemaOnCount = 0;
  static int sistemaOffCount = 0;

  if (tipo == "sensor") {
    addLog("Movimiento detectado #" + String(++sensorCount));
  } else if (tipo == "sistemaOn") {
    addLog("Sistema activado #" + String(++sistemaOnCount));
  } else if (tipo == "sistemaOff") {
    addLog("Sistema desactivado #" + String(++sistemaOffCount));
  }
  pressCount++;
}