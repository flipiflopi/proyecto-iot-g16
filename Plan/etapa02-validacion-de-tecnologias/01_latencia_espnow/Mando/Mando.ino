/*
  ============================================================================
  PRUEBA DE LATENCIA ESP-NOW  -  PLACA "MANDO"  (la que mide)
  ============================================================================
  Envía paquetes "ping" a 100 Hz por ESP-NOW (como haría el mando real con la
  orientación de la IMU) y mide cuánto tardan en volver ("pong") desde la BASE.
  Al terminar cada ensayo muestra: paquetes perdidos, RTT (ida y vuelta) y la
  latencia estimada solo de ida (RTT / 2).

  Monitor serie a 115200 baudios. Pulsa Enter para lanzar cada ensayo.
  Funciona con ESP32 clásico y ESP32-C3, core Arduino-ESP32 2.x y 3.x.
  ============================================================================
*/
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <algorithm>

// ---------------- CONFIGURACIÓN ----------------
const uint8_t  CANAL      = 1;          // Canal WiFi. DEBE coincidir con la BASE.
                                        // Si la BASE se conecta a una WiFi, poned aquí
                                        // el canal que ella imprime al arrancar.
const uint32_t PERIODO_MS = 10;         // 10 ms = 100 Hz (frecuencia del mando real)
const uint16_t N_PAQUETES = 1000;       // Paquetes por ensayo (10 s a 100 Hz)
const uint32_t MAGIC      = 0x1A7E2026; // Firma para ignorar paquetes de otros grupos

uint8_t BROADCAST[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Envío a todos: no hace falta la MAC

// Paquete del mismo tamaño que el real (cuaternión de orientación = 4 floats)
typedef struct __attribute__((packed)) {
  uint32_t magic;
  uint8_t  tipo;   // 0 = ping (mando -> base), 1 = pong (base -> mando)
  uint16_t seq;    // Número de paquete
  uint32_t t_us;   // micros() del mando al enviar (la base lo devuelve sin tocar)
  float    q[4];   // Relleno: simula la orientación
} Paquete;

volatile uint32_t rtt_us[N_PAQUETES];  // RTT medido de cada paquete
volatile bool     llegado[N_PAQUETES]; // ¿Ha vuelto este paquete?
uint32_t erroresEnvio = 0;

// ---------- Callback de recepción (la firma cambia entre core 2.x y 3.x) ----------
#if ESP_ARDUINO_VERSION_MAJOR >= 3
void alRecibir(const esp_now_recv_info_t *info, const uint8_t *datos, int len) {
#else
void alRecibir(const uint8_t *mac, const uint8_t *datos, int len) {
#endif
  uint32_t ahora = micros();               // Lo primero: tomar la hora de llegada
  if (len != sizeof(Paquete)) return;
  Paquete p;
  memcpy(&p, datos, sizeof(p));
  if (p.magic != MAGIC || p.tipo != 1 || p.seq >= N_PAQUETES) return;
  rtt_us[p.seq]  = ahora - p.t_us;
  llegado[p.seq] = true;
}

// ---------- Un ensayo completo ----------
void ensayo() {
  for (uint16_t i = 0; i < N_PAQUETES; i++) llegado[i] = false;
  erroresEnvio = 0;

  Serial.printf("\nEnviando %u paquetes a %lu Hz por el canal %u (tarda ~%lu s)...\n",
                N_PAQUETES, 1000UL / PERIODO_MS, CANAL,
                (unsigned long)(N_PAQUETES * PERIODO_MS / 1000));

  Paquete p = {};
  p.magic = MAGIC;
  p.tipo  = 0;
  uint32_t siguiente = millis();

  for (uint16_t i = 0; i < N_PAQUETES; i++) {
    while ((int32_t)(millis() - siguiente) < 0) delay(1); // Espera al siguiente periodo
    siguiente += PERIODO_MS;
    p.seq  = i;
    p.t_us = micros();
    if (esp_now_send(BROADCAST, (uint8_t *)&p, sizeof(p)) != ESP_OK) erroresEnvio++;
  }
  delay(500); // Margen para que vuelvan los últimos

  // ----- Estadísticas -----
  static uint32_t v[N_PAQUETES];
  uint16_t n = 0;
  for (uint16_t i = 0; i < N_PAQUETES; i++) if (llegado[i]) v[n++] = rtt_us[i];

  if (n == 0) {
    Serial.println("No ha vuelto NINGÚN paquete. Revisa: BASE encendida, mismo CANAL,");
    Serial.println("y en MODO 1 que el script eco_pc.py esté funcionando.");
    return;
  }
  std::sort(v, v + n);
  uint64_t suma = 0;
  for (uint16_t i = 0; i < n; i++) suma += v[i];
  auto ms  = [](uint32_t us) { return us / 1000.0f; };
  auto pct = [&](int p) { return v[(uint32_t)(n - 1) * p / 100]; };
  float media = (suma / (float)n) / 1000.0f;

  Serial.println("--------------------------------------------------------------");
  Serial.printf("Recibidos: %u/%u  (perdidos %.1f %%, errores de envío %lu)\n",
                n, N_PAQUETES, 100.0f * (N_PAQUETES - n) / N_PAQUETES, erroresEnvio);
  Serial.printf("RTT ida+vuelta [ms]: min %.2f | media %.2f | p50 %.2f | p95 %.2f | p99 %.2f | max %.2f\n",
                ms(v[0]), media, ms(pct(50)), ms(pct(95)), ms(pct(99)), ms(v[n - 1]));
  Serial.printf(">> LATENCIA SOLO IDA (RTT/2): media %.2f ms | p95 %.2f ms | p99 %.2f ms\n",
                media / 2, ms(pct(95)) / 2, ms(pct(99)) / 2);
  Serial.println("--------------------------------------------------------------");
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.setSleep(false);                                // El ahorro de energía añade latencia
  esp_wifi_set_channel(CANAL, WIFI_SECOND_CHAN_NONE);  // Fija el canal

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error iniciando ESP-NOW");
    while (true) delay(1000);
  }
  esp_now_register_recv_cb(alRecibir);

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, BROADCAST, 6);
  peer.channel = 0;      // 0 = el canal actual
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  Serial.printf("\nMANDO listo. MAC: %s  Canal: %u\n", WiFi.macAddress().c_str(), CANAL);
}

void loop() {
  Serial.println("\nPulsa Enter para lanzar un ensayo...");
  while (!Serial.available()) delay(10);
  while (Serial.available()) Serial.read(); // Vacía lo escrito
  ensayo();
}
