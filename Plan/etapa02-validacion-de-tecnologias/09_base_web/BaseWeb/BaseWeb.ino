/*
  ============================================================================
  ETAPA 2 - PRUEBA 09: BASE CON ESP-NOW + WEB (WebSocket 50 Hz) + MQTT A LA VEZ
  ============================================================================
  Valida la arquitectura elegida (Plan/ARQUITECTURA.md): la base, a la vez,
   1) está conectada como cliente a la WiFi (hotspot del móvil o router propio);
   2) recibe los 2 mandos por ESP-NOW en el canal de esa WiFi (los mandos lo
      buscan solos, como en la prueba 02);
   3) sirve la página del juego y le envía las posiciones por WebSocket a 50 Hz;
   4) habla MQTT con el broker de la universidad:
        - publica cada 5 s  II16/juego/estado   (estadísticas, JSON)
        - escucha           II16/juego/orden    (lo que llegue se muestra en la página)

  Página: http://<IP que sale en el monitor serie>/   (el PC, en la misma WiFi)
  Monitor serie a 115200: cada 5 s, paquetes y pérdidas de cada mando, WebSocket,
  memoria libre y pila mínima de las tareas.

  Librerías (Gestor de librerías, buscad "ESP32Async"):
   - "ESP Async WebServer" de ESP32Async   (NO la antigua de me-no-dev)
   - "Async TCP" de ESP32Async
   - "PubSubClient" de Nick O'Leary  (ya instalada para la prueba 03)
  Copiad secrets.example.h como secrets.h y rellenadlo.
  ============================================================================
*/
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <PubSubClient.h>
#include "secrets.h"     // WIFI_SSID, WIFI_PASS, MQTT_SERVER, MQTT_PORT, MQTT_USER, MQTT_PASS
#include "pagina.h"      // PAGINA: HTML/JS del juego de prueba (en la flash)

// ---------------- CONFIGURACIÓN ----------------
const uint32_t MAGIC        = 0x1A7E2026;
const uint32_t PERIODO_WS_MS = 20;            // 50 Hz hacia el navegador
const char*    TOPIC_ESTADO = "II16/juego/estado";
const char*    TOPIC_ORDEN  = "II16/juego/orden";

// ---------------- PROTOCOLO ESP-NOW ----------------
// Emparejamiento (igual que en la prueba 02)
enum Tipo : uint8_t { BUSCAR_BASE = 1, SOY_BASE = 2, ORIENTACION = 10 };
typedef struct __attribute__((packed)) {
  uint32_t magic;
  uint8_t  tipo;
  uint8_t  canal;
  uint16_t seq;
  uint32_t dato;
} Msg;

// Orientación (igual que en la prueba 08)
typedef struct __attribute__((packed)) {
  uint32_t magic;
  uint8_t  tipo;      // 10
  uint8_t  id;        // 1 cazador, 2 pájaro
  uint16_t seq;
  float    yaw, pitch, roll;
  uint8_t  botones;   // bit0 = gatillo / turbo
} PaqueteOrientacion;

// Lo que el callback de ESP-NOW deja en la cola (no se procesa en la tarea de la WiFi)
typedef struct {
  uint8_t mac[6];
  uint8_t len;
  uint8_t datos[32];
} Recibido;

// ---------------- ESTADO ----------------
AsyncWebServer servidor(80);
AsyncWebSocket ws("/ws/mandos");
WiFiClient clienteWiFi;
PubSubClient mqtt(clienteWiFi);

QueueHandle_t colaRx;
TaskHandle_t  hProceso = nullptr, hWeb = nullptr;
uint8_t canalActual = 0;
String  chipId;

// Último dato de cada mando (índice 1 y 2), protegido con un spinlock
portMUX_TYPE cerrojo = portMUX_INITIALIZER_UNLOCKED;
PaqueteOrientacion ultimo[3];
bool nuevo[3] = {false, false, false};

// Contadores acumulados (se restan para sacar los de cada intervalo)
volatile uint32_t rx[3] = {0, 0, 0}, perd[3] = {0, 0, 0};
volatile uint32_t wsEnviados = 0, wsSaltados = 0, colaLlena = 0;
int32_t ultimoSeq[3] = {-1, -1, -1};
volatile uint32_t ordenesMQTT = 0;
volatile bool mqttOk = false;     // copia de mqtt.connected() para la tarea web (PubSubClient no es multitarea)

