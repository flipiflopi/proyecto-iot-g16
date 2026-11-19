/*
  ============================================================================
  PRUEBA DE LATENCIA ESP-NOW  -  PLACA "BASE"  (la que devuelve)
  ============================================================================
  MODO 0: devuelve cada ping directamente por ESP-NOW  -> mide solo la radio.
  MODO 1: manda cada ping al PC por USB, el PC (eco_pc.py) lo devuelve y la
          base lo reenvía al mando -> mide ESP-NOW + USB + PC (cadena real).

  CONECTAR_WIFI 1: además se conecta a una WiFi (p. ej. el hotspot del móvil),
          como hará la base real para MQTT. Así se prueba ESP-NOW + WiFi a la vez.
          El canal lo impone el router: la base lo imprime al arrancar y hay que
          poner ese mismo CANAL en el MANDO.

  Usa una cola de FreeRTOS: el callback de recepción no envía nada, solo encola,
  y una tarea hace el trabajo (igual que se hará en el proyecto).

  ¡NO subáis a GitHub vuestra SSID/contraseña reales! Dejad los marcadores.
  ============================================================================
*/
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

// ---------------- CONFIGURACIÓN ----------------
#define MODO          0             // 0 = eco directo, 1 = vía PC (con eco_pc.py)
#define CONECTAR_WIFI 0             // 0 = solo ESP-NOW, 1 = también conectada a una WiFi

const uint8_t  CANAL   = 1;         // Canal si NO se conecta a WiFi (igual que en el MANDO)
const char*    SSID    = "NOMBRE_DE_LA_WIFI";  // Solo si CONECTAR_WIFI = 1 (2,4 GHz, WPA2)
const char*    PASS    = "CLAVE_DE_LA_WIFI";   // (eduroam NO sirve: usad el hotspot del móvil)
const uint32_t BAUDIOS = 921600;    // Serie rápida para el MODO 1 (debe coincidir con eco_pc.py)
const uint32_t MAGIC   = 0x1A7E2026;

