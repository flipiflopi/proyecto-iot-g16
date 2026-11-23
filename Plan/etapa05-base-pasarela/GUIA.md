# Etapa 5 — Base / pasarela

> **Estado: BORRADOR SIN TERMINAR ⚠️** — Falta el código y la guía detallada.
> Hay que revisarla y completarla **cuando se cierre las etapas 2 y 4**, porque depende de sus resultados.

## Objetivo
La base conecta el mundo ESP-NOW (mandos) con el navegador (tiempo real: **servidor web + WebSocket en la propia base**) y con MQTT (eventos, órdenes y configuración desde Node-RED). Ver [../ARQUITECTURA.md](../ARQUITECTURA.md).

## Tareas previstas
- [ ] ESP-NOW ↔ MQTT: publicar en nombre de los mandos con los topics `II16/esp32c3-XXXX/...` (prueba 03)
- [ ] Latido de los mandos → publicar `conexion` `online:false` si dejan de responder (equivalente a LWT)
- [ ] Servidor web (LittleFS) + WebSocket a 50 Hz hacia el navegador (prueba 09). El reenvío por USB de la 08 queda como plan B
- [ ] Órdenes MQTT → mandos por ESP-NOW, individuales y **de grupo** (`II16/all/...`)
- [ ] LED RGB de la base con el estado de la partida (esperando, jugando, fin)

## Terminada cuando
Con 2 mandos simulados, todo lo que pasa en los mandos aparece en MQTT y en la web servida por la base, y las órdenes MQTT llegan a los mandos.
