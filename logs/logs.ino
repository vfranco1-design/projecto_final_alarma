#include <WiFi.h>
#include <WebServer.h>
#include <time.h>  // <-- AÑADIDO para la hora

// CONFIGURA ESTO CON TUS DATOS REALES
const char* ssid = "Wewewewe";      // NOMBRE EXACTO de tu WiFi
const char* password = "GoAfkPls";  // CONTRASEÑA EXACTA

// Configuración de hora (AÑADIDO)
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;     // GMT+1 (España)
const int   daylightOffset_sec = 3600; // Horario de verano

// Botón en pin 14
#define BUTTON_PIN 22

WebServer server(80);

// Variables (AÑADIDO timeConfigured)
int pressCount = 0;
String ipAddress = "0.0.0.0";
bool timeConfigured = false;  // <-- AÑADIDO

// Array para logs
String logs[5];
int logCount = 0;

// FUNCIÓN NUEVA para obtener hora actual
String getCurrentTime() {
  if (!timeConfigured) {
    return "--:--:--";
  }
 
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    return "Hora no disp.";
  }
 
  char timeString[9];
  strftime(timeString, sizeof(timeString), "%H:%M:%S", &timeinfo);
  return String(timeString);
}

// FUNCIÓN NUEVA para configurar hora
void setupTime() {
  Serial.println("⏰ Configurando hora NTP...");
 
  // Configurar servidor NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
 
  // Esperar a sincronizar
  struct tm timeinfo;
  for (int i = 0; i < 10; i++) {
    delay(500);
    if (getLocalTime(&timeinfo)) {
      timeConfigured = true;
      Serial.print("✅ Hora sincronizada: ");
     
      char timeString[20];
      strftime(timeString, sizeof(timeString), "%H:%M:%S %d/%m/%Y", &timeinfo);
      Serial.println(timeString);
      return;
    }
    Serial.print(".");
  }
 
  Serial.println("⚠️  No se pudo sincronizar la hora");
}

void addLog(String msg) {
  // MODIFICADO: Ahora incluye la hora
  String timestamp = getCurrentTime();
  String logEntry = "[" + timestamp + "] " + msg;
 
  Serial.println(logEntry);
 
  // Mover logs hacia arriba si está lleno
  if (logCount >= 5) {
    for (int i = 0; i < 4; i++) {
      logs[i] = logs[i + 1];
    }
    logs[4] = logEntry;
  } else {
    logs[logCount] = logEntry;
    logCount++;
  }
}

