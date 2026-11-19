/*
  ============================================================================
  ETAPA 2 - PRUEBA 02: EMPAREJAMIENTO ESP-NOW  -  PLACA "MANDO"
  ============================================================================
  1) Al arrancar busca la base canal por canal (1..13) enviando BUSCAR_BASE.
     Cuando la base responde, guarda su MAC y su canal.
  2) Cada pulsación del botón BOOT envía un "disparo" UNICAST a la base.
     ESP-NOW confirma la entrega (ACK) y reintenta automáticamente.
  3) Escribiendo 'r' + Enter lanza una ráfaga de 1000 eventos a 100 Hz y
     muestra cuántos fueron confirmados.
  Si fallan muchos envíos seguidos (p. ej. la base cambió de canal), vuelve a buscarla.

  Monitor serie a 115200.
  ============================================================================
*/
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_idf_version.h>

// ---------------- CONFIGURACIÓN ----------------
const int      PIN_BOTON  = 9;     // Botón BOOT: GPIO9 en ESP32-C3 (en ESP32 clásico es el 0)
const uint32_t MAGIC      = 0x1A7E2026;
const uint16_t N_RAFAGA   = 1000;  // Eventos de la ráfaga de prueba
const uint32_t PERIODO_RAFAGA_MS = 10;
const uint8_t  FALLOS_PARA_REBUSCAR = 10;

uint8_t BROADCAST[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// ---------------- PROTOCOLO (igual en base y mando) ----------------
enum Tipo : uint8_t { BUSCAR_BASE = 1, SOY_BASE = 2, EVENTO = 3 };

typedef struct __attribute__((packed)) {
  uint32_t magic;
  uint8_t  tipo;
  uint8_t  canal;
  uint16_t seq;
  uint32_t dato;
} Msg;

// ---------------- ESTADO ----------------
volatile bool baseEncontrada = false;
uint8_t macBase[6];
volatile uint8_t canalBase = 0;

volatile uint32_t enviosOK = 0, enviosFallo = 0;
volatile uint8_t  fallosSeguidos = 0;
uint16_t seq = 0;

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
  Serial.printf("Base encontrada: %s en el canal %u\n", macATexto(macBase).c_str(), canalBase);
  Serial.println("Pulsa BOOT para disparar, o escribe 'r' + Enter para una ráfaga de prueba.");
}

bool enviarEvento(uint32_t dato) {
  Msg m = {};
  m.magic = MAGIC;
  m.tipo  = EVENTO;
  m.seq   = seq++;
  m.dato  = dato;
  return esp_now_send(macBase, (uint8_t *)&m, sizeof(m)) == ESP_OK;
}

void rafaga() {
  enviosOK = 0; enviosFallo = 0;
  uint32_t errLocal = 0;
  Serial.printf("\nRáfaga de %u eventos unicast a %lu Hz...\n", N_RAFAGA, 1000UL / PERIODO_RAFAGA_MS);
  uint32_t siguiente = millis();
  for (uint16_t i = 0; i < N_RAFAGA; i++) {
    while ((int32_t)(millis() - siguiente) < 0) delay(1);
    siguiente += PERIODO_RAFAGA_MS;
    if (!enviarEvento(2)) errLocal++;
  }
  delay(300);
  Serial.println("--------------------------------------------------");
  Serial.printf("Confirmados (ACK): %lu | Fallidos tras reintentos: %lu | No enviados: %lu\n",
                enviosOK, enviosFallo, errLocal);
  Serial.printf("Entrega confirmada: %.1f %%\n", 100.0f * enviosOK / N_RAFAGA);
  Serial.println("En la BASE escribe 'e' para ver cuántos le llegaron.");
  Serial.println("--------------------------------------------------");
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

  Serial.printf("MANDO. MAC: %s\n", WiFi.macAddress().c_str());
  buscarBase();
}

void loop() {
  // --- Botón BOOT con antirrebote sencillo: un disparo por pulsación ---
  static bool anterior = HIGH;
  static uint32_t tCambio = 0;
  bool actual = digitalRead(PIN_BOTON);
  if (actual != anterior && millis() - tCambio > 30) {
    tCambio = millis();
    anterior = actual;
    if (actual == LOW) {                 // pulsado
      uint32_t okAntes = enviosOK, falloAntes = enviosFallo;
      enviarEvento(1);
      uint32_t t0 = millis();
      while (enviosOK == okAntes && enviosFallo == falloAntes && millis() - t0 < 100) delay(1);
      Serial.printf("Disparo #%u -> %s\n", seq - 1,
                    enviosOK > okAntes ? "CONFIRMADO por la base" : "FALLO (sin ACK)");
    }
  }

  // --- Comandos por serie ---
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'r') rafaga();
    if (c == 'b') buscarBase();
  }

  // --- Si la base deja de responder, volver a buscarla ---
  if (fallosSeguidos >= FALLOS_PARA_REBUSCAR) {
    Serial.println("\nLa base no responde (¿apagada o cambió de canal?)");
    buscarBase();
  }
  delay(2);
}
