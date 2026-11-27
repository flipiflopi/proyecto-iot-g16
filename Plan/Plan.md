# Plan del proyecto por etapas

Entrega: **~20 de diciembre**. Sin estimaciones por semanas: se avanza lo antes posible.
Código en casa, pruebas en el laboratorio con sus ESP32-C3.

**Arquitectura:** ver [ARQUITECTURA.md](ARQUITECTURA.md) (decisión del 2026-11-23: el juego corre en la base; Node-RED para dashboard y BD).

Cada etapa tiene su carpeta `etapaN/` con su código y una **GUIA.md** detallada (qué hacer y cómo probarlo).
Estado de las guías: **etapa 1** en curso · **etapa 2** completa (código + guía) · **etapas 3-12** borradores pendientes de revisar.

Leyenda: ✅ hecho · ⬜ pendiente · 🟢 se puede hacer **sin los sensores** · 🔴 necesita los sensores · 🔀 se puede repartir entre los 3

---

## Mientras llegan los sensores: qué se puede hacer

**Casi todo.** Solo necesitan las IMU (2 × MPU6050) la **etapa 3** (puntería real) y el **montaje final**.
El resto se hace con las ESP32 del laboratorio, que ya traen todo lo necesario para probar:

| Lo que habrá en el proyecto | Cómo se prueba sin comprarlo |
|---|---|
| IMU MPU6050 (cazador y pájaro) | **Mando simulado**: el ESP32 genera una orientación falsa (círculos, barridos) y la envía igual que el real. O un **mando virtual** con el ratón del PC |
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
- `SENSOR_REAL`: lee el sensor real: **MPU6050** en los dos mandos (se programa cuando lleguen).

Así, **todo lo demás** (ESP-NOW, base, juego, Node-RED, Telegram, FOTA) se desarrolla y se prueba
ahora, y cuando llegue el sensor solo se cambia esa constante.

### Mando virtual con el ratón (para el juego)
Una página o un nodo de Node-RED que convierte el movimiento del ratón en "orientación del mando"
y la inyecta en el mismo sitio por el que llegarán los datos reales. Permite hacer y probar el juego
**en casa, sin ninguna placa**.

---

## Etapa 1 — Organización y compras 🟢
📁 Carpeta y guía: [etapa01-organizacion-y-compras/GUIA.md](etapa01-organizacion-y-compras/GUIA.md)
- ⬜ Hacer el pedido de AliExpress → ver [etapa01-organizacion-y-compras/COMPRAS.md](etapa01-organizacion-y-compras/COMPRAS.md) (lo primero: tarda en llegar)
- ✅ Repositorio GitHub privado creado
- ⬜ Invitar a los otros 2 integrantes (y al profesor si lo pide) como colaboradores
- ⬜ Cada uno: clonar el repo, Arduino IDE con core ESP32 3.x, Node-RED en Docker
- ⬜ Recoger las credenciales del grupo **II16** del Campus Virtual (MQTT y MongoDB) → van en `secrets.h`, **nunca** al repo
- ⬜ Crear el **bot de Telegram** (@BotFather) y el **canal de difusión**
- ⬜ Enviar al profesor la propuesta final **por escrito** (correo) y guardar su respuesta
- ⬜ Preguntar si la universidad tiene **impresora 3D** que podamos usar (ver etapa 10)

**Hecho cuando:** pedido hecho, los 3 con el entorno funcionando y el visto bueno por escrito.

## Etapa 2 — Validación de tecnologías 🟢 🔀
📁 Carpeta y guía: [etapa02-validacion-de-tecnologias/GUIA.md](etapa02-validacion-de-tecnologias/GUIA.md)
Pruebas pequeñas e independientes, cada una en su carpeta dentro de `etapa02-validacion-de-tecnologias/`. Estado al 2026-11-23:
- ✅ **Latencia ESP-NOW** (mando → base → PC): ~4 ms de media. Ver `etapa02-validacion-de-tecnologias/01_latencia_espnow`
- ✅ **ESP-NOW unicast con ACK + canal automático** (02): el mando encuentra solo a la base y los disparos no se pierden (ráfaga 1000/1000, 100 %)
- ✅ **MQTT** con el broker `iot.ac.uma.es` (usuario II16) (03)
- ✅ **FOTA básica** (04; la completa, en la etapa 9)
- ✅ **FreeRTOS** (05), **botón BOOT** (06), **LED RGB** (07)
- ✅ **Mandos → base → PC → Node-RED → web** (08): ~100 msg/s, pérdidas ≤ 0,4 %
- ✅ **Base con ESP-NOW + web (WebSocket 50 Hz) + MQTT a la vez** (09, nueva por el cambio de arquitectura): funciona; arquitectura validada

**Hecho cuando:** cada prueba funciona sola y queda documentada.

