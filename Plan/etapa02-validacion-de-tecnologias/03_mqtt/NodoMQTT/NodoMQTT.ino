/*
  ============================================================================
  ETAPA 2 - PRUEBA 03: MQTT CON EL BROKER DE LA UNIVERSIDAD
  ============================================================================
  Qué demuestra (con los topics del enunciado, grupo II16):
   - Conexión al broker iot.ac.uma.es con usuario/clave del grupo y un
     identificador de cliente único (el hostname del chip: esp32c3-XXXXXX).
   - II16/<id>/conexion : mensaje RETENIDO {"CHIPID":..,"online":true}
                          + LWT (última voluntad) {"CHIPID":..,"online":false}
   - II16/<id>/datos    : publicación periódica (cada "envia" segundos)
   - II16/<id>/config   : suscripción; {"envia":10} cambia el periodo (null = no cambia)
   - II16/<id>/ping     : suscripción; responde en II16/<id>/pong con lo mismo
                          (sirve para medir la latencia ida y vuelta por MQTT)
   - Reconexión automática a WiFi y MQTT si se caen.

  Librerías (Gestor de librerías): PubSubClient (Nick O'Leary) y ArduinoJson (v7).
  Copia secrets.example.h como secrets.h en esta carpeta y rellénalo.
  Monitor serie a 115200.
  ============================================================================
*/
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "secrets.h"   // WIFI_SSID, WIFI_PASS, MQTT_SERVER, MQTT_PORT, MQTT_USER, MQTT_PASS

// ---------------- CONFIGURACIÓN ----------------
const char*    GRUPO = "II16";             // Prefijo obligatorio de los topics
uint32_t       periodoEnvioS = 30;         // Periodo de "datos" (el enunciado usa 300 s por defecto)
const uint32_t REINTENTO_MS  = 5000;       // Espera entre intentos de reconexión

WiFiClient   clienteWiFi;
PubSubClient mqtt(clienteWiFi);

String chipId;                  // esp32c3-XXXXXX
String tConexion, tDatos, tConfig, tPing, tPong;

// ---------- Identificador único del dispositivo ----------
String obtenerChipId() {
  String h = WiFi.getHostname();
  if (h.length() == 0) {        // Por si el hostname aún no está disponible
    uint8_t mac[6];
    WiFi.macAddress(mac);
    char b[20];
    snprintf(b, sizeof(b), "esp32c3-%02X%02X%02X", mac[3], mac[4], mac[5]);
    h = b;
  }
  return h;
}

// ---------- WiFi ----------
void conectarWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  Serial.printf("Conectando a la WiFi \"%s\"", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);   // Muchas ESP32-C3 conectan mejor con potencia reducida
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 20000) { delay(300); Serial.print("."); }
  if (WiFi.status() == WL_CONNECTED)
    Serial.printf("\nWiFi OK. IP %s  RSSI %d dBm\n", WiFi.localIP().toString().c_str(), WiFi.RSSI());
  else
    Serial.println("\nWiFi NO conecta (reintento más tarde)");
}

// ---------- Mensajes ----------
void publicarConexion(bool online) {
  JsonDocument doc;
  doc["CHIPID"] = chipId;
  doc["online"] = online;
  char buf[96];
  serializeJson(doc, buf);
  mqtt.publish(tConexion.c_str(), buf, true);   // true = RETENIDO
}

void publicarDatos() {
  JsonDocument doc;
  doc["CHIPID"] = chipId;
  doc["Uptime"] = millis();
  JsonObject wifi = doc["Wifi"].to<JsonObject>();
  wifi["SSId"] = WiFi.SSID();
  wifi["IP"]   = WiFi.localIP().toString();
  wifi["RSSI"] = WiFi.RSSI();
  doc["envia"] = periodoEnvioS;
  char buf[256];
  serializeJson(doc, buf);
  bool ok = mqtt.publish(tDatos.c_str(), buf);
  Serial.printf("-> %s %s %s\n", tDatos.c_str(), buf, ok ? "" : "(ERROR al publicar)");
}

