# Plan del proyecto por etapas

Entrega: **~20 de diciembre**. Sin estimaciones por semanas: se avanza lo antes posible.
Código en casa, pruebas en el laboratorio con sus ESP32-C3.

Leyenda: ✅ hecho · ⬜ pendiente · 🟢 se puede hacer **sin los sensores** · 🔴 necesita los sensores · 🔀 se puede repartir entre los 3

---

## Mientras llegan los sensores: qué se puede hacer

**Casi todo.** Solo necesitan los BNO085 la **etapa 3** (puntería real) y el **montaje final**.
El resto se hace con las ESP32 del laboratorio, que ya traen todo lo necesario para probar:

| Lo que habrá en el proyecto | Cómo se prueba sin comprarlo |
|---|---|
| IMU BNO085 (orientación) | **Mando simulado**: el ESP32 genera una orientación falsa (círculos, barridos) y la envía igual que el real. O un **mando virtual** con el ratón del PC |
| Gatillo / turbo | Botón **BOOT** de la placa (GPIO9) |
| Motor de vibración (GPIO5) | La salida GPIO5 funciona igual sin nada conectado; se ve en el monitor serie o con un LED si el laboratorio tiene |
| Estado / respuesta visual | **LED RGB** integrado de la placa |

### La clave: separar el sensor del resto del código
El firmware leerá la orientación a través de **una sola función**, por ejemplo:
```cpp
bool leerOrientacion(Orientacion &o);   // devuelve el cuaternión / ángulos actuales
```
Con dos versiones, elegidas con una constante:
- `SENSOR_SIMULADO`: genera movimiento falso (p. ej. un círculo lento para el cazador y un balanceo para el pájaro).
- `SENSOR_BNO085`: lee el sensor real (se programa cuando llegue).

Así, **todo lo demás** (ESP-NOW, base, juego, Node-RED, Telegram, FOTA) se desarrolla y se prueba
ahora, y cuando llegue el sensor solo se cambia esa constante.

### Mando virtual con el ratón (para el juego)
Una página o un nodo de Node-RED que convierte el movimiento del ratón en "orientación del mando"
y la inyecta en el mismo sitio por el que llegarán los datos reales. Permite hacer y probar el juego
**en casa, sin ninguna placa**.

---

## Etapa 1 — Organización y compras 🟢
- ⬜ Hacer el pedido de AliExpress → ver [COMPRAS.md](COMPRAS.md) (lo primero: tarda en llegar)
- ✅ Repositorio GitHub privado creado
- ⬜ Invitar a los otros 2 integrantes (y al profesor si lo pide) como colaboradores
- ⬜ Cada uno: clonar el repo, Arduino IDE con core ESP32 3.x, Node-RED en Docker
- ⬜ Recoger las credenciales del grupo **II16** del Campus Virtual (MQTT y MongoDB) → van en `secrets.h`, **nunca** al repo
- ⬜ Crear el **bot de Telegram** (@BotFather) y el **canal de difusión**
- ⬜ Enviar al profesor la propuesta final **por escrito** (correo) y guardar su respuesta
- ⬜ Preguntar si la universidad tiene **impresora 3D** que podamos usar (ver etapa 10)
- ⬜ Repartir responsables (propuesta abajo)

**Hecho cuando:** pedido hecho, los 3 con el entorno funcionando y el visto bueno por escrito.

## Etapa 2 — Validación de tecnologías 🟢 🔀
Pruebas pequeñas e independientes, cada una en `firmware/pruebas/<nombre>` con su README:
- ✅ **Latencia ESP-NOW** (mando → base → PC): ~4 ms de media. Ver `firmware/pruebas/latencia_espnow`
- ⬜ **ESP-NOW unicast con ACK**: el mando conoce la MAC de la base y los disparos no se pierden
- ⬜ **Canal automático**: el mando encuentra solo el canal WiFi de la base
- ⬜ **MQTT** con el broker `iot.ac.uma.es` (usuario II16): publicar/suscribir, mensaje retenido y LWT
- ⬜ **FOTA básica**: una placa se actualiza desde el servidor OTA de la universidad
- ⬜ **FreeRTOS**: esqueleto con tareas, colas y temporizadores
- ⬜ **Botón BOOT**: pulsación simple / doble / larga con antirrebote
- ⬜ **LED RGB**: cambio gradual de color e intensidad
- ⬜ **Node-RED**: recibir por puerto serie desde la base y reenviarlo a una web por WebSocket

**Hecho cuando:** cada prueba funciona sola y queda documentada.

## Etapa 3 — Prototipo de puntería con IMU 🔴
- ⬜ Leer los 2 BNO085 (cazador y pájaro; mismo sensor y misma librería) → implementar `SENSOR_BNO085`
- ⬜ Orientación estable: cuaternión/ángulos; medir la **deriva** en 5-10 min
- ⬜ Calibración: apuntar al centro + botón; botón de recentrar
- ⬜ Comprobar que la **vibración del motor no estropea la lectura** de la IMU
- ⬜ Página web mínima: un punto que sigue a la mano (cazador) y un avión que se inclina (pájaro)

