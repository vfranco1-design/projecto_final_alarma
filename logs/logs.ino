#include <WiFi.h>
#include <WebServer.h>
#include <time.h>

const char* ssid     = "Wewewewe";
const char* password = "GoAfkPls";
#define BUTTON_PIN 22

WebServer server(80);
int pressCount = 0;
String ipAddress = "0.0.0.0";
bool timeOk = false;
String logs[5];
int logCount = 0;

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

void setup() {
  Serial.begin(115200);
  delay(3000);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

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
      "<h2 class='count'>Pulsaciones: " + String(pressCount) + "</h2>"
      "<p style='text-align:center'>Boton en <strong>GPIO " + String(BUTTON_PIN) + "</strong></p>"
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
}

void loop() {
  server.handleClient();
  static bool lastBtn = HIGH;
  bool btn = digitalRead(BUTTON_PIN);

  if (btn == LOW && lastBtn == HIGH) {
    delay(30);
    if (digitalRead(BUTTON_PIN) == LOW) {
      addLog("Boton presionado #" + String(++pressCount));
      while (digitalRead(BUTTON_PIN) == LOW) { delay(10); server.handleClient(); }
    }
  }
  lastBtn = btn;
  delay(10);
}