## Etapa 3 — Prototipo de puntería con IMU 🔴
📁 Carpeta y guía: [etapa03-punteria-con-imu/GUIA.md](etapa03-punteria-con-imu/GUIA.md)
- ⬜ Leer el MPU6050 (cazador y pájaro) con el DMP → implementar `SENSOR_REAL`
- ⬜ Orientación estable: cuaternión/ángulos; medir la **deriva** en 5-10 min
- ⬜ Calibración: apuntar al centro + botón; botón de recentrar
- ⬜ Comprobar que la **vibración del motor no estropea la lectura** de la IMU
- ⬜ Página web mínima: un punto que sigue a la mano (cazador) y un avión que se inclina (pájaro)

**Hecho cuando:** el cursor sigue bien a la mano y la deriva es aceptable con recentrado.
⚠️ Es el **mayor riesgo técnico**: se hace en cuanto lleguen los sensores; si falla, se replantea.

**Preparable antes de que lleguen (🟢):** instalar la librería del MPU6050, leer su documentación,
dejar escrito el código de lectura y el de calibración, y la página web con el mando virtual.

## Etapa 4 — Firmware común de los nodos 🟢 🔀
📁 Carpeta y guía: [etapa04-firmware-comun/GUIA.md](etapa04-firmware-comun/GUIA.md)
Un único firmware; el rol (BASE / CAZADOR / PÁJARO) se decide por configuración (MAC o memoria).
- ⬜ Estructura FreeRTOS: tareas de sensor, comunicaciones, actuadores y botón; colas entre ellas
- ⬜ Sensor detrás de `leerOrientacion()` con modo **simulado** y modo **real** (MPU6050 en los dos mandos)
- ⬜ Protocolo de mensajes ESP-NOW (posición, disparo, impacto, órdenes, latido, versión)
- ⬜ **LED RGB** con cambios graduales (±1 % cada X ms, configurable)
- ⬜ **GPIO5** = motor de vibración, con lógica normal/inversa configurable
- ⬜ **Botón**: simple / doble / largo, con antirrebote
- ⬜ Configuración por mensaje (`config`) y guardada en flash (persistente)

## Etapa 5 — Base / pasarela (y servidor del juego) 🟢 🔀
📁 Carpeta y guía: [etapa05-base-pasarela/GUIA.md](etapa05-base-pasarela/GUIA.md)
- ⬜ ESP-NOW ↔ MQTT: publica en nombre de los mandos con los **topics obligatorios** (`II16/esp32c3-XXXX/...`)
- ⬜ Latido de los mandos → `conexion` `online:false` si dejan de responder (equivalente a LWT)
- ⬜ ~~Reenvío en tiempo real al PC por USB serie~~ → **servidor web + WebSocket 50 Hz** en la propia base (ver [ARQUITECTURA.md](ARQUITECTURA.md))
- ⬜ LED RGB de la base con el estado de la partida (esperando, jugando, fin)
- ⬜ Órdenes individuales y **de grupo** (topic `II16/all/...`)

**Se prueba con** 3 placas del laboratorio: base + 2 mandos simulados.

## Etapa 6 — Juego (lógica y web en la base) + dashboard en Node-RED 🟢 🔀
📁 Carpeta y guía: [etapa06-juego-web/GUIA.md](etapa06-juego-web/GUIA.md)
- ⬜ Canvas sencillo (página en LittleFS de la base): escenario, cursor del cazador, pájaro pilotado
- ⬜ Detección de impacto (la decide un solo sitio: la **base**)
- ⬜ **Dashboard en Node-RED** (`@flowfuse/node-red-dashboard`): nombres, quién es quién, duración, iniciar partida, historial; configuración a la base por MQTT
- ⬜ Partida según las reglas de [etapa06-juego-web/REGLAS.md](etapa06-juego-web/REGLAS.md) (60 s, 3 vidas, puntos según el tiempo); impacto → vibración en ambos mandos
- ⬜ Escenarios cargados desde MongoDB

**Se prueba con** el mando virtual (ratón) en casa y con los mandos simulados en el laboratorio.

## Etapa 7 — Base de datos (Node-RED + MongoDB) 🟢 🔀
📁 Carpeta y guía: [etapa07-base-de-datos/GUIA.md](etapa07-base-de-datos/GUIA.md)
- ⬜ Guardar con fecha el **log** de eventos: conexiones/desconexiones, disparos, impactos, órdenes, actualizaciones (con su origen)
- ⬜ Guardar las **partidas** (jugadores, duración, puntuación) y los **escenarios**
- ⬜ Consultas para el ranking de Telegram y para cargar los escenarios en el juego

**Se prueba con** datos de los mandos simulados (o mensajes MQTT inyectados a mano desde Node-RED).

## Etapa 8 — Telegram 🟢 🔀
📁 Carpeta y guía: [etapa08-telegram/GUIA.md](etapa08-telegram/GUIA.md)
- ⬜ Bot: `/estado`, `/partida`, `/ranking`, `/actualizar`, control de LED/vibración
- ⬜ Canal: resultados de partidas, récords, desconexiones, actualizaciones

