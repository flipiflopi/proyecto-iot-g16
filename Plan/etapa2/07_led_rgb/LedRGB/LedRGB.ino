/*
  ============================================================================
  ETAPA 2 - PRUEBA 07: LED RGB CON CAMBIOS GRADUALES
  ============================================================================
  Lo que pide el enunciado:
   - Intensidad 0..100 % y color RGB 0..255 x3.
   - Los cambios se hacen de forma GRADUAL: ±1 % cada "velocidad" ms (10 ms por defecto),
     configurable.
   - Al terminar el cambio se envía UN SOLO mensaje de estado con el valor final
     (no los intermedios). Aquí se imprime por el monitor serie en JSON.

  Comandos por el monitor serie (115200, fin de línea "Nueva línea"):
     b 75          -> brillo al 75 %
     c 255 0 0     -> color rojo
     v 20          -> velocidad: 20 ms por cada 1 %
     on / off      -> encender / apagar (apagar = brillo 0, recuerda el anterior)
     ?             -> estado actual

  Una tarea FreeRTOS hace el fundido; los comandos le llegan por una cola.
  ============================================================================
*/
#include <Arduino.h>

#ifdef RGB_BUILTIN
const int PIN_RGB = RGB_BUILTIN;   // LED RGB integrado de la placa
#else
const int PIN_RGB = 8;             // Placa sin LED integrado: LED WS2812 externo en este pin
#endif

// ---------------- Comandos hacia la tarea del LED ----------------
enum TipoCmd : uint8_t { CMD_BRILLO, CMD_COLOR, CMD_VELOCIDAD };
struct Comando {
  TipoCmd tipo;
  int a, b, c;
};
QueueHandle_t colaComandos;

void escribirLed(uint8_t r, uint8_t g, uint8_t b) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  rgbLedWrite(PIN_RGB, r, g, b);
#else
  neopixelWrite(PIN_RGB, r, g, b);
#endif
}

// ---------------- Tarea del fundido ----------------
void tareaLed(void *) {
  int brillo = 0, brilloObjetivo = 30;          // %
  float col[3] = {0, 120, 255};                 // color mostrado ahora
  float colOrigen[3] = {0, 120, 255};
  int   colObjetivo[3] = {0, 120, 255};
  int   progresoColor = 100;                    // 0..100 % del fundido de color
  int   velocidadMs = 10;                       // ms por cada 1 %
  bool  cambiando = true;

  while (true) {
    // 1) ¿Ha llegado un comando nuevo?
    Comando cmd;
    while (xQueueReceive(colaComandos, &cmd, 0) == pdTRUE) {
      if (cmd.tipo == CMD_BRILLO) {
        brilloObjetivo = constrain(cmd.a, 0, 100);
      } else if (cmd.tipo == CMD_COLOR) {
        for (int i = 0; i < 3; i++) colOrigen[i] = col[i];
        colObjetivo[0] = constrain(cmd.a, 0, 255);
        colObjetivo[1] = constrain(cmd.b, 0, 255);
        colObjetivo[2] = constrain(cmd.c, 0, 255);
        progresoColor = 0;
      } else if (cmd.tipo == CMD_VELOCIDAD) {
        velocidadMs = constrain(cmd.a, 1, 1000);
        Serial.printf("{\"velocidad\":%d}\n", velocidadMs);
      }
      cambiando = true;
    }

    // 2) Un paso del fundido: ±1 % de brillo y +1 % del cambio de color
    if (brillo < brilloObjetivo) brillo++;
    else if (brillo > brilloObjetivo) brillo--;
    if (progresoColor < 100) {
      progresoColor++;
      for (int i = 0; i < 3; i++)
        col[i] = colOrigen[i] + (colObjetivo[i] - colOrigen[i]) * progresoColor / 100.0f;
    }

    escribirLed(col[0] * brillo / 100, col[1] * brillo / 100, col[2] * brillo / 100);

    // 3) Al terminar: UN mensaje con el estado final
    if (cambiando && brillo == brilloObjetivo && progresoColor >= 100) {
      cambiando = false;
      Serial.printf("{\"LED\":%d,\"R\":%d,\"G\":%d,\"B\":%d,\"origen\":\"serie\"}\n",
                    brillo, colObjetivo[0], colObjetivo[1], colObjetivo[2]);
    }

    vTaskDelay(pdMS_TO_TICKS(cambiando ? velocidadMs : 20));
  }
}

// ---------------- Lectura de comandos por serie ----------------
int brilloRecordado = 30;
int brilloPedido = 30;

void procesarLinea(String l) {
  l.trim();
  Comando cmd = {};
  if (l.startsWith("b ")) {
    cmd.tipo = CMD_BRILLO; cmd.a = l.substring(2).toInt();
    brilloPedido = cmd.a;
    if (cmd.a > 0) brilloRecordado = cmd.a;
  } else if (l.startsWith("c ")) {
    cmd.tipo = CMD_COLOR;
    if (sscanf(l.c_str() + 2, "%d %d %d", &cmd.a, &cmd.b, &cmd.c) != 3) { Serial.println("Uso: c R G B"); return; }
  } else if (l.startsWith("v ")) {
    cmd.tipo = CMD_VELOCIDAD; cmd.a = l.substring(2).toInt();
  } else if (l == "on") {
    cmd.tipo = CMD_BRILLO; cmd.a = brilloRecordado; brilloPedido = cmd.a;
  } else if (l == "off") {
    cmd.tipo = CMD_BRILLO; cmd.a = 0; brilloPedido = 0;
  } else if (l == "?") {
    Serial.printf("Brillo pedido %d %%, recordado %d %%\n", brilloPedido, brilloRecordado);
    return;
  } else {
    Serial.println("Comandos: b N | c R G B | v MS | on | off | ?");
    return;
  }
  xQueueSend(colaComandos, &cmd, 0);
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  colaComandos = xQueueCreate(8, sizeof(Comando));
  xTaskCreate(tareaLed, "led", 3072, nullptr, 2, nullptr);
  Serial.println("\n=== LED RGB gradual. Comandos: b N | c R G B | v MS | on | off | ? ===");
}

void loop() {
  static String linea;
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') { if (linea.length()) procesarLinea(linea); linea = ""; }
    else linea += c;
  }
  delay(5);
}
