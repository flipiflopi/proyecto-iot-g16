# Etapa 11 — Integración y robustez

> **Estado: BORRADOR SIN TERMINAR ⚠️** — Falta el código y la guía detallada.
> Hay que revisarla y completarla **cuando se cierre las etapas 3 a 10**, porque depende de sus resultados.

## Objetivo
Juntar todo con los sensores reales y comprobar que aguanta los fallos.

## Tareas previstas
- [ ] Cambiar `SENSOR_SIMULADO` → `SENSOR_REAL` y probar el sistema completo
- [ ] Pruebas de fallo: mando apagado, WiFi caída, broker caído, sensor desconectado, base reiniciada
- [ ] Partida completa de principio a fin, varias veces seguidas
- [ ] Anotar cada problema y su solución (para la memoria)

## Terminada cuando
Varias partidas seguidas sin fallos y el sistema se recupera solo de los fallos probados.
