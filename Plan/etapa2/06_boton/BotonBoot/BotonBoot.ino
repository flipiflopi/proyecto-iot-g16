/*
  ============================================================================
  ETAPA 2 - PRUEBA 06: BOTÓN BOOT - PULSACIÓN SIMPLE, DOBLE Y LARGA
  ============================================================================
  Lo que pide el enunciado para el botón BOOT (GPIO9, INPUT_PULLUP):
   - Pulsación SIMPLE : el LED alterna entre apagado y su estado anterior
   - Pulsación DOBLE  : el LED al máximo conservando el color
   - Pulsación LARGA  : comprobar actualización (FOTA). Aquí solo se simula
   - Antirrebote por software

  Funcionamiento (máquina de estados en una tarea FreeRTOS que muestrea cada 5 ms):
   - Antirrebote: un cambio solo cuenta si la lectura se mantiene 30 ms.
   - LARGA: se detecta en cuanto se llevan 1000 ms pulsado (sin esperar a soltar).
   - DOBLE: segunda pulsación antes de 300 ms desde que se soltó la primera.
   - SIMPLE: se confirma cuando pasan 300 ms sin segunda pulsación.
  Muestra también cuántos rebotes se han filtrado.

  Monitor serie a 115200.
  ============================================================================
*/
#include <Arduino.h>

// ---------------- CONFIGURACIÓN ----------------
const int      PIN_BOTON      = 9;    // BOOT: GPIO9 en ESP32-C3 (0 en ESP32 clásico)
const uint32_t ANTIRREBOTE_MS = 30;
const uint32_t LARGA_MS       = 1000;
const uint32_t DOBLE_MS       = 300;

enum TipoPulsacion : uint8_t { SIMPLE, DOBLE, LARGA };
QueueHandle_t colaPulsaciones;
volatile uint32_t rebotesFiltrados = 0;

// ---------- LED RGB integrado ----------
uint8_t colorR = 0, colorG = 40, colorB = 60;   // color "programado"
uint8_t brillo = 30;                            // 0..100
bool    ledEncendido = true;

void aplicarLed() {
  uint8_t r = 0, g = 0, b = 0;
  if (ledEncendido) {
    r = colorR * brillo / 100; g = colorG * brillo / 100; b = colorB * brillo / 100;
  }
#ifdef RGB_BUILTIN
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  rgbLedWrite(RGB_BUILTIN, r, g, b);
#else
  neopixelWrite(RGB_BUILTIN, r, g, b);
#endif
#endif
}

// ---------- Máquina de estados del botón ----------
void tareaBoton(void *) {
  enum Estado { REPOSO, PULSADO, ESPERA_DOBLE, SEGUNDA, ESPERA_SOLTAR } estado = REPOSO;
  bool lecturaAnterior = HIGH, estable = HIGH;
  uint32_t tLectura = 0, tEstado = 0;

  while (true) {
    uint32_t ahora = millis();
    bool lectura = digitalRead(PIN_BOTON);

    // --- Antirrebote ---
    if (lectura != lecturaAnterior) {
      if (ahora - tLectura < ANTIRREBOTE_MS) rebotesFiltrados++;
      lecturaAnterior = lectura;
      tLectura = ahora;
    }
    bool cambio = false;
    if (lectura != estable && ahora - tLectura >= ANTIRREBOTE_MS) { estable = lectura; cambio = true; }
    bool pulsado = (estable == LOW);
    TipoPulsacion p;

    // --- Estados ---
    switch (estado) {
      case REPOSO:
        if (cambio && pulsado) { estado = PULSADO; tEstado = ahora; }
        break;
      case PULSADO:                                   // primera pulsación en curso
        if (ahora - tEstado >= LARGA_MS) { p = LARGA; xQueueSend(colaPulsaciones, &p, 0); estado = ESPERA_SOLTAR; }
        else if (cambio && !pulsado) { estado = ESPERA_DOBLE; tEstado = ahora; }
        break;
      case ESPERA_DOBLE:                              // soltado: ¿llega una segunda?
        if (cambio && pulsado) { p = DOBLE; xQueueSend(colaPulsaciones, &p, 0); estado = SEGUNDA; }
        else if (ahora - tEstado >= DOBLE_MS) { p = SIMPLE; xQueueSend(colaPulsaciones, &p, 0); estado = REPOSO; }
        break;
      case SEGUNDA:
      case ESPERA_SOLTAR:                             // esperar a que suelte para volver al reposo
        if (cambio && !pulsado) estado = REPOSO;
        break;
    }
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

// ---------- Acciones de cada pulsación ----------
void tareaAcciones(void *) {
  TipoPulsacion p;
  while (true) {
    if (xQueueReceive(colaPulsaciones, &p, portMAX_DELAY) != pdTRUE) continue;
    switch (p) {
      case SIMPLE:
        ledEncendido = !ledEncendido;
        Serial.printf("[SIMPLE] LED %s\n", ledEncendido ? "ENCENDIDO (estado anterior)" : "APAGADO");
        break;
      case DOBLE:
        ledEncendido = true;
        brillo = 100;
        Serial.println("[DOBLE]  LED al máximo conservando el color");
        break;
      case LARGA:
        Serial.println("[LARGA]  Comprobaría actualización FOTA (simulado)");
        break;
    }
    aplicarLed();
    Serial.printf("         (rebotes filtrados hasta ahora: %lu)\n", rebotesFiltrados);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  pinMode(PIN_BOTON, INPUT_PULLUP);
  aplicarLed();

  colaPulsaciones = xQueueCreate(8, sizeof(TipoPulsacion));
  xTaskCreate(tareaBoton,    "boton",    2048, nullptr, 3, nullptr);
  xTaskCreate(tareaAcciones, "acciones", 3072, nullptr, 2, nullptr);

  Serial.println("\n=== Prueba del botón BOOT: pulsación simple, doble y larga ===");
#ifndef RGB_BUILTIN
  Serial.println("AVISO: esta placa no tiene LED RGB integrado; solo se verán los mensajes.");
#endif
}

void loop() {
  vTaskDelete(nullptr);
}