## Etapa 9 — FOTA completo 🟢
📁 Carpeta y guía: [etapa09-fota/GUIA.md](etapa09-fota/GUIA.md)
- ⬜ Base: al arrancar, por MQTT, pulsación larga y periódica (configurable, 0 = no)
- ⬜ Mandos: orden por ESP-NOW desde la base → WiFi temporal → descarga → vuelta a ESP-NOW
- ⬜ Versión de cada nodo consultable por Telegram (`/estado`); no actualizar durante una partida
- ⬜ Demo: cambiar el color del LED en la v2 y `/actualizar`

## Etapa 10 — Soportes de los mandos 🟢 (diseño) / 🔴 (montaje final)
📁 Carpeta y guía: [etapa10-soportes-de-los-mandos/GUIA.md](etapa10-soportes-de-los-mandos/GUIA.md)
Ver la sección [Soportes de los mandos](#soportes-de-los-mandos) más abajo.
- ⬜ Elegir opción: impresión 3D o cartón / contrachapado
- ⬜ Medir el powerbank, la placa y los módulos MPU6050 (medidas del anuncio hasta que lleguen)
- ⬜ Diseñar (Tinkercad / Fusion 360) o recortar los soportes
- ⬜ Montar y cablear; comprobar acceso al USB, al botón y que el LED se ve

## Etapa 11 — Integración y robustez 🔴
📁 Carpeta y guía: [etapa11-integracion-y-robustez/GUIA.md](etapa11-integracion-y-robustez/GUIA.md)
- ⬜ Cambiar `SENSOR_SIMULADO` → `SENSOR_REAL` y probar el sistema completo
- ⬜ Pruebas de fallo: mando apagado, WiFi caída, broker caído, sensor desconectado
- ⬜ Prueba completa de una partida de principio a fin, varias veces

## Etapa 12 — Memoria y defensa 🟢 (casi todo)
📁 Carpeta y guía: [etapa12-memoria-y-defensa/GUIA.md](etapa12-memoria-y-defensa/GUIA.md)
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
- **Cuerpo / "cañón":** la placa ESP32-C3 y el MPU6050.
  - El MPU6050 **bien fijo** (tornillos o encaje) y **alineado con el cañón**: su orientación es la de la pistola.
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
- El MPU6050 en el centro y **plano**: su inclinación es la del avión.
- El botón de turbo bajo el pulgar.

**Notas:** preguntad si la universidad tiene impresora 3D. Primero imprimid una versión rápida y
de baja calidad para comprobar medidas. Guardad los `.stl` en `etapa10-soportes-de-los-mandos/`.

### Opción B — Rápida: base sobre el powerbank (sin impresora)
- Un trozo de **cartón duro, contrachapado fino o metacrilato** del tamaño del powerbank.
- Se pega **encima del powerbank** con cinta de doble cara o bridas.
- Encima, la placa y la IMU (MPU6050) fijos con cinta de doble cara o bridas, y el motor de vibración.
- Para el cazador, el pulsador en el lateral, donde caiga el dedo.
- Para la demo funciona igual de bien; la opción A es solo más vistosa.

**Mismas reglas en las dos opciones:** IMU rígida y bien orientada, motor separado de la IMU,
USB y botón accesibles, LED visible, y cables sujetos para que no se suelten al mover la mano.

---

## Riesgos y decisiones pendientes
- **Puntería con IMU (deriva)** → se valida en la etapa 3 en cuanto lleguen los sensores.
- **Vibración vs. IMU**: el motor puede meter ruido en la lectura → separarlos en el soporte y comprobarlo.
- **Deriva del yaw del cazador** (MPU6050, sin magnetómetro): ~1-2° por partida de 30-60 s → calibración al arrancar y recentrado.
- **Node-RED en Docker en los 3 PCs**: si los 3 están conectados a la vez al mismo broker y a la
  misma base de datos, se duplicarán registros y mensajes de Telegram. Solo una instancia debe
  ser la "oficial"; para desarrollar, usad topics/colecciones de prueba propios o un bot distinto.
- **Botón BOOT**: el enunciado le asigna funciones fijas; si se usa como gatillo, pedir permiso o usar un pulsador aparte.
- **WiFi**: `infind` no conectó; eduroam no vale. **Decidido:** hotspot del móvil o router propio (2,4 GHz, WPA2); la base, el PC y el navegador en esa red (opción A de [ARQUITECTURA.md](ARQUITECTURA.md)).
- ~~**Base sobrecargada** (ESP-NOW + web + MQTT a la vez)~~ → descartado: la prueba 09 funcionó. Plan B si hiciera falta: juego en Node-RED por USB (la 08).
- **Impresora 3D**: si no hay, opción B (cartón/contrachapado sobre el powerbank).
