# Etapa 8 — Telegram

> **Estado: BORRADOR SIN TERMINAR ⚠️** — Falta el código y la guía detallada.
> Hay que revisarla y completarla **cuando se cierre las etapas 5 y 7**, porque depende de sus resultados.

## Objetivo
Bot y canal de Telegram implementados en Node-RED.

## Tareas previstas
- [ ] Bot (token de la etapa 1, guardado en las credenciales del nodo, no en el flujo): `/estado`, `/partida`, `/ranking`, `/actualizar`, control de LED/vibración
- [ ] Canal: resultados de partidas, récords, desconexiones, actualizaciones
- [ ] Solo una instancia de Node-RED con el bot real a la vez (si no, se duplican los mensajes)

## Terminada cuando
Desde Telegram se puede consultar el estado, lanzar una partida y ver el ranking, y el canal publica los eventos.
