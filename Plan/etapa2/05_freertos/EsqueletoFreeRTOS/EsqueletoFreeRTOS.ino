/*
  ============================================================================
  ETAPA 2 - PRUEBA 05: ESQUELETO FreeRTOS DE UN MANDO
  ============================================================================
  Estructura que tendrá el firmware real, con el sensor SIMULADO:

   tareaSensor (prio 3, 100 Hz)  --colaOrientacion (buzón: solo el último dato)-->
   tareaComunicaciones (prio 2, 50 Hz)   (en el proyecto: enviará por ESP-NOW)

   tareaBoton (prio 3, 200 Hz) --colaEventos--> tareaEventos (prio 4)
   temporizadorLatido (software, cada 2 s) --colaEventos--> tareaEventos

   tareaMonitor (prio 1, cada 10 s): uso de pila de cada tarea y memoria libre

  Sincronización usada:
   - Colas (xQueueOverwrite / xQueuePeek como "buzón", xQueueSend / xQueueReceive)
   - Temporizador software (xTimerCreate)
   - Mutex (xSemaphoreCreateMutex) para que dos tareas no escriban a la vez en el Serial
   - vTaskDelayUntil para periodos exactos

  Monitor serie a 115200.
  ============================================================================
*/
#include <Arduino.h>

const int PIN_BOTON = 9;   // BOOT: GPIO9 en ESP32-C3 (0 en ESP32 clásico)

// ---------------- Tipos de datos ----------------
struct Orientacion {
  float yaw, pitch, roll;   // grados
  uint32_t t_ms;            // cuándo se midió
};

enum TipoEvento : uint8_t { EV_DISPARO, EV_SOLTAR, EV_LATIDO };
struct Evento {
  TipoEvento tipo;
  uint32_t   t_ms;
};

// ---------------- Objetos de FreeRTOS ----------------
QueueHandle_t     colaOrientacion;   // longitud 1: siempre el dato más reciente
QueueHandle_t     colaEventos;       // eventos discretos (no se pueden perder)
SemaphoreHandle_t mutexSerie;
TimerHandle_t     temporizadorLatido;
TaskHandle_t      hSensor, hComs, hBoton, hEventos, hMonitor;

volatile uint32_t muestrasSensor = 0, enviosComs = 0;

// printf protegido por mutex
void registrar(const char *fmt, ...) {
  char buf[160];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  if (xSemaphoreTake(mutexSerie, pdMS_TO_TICKS(50)) == pdTRUE) {
    Serial.print(buf);
    xSemaphoreGive(mutexSerie);
  }
}

// ---------------- Tareas ----------------
// Sensor simulado: describe un círculo (como si la mano apuntara en círculos)
void tareaSensor(void *) {
  TickType_t ultimo = xTaskGetTickCount();
  float ang = 0;
  while (true) {
    Orientacion o;
    ang += 0.02f;
    o.yaw   = 20.0f * cosf(ang);
    o.pitch = 10.0f * sinf(ang);
    o.roll  = 0;
    o.t_ms  = millis();
    xQueueOverwrite(colaOrientacion, &o);   // sustituye el dato anterior
    muestrasSensor++;
    vTaskDelayUntil(&ultimo, pdMS_TO_TICKS(10));   // 100 Hz exactos
  }
}

// Comunicaciones: coge el último dato y lo "envía"
void tareaComunicaciones(void *) {
  TickType_t ultimo = xTaskGetTickCount();
  uint32_t tImpresion = 0;
  while (true) {
    Orientacion o;
    if (xQueuePeek(colaOrientacion, &o, 0) == pdTRUE) {
      enviosComs++;   // aquí irá esp_now_send(...)
      if (millis() - tImpresion >= 1000) {
        tImpresion = millis();
        registrar("[COMS] yaw %6.1f  pitch %6.1f  (edad del dato %lu ms) | sensor %lu Hz, envíos %lu Hz\n",
            o.yaw, o.pitch, millis() - o.t_ms, muestrasSensor, enviosComs);
        muestrasSensor = 0;
        enviosComs = 0;
      }
    }
    vTaskDelayUntil(&ultimo, pdMS_TO_TICKS(20));   // 50 Hz
  }
}

// Botón: lee cada 5 ms con antirrebote y genera eventos
void tareaBoton(void *) {
  bool estable = HIGH, anterior = HIGH;
  uint32_t tCambio = 0;
  while (true) {
    bool lectura = digitalRead(PIN_BOTON);
    if (lectura != anterior) { anterior = lectura; tCambio = millis(); }
    if (lectura != estable && millis() - tCambio >= 30) {   // estable 30 ms
      estable = lectura;
      Evento e = { estable == LOW ? EV_DISPARO : EV_SOLTAR, millis() };
      xQueueSend(colaEventos, &e, 0);
    }
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

// Procesa los eventos en orden (prioridad alta: reacción inmediata)
void tareaEventos(void *) {
  Evento e;
  while (true) {
    if (xQueueReceive(colaEventos, &e, portMAX_DELAY) == pdTRUE) {
      switch (e.tipo) {
        case EV_DISPARO: registrar("[EVENTO] DISPARO  (latencia de reacción %lu ms)\n", millis() - e.t_ms); break;
        case EV_SOLTAR:  registrar("[EVENTO] botón soltado\n"); break;
        case EV_LATIDO:  registrar("[EVENTO] latido (temporizador software) t=%lu s\n", e.t_ms / 1000); break;
      }
    }
  }
}

// Temporizador software: se ejecuta en la tarea de temporizadores de FreeRTOS
void alLatido(TimerHandle_t) {
  Evento e = { EV_LATIDO, millis() };
  xQueueSend(colaEventos, &e, 0);   // nunca bloquear dentro de un temporizador
}

// Monitor: cuánta pila le sobra a cada tarea (para dimensionarlas en la memoria)
void tareaMonitor(void *) {
  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
    registrar("[MONITOR] pila libre (palabras): sensor %u | coms %u | boton %u | eventos %u | heap libre %u B\n",
        uxTaskGetStackHighWaterMark(hSensor), uxTaskGetStackHighWaterMark(hComs),
        uxTaskGetStackHighWaterMark(hBoton), uxTaskGetStackHighWaterMark(hEventos),
        ESP.getFreeHeap());
  }
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  pinMode(PIN_BOTON, INPUT_PULLUP);

  mutexSerie      = xSemaphoreCreateMutex();
  colaOrientacion = xQueueCreate(1, sizeof(Orientacion));
  colaEventos     = xQueueCreate(16, sizeof(Evento));

  xTaskCreate(tareaSensor,         "sensor",  3072, nullptr, 3, &hSensor);
  xTaskCreate(tareaComunicaciones, "coms",    4096, nullptr, 2, &hComs);
  xTaskCreate(tareaBoton,          "boton",   2048, nullptr, 3, &hBoton);
  xTaskCreate(tareaEventos,        "eventos", 4096, nullptr, 4, &hEventos);
  xTaskCreate(tareaMonitor,        "monitor", 3072, nullptr, 1, &hMonitor);

  temporizadorLatido = xTimerCreate("latido", pdMS_TO_TICKS(2000), pdTRUE, nullptr, alLatido);
  xTimerStart(temporizadorLatido, 0);

  registrar("\n=== Esqueleto FreeRTOS en marcha. Pulsa BOOT para generar disparos ===\n");
}

void loop() {
  vTaskDelete(nullptr);   // loop() no se usa: todo lo hacen las tareas
}
