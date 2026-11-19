/*
  ============================================================================
  ETAPA 2 - PRUEBA 02: EMPAREJAMIENTO ESP-NOW  -  PLACA "BASE"
  ============================================================================
  Qué demuestra:
   1) Canal automático: la base está en un canal que el mando NO conoce (el de
      la WiFi si CONECTAR_WIFI = 1, o CANAL_SIN_WIFI si no). El mando lo encuentra.
   2) Unicast con ACK: el mando aprende la MAC de la base y le envía los
      "disparos" dirigidos a ella. ESP-NOW confirma cada envío y reintenta solo.

  Protocolo:
   - Mando -> broadcast  BUSCAR_BASE   (en cada canal, hasta encontrarla)
   - Base  -> unicast    SOY_BASE      (respuesta con su canal)
   - Mando -> unicast    EVENTO        (disparo / ráfaga de prueba)

  Monitor serie a 115200. Funciona con ESP32-C3 y ESP32, core 2.x y 3.x.
  Si CONECTAR_WIFI = 1, copiad secrets.example.h como secrets.h y rellenadlo.
  ============================================================================
*/
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

// ---------------- CONFIGURACIÓN ----------------
#define CONECTAR_WIFI 0          // 1 = la base se conecta a la WiFi (canal impuesto por el router)
const uint8_t  CANAL_SIN_WIFI = 6; // Canal si NO hay WiFi (distinto de 1 a propósito: el mando lo busca)
const uint32_t MAGIC = 0x1A7E2026;

#if CONECTAR_WIFI
#include "secrets.h"             // WIFI_SSID, WIFI_PASS
#endif

// ---------------- PROTOCOLO (igual en base y mando) ----------------
enum Tipo : uint8_t { BUSCAR_BASE = 1, SOY_BASE = 2, EVENTO = 3 };

typedef struct __attribute__((packed)) {
  uint32_t magic;
  uint8_t  tipo;
  uint8_t  canal;   // SOY_BASE: canal de la base
  uint16_t seq;     // EVENTO: número de evento
  uint32_t dato;    // EVENTO: dato libre (p. ej. 1 = disparo, 2 = ráfaga)
} Msg;

typedef struct {
  uint8_t mac[6];
  Msg     msg;
} Recibido;

QueueHandle_t colaRx;   // callback de recepción -> tarea de proceso
uint8_t canalActual = 0;

// Estadísticas de eventos recibidos
uint32_t eventosRecibidos = 0;
uint32_t duplicados = 0;
int32_t  ultimoSeq = -1;

// ---------- Callback de recepción (firma distinta en core 2.x y 3.x) ----------
#if ESP_ARDUINO_VERSION_MAJOR >= 3
void alRecibir(const esp_now_recv_info_t *info, const uint8_t *datos, int len) {
  const uint8_t *mac = info->src_addr;
#else
void alRecibir(const uint8_t *mac, const uint8_t *datos, int len) {
#endif
  if (len != sizeof(Msg)) return;
  Recibido r;
  memcpy(r.mac, mac, 6);
  memcpy(&r.msg, datos, sizeof(Msg));
  if (r.msg.magic != MAGIC) return;
  xQueueSend(colaRx, &r, 0);   // No bloquear la tarea de la WiFi
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
  peer.channel = 0;      // canal actual
  peer.encrypt = false;
  esp_now_add_peer(&peer);
}

// ---------- Tarea que procesa lo recibido ----------
void tareaProceso(void *) {
  Recibido r;
  while (true) {
    if (xQueueReceive(colaRx, &r, portMAX_DELAY) != pdTRUE) continue;

    if (r.msg.tipo == BUSCAR_BASE) {
      // Un mando nos busca: le respondemos directamente (unicast) con nuestro canal
      asegurarPeer(r.mac);
      Msg resp = {};
      resp.magic = MAGIC;
      resp.tipo  = SOY_BASE;
      resp.canal = canalActual;
      esp_now_send(r.mac, (uint8_t *)&resp, sizeof(resp));
      Serial.printf("[EMPAREJAMIENTO] Mando %s me busca -> le respondo (canal %u)\n",
                    macATexto(r.mac).c_str(), canalActual);

    } else if (r.msg.tipo == EVENTO) {
      if ((int32_t)r.msg.seq == ultimoSeq) { duplicados++; continue; } // reintento duplicado
      ultimoSeq = r.msg.seq;
      eventosRecibidos++;
      if (r.msg.dato == 1)  // disparo individual: lo mostramos
        Serial.printf("[EVENTO] Disparo #%u de %s  (total recibidos %lu)\n",
                      r.msg.seq, macATexto(r.mac).c_str(), eventosRecibidos);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);

#if CONECTAR_WIFI
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);   // Muchas ESP32-C3 conectan mejor con potencia reducida
  Serial.print("Conectando a la WiFi");
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 20000) { delay(300); Serial.print("."); }
  if (WiFi.status() == WL_CONNECTED) {
    canalActual = WiFi.channel();
    Serial.printf("\nConectada. Canal del router: %u\n", canalActual);
  } else {
    Serial.println("\nNO conecta: sigo sin WiFi en CANAL_SIN_WIFI");
    WiFi.disconnect(true); delay(100); WiFi.mode(WIFI_STA);
    esp_wifi_set_channel(CANAL_SIN_WIFI, WIFI_SECOND_CHAN_NONE);
    canalActual = CANAL_SIN_WIFI;
  }
#else
  WiFi.disconnect();
  esp_wifi_set_channel(CANAL_SIN_WIFI, WIFI_SECOND_CHAN_NONE);
  canalActual = CANAL_SIN_WIFI;
#endif

  if (esp_now_init() != ESP_OK) { Serial.println("Error iniciando ESP-NOW"); while (true) delay(1000); }
  esp_now_register_recv_cb(alRecibir);

  colaRx = xQueueCreate(32, sizeof(Recibido));
  xTaskCreate(tareaProceso, "proceso", 4096, nullptr, 5, nullptr);

  Serial.printf("BASE lista en el canal %u. MAC: %s\n", canalActual, WiFi.macAddress().c_str());
  Serial.println("Escribe 'e' + Enter para ver las estadísticas de eventos recibidos.");
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'e') {
      Serial.printf("[ESTADÍSTICAS] Eventos recibidos: %lu | duplicados descartados: %lu\n",
                    eventosRecibidos, duplicados);
    } else if (c == 'r') {
      eventosRecibidos = 0; duplicados = 0; ultimoSeq = -1;
      Serial.println("[ESTADÍSTICAS] Contadores a cero");
    }
  }
  delay(10);
}