**Hecho cuando:** el cursor sigue bien a la mano y la deriva es aceptable con recentrado.
⚠️ Es el **mayor riesgo técnico**: se hace en cuanto lleguen los sensores; si falla, se replantea.

**Preparable antes de que lleguen (🟢):** elegir la librería del BNO085, leer su documentación,
dejar escrito el código de lectura y el de calibración, y la página web con el mando virtual.

## Etapa 4 — Firmware común de los nodos 🟢 🔀
Un único firmware; el rol (BASE / CAZADOR / PÁJARO) se decide por configuración (MAC o memoria).
- ⬜ Estructura FreeRTOS: tareas de sensor, comunicaciones, actuadores y botón; colas entre ellas
- ⬜ Sensor detrás de `leerOrientacion()` con modo **simulado** y modo **BNO085**
- ⬜ Protocolo de mensajes ESP-NOW (posición, disparo, impacto, órdenes, latido, versión)
- ⬜ **LED RGB** con cambios graduales (±1 % cada X ms, configurable)
- ⬜ **GPIO5** = motor de vibración, con lógica normal/inversa configurable
- ⬜ **Botón**: simple / doble / largo, con antirrebote
- ⬜ Configuración por mensaje (`config`) y guardada en flash (persistente)

## Etapa 5 — Base / pasarela 🟢 🔀
- ⬜ ESP-NOW ↔ MQTT: publica en nombre de los mandos con los **topics obligatorios** (`II16/esp32c3-XXXX/...`)
- ⬜ Latido de los mandos → `conexion` `online:false` si dejan de responder (equivalente a LWT)
- ⬜ Reenvío en tiempo real al PC por USB serie
- ⬜ LED RGB de la base con el estado de la partida (esperando, jugando, fin)
- ⬜ Órdenes individuales y **de grupo** (topic `II16/all/...`)

**Se prueba con** 3 placas del laboratorio: base + 2 mandos simulados.

## Etapa 6 — Juego (web servida por Node-RED) 🟢 🔀
- ⬜ Canvas sencillo: escenario, cursor del cazador, pájaro pilotado
- ⬜ Detección de impacto (la decide un solo sitio: el juego)
- ⬜ Partida: inicio, tiempo, vidas, fin; evento de impacto → vibración en ambos mandos
- ⬜ Escenarios cargados desde MongoDB

**Se prueba con** el mando virtual (ratón) en casa y con los mandos simulados en el laboratorio.

## Etapa 7 — SCADA en Node-RED + MongoDB 🟢 🔀
- ⬜ Guardar con fecha: estado y datos de los nodos, cada disparo, partidas y **log** (con origen MQTT/botón)
- ⬜ Dashboard por pestañas: estado de los nodos, control individual y de grupo, configuración
- ⬜ Histórico: consultas por fechas con atajos (día/semana/mes), gráficas, estadísticas, **CSV**
- ⬜ Estadísticas de juego: % de acierto, tiempo de reacción, mapa de calor de disparos
- ⬜ **Alarmas** con umbrales configurables (p. ej. temperatura de la placa, señal débil, pérdidas
  altas) → aviso en el dashboard + Telegram + log.
  ⚠️ El enunciado pide umbrales de temperatura y humedad: confirmar con el profesor qué parámetros valen.

**Se prueba con** datos de los mandos simulados (o mensajes MQTT inyectados a mano desde Node-RED).

## Etapa 8 — Telegram 🟢 🔀
- ⬜ Bot: `/estado`, `/partida`, `/ranking`, `/actualizar`, control de LED/vibración
- ⬜ Canal: resultados de partidas, récords, alarmas, desconexiones, actualizaciones

## Etapa 9 — FOTA completo 🟢
- ⬜ Base: al arrancar, por MQTT, pulsación larga y periódica (configurable, 0 = no)
- ⬜ Mandos: orden por ESP-NOW desde la base → WiFi temporal → descarga → vuelta a ESP-NOW
- ⬜ Versión de cada nodo visible en el dashboard; no actualizar durante una partida
- ⬜ Demo: cambiar el color del LED en la v2 y `/actualizar`

