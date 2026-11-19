# Etapa 9 — FOTA completo

> **Estado: BORRADOR SIN TERMINAR ⚠️** — Falta el código y la guía detallada.
> Hay que revisarla y completarla **cuando se cierre la etapa 2 (prueba 04) y la 5**, porque depende de sus resultados.

## Objetivo
Actualizar las 3 placas por WiFi con una sola orden.

## Tareas previstas
- [ ] Integrar el **servidor OTA de la universidad** (`https://iot.ac.uma.es:1880/esp8266-ota`) siguiendo el ejemplo del Campus Virtual (la prueba 04 usa un servidor en el PC)
- [ ] Base: al arrancar, por MQTT (`.../FOTA`), pulsación larga y periódica (configurable, 0 = no)
- [ ] Mandos: orden por ESP-NOW desde la base → WiFi temporal → descarga → vuelta a ESP-NOW
- [ ] Versión de cada nodo consultable por Telegram (`/estado`); no actualizar durante una partida
- [ ] Demo: cambiar el color del LED en la v2 y `/actualizar`

## Terminada cuando
`/actualizar` en Telegram actualiza las 3 placas y todas informan de su nueva versión.
