# Plan del proyecto por etapas

Entrega: **~20 de diciembre**. Sin estimaciones por semanas: se avanza lo antes posible.
Las etapas 2 y 3 se pueden hacer **mientras llega el pedido** de AliExpress, con las ESP32 del laboratorio.

Leyenda: ✅ hecho · ⬜ pendiente · 🔀 se puede repartir en paralelo entre los 3

---

## Etapa 1 — Organización y compras
- ⬜ Hacer el pedido de AliExpress → ver [COMPRAS.md](COMPRAS.md) (lo primero: tarda en llegar)
- ✅ Repositorio GitHub privado creado
- ⬜ Invitar a los otros 2 integrantes (y al profesor si lo pide) como colaboradores
- ⬜ Cada uno: clonar el repo, Arduino IDE con core ESP32 3.x, Node-RED en Docker (código en casa, pruebas en el laboratorio con sus ESP32-C3)
- ⬜ Recoger las credenciales del grupo **II16** del Campus Virtual (MQTT y MongoDB) → van en `secrets.h`, **nunca** al repo
- ⬜ Crear el **bot de Telegram** (@BotFather) y el **canal de difusión**
- ⬜ Enviar al profesor la propuesta final **por escrito** (correo) y guardar su respuesta
- ⬜ Repartir responsables (propuesta abajo)

**Hecho cuando:** pedido hecho, los 3 con el entorno funcionando y el visto bueno por escrito.

## Etapa 2 — Validación de tecnologías (con las ESP32 del laboratorio) 🔀
Pruebas pequeñas e independientes, cada una en `firmware/pruebas/<nombre>`:
- ✅ **Latencia ESP-NOW** (mando → base → PC): ~4 ms de media. Ver `firmware/pruebas/latencia_espnow`
- ⬜ **ESP-NOW unicast con ACK**: el mando conoce la MAC de la base y los disparos no se pierden
- ⬜ **Canal automático**: el mando encuentra solo el canal WiFi de la base
- ⬜ **MQTT** con el broker `iot.ac.uma.es` (usuario II16): publicar/suscribir, mensaje retenido y LWT
- ⬜ **FOTA básica**: una placa se actualiza desde el servidor OTA de la universidad
- ⬜ **FreeRTOS**: esqueleto con tareas, colas y temporizadores
- ⬜ **Node-RED**: recibir por puerto serie desde la base y reenviarlo a una web por WebSocket

**Hecho cuando:** cada prueba funciona sola y queda documentada en su README.

## Etapa 3 — Prototipo de puntería con IMU (cuando lleguen los sensores)
- ⬜ Leer BNO085 (cazador) y MPU6050 (pájaro)
- ⬜ Orientación estable: cuaternión/ángulos; medir la **deriva** en 5-10 min
- ⬜ Calibración: apuntar al centro + botón; botón de recentrar
- ⬜ Página web mínima: un punto que sigue a la mano (cazador) y un avión que se inclina (pájaro)

**Hecho cuando:** el cursor sigue bien a la mano y la deriva es aceptable con recentrado.
⚠️ Es el **mayor riesgo técnico** del proyecto: si falla, se replantea antes de seguir.

## Etapa 4 — Firmware común de los nodos 🔀
Un único firmware; el rol (BASE / CAZADOR / PÁJARO) se decide por configuración (MAC o memoria).
- ⬜ Estructura FreeRTOS: tareas de sensor, comunicaciones, actuadores y botón; colas entre ellas
- ⬜ Protocolo de mensajes ESP-NOW (posición, disparo, impacto, órdenes, latido, versión)
- ⬜ **LED RGB** con cambios graduales (±1 % cada X ms, configurable)
- ⬜ **GPIO5** = motor de vibración, con lógica normal/inversa configurable
- ⬜ **Botón**: simple / doble / largo, con antirrebote
- ⬜ Configuración por mensaje (`config`) y guardada en flash (persistente)

## Etapa 5 — Base / pasarela 🔀
- ⬜ ESP-NOW ↔ MQTT: publica en nombre de los mandos con los **topics obligatorios** (`II16/esp32c3-XXXX/...`)
- ⬜ Latido de los mandos → `conexion` `online:false` si dejan de responder (equivalente a LWT)
- ⬜ Reenvío en tiempo real al PC por USB serie
- ⬜ LED RGB de la base con el estado de la partida (esperando, jugando, fin)
- ⬜ Órdenes individuales y **de grupo** (topic `II16/all/...`)

