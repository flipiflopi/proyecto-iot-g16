# Etapa 6 — Juego (lógica y web en la base) + dashboard en Node-RED

> **Estado: BORRADOR SIN TERMINAR ⚠️** — Falta el código y la guía detallada.
> Hay que revisarla y completarla **cuando se cierre la etapa 2 (prueba 08, que ya da la página base)**, porque depende de sus resultados.

## Objetivo
El juego "Cazador vs. Pájaro": la **lógica corre en la base** y la página web la **sirve la base** (WebSocket a 50 Hz).
Node-RED hace el **dashboard** de gestión (jugadores, roles, duración, historial). Ver [../ARQUITECTURA.md](../ARQUITECTURA.md).

## Tareas previstas
- [ ] Partir de la página `/etapa2` (prueba 08) y de la de la prueba 09, servida por la base
- [ ] **Mando virtual con el ratón** para desarrollar sin placas
- [ ] Escenario, punto de mira del cazador, pájaro pilotado
- [ ] Detección de impacto (la decide un solo sitio: la **base**)
- [ ] Dashboard Node-RED (`@flowfuse/node-red-dashboard`): nombres, quién es cazador/pájaro, duración, iniciar; recibe `II16/juego/resultado` y lo guarda en MongoDB
- [ ] Partida según [REGLAS.md](REGLAS.md) (propuesta: 60 s, 3 vidas, puntos según el tiempo, cargador, turbo); impacto → vibración en los dos mandos
- [ ] Escenarios cargados desde MongoDB (etapa 7)

## Terminada cuando
Se puede jugar una partida completa con los mandos simulados o con el ratón.