uint8_t BROADCAST[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct __attribute__((packed)) {
  uint32_t magic;
  uint8_t  tipo;   // 0 = ping, 1 = pong
  uint16_t seq;
  uint32_t t_us;
  float    q[4];
} Paquete;

QueueHandle_t colaPings; // Cola: callback de recepción -> tarea de eco

// ---------- Callback de recepción (firma distinta en core 2.x y 3.x) ----------
#if ESP_ARDUINO_VERSION_MAJOR >= 3
void alRecibir(const esp_now_recv_info_t *info, const uint8_t *datos, int len) {
#else
void alRecibir(const uint8_t *mac, const uint8_t *datos, int len) {
#endif
  if (len != sizeof(Paquete)) return;
  Paquete p;
  memcpy(&p, datos, sizeof(p));
  if (p.magic != MAGIC || p.tipo != 0) return;
  xQueueSend(colaPings, &p, 0);  // Rápido: no bloquear la tarea de la WiFi
}

void enviarPong(uint16_t seq, uint32_t t_us) {
  Paquete r = {};
  r.magic = MAGIC;
  r.tipo  = 1;
  r.seq   = seq;
  r.t_us  = t_us; // Hora ORIGINAL del mando: no hace falta sincronizar relojes
  esp_now_send(BROADCAST, (uint8_t *)&r, sizeof(r));
}

// ---------- Tarea: saca pings de la cola ----------
void tareaEco(void *) {
  Paquete p;
  while (true) {
    if (xQueueReceive(colaPings, &p, portMAX_DELAY) == pdTRUE) {
#if MODO == 0
      enviarPong(p.seq, p.t_us);                                   // Eco directo
#else
      Serial.printf("P,%u,%lu\n", p.seq, (unsigned long)p.t_us);   // Al PC
#endif
    }
  }
}

#if MODO == 1
// ---------- Tarea: lee lo que devuelve el PC ("P,seq,t_us\n") y lo reenvía ----------
void tareaSerie(void *) {
  char buf[40];
  uint8_t n = 0;
  while (true) {
    while (Serial.available()) {
      char c = Serial.read();
      if (c == '\n') {
        buf[n] = '\0';
        unsigned int seq;
        unsigned long t;
        if (sscanf(buf, "P,%u,%lu", &seq, &t) == 2) enviarPong(seq, t);
        n = 0;
      } else if (n < sizeof(buf) - 1) {
        buf[n++] = c;
      }
    }
    vTaskDelay(1); // Cede la CPU un tick
  }
}
#endif

void setup() {
  Serial.begin(BAUDIOS);
  delay(1500);

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false); // El ahorro de energía añade latencia

#if CONECTAR_WIFI
  // 1) Diagnóstico: ¿se ve la red? ¿en qué canal y con cuánta señal?
  Serial.println("Buscando redes WiFi...");
  int nRedes = WiFi.scanNetworks();
  bool vista = false;
  for (int i = 0; i < nRedes; i++) {
    bool esLaNuestra = (WiFi.SSID(i) == SSID);
    vista |= esLaNuestra;
    if (esLaNuestra || i < 10)
      Serial.printf("  %s%-24s canal %2d  señal %4d dBm\n", esLaNuestra ? ">> " : "   ",
                    WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i));
  }
  if (!vista) Serial.printf("AVISO: no se ve la red \"%s\" (¿nombre mal escrito o solo 5 GHz?)\n", SSID);
  WiFi.scanDelete();

  // 2) Conexión. Potencia de emisión reducida: muchas placas ESP32-C3 baratas
  //    (p. ej. SuperMini) tienen la antena mal diseñada y NO conectan a máxima potencia.
  WiFi.begin(SSID, PASS);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  Serial.println("Conectando a la WiFi (máx. 20 s)...");
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 20000) delay(300);

  if (WiFi.status() != WL_CONNECTED) {
    int st = WiFi.status();
    Serial.printf("NO conecta. Estado = %d (%s)\n", st,
                  st == WL_NO_SSID_AVAIL   ? "no encuentra la red" :
                  st == WL_CONNECT_FAILED  ? "contraseña incorrecta o rechazada" :
                  st == WL_DISCONNECTED    ? "desconectada / sin respuesta del router" : "otro");
    Serial.println("La prueba sigue SIN WiFi en el canal fijo CANAL.");
    WiFi.disconnect(true);
    delay(100);
    WiFi.mode(WIFI_STA);
    esp_wifi_set_channel(CANAL, WIFI_SECOND_CHAN_NONE);
  } else {
    Serial.printf("Conectada. IP %s  señal %d dBm. CANAL del router = %d  -> pon CANAL = %d en el MANDO\n",
                  WiFi.localIP().toString().c_str(), WiFi.RSSI(), WiFi.channel(), WiFi.channel());
  }
#else
  WiFi.disconnect();
  esp_wifi_set_channel(CANAL, WIFI_SECOND_CHAN_NONE);
#endif

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error iniciando ESP-NOW");
    while (true) delay(1000);
  }
  esp_now_register_recv_cb(alRecibir);

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, BROADCAST, 6);
  peer.channel = 0;   // 0 = el canal actual
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  colaPings = xQueueCreate(32, sizeof(Paquete));
  xTaskCreate(tareaEco, "eco", 4096, nullptr, 5, nullptr);
#if MODO == 1
  xTaskCreate(tareaSerie, "serie", 4096, nullptr, 5, nullptr);
#endif

  Serial.printf("BASE lista (MODO %d, WiFi %s). MAC: %s\n",
                MODO, CONECTAR_WIFI ? "SI" : "NO", WiFi.macAddress().c_str());
}

void loop() {
  vTaskDelay(portMAX_DELAY); // Todo lo hacen las tareas
}
