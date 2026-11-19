/*
  ============================================================================
  ETAPA 2 - PRUEBA 04: FOTA BÁSICA (actualización del firmware por WiFi)
  ============================================================================
  Esquema sencillo, probado con un servidor en vuestro propio PC:
    1) El ESP32 descarga  URL_VERSION  (un archivo de texto con un número, p. ej. "2").
    2) Si ese número es MAYOR que FW_VERSION, descarga URL_FIRMWARE (.bin),
       lo graba en la otra partición y se reinicia con la versión nueva.
    3) Si la descarga falla, sigue funcionando con la versión antigua.

  Cuándo comprueba si hay actualización (como pide el enunciado):
    - al arrancar
    - con una pulsación LARGA (>= 1 s) del botón BOOT
    - escribiendo 'u' + Enter en el monitor serie (simula la orden por MQTT)
    - periódicamente cada PERIODO_COMPROBACION_MIN minutos (0 = nunca)

  El LED RGB muestra la versión: v1 = verde, v2 = morado, otras = azul.
  Así se VE que la actualización ha funcionado.

  Copia secrets.example.h como secrets.h en esta carpeta y rellénalo.
  Monitor serie a 115200.
  ============================================================================
*/
#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include "secrets.h"   // WIFI_SSID, WIFI_PASS, URL_VERSION, URL_FIRMWARE

// ---------------- CONFIGURACIÓN ----------------
#define FW_VERSION 1                         // <-- SUBIDLA (2, 3...) en cada versión nueva
const uint32_t PERIODO_COMPROBACION_MIN = 0; // 0 = sin comprobaciones periódicas
const int      PIN_BOTON = 9;                // BOOT: GPIO9 en ESP32-C3 (0 en ESP32 clásico)
const uint32_t PULSACION_LARGA_MS = 1000;

// ---------- LED RGB integrado ----------
void ponerLed(uint8_t r, uint8_t g, uint8_t b) {
#ifdef RGB_BUILTIN
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  rgbLedWrite(RGB_BUILTIN, r, g, b);
#else
  neopixelWrite(RGB_BUILTIN, r, g, b);
#endif
#endif
}

void ledSegunVersion() {
  if (FW_VERSION == 1)      ponerLed(0, 40, 0);    // verde
  else if (FW_VERSION == 2) ponerLed(30, 0, 40);   // morado
  else                      ponerLed(0, 0, 40);    // azul
}

// ---------- WiFi ----------
bool conectarWiFi() {
  if (WiFi.status() == WL_CONNECTED) return true;
  Serial.printf("Conectando a la WiFi \"%s\"", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 20000) { delay(300); Serial.print("."); }
  bool ok = WiFi.status() == WL_CONNECTED;
  Serial.println(ok ? "\nWiFi OK. IP " + WiFi.localIP().toString() : "\nWiFi NO conecta");
  return ok;
}

// ---------- Paso 1: ¿qué versión hay en el servidor? ----------
int versionEnServidor() {
  WiFiClient cliente;
  HTTPClient http;
  http.setTimeout(5000);
  if (!http.begin(cliente, URL_VERSION)) return -1;
  int codigo = http.GET();
  int v = -1;
  if (codigo == HTTP_CODE_OK) v = http.getString().toInt();
  else Serial.printf("  No se pudo leer %s (HTTP %d)\n", URL_VERSION, codigo);
  http.end();
  return v;
}

// ---------- Paso 2: descargar y grabar ----------
void comprobarActualizacion(const char *origen) {
  Serial.printf("\n[FOTA] Comprobando actualización (origen: %s). Versión actual: %d\n", origen, FW_VERSION);
  if (!conectarWiFi()) return;

  int vServidor = versionEnServidor();
  if (vServidor < 0) { Serial.println("[FOTA] Servidor no disponible: sigo con la versión actual"); return; }
  Serial.printf("[FOTA] Versión en el servidor: %d\n", vServidor);
  if (vServidor <= FW_VERSION) { Serial.println("[FOTA] Ya tengo la última versión"); return; }

  Serial.printf("[FOTA] Descargando %s ...\n", URL_FIRMWARE);
  ponerLed(0, 0, 60);   // azul mientras actualiza

  httpUpdate.onProgress([](int actual, int total) {
    static int ultimo = -10;
    int pct = total > 0 ? actual * 100 / total : 0;
    if (pct >= ultimo + 10) { ultimo = pct; Serial.printf("  %d %%\n", pct); }
  });
  httpUpdate.rebootOnUpdate(true);

  WiFiClient cliente;
  t_httpUpdate_return r = httpUpdate.update(cliente, URL_FIRMWARE);
  // Si todo va bien, la placa se reinicia dentro de update() y no se llega aquí
  if (r == HTTP_UPDATE_FAILED)
    Serial.printf("[FOTA] ERROR %d: %s. Sigo con la versión %d\n",
                  httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str(), FW_VERSION);
  else if (r == HTTP_UPDATE_NO_UPDATES)
    Serial.println("[FOTA] El servidor dice que no hay actualización");
  ledSegunVersion();
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  pinMode(PIN_BOTON, INPUT_PULLUP);
  ledSegunVersion();

  Serial.printf("\n=== Nodo FOTA - firmware versión %d ===\n", FW_VERSION);
  Serial.println("Pulsación larga de BOOT o 'u' + Enter = comprobar actualización");

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  comprobarActualizacion("arranque");
}

void loop() {
  static uint32_t tPulsado = 0;
  static bool largaHecha = false;
  static uint32_t ultimaComprobacion = millis();

  // Pulsación larga del botón BOOT
  if (digitalRead(PIN_BOTON) == LOW) {
    if (tPulsado == 0) tPulsado = millis();
    if (!largaHecha && millis() - tPulsado >= PULSACION_LARGA_MS) {
      largaHecha = true;
      comprobarActualizacion("botón");
    }
  } else {
    tPulsado = 0;
    largaHecha = false;
  }

  // Orden por el monitor serie (en el proyecto vendrá por MQTT)
  if (Serial.available() && Serial.read() == 'u') comprobarActualizacion("orden remota (serie)");

  // Comprobación periódica
  if (PERIODO_COMPROBACION_MIN > 0 &&
      millis() - ultimaComprobacion >= PERIODO_COMPROBACION_MIN * 60000UL) {
    ultimaComprobacion = millis();
    comprobarActualizacion("periódica");
  }
  delay(10);
}
