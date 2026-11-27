# Etapa 3 — Prototipo de puntería con IMU (2 × MPU6050)

> **Estado: BORRADOR SIN TERMINAR ⚠️** — Falta el código y la guía detallada.
> Hay que revisarla y completarla **cuando se cierre la etapa 2 y lleguen los sensores**, porque depende de sus resultados.

**Necesita los sensores 🔴.** Es el **mayor riesgo técnico** del proyecto: se hace en cuanto lleguen.

## Objetivo
Comprobar que con las IMU se sabe hacia dónde apunta la mano con suficiente precisión y poca deriva:
- **Los dos mandos → MPU6050** (6 ejes), con el DMP del chip.
- **Cazador:** yaw y pitch (hacia dónde apunta). El yaw deriva (sin magnetómetro): se compensa con calibración al arrancar y recentrado.
- **Pájaro:** inclinación (pitch y roll), que no deriva porque el acelerómetro la corrige.
- Por qué 2 × MPU6050: ver [COMPRAS.md](../etapa01-organizacion-y-compras/COMPRAS.md#por-qué-2--mpu6050-y-no-bno085-ni-bno055).

## Tareas previstas
- [ ] Soldar pines y conectar las IMU por I2C (VCC a **3,3 V**). ⚠️ En la ESP32-C3 del laboratorio el LED RGB está en GPIO8 y BOOT en GPIO9, que son los pines I2C por defecto: usar `Wire.begin(SDA, SCL)` con otros pines (p. ej. 6 y 7, comprobarlo en la placa)
- [ ] **MPU6050 (los dos mandos):** librería *MPU6050* de Electronic Cats (i2cdevlib) con el **DMP** (`MPU6050_6Axis_MotionApps20.h`)
  - Dirección I2C `0x68`. Comprobar `WHO_AM_I` (`0x68` auténtico; `0x70` = MPU6500, quitar la comprobación de la librería si lo rechaza)
  - Calibrar el giroscopio al arrancar (`CalibrateGyro` / `CalibrateAccel`) con el mando **quieto 2-3 s**
- [ ] Convertir el cuaternión del DMP a yaw / pitch / roll
- [ ] **Medir la deriva del yaw** (cazador): mando quieto 5-10 min, anotar °/min. Debe quedar en pocos grados por partida (30-60 s)
- [ ] **Calibración**: apuntar al centro de la pantalla + botón = origen; botón de recentrar y recentrado al empezar cada partida
- [ ] Comprobar si la **vibración del motor** ensucia la lectura
- [ ] Sustituir los datos simulados de `MandoSimulado` (etapa 2, prueba 08) por los del sensor y ver el cursor en la página `/etapa2`

## Se puede preparar ya (🟢)
Instalar la librería del MPU6050, leer su documentación y dejar escrito el código de lectura y calibración
detrás de `leerOrientacion()`.

## Terminada cuando
El cursor sigue bien a la mano y la deriva es aceptable con recentrado. Si no, replantear antes de seguir.