// ---------- ESP-NOW: callback de recepción (tarea de la WiFi: solo copiar y salir) ----------
#if ESP_ARDUINO_VERSION_MAJOR >= 3
void alRecibir(const esp_now_recv_info_t *info, const uint8_t *datos, int len) {
  const uint8_t *mac = info->src_addr;
#else
void alRecibir(const uint8_t *mac, const uint8_t *datos, int len) {
#endif
  if (len < 5 || len > 32) return;
  Recibido r;
  memcpy(r.mac, mac, 6);
  r.len = len;
  memcpy(r.datos, datos, len);
  if (xQueueSend(colaRx, &r, 0) != pdTRUE) colaLlena++;
}

String macATexto(const uint8_t *m) {
  char b[18];
  snprintf(b, sizeof(b), "%02X:%02X:%02X:%02X:%02X:%02X", m[0], m[1], m[2], m[3], m[4], m[5]);
  return String(b);
}

void asegurarPeer(const uint8_t *mac) {
  if (esp_now_is_peer_exist(mac)) return;
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, mac, 6);
  peer.channel = 0;          // canal actual (el de la WiFi)
  peer.ifidx   = WIFI_IF_STA;
  peer.encrypt = false;
  esp_now_add_peer(&peer);
}

// ---------- Tarea: procesa lo recibido por ESP-NOW ----------
void tareaProceso(void *) {
  Recibido r;
  while (true) {
    if (xQueueReceive(colaRx, &r, portMAX_DELAY) != pdTRUE) continue;
    uint32_t magic;
    memcpy(&magic, r.datos, 4);
    if (magic != MAGIC) continue;
    uint8_t tipo = r.datos[4];

    if (tipo == BUSCAR_BASE && r.len == sizeof(Msg)) {
      // Un mando nos busca: le respondemos (unicast) con nuestro canal
      asegurarPeer(r.mac);
      Msg resp = {};
      resp.magic = MAGIC;
      resp.tipo  = SOY_BASE;
      resp.canal = canalActual;
      esp_now_send(r.mac, (uint8_t *)&resp, sizeof(resp));
      Serial.printf("[EMPAREJAMIENTO] Mando %s me busca -> le respondo (canal %u)\n",
                    macATexto(r.mac).c_str(), canalActual);

    } else if (tipo == ORIENTACION && r.len == sizeof(PaqueteOrientacion)) {
      PaqueteOrientacion p;
      memcpy(&p, r.datos, sizeof(p));
      if (p.id < 1 || p.id > 2) continue;
      // Pérdidas = huecos en la numeración
      if (ultimoSeq[p.id] >= 0) {
        uint16_t salto = (uint16_t)(p.seq - (uint16_t)ultimoSeq[p.id]);
        if (salto > 1 && salto < 1000) perd[p.id] += salto - 1;
      }
      ultimoSeq[p.id] = p.seq;
      rx[p.id]++;
      taskENTER_CRITICAL(&cerrojo);
      // El disparo no se puede perder aunque lleguen 2 paquetes entre envíos al navegador
      uint8_t botonesPrevios = nuevo[p.id] ? ultimo[p.id].botones : 0;
      ultimo[p.id] = p;
      ultimo[p.id].botones |= botonesPrevios;
      nuevo[p.id] = true;
      taskEXIT_CRITICAL(&cerrojo);
    }
  }
}

// ---------- Tarea: envía al navegador por WebSocket a 50 Hz ----------
void tareaWeb(void *) {
  TickType_t siguiente = xTaskGetTickCount();
  uint32_t tStats = millis();
  uint32_t rxPrev[3] = {0, 0, 0}, perdPrev[3] = {0, 0, 0}, wsPrev = 0, saltPrev = 0;
  char buf[200];

  while (true) {
    vTaskDelayUntil(&siguiente, pdMS_TO_TICKS(PERIODO_WS_MS));
    for (int id = 1; id <= 2; id++) {
      PaqueteOrientacion p;
      bool hay;
      taskENTER_CRITICAL(&cerrojo);
      hay = nuevo[id];
      if (hay) { p = ultimo[id]; nuevo[id] = false; }
      taskEXIT_CRITICAL(&cerrojo);
      if (!hay || ws.count() == 0) continue;

      // Si algún navegador no da abasto, se salta este dato (mejor que llenar la memoria)
      if (!ws.availableForWriteAll()) { wsSaltados++; continue; }
      snprintf(buf, sizeof(buf), "{\"id\":%u,\"yaw\":%.2f,\"pitch\":%.2f,\"roll\":%.2f,\"disparo\":%u}",
               p.id, p.yaw, p.pitch, p.roll, p.botones & 1);
      ws.textAll(buf);
      wsEnviados++;
    }

    // Cada segundo, estadísticas para la página
    if (millis() - tStats >= 1000) {
      tStats += 1000;
      uint32_t r1 = rx[1] - rxPrev[1], r2 = rx[2] - rxPrev[2];
      uint32_t p1 = perd[1] - perdPrev[1], p2 = perd[2] - perdPrev[2];
      uint32_t we = wsEnviados - wsPrev, ws_s = wsSaltados - saltPrev;
      rxPrev[1] = rx[1]; rxPrev[2] = rx[2]; perdPrev[1] = perd[1]; perdPrev[2] = perd[2];
      wsPrev = wsEnviados; saltPrev = wsSaltados;
      if (ws.count() > 0 && ws.availableForWriteAll()) {
        snprintf(buf, sizeof(buf),
                 "{\"tipo\":\"stats\",\"rx1\":%lu,\"p1\":%lu,\"rx2\":%lu,\"p2\":%lu,\"ws\":%lu,\"salt\":%lu,"
                 "\"heap\":%lu,\"heapMin\":%lu,\"cli\":%u,\"canal\":%u,\"rssi\":%d,\"mqtt\":%d}",
                 r1, p1, r2, p2, we, ws_s, (unsigned long)ESP.getFreeHeap(), (unsigned long)ESP.getMinFreeHeap(),
                 (unsigned)ws.count(), canalActual, WiFi.RSSI(), mqttOk ? 1 : 0);
        ws.textAll(buf);
      }
    }
  }
}

