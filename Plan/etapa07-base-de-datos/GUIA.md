# Etapa 7 — Base de datos (Node-RED + MongoDB)

> **Estado: BORRADOR SIN TERMINAR ⚠️** — Falta el código y la guía detallada.
> Hay que revisarla y completarla **cuando se cierre las etapas 5 y 6**, porque depende de sus resultados.

## Objetivo
Guardar en MongoDB (servidor de la universidad, base de datos `II16`) lo que pasa en el sistema.

## Tareas previstas
- [ ] Conectar Node-RED a MongoDB (`iot.ac.uma.es:27017`, usuario II16)
- [ ] **Log** de eventos con fecha: conexiones/desconexiones, disparos, impactos, órdenes, actualizaciones (con su origen)
- [ ] **Partidas** (jugadores, duración, puntuación) y **escenarios**
- [ ] Consultas para el ranking de Telegram y para cargar escenarios en el juego
- [ ] Para desarrollar: colecciones de prueba propias, para no mezclar datos entre los 3

## Terminada cuando
Cada evento y cada partida quedan guardados con fecha y se pueden consultar.
