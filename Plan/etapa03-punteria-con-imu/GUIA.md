# Etapa 3 — Prototipo de puntería con IMU (BNO055 + MPU6050)

> **Estado: BORRADOR SIN TERMINAR ⚠️** — Falta el código y la guía detallada.
> Hay que revisarla y completarla **cuando se cierre la etapa 2 y lleguen los sensores**, porque depende de sus resultados.

**Necesita los sensores 🔴.** Es el **mayor riesgo técnico** del proyecto: se hace en cuanto lleguen.

## Objetivo
Comprobar que con las IMU se sabe hacia dónde apunta la mano con suficiente precisión y poca deriva:
- **Cazador → BNO055** (9 ejes): orientación completa (yaw, pitch, roll) calculada en el chip.
- **Pájaro → MPU6050** (6 ejes): inclinación (pitch y roll) con el DMP del chip.

## Tareas previstas
- [ ] Soldar pines y conectar las IMU por I2C (VCC a **3,3 V**). ⚠️ En la ESP32-C3 del laboratorio el LED RGB está en GPIO8 y BOOT en GPIO9, que son los pines I2C por defecto: usar `Wire.begin(SDA, SCL)` con otros pines (p. ej. 6 y 7, comprobarlo en la placa)
- [ ] **Cazador (BNO055):** librería *Adafruit BNO055* (+ *Adafruit Unified Sensor*)
  - Dirección I2C `0x28` o `0x29` (si no lo detecta, probar `Adafruit_BNO055(55, 0x29)`)
  - Modo **NDOF** (con magnetómetro). Si el motor de vibración desvía la brújula, probar modo **IMUPLUS** (sin magnetómetro, como un 6 ejes)
  - Si da errores de I2C con el ESP32 (el BNO055 alarga el reloj, *clock stretching*): `Wire.setClock(100000)` y `Wire.setTimeOut(...)` más alto
  - Revisar el estado de calibración (`getCalibration`) y, si conviene, guardar los offsets en flash
- [ ] **Pájaro (MPU6050):** librería *MPU6050* de Electronic Cats (i2cdevlib) con el **DMP** (`MPU6050_6Axis_MotionApps20.h`)
  - Dirección I2C `0x68`. Comprobar `WHO_AM_I` (`0x68` auténtico; `0x70` = MPU6500, quitar la comprobación de la librería si lo rechaza)
  - Calibrar el giroscopio al arrancar (`CalibrateGyro` / `CalibrateAccel`) con el mando **quieto 2-3 s**
- [ ] Convertir a yaw / pitch / roll (los dos sensores dan un cuaternión)
- [ ] **Medir la deriva**: mando quieto 5-10 min, anotar cuánto se desvía el yaw del cazador
- [ ] **Calibración**: apuntar al centro de la pantalla + botón = origen; botón de recentrar
- [ ] Comprobar si la **vibración del motor** ensucia la lectura (y, en el BNO055, si su imán afecta al magnetómetro)
- [ ] Sustituir los datos simulados de `MandoSimulado` (etapa 2, prueba 08) por los del sensor y ver el cursor en la página `/etapa2`

## Se puede preparar ya (🟢)
Instalar las dos librerías, leer su documentación y dejar escrito el código de lectura y calibración
de cada sensor detrás de `leerOrientacion()`.

## Terminada cuando
El cursor sigue bien a la mano y la deriva es aceptable con recentrado. Si no, replantear antes de seguir.