// ---------- WebSocket: conexiones ----------
void alEventoWS(AsyncWebSocket *s, AsyncWebSocketClient *c, AwsEventType tipo, void *arg, uint8_t *d, size_t len) {
  if (tipo == WS_EVT_CONNECT)
    Serial.printf("[WEB] Navegador #%u conectado desde %s (total %u)\n", c->id(), c->remoteIP().toString().c_str(), (unsigned)s->count());
  else if (tipo == WS_EVT_DISCONNECT)
    Serial.printf("[WEB] Navegador #%u desconectado\n", c->id());
}

// ---------- MQTT ----------
void alRecibirMQTT(char *topic, byte *payload, unsigned int len) {
  ordenesMQTT++;
  char texto[64];
  unsigned int n = len < sizeof(texto) - 1 ? len : sizeof(texto) - 1;
  // Solo caracteres imprimibles y sin comillas (va dentro de un JSON)
  for (unsigned int i = 0; i < n; i++) texto[i] = (payload[i] >= 32 && payload[i] != '"' && payload[i] != '\\') ? payload[i] : '?';
  texto[n] = 0;
  Serial.printf("[MQTT] <- %s: %s\n", topic, texto);
  char buf[110];
  snprintf(buf, sizeof(buf), "{\"tipo\":\"orden\",\"texto\":\"%s\"}", texto);
  if (ws.count() > 0) ws.textAll(buf);
}

void conectarMQTT() {
  static uint32_t tIntento = 0;
  if (mqtt.connected() || WiFi.status() != WL_CONNECTED) return;
  if (tIntento != 0 && millis() - tIntento < 5000) return;   // reintento cada 5 s, sin bloquear
  tIntento = millis();
  Serial.printf("[MQTT] Conectando a %s como %s...", MQTT_SERVER, chipId.c_str());
  if (mqtt.connect(chipId.c_str(), MQTT_USER, MQTT_PASS)) {
    mqtt.subscribe(TOPIC_ORDEN);
    Serial.printf(" OK. Suscrito a %s\n", TOPIC_ORDEN);
  } else {
    Serial.printf(" FALLO (estado %d)\n", mqtt.state());
  }
}

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("\n=== PRUEBA 09: base con ESP-NOW + web + MQTT ===");

  // 1) WiFi como cliente del hotspot / router
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);                  // Sin ahorro de energía: si duerme, pierde paquetes ESP-NOW
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);    // Muchas ESP32-C3 conectan mejor con potencia reducida
  Serial.printf("Conectando a la WiFi \"%s\"", WIFI_SSID);
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(300); Serial.print(".");
    if (millis() - t0 > 30000) { Serial.println("\nNO conecta. ¿SSID/clave? ¿Es 2,4 GHz y WPA2-Personal? Reinicio."); delay(2000); ESP.restart(); }
  }
  canalActual = WiFi.channel();
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char b[20];
  snprintf(b, sizeof(b), "esp32c3-%02X%02X%02X", mac[3], mac[4], mac[5]);
  chipId = b;
  Serial.printf("\nConectada. IP %s · canal %u · RSSI %d dBm · MAC %s\n",
                WiFi.localIP().toString().c_str(), canalActual, WiFi.RSSI(), WiFi.macAddress().c_str());

  // 2) ESP-NOW (en el canal de la WiFi)
  if (esp_now_init() != ESP_OK) { Serial.println("Error iniciando ESP-NOW"); while (true) delay(1000); }
  esp_now_register_recv_cb(alRecibir);
  colaRx = xQueueCreate(32, sizeof(Recibido));
  xTaskCreate(tareaProceso, "proceso", 4096, nullptr, 5, &hProceso);

  // 3) Servidor web + WebSocket
  ws.onEvent(alEventoWS);
  servidor.addHandler(&ws);
  servidor.on("/", HTTP_GET, [](AsyncWebServerRequest *req) { req->send(200, "text/html", PAGINA); });
  servidor.onNotFound([](AsyncWebServerRequest *req) { req->send(404, "text/plain", "No existe"); });
  servidor.begin();
  xTaskCreate(tareaWeb, "web", 4096, nullptr, 4, &hWeb);

  // 4) MQTT
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(alRecibirMQTT);
  mqtt.setBufferSize(512);
  mqtt.setKeepAlive(15);
  mqtt.setSocketTimeout(3);
  conectarMQTT();

  Serial.printf("\n>>> Abrid en el navegador:  http://%s/\n", WiFi.localIP().toString().c_str());
  Serial.println(">>> Mandos: subid MandoWeb (ID_MANDO 1 y 2); buscan solos el canal.\n");
}

