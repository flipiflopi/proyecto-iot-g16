# Arquitectura del sistema (decisión del 2026-11-23)

> Decidido en el laboratorio el 2026-11-23, tras superar la prueba 08 de la etapa 2.
> **Sustituye** a la idea inicial de que el juego lo sirviera Node-RED con los datos llegando por USB.

## Resumen
- **El juego (lógica + página web) corre en la ESP32-C3 base.**
- **Node-RED** queda para la **gestión**: dashboard (jugadores, roles, duración), historial de partidas
  (MongoDB) y Telegram.

```
 [Cazador] --ESP-NOW-->                                   --WiFi (WebSocket 50 Hz)--> navegador: juego
                         [BASE C3]                                                     (página servida por la base)
 [Pájaro]  --ESP-NOW-->  · lógica del juego
           <--ESP-NOW--  · servidor web + WebSocket       --WiFi/MQTT--> broker iot.ac.uma.es
            (vibración,  · LED RGB de estado                                   |
             órdenes)                                         Node-RED (PC): dashboard + MongoDB + Telegram
```

Todo sobre **la misma WiFi**: un **router propio o el hotspot de un móvil** (2,4 GHz, WPA2-Personal).
El PC con el navegador y Node-RED se conecta a esa misma WiFi.

## Decisiones

| # | Decisión | Motivo |
|---|---|---|
| 1 | La **base** hace la **lógica del juego**: estado de la partida, tiempo, aciertos, puntos, vibración | Recibe los mandos directamente (ESP-NOW, ~4 ms medidos en la 01): decide al instante y la vibración no tiene que pasar por el PC. La partida sigue aunque se cuelgue el PC |
| 2 | La base **sirve la página HTML/JS** desde su flash (LittleFS) con `ESPAsyncWebServer` y envía las posiciones por **WebSocket a 50 Hz** | Se elimina el puente serie→UDP del PC. El navegador solo dibuja |
| 3 | **Node-RED** solo para gestión: **dashboard** (nombres, quién es cazador/pájaro, duración, iniciar partida, historial), **MongoDB** y **Telegram** | Cumple con Node-RED/MQTT/BD sin meterlo en el camino de tiempo real |
| 4 | Node-RED ↔ base por **MQTT** (broker `iot.ac.uma.es`, ya validado en la 03) | Configuración de la partida hacia la base y resultados hacia Node-RED |
| 5 | **Red: opción A**. La base se une como cliente (STA) a un router propio o al hotspot de un móvil | El PC, la base y el broker se ven entre sí y hay internet para MQTT. El canal lo impone el router, y los mandos lo encuentran solos (prueba 02) |
| 6 | **No usar eduroam** ni la WiFi de la universidad | WPA2-Enterprise y aislamiento entre clientes: el navegador no vería la base |

### Opciones descartadas
- **B. La base crea su propia WiFi (AP):** no depende de nada, pero el PC pierde internet y no hay broker MQTT.
- **C. AP + STA a la vez:** más complejo, y el AP queda forzado al canal del router.
- **Juego en Node-RED con los datos por USB** (lo de la prueba 08): funciona (100 msg/s, ≤ 0,4 % de pérdidas),
  pero necesita el PC y el puente Python en el camino de tiempo real. **Queda como plan B** si la prueba 09 falla.
- **Mandos por WiFi directamente al PC:** más latencia y jitter, dependencia del router y más consumo de batería.

## MQTT entre Node-RED y la base (propuesta, a cerrar en las etapas 5-7)
Respetando los topics obligatorios `II16/...`:

| Sentido | Topic (propuesta) | Contenido |
|---|---|---|
| Node-RED → base | `II16/juego/config` | Nombres, quién es cazador y quién pájaro, duración |
| Node-RED → base | `II16/juego/orden` | `start`, `stop` |
| Base → Node-RED | `II16/juego/estado` | Esperando, jugando, fin y tiempo restante (retenido) |
| Base → Node-RED | `II16/juego/resultado` | Puntos, aciertos y duración; Node-RED lo guarda en MongoDB |

Para el dashboard: **`@flowfuse/node-red-dashboard`** (Dashboard 2.0). El antiguo `node-red-dashboard` está abandonado.

## Notas técnicas para el firmware de la base
- Librerías: **`ESP32Async/ESPAsyncWebServer`** + **`ESP32Async/AsyncTCP`** (forks mantenidos, compatibles con el core 3.x).
  Las originales de me-no-dev dan problemas con el core 3.x.
- **No enviar por WebSocket desde el callback de ESP-NOW**, porque se ejecuta en la tarea de la WiFi.
  El callback guarda el dato (cola o variable con mutex) y una **tarea FreeRTOS** hace `ws.textAll()` a 50 Hz (patrón de la prueba 05).
- Mensajes cortos (JSON compacto o binario) y `ws.cleanupClients()` periódico.
- Pensado para **1-2 navegadores** conectados. Con más, bajar la frecuencia.

## Qué hay que validar
Ver la **prueba 09** en [etapa02-validacion-de-tecnologias/GUIA.md](etapa02-validacion-de-tecnologias/GUIA.md#09--base-con-esp-now--web--mqtt-a-la-vez):
la base haciendo **ESP-NOW + servidor web/WebSocket a 50 Hz + MQTT a la vez**, conectada al hotspot.
Es el único riesgo nuevo de esta arquitectura.