void setup() {
  Serial.begin(115200);
  delay(3000); // Espera más tiempo para boot
 
  Serial.println("\n\n====================================");
  Serial.println("        ESP32 BUTTON LOGGER");
  Serial.println("====================================");
 
  // Configurar botón
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.println("Pin 14 configurado como INPUT_PULLUP");
 
  // Mostrar información de red disponible
  Serial.println("\n=== ESCANEANDO REDES WIFI ===");
  int n = WiFi.scanNetworks();
 
  if (n == 0) {
    Serial.println("No se encontraron redes WiFi!");
  } else {
    Serial.print("Se encontraron ");
    Serial.print(n);
    Serial.println(" redes:");
   
    for (int i = 0; i < n; i++) {
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(" dBm)");
      Serial.println(WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? " Abierta" : " Protegida");
     
      // Ver si encontramos tu red
      if (WiFi.SSID(i) == ssid) {
        Serial.println("  ✓ ¡ESTA RED ES LA TUYA!");
      }
    }
  }
 
  // Intentar conectar al WiFi
  Serial.println("\n=== INTENTANDO CONEXION ===");
  Serial.print("Conectando a: ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
 
  // Mostrar progreso detallado
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
   
    switch(WiFi.status()) {
      case WL_IDLE_STATUS:
        Serial.print("[IDLE]");
        break;
      case WL_NO_SSID_AVAIL:
        Serial.print("[SSID_NO_DISP]");
        break;
      case WL_CONNECT_FAILED:
        Serial.print("[FALLÓ]");
        break;
      case WL_DISCONNECTED:
        Serial.print("[DESCONECTADO]");
        break;
    }
   
    attempts++;
  }
 
  Serial.println();
 
  // Verificar conexión
  if (WiFi.status() == WL_CONNECTED) {
    ipAddress = WiFi.localIP().toString();
   
    Serial.println("\n✅ ¡WIFI CONECTADO!");
    Serial.println("====================================");
    Serial.print("📡 DIRECCION IP: ");
    Serial.println(ipAddress);
    Serial.println("🌐 PARA ACCEDER:");
    Serial.print("   Abre un navegador y ve a: http://");
    Serial.println(ipAddress);
    Serial.println("====================================\n");
   
    // AÑADIDO: Configurar hora DESPUÉS de conectar WiFi
    setupTime();
   
    addLog("Sistema iniciado");
    addLog("Conectado a: " + String(ssid));
    addLog("IP: " + ipAddress);
    addLog("Boton en GPIO 14");
   
  } else {
    Serial.println("\n❌ ERROR: NO SE PUDO CONECTAR AL WIFI");
    Serial.println("====================================");
    Serial.println("POSIBLES SOLUCIONES:");
    Serial.println("1. Verifica que el nombre del WiFi sea EXACTO:");
    Serial.print("   Actual: '");
    Serial.print(ssid);
    Serial.println("'");
    Serial.println("2. Verifica la contraseña");
    Serial.println("3. Asegúrate de que el WiFi esté activo");
    Serial.println("4. El ESP32 solo funciona con WiFi 2.4GHz (no 5GHz)");
    Serial.println("5. Reinicia el router si es necesario");
    Serial.println("====================================");
   
    // No continuar sin WiFi
    while(1) {
      delay(1000);
      Serial.println("Reinicia la placa para intentar de nuevo...");
    }
  }
 
  // Configurar servidor web SOLO si hay WiFi
  server.on("/", []() {
    String html = "<!DOCTYPE html><html><head>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<title>ESP32 Boton</title>";
    html += "<style>";
    html += "body { font-family: Arial; padding: 20px; }";
    html += "h1 { color: #333; }";
    html += ".ip-box { background: #4CAF50; color: white; padding: 15px; font-size: 20px; border-radius: 5px; text-align: center; }";
    html += ".time-box { background: #3498db; color: white; padding: 10px; border-radius: 5px; text-align: center; margin: 10px 0; }"; // AÑADIDO
    html += ".count { font-size: 24px; color: #2196F3; text-align: center; margin: 20px; }";
    html += ".log-box { border: 2px solid #ddd; padding: 15px; margin-top: 20px; border-radius: 5px; }";
    html += ".log-item { padding: 5px 0; border-bottom: 1px solid #eee; }";
    html += ".log-time { color: #666; font-size: 0.9em; font-family: monospace; }"; // AÑADIDO
    html += ".log-msg { color: #333; }"; // AÑADIDO
    html += "</style>";
    html += "</head><body>";
   
    html += "<h1>ESP32 Button Logger</h1>";
    html += "<div class='ip-box'>IP: " + ipAddress + "</div>";
   
    // AÑADIDO: Mostrar hora actual
    html += "<div class='time-box'>";
    html += "🕐 Hora actual: <strong>" + getCurrentTime() + "</strong>";
    html += "</div>";
   
    html += "<h2 class='count'>Pulsaciones: " + String(pressCount) + "</h2>";
    html += "<p style='text-align: center;'>Boton conectado en <strong>GPIO 14</strong></p>";
    html += "<p style='text-align: center;'><a href='/'>Actualizar</a> | <a href='/clear'>Borrar Logs</a></p>";
   
    html += "<div class='log-box'>";
    html += "<h3>Historial:</h3>";
   
    if (logCount == 0) {
      html += "<p>Presiona el botón para ver logs aquí...</p>";
    } else {
      for (int i = 0; i < logCount; i++) {
        // Separar hora del mensaje
        String logEntry = logs[i];
        int bracketEnd = logEntry.indexOf("] ");
        String logTime = logEntry.substring(0, bracketEnd + 1);
        String logMsg = logEntry.substring(bracketEnd + 2);
       
        html += "<div class='log-item'>";
        html += "<div class='log-time'>" + logTime + "</div>";
        html += "<div class='log-msg'>" + logMsg + "</div>";
        html += "</div>";
      }
    }
   
    html += "</div>";
    html += "</body></html>";
   
    server.send(200, "text/html", html);
  });
 
  server.on("/clear", []() {
    for (int i = 0; i < 5; i++) logs[i] = "";
    logCount = 0;
    pressCount = 0;
    addLog("Logs borrados - Sistema reiniciado");
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  });
 
  server.begin();
  Serial.println("✅ Servidor web iniciado");
  addLog("Servidor web listo");
}

void loop() {
  server.handleClient();
 
  // Leer botón
  static bool lastButtonState = HIGH;
 
  if (digitalRead(BUTTON_PIN) == LOW && lastButtonState == HIGH) {
    delay(30); // Debounce
   
    if (digitalRead(BUTTON_PIN) == LOW) {
      pressCount++;
      // MODIFICADO: Ahora addLog ya incluye la hora automáticamente
      addLog("Boton presionado #" + String(pressCount));
     
      // Esperar a que suelten el botón
      while(digitalRead(BUTTON_PIN) == LOW) {
        delay(10);
        server.handleClient();
      }
    }
  }
 
  lastButtonState = digitalRead(BUTTON_PIN);
  delay(10);
}
