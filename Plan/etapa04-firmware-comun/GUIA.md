# Etapa 4 — Firmware común de los nodos

> **Estado: BORRADOR SIN TERMINAR ⚠️** — Falta el código y la guía detallada.
> Hay que revisarla y completarla **cuando se cierre la etapa 2 (y la 3 para el sensor real)**, porque depende de sus resultados.

## Objetivo
Un **único firmware** para las 3 placas; el rol (BASE / CAZADOR / PÁJARO) se decide por configuración
(MAC o memoria). Une las pruebas de la etapa 2 en un programa ordenado.

## Tareas previstas
- [ ] Estructura FreeRTOS (prueba 05): tareas de sensor, comunicaciones, actuadores y botón; colas
- [ ] Sensor detrás de `leerOrientacion()` con modo **simulado** y modo **real** (MPU6050 en los dos mandos)
- [ ] Protocolo ESP-NOW común: orientación (broadcast, 50-100 Hz); disparo, impacto y órdenes (unicast con ACK, prueba 02); latido; versión
- [ ] Emparejamiento automático con la base (prueba 02)
- [ ] LED RGB gradual (prueba 07) y botón simple/doble/larga (prueba 06)
- [ ] GPIO5 = motor de vibración, lógica normal/inversa configurable
- [ ] Configuración por mensaje y guardada en flash (`Preferences`)

## Terminada cuando
Las 3 placas llevan el mismo binario, cada una se comporta según su rol y los mandos funcionan en modo simulado.
