# Etapa 3 — Prototipo de puntería con IMU (BNO085)

> **Estado: BORRADOR SIN TERMINAR ⚠️** — Falta el código y la guía detallada.
> Hay que revisarla y completarla **cuando se cierre la etapa 2 y lleguen los sensores**, porque depende de sus resultados.

**Necesita los sensores 🔴.** Es el **mayor riesgo técnico** del proyecto: se hace en cuanto lleguen.

## Objetivo
Comprobar que con el BNO085 se sabe hacia dónde apunta la mano con suficiente precisión y poca deriva.

## Tareas previstas
- [ ] Soldar pines y conectar el BNO085 por I2C. ⚠️ En la ESP32-C3 del laboratorio el LED RGB está en GPIO8 y BOOT en GPIO9, que son los pines I2C por defecto: usar `Wire.begin(SDA, SCL)` con otros pines (p. ej. 6 y 7, comprobarlo en la placa)
- [ ] Elegir y probar la librería (p. ej. *SparkFun BNO08x* o *Adafruit BNO08x*) y leer el **Game Rotation Vector** (cuaternión)
- [ ] Convertir a yaw / pitch / roll
- [ ] **Medir la deriva**: mando quieto 5-10 min, anotar cuánto se desvía el yaw
- [ ] **Calibración**: apuntar al centro de la pantalla + botón = origen; botón de recentrar
- [ ] Comprobar si la **vibración del motor** ensucia la lectura
- [ ] Sustituir los datos simulados de `MandoSimulado` (etapa 2, prueba 08) por los del sensor y ver el cursor en la página `/etapa2`

## Se puede preparar ya (🟢)
Elegir librería, leer su documentación y dejar escrito el código de lectura y calibración.

## Terminada cuando
El cursor sigue bien a la mano y la deriva es aceptable con recentrado. Si no, replantear antes de seguir.