## Etapa 10 — Soportes de los mandos 🟢 (diseño) / 🔴 (montaje final)
Ver la sección [Soportes de los mandos](#soportes-de-los-mandos) más abajo.
- ⬜ Elegir opción: impresión 3D o cartón / contrachapado
- ⬜ Medir el powerbank, la placa y el módulo BNO085 (medidas del anuncio hasta que llegue)
- ⬜ Diseñar (Tinkercad / Fusion 360) o recortar los soportes
- ⬜ Montar y cablear; comprobar acceso al USB, al botón y que el LED se ve

## Etapa 11 — Integración y robustez 🔴
- ⬜ Cambiar `SENSOR_SIMULADO` → `SENSOR_BNO085` y probar el sistema completo
- ⬜ Pruebas de fallo: mando apagado, WiFi caída, broker caído, sensor desconectado
- ⬜ Prueba completa de una partida de principio a fin, varias veces

## Etapa 12 — Memoria y defensa 🟢 (casi todo)
- ⬜ Memoria PDF: introducción, HW (pines), SW (diagramas, FreeRTOS, robustez, librerías),
  resultados (incluida la prueba de latencia), conclusiones → se puede ir escribiendo desde ya
- ⬜ **Manual de usuario**
- ⬜ Lista de ficheros: `.ino`, flujos Node-RED `.json`, diseños 3D (`.stl`) si se hacen
- ⬜ Vídeo de la demo (por si algo falla el día de la defensa) y ensayo 🔴

---

## Soportes de los mandos

### Opción A — Impresión 3D
**Cazador: forma de pistola**
- **Mango:** hueco a medida del powerbank (se mete por abajo; tapa o ajuste a presión).
  El cable USB sube por dentro hasta la placa.
- **Cuerpo / "cañón":** la placa ESP32-C3 y el BNO085.
  - El BNO085 **bien fijo** (tornillos o encaje) y **alineado con el cañón**: su orientación es la de la pistola.
  - Lo más lejos posible del motor de vibración, para que la vibración no ensucie la lectura.
- **Gatillo:** el pulsador donde va el dedo índice.
- **Motor de vibración:** en el mango o en la parte trasera, para notar el retroceso en la mano.
- **Aberturas:** USB de la placa (para programar sin desmontar), botón BOOT y ventana para ver el LED RGB.
- Que se vea claramente como **un juguete** (colores llamativos, forma simplificada).

**Pájaro: otra forma**
- Opción "avión": pieza plana con forma de **ala delta** que se sujeta en la palma con una correa
  o un velcro; el powerbank va debajo, como "fuselaje".
- Opción "mando": pieza tipo **volante / mando horizontal** que se coge con las dos manos y se
  inclina; el powerbank en el centro.
- El BNO085 en el centro y **plano**: su inclinación es la del avión.
- El botón de turbo bajo el pulgar.

**Notas:** preguntad si la universidad tiene impresora 3D. Primero imprimid una versión rápida y
de baja calidad para comprobar medidas. Guardad los `.stl` en `hardware/3d/`.

### Opción B — Rápida: base sobre el powerbank (sin impresora)
- Un trozo de **cartón duro, contrachapado fino o metacrilato** del tamaño del powerbank.
- Se pega **encima del powerbank** con cinta de doble cara o bridas.
- Encima, la placa y el BNO085 fijos con cinta de doble cara o bridas, y el motor de vibración.
- Para el cazador, el pulsador en el lateral, donde caiga el dedo.
- Para la demo funciona igual de bien; la opción A es solo más vistosa.

**Mismas reglas en las dos opciones:** IMU rígida y bien orientada, motor separado de la IMU,
USB y botón accesibles, LED visible, y cables sujetos para que no se suelten al mover la mano.

---

## Reparto propuesto (ajustadlo)
| Persona | Responsable de | Etapas |
|---|---|---|
| Integrante 1 | Mandos: sensor (simulado y real), puntería, vibración, botones, soportes | 3, 4, 10 |
| Integrante 2 | Base/pasarela, MQTT, FOTA, MongoDB | 2, 5, 7 (datos), 9 |
| Integrante 3 | Juego web, mando virtual, dashboard, Telegram | 6, 7 (interfaz), 8 |

Todos: etapas 11 y 12, y revisar el trabajo de los demás (Pull Requests).

## Riesgos y decisiones pendientes
- **Puntería con IMU (deriva)** → se valida en la etapa 3 en cuanto lleguen los sensores.
- **Vibración vs. IMU**: el motor puede meter ruido en la lectura → separarlos en el soporte y comprobarlo.
- **Alarmas de temperatura/humedad** del enunciado → confirmar con el profesor los parámetros de alarma.
- **Node-RED en Docker en los 3 PCs**: si los 3 están conectados a la vez al mismo broker y a la
  misma base de datos, se duplicarán registros y mensajes de Telegram. Solo una instancia debe
  ser la "oficial"; para desarrollar, usad topics/colecciones de prueba propios o un bot distinto.
- **Botón BOOT**: el enunciado le asigna funciones fijas; si se usa como gatillo, pedir permiso o usar un pulsador aparte.
- **WiFi**: `infind` no conectó; eduroam no vale. Plan: hotspot del móvil (2,4 GHz, WPA2).
- **Impresora 3D**: si no hay, opción B (cartón/contrachapado sobre el powerbank).
