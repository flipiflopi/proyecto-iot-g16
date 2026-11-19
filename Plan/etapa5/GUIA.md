# Etapa 5 — Base / pasarela

> **Estado: BORRADOR SIN TERMINAR ⚠️** — Falta el código y la guía detallada.
> Hay que revisarla y completarla **cuando se cierre las etapas 2 y 4**, porque depende de sus resultados.

## Objetivo
La base conecta el mundo ESP-NOW (mandos) con el PC (tiempo real) y con MQTT (eventos y órdenes).

## Tareas previstas
- [ ] ESP-NOW ↔ MQTT: publicar en nombre de los mandos con los topics `II16/esp32c3-XXXX/...` (prueba 03)
- [ ] Latido de los mandos → publicar `conexion` `online:false` si dejan de responder (equivalente a LWT)
- [ ] Reenvío en tiempo real al PC por USB serie (prueba 08)
- [ ] Órdenes MQTT → mandos por ESP-NOW, individuales y **de grupo** (`II16/all/...`)
- [ ] LED RGB de la base con el estado de la partida (esperando, jugando, fin)

## Terminada cuando
Con 2 mandos simulados, todo lo que pasa en los mandos aparece en MQTT y en el PC, y las órdenes MQTT llegan a los mandos.