// ---------------- LOOP: MQTT, limpieza del WebSocket y estadísticas cada 5 s ----------------
void loop() {
  static uint32_t t5 = millis(), tLimpia = millis(), tInicio = millis();
  static uint32_t rxPrev[3] = {0, 0, 0}, perdPrev[3] = {0, 0, 0}, wsPrev = 0, saltPrev = 0;
  static bool wifiAvisado = false;

  if (WiFi.status() != WL_CONNECTED) {
    if (!wifiAvisado) { Serial.println("[WIFI] Conexión perdida, reconectando..."); wifiAvisado = true; }
  } else if (wifiAvisado) {
    wifiAvisado = false;
    canalActual = WiFi.channel();
    Serial.printf("[WIFI] Reconectada (canal %u)\n", canalActual);
  }

  conectarMQTT();
  mqtt.loop();
  mqttOk = mqtt.connected();

  if (millis() - tLimpia >= 1000) { tLimpia = millis(); ws.cleanupClients(); }

  if (millis() - t5 >= 5000) {
    t5 += 5000;
    uint32_t r1 = rx[1] - rxPrev[1], r2 = rx[2] - rxPrev[2];
    uint32_t p1 = perd[1] - perdPrev[1], p2 = perd[2] - perdPrev[2];
    uint32_t we = wsEnviados - wsPrev, ws_s = wsSaltados - saltPrev;
    rxPrev[1] = rx[1]; rxPrev[2] = rx[2]; perdPrev[1] = perd[1]; perdPrev[2] = perd[2];
    wsPrev = wsEnviados; saltPrev = wsSaltados;

    Serial.printf("t=%lus | mando1: %lu paq (%lu perd) | mando2: %lu paq (%lu perd) | WS: %lu env, %lu salt, %u nav | "
                  "MQTT %s | heap %lu (min %lu) | pila libre (bytes): proceso %u, web %u, loop %u | cola llena %lu\n",
                  (millis() - tInicio) / 1000, r1, p1, r2, p2, we, ws_s, (unsigned)ws.count(),
                  mqtt.connected() ? "OK" : "NO", (unsigned long)ESP.getFreeHeap(), (unsigned long)ESP.getMinFreeHeap(),
                  (unsigned)uxTaskGetStackHighWaterMark(hProceso), (unsigned)uxTaskGetStackHighWaterMark(hWeb),
                  (unsigned)uxTaskGetStackHighWaterMark(nullptr), colaLlena);

    if (mqtt.connected()) {
      char buf[220];
      snprintf(buf, sizeof(buf),
               "{\"CHIPID\":\"%s\",\"rx1\":%lu,\"perd1\":%lu,\"rx2\":%lu,\"perd2\":%lu,\"ws_env\":%lu,\"ws_salt\":%lu,"
               "\"navegadores\":%u,\"heap\":%lu,\"canal\":%u,\"rssi\":%d}",
               chipId.c_str(), r1, p1, r2, p2, we, ws_s, (unsigned)ws.count(),
               (unsigned long)ESP.getFreeHeap(), canalActual, WiFi.RSSI());
      mqtt.publish(TOPIC_ESTADO, buf);
    }
  }
  delay(2);
}