## Etapa 6 — Juego (web servida por Node-RED) 🔀
- ⬜ Canvas sencillo: escenario, cursor del cazador, pájaro pilotado
- ⬜ Detección de impacto (la decide un solo sitio: el juego)
- ⬜ Partida: inicio, tiempo, vidas, fin; evento de impacto → vibración en ambos mandos
- ⬜ Escenarios cargados desde MongoDB

## Etapa 7 — SCADA en Node-RED + MongoDB 🔀
- ⬜ Guardar con fecha: estado y datos de los nodos, cada disparo, partidas y **log** (con origen MQTT/botón)
- ⬜ Dashboard por pestañas: estado de los nodos, control individual y de grupo, configuración
- ⬜ Histórico: consultas por fechas con atajos (día/semana/mes), gráficas, estadísticas, **CSV**
- ⬜ Estadísticas de juego: % de acierto, tiempo de reacción, mapa de calor de disparos
- ⬜ **Alarmas** con umbrales configurables (p. ej. temperatura de la placa,
  señal débil, pérdidas altas) → aviso en el dashboard + Telegram + log.
  ⚠️ El enunciado pide umbrales de temperatura y humedad: confirmar con el profesor qué parámetros valen.

## Etapa 8 — Telegram 🔀
- ⬜ Bot: `/estado`, `/partida`, `/ranking`, `/actualizar`, control de LED/vibración
- ⬜ Canal: resultados de partidas, récords, alarmas, desconexiones, actualizaciones

## Etapa 9 — FOTA completo
- ⬜ Base: al arrancar, por MQTT, pulsación larga y periódica (configurable, 0 = no)
- ⬜ Mandos: orden por ESP-NOW desde la base → WiFi temporal → descarga → vuelta a ESP-NOW
- ⬜ Versión de cada nodo visible en el dashboard; no actualizar durante una partida
- ⬜ Demo: cambiar el color del LED en la v2 y `/actualizar`

## Etapa 10 — Hardware final y robustez
- ⬜ Powerbanks, montaje en la mano/muñeca, cableado definitivo
- ⬜ Pruebas de fallo: mando apagado, WiFi caída, broker caído, sensor desconectado
- ⬜ Prueba completa de una partida de principio a fin, varias veces

## Etapa 11 — Memoria y defensa
- ⬜ Memoria PDF: introducción, HW (pines), SW (diagramas, FreeRTOS, robustez, librerías),
  resultados (incluida la prueba de latencia), conclusiones
- ⬜ **Manual de usuario**
- ⬜ Lista de ficheros: `.ino`, flujos Node-RED `.json`, Wokwi si se usa
- ⬜ Vídeo de la demo (por si algo falla el día de la defensa) y ensayo

---

## Reparto propuesto (ajustadlo)
| Persona | Responsable de | Etapas |
|---|---|---|
| Integrante 1 | Mandos: IMU, puntería, vibración, botones | 3, 4, parte de 9 |
| Integrante 2 | Base/pasarela, MQTT, FOTA, MongoDB | 2, 5, 7 (datos), 9 |
| Integrante 3 | Juego web, dashboard, Telegram | 6, 7 (interfaz), 8 |

Todos: etapa 10, 11 y revisar el trabajo de los demás (Pull Requests).

## Riesgos y decisiones pendientes
- **Puntería con IMU (deriva)** → se valida en la etapa 3 antes de construir el resto.
- **Alarmas de temperatura/humedad** del enunciado → confirmar con el profesor los parámetros de alarma.
- **Node-RED en Docker en los 3 PCs**: si los 3 están conectados a la vez al mismo broker y a la
  misma base de datos, se duplicarán registros y mensajes de Telegram. Solo una instancia debe
  ser la "oficial"; para desarrollar, usad topics/colecciones de prueba propios o un bot distinto.
- **Botón BOOT**: el enunciado le asigna funciones fijas; si se usa como gatillo, pedir permiso o usar un pulsador aparte.
- **WiFi**: `infind` no conectó; eduroam no vale. Plan: hotspot del móvil (2,4 GHz, WPA2).
