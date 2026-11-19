/*
  ============================================================================
  ETAPA 2 - PRUEBA 08: BASE -> PUERTO SERIE  (mando -> base -> PC -> Node-RED -> web)
  ============================================================================
  Recibe por ESP-NOW los paquetes de orientación de los mandos y los escribe
  por el puerto serie, una línea por paquete:

      O,<id>,<yaw>,<pitch>,<roll>,<botones>

  En el PC, puente_serie_udp.py lee estas líneas y las manda a Node-RED.
  Serie a 921600 baudios (rápido). Cada 5 s imprime una línea "#" con estadísticas
  (el puente las muestra pero no las reenvía).
  ============================================================================
*/
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

const uint8_t  CANAL   = 1;         // Igual que en MandoSimulado
const uint32_t BAUDIOS = 921600;
const uint32_t MAGIC   = 0x1A7E2026;

typedef struct __attribute__((packed)) {
  uint32_t magic;
  uint8_t  tipo;
  uint8_t  id;
  uint16_t seq;
  float    yaw, pitch, roll;
  uint8_t  botones;
} PaqueteOrientacion;

QueueHandle_t colaRx;
volatile uint32_t recibidos[3] = {0, 0, 0};
volatile uint32_t perdidos[3]  = {0, 0, 0};
int32_t ultimoSeq[3] = {-1, -1, -1};

#if ESP_ARDUINO_VERSION_MAJOR >= 3
void alRecibir(const esp_now_recv_info_t *info, const uint8_t *datos, int len) {
#else
void alRecibir(const uint8_t *mac, const uint8_t *datos, int len) {
#endif
  if (len != sizeof(PaqueteOrientacion)) return;
  PaqueteOrientacion p;
  memcpy(&p, datos, sizeof(p));
  if (p.magic != MAGIC || p.tipo != 10 || p.id < 1 || p.id > 2) return;
  xQueueSend(colaRx, &p, 0);
}

void tareaSerie(void *) {
  PaqueteOrientacion p;
  char linea[64];
  while (true) {
    if (xQueueReceive(colaRx, &p, portMAX_DELAY) != pdTRUE) continue;
    // Paquetes perdidos (huecos en la numeración)
    if (ultimoSeq[p.id] >= 0) {
      uint16_t salto = (uint16_t)(p.seq - (uint16_t)ultimoSeq[p.id]);
      if (salto > 1 && salto < 1000) perdidos[p.id] += salto - 1;
    }
    ultimoSeq[p.id] = p.seq;
    recibidos[p.id]++;
    snprintf(linea, sizeof(linea), "O,%u,%.2f,%.2f,%.2f,%u\n", p.id, p.yaw, p.pitch, p.roll, p.botones);
    Serial.print(linea);
  }
}

void setup() {
  Serial.begin(BAUDIOS);
  delay(1500);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.setSleep(false);
  esp_wifi_set_channel(CANAL, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) { Serial.println("# Error iniciando ESP-NOW"); while (true) delay(1000); }
  esp_now_register_recv_cb(alRecibir);

  colaRx = xQueueCreate(32, sizeof(PaqueteOrientacion));
  xTaskCreate(tareaSerie, "serie", 4096, nullptr, 5, nullptr);
  Serial.printf("# BASE SERIE lista en el canal %u\n", CANAL);
}

void loop() {
  delay(5000);
  Serial.printf("# mando1: %lu paquetes (%lu perdidos) | mando2: %lu paquetes (%lu perdidos) en 5 s\n",
                recibidos[1], perdidos[1], recibidos[2], perdidos[2]);
  for (int i = 1; i <= 2; i++) { recibidos[i] = 0; perdidos[i] = 0; }
}
