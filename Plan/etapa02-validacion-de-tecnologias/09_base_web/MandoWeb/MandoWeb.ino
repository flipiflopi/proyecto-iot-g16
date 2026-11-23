/*
  ============================================================================
  ETAPA 2 - PRUEBA 09: MANDO SIMULADO QUE BUSCA LA BASE
  ============================================================================
  Es el MandoSimulado de la prueba 08 (orientación falsa a 50 Hz + BOOT = disparo)
  con el emparejamiento de la prueba 02:
   1) Al arrancar busca la base canal por canal (1..13) con BUSCAR_BASE.
      La base está en el canal de la WiFi (hotspot), que el mando no conoce.
   2) Envía la orientación UNICAST a la MAC de la base (con ACK).
   3) Si fallan muchos envíos seguidos (base apagada o cambió de canal), la vuelve a buscar.

  Monitor serie a 115200. Cada 5 s muestra envíos confirmados y fallidos.
  'b' + Enter: forzar una nueva búsqueda.
  ============================================================================
*/
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_idf_version.h>

// ---------------- CONFIGURACIÓN ----------------
#define ID_MANDO 1                       // 1 = cazador, 2 = pájaro (cambiadlo en la segunda placa)
const uint32_t PERIODO_MS = 20;          // 50 Hz
const int      PIN_BOTON  = 9;           // BOOT: GPIO9 en ESP32-C3 (0 en ESP32 clásico)
const uint32_t MAGIC      = 0x1A7E2026;
const uint8_t  FALLOS_PARA_REBUSCAR = 25; // 25 envíos seguidos sin ACK = 0,5 s sin base