// ---------- Mensajes recibidos ----------
void alRecibirMQTT(char *topic, byte *payload, unsigned int len) {
  String t = topic;
  Serial.printf("<- %s %.*s\n", topic, len, (const char *)payload);

  if (t == tPing) {            // Eco para medir latencia
    // OJO: "payload" apunta al búfer interno de PubSubClient, y publish() escribe en ese
    // mismo búfer. Si se publica directamente, el topic nuevo pisa el contenido
    // (llegaba "on" en vez del número). Hay que copiarlo antes.
    uint8_t copia[64];
    unsigned int n = len < sizeof(copia) ? len : sizeof(copia);
    memcpy(copia, payload, n);
    mqtt.publish(tPong.c_str(), copia, n);
    return;
  }

  if (t == tConfig) {
    JsonDocument doc;
    if (deserializeJson(doc, payload, len)) { Serial.println("   config: JSON no válido, se ignora"); return; }
    // Si el campo falta o es null se mantiene el valor anterior (como pide el enunciado)
    if (!doc["envia"].isNull()) {
      uint32_t v = doc["envia"].as<uint32_t>();
      if (v >= 1 && v <= 3600) {
        periodoEnvioS = v;
        Serial.printf("   config: nuevo periodo de envío = %lu s\n", periodoEnvioS);
      } else {
        Serial.println("   config: 'envia' fuera de rango (1..3600), se ignora");
      }
    }
  }
}

// ---------- MQTT ----------
bool conectarMQTT() {
  Serial.printf("Conectando a MQTT %s:%d como \"%s\"...", MQTT_SERVER, MQTT_PORT, chipId.c_str());

  // LWT: si el dispositivo desaparece sin despedirse, el broker publica esto (retenido)
  JsonDocument lwt;
  lwt["CHIPID"] = chipId;
  lwt["online"] = false;
  char lwtBuf[96];
  serializeJson(lwt, lwtBuf);

  bool ok = mqtt.connect(chipId.c_str(), MQTT_USER, MQTT_PASS,
                         tConexion.c_str(), 1, true, lwtBuf);   // topic LWT, QoS 1, retenido
  if (!ok) {
    Serial.printf(" FALLO (estado %d). ¿Usuario/clave del grupo correctos?\n", mqtt.state());
    return false;
  }
  Serial.println(" OK");
  publicarConexion(true);
  mqtt.subscribe(tConfig.c_str());
  mqtt.subscribe(tPing.c_str());
  Serial.printf("Suscrito a %s y %s\n", tConfig.c_str(), tPing.c_str());
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  chipId = obtenerChipId();

  String base = String(GRUPO) + "/" + chipId;
  tConexion = base + "/conexion";
  tDatos    = base + "/datos";
  tConfig   = base + "/config";
  tPing     = base + "/ping";
  tPong     = base + "/pong";

  Serial.printf("\nNodo MQTT. CHIPID = %s\nTopics: %s/...\n", chipId.c_str(), base.c_str());

  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(alRecibirMQTT);
  mqtt.setBufferSize(512);
  mqtt.setKeepAlive(15);   // El LWT salta ~1,5 x keepalive después de perder la conexión

  conectarWiFi();
}

void loop() {
  static uint32_t ultimoIntento = 0, ultimoEnvio = 0;

  if (WiFi.status() != WL_CONNECTED) {
    if (millis() - ultimoIntento > REINTENTO_MS) { ultimoIntento = millis(); conectarWiFi(); }
    delay(10);
    return;
  }

  if (!mqtt.connected()) {
    if (millis() - ultimoIntento > REINTENTO_MS) {
      ultimoIntento = millis();
      if (conectarMQTT()) ultimoEnvio = 0;   // Publica datos nada más conectar
    }
    delay(10);
    return;
  }

  mqtt.loop();

  if (ultimoEnvio == 0 || millis() - ultimoEnvio >= periodoEnvioS * 1000UL) {
    ultimoEnvio = millis();
    publicarDatos();
  }
  delay(10);
}
