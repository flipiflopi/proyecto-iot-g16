/*
  ============================================================================
  ETAPA 2 - PRUEBA 08: MANDO SIMULADO  (mando -> base -> PC -> Node-RED -> web)
  ============================================================================
  Envía por ESP-NOW (broadcast, canal CANAL) a 50 Hz una orientación FALSA:
   - ID_MANDO 1 (cazador): el punto describe un círculo
   - ID_MANDO 2 (pájaro) : balanceo de alabeo (roll) y cabeceo (pitch)
  Al pulsar BOOT envía "disparo" (bit en el campo botones).

  Es exactamente el formato de datos que enviará el mando real; cuando lleguen
  las IMU (MPU6050 en los dos mandos) solo cambia de dónde salen yaw/pitch/roll.

  Monitor serie a 115200.
  ============================================================================
*/
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

// ---------------- CONFIGURACIÓN ----------------
#define ID_MANDO 1                  // 1 = cazador, 2 = pájaro (cambiadlo en la segunda placa)
const uint8_t  CANAL      = 1;      // Igual que en BaseSerie
const uint32_t PERIODO_MS = 20;     // 50 Hz
const int      PIN_BOTON  = 9;      // BOOT: GPIO9 en ESP32-C3 (0 en ESP32 clásico)
const uint32_t MAGIC      = 0x1A7E2026;

uint8_t BROADCAST[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// ---------------- Paquete (igual en BaseSerie) ----------------
typedef struct __attribute__((packed)) {
  uint32_t magic;
  uint8_t  tipo;      // 10 = orientación
  uint8_t  id;        // 1 cazador, 2 pájaro
  uint16_t seq;
  float    yaw, pitch, roll;   // grados
  uint8_t  botones;   // bit0 = gatillo / turbo pulsado
} PaqueteOrientacion;

void setup() {
  Serial.begin(115200);
  delay(1500);
  pinMode(PIN_BOTON, INPUT_PULLUP);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.setSleep(false);
  esp_wifi_set_channel(CANAL, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) { Serial.println("Error iniciando ESP-NOW"); while (true) delay(1000); }
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, BROADCAST, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  Serial.printf("MANDO SIMULADO %d (%s) en el canal %u\n", ID_MANDO,
                ID_MANDO == 1 ? "cazador" : "pájaro", CANAL);
}

void loop() {
  static uint32_t siguiente = millis();
  static uint16_t seq = 0;
  static float t = 0;

  while ((int32_t)(millis() - siguiente) < 0) delay(1);
  siguiente += PERIODO_MS;
  t += PERIODO_MS / 1000.0f;

  PaqueteOrientacion p = {};
  p.magic = MAGIC;
  p.tipo  = 10;
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

  esp_now_send(BROADCAST, (uint8_t *)&p, sizeof(p));
}