uint8_t BROADCAST[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// ---------------- PROTOCOLO (igual en BaseWeb) ----------------
enum Tipo : uint8_t { BUSCAR_BASE = 1, SOY_BASE = 2, ORIENTACION = 10 };
typedef struct __attribute__((packed)) {
  uint32_t magic;
  uint8_t  tipo;
  uint8_t  canal;
  uint16_t seq;
  uint32_t dato;
} Msg;

typedef struct __attribute__((packed)) {
  uint32_t magic;
  uint8_t  tipo;      // 10
  uint8_t  id;        // 1 cazador, 2 pájaro
  uint16_t seq;
  float    yaw, pitch, roll;   // grados
  uint8_t  botones;   // bit0 = gatillo / turbo pulsado
} PaqueteOrientacion;

// ---------------- ESTADO ----------------
volatile bool baseEncontrada = false;
uint8_t macBase[6];
volatile uint8_t canalBase = 0;
volatile uint32_t enviosOK = 0, enviosFallo = 0;
volatile uint8_t  fallosSeguidos = 0;

// ---------- Callbacks ----------
#if ESP_ARDUINO_VERSION_MAJOR >= 3
void alRecibir(const esp_now_recv_info_t *info, const uint8_t *datos, int len) {
  const uint8_t *mac = info->src_addr;
#else
void alRecibir(const uint8_t *mac, const uint8_t *datos, int len) {
#endif
  if (len != sizeof(Msg)) return;
  Msg m;
  memcpy(&m, datos, sizeof(m));
  if (m.magic != MAGIC || m.tipo != SOY_BASE || baseEncontrada) return;
  memcpy(macBase, mac, 6);
  canalBase = m.canal;
  baseEncontrada = true;
}

// Resultado de cada envío UNICAST (la firma cambió en ESP-IDF 5.5 / core 3.3)
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
void alEnviar(const esp_now_send_info_t *info, esp_now_send_status_t estado) {
#else
void alEnviar(const uint8_t *mac, esp_now_send_status_t estado) {
#endif
  if (estado == ESP_NOW_SEND_SUCCESS) { enviosOK++; fallosSeguidos = 0; }
  else                                { enviosFallo++; if (fallosSeguidos < 255) fallosSeguidos++; }
}

String macATexto(const uint8_t *m) {
  char b[18];
  snprintf(b, sizeof(b), "%02X:%02X:%02X:%02X:%02X:%02X", m[0], m[1], m[2], m[3], m[4], m[5]);
  return String(b);
}

void anadirPeer(const uint8_t *mac) {
  if (esp_now_is_peer_exist(mac)) return;
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, mac, 6);
  peer.channel = 0;   // canal actual
  peer.encrypt = false;
  esp_now_add_peer(&peer);
}

// ---------- Búsqueda de la base canal por canal ----------
void buscarBase() {
  baseEncontrada = false;
  if (esp_now_is_peer_exist(macBase)) esp_now_del_peer(macBase);
  Serial.println("\nBuscando la base...");

  Msg m = {};
  m.magic = MAGIC;
  m.tipo  = BUSCAR_BASE;

  while (!baseEncontrada) {
    for (uint8_t ch = 1; ch <= 13 && !baseEncontrada; ch++) {
      esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
      Serial.printf("  canal %2u...", ch);
      for (int intento = 0; intento < 3 && !baseEncontrada; intento++) {
        esp_now_send(BROADCAST, (uint8_t *)&m, sizeof(m));
        delay(40);
      }
      Serial.println(baseEncontrada ? " ¡RESPUESTA!" : "");
    }
  }

  esp_wifi_set_channel(canalBase, WIFI_SECOND_CHAN_NONE);
  anadirPeer(macBase);
  fallosSeguidos = 0;
  Serial.printf("Base encontrada: %s en el canal %u. Enviando a 50 Hz.\n", macATexto(macBase).c_str(), canalBase);
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  pinMode(PIN_BOTON, INPUT_PULLUP);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.setSleep(false);

  if (esp_now_init() != ESP_OK) { Serial.println("Error iniciando ESP-NOW"); while (true) delay(1000); }
  esp_now_register_recv_cb(alRecibir);
  esp_now_register_send_cb(alEnviar);

  esp_now_peer_info_t peer = {};          // Peer broadcast para la búsqueda
  memcpy(peer.peer_addr, BROADCAST, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  Serial.printf("MANDO WEB %d (%s). MAC: %s\n", ID_MANDO, ID_MANDO == 1 ? "cazador" : "pájaro",
                WiFi.macAddress().c_str());
  buscarBase();
}

void loop() {
  static uint32_t siguiente = millis();
  static uint32_t tStats = millis();
  static uint16_t seq = 0;
  static float t = 0;

  // --- Si la base deja de responder, volver a buscarla ---
  if (fallosSeguidos >= FALLOS_PARA_REBUSCAR) {
    Serial.println("\nLa base no responde (¿apagada o cambió de canal?)");
    buscarBase();
    siguiente = millis();
  }
  if (Serial.available() && Serial.read() == 'b') { buscarBase(); siguiente = millis(); }

  // --- 50 Hz ---
  while ((int32_t)(millis() - siguiente) < 0) delay(1);
  siguiente += PERIODO_MS;
  t += PERIODO_MS / 1000.0f;

  PaqueteOrientacion p = {};
  p.magic = MAGIC;
  p.tipo  = ORIENTACION;
  p.id    = ID_MANDO;
  p.seq   = seq++;
#if ID_MANDO == 1
  p.yaw   = 25.0f * cosf(t * 0.8f);     // círculo
  p.pitch = 15.0f * sinf(t * 0.8f);
  p.roll  = 0;
#else
  p.yaw   = 0;
  p.pitch = 12.0f * sinf(t * 0.5f);     // sube y baja
  p.roll  = 35.0f * sinf(t * 0.9f);     // se inclina a los lados
#endif
  p.botones = (digitalRead(PIN_BOTON) == LOW) ? 1 : 0;
  esp_now_send(macBase, (uint8_t *)&p, sizeof(p));

  // --- Estadísticas cada 5 s ---
  if (millis() - tStats >= 5000) {
    tStats += 5000;
    Serial.printf("5 s: %lu confirmados, %lu sin ACK (canal %u)\n", enviosOK, enviosFallo, canalBase);
    enviosOK = 0; enviosFallo = 0;
  }
}
