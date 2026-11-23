# Etapa 2 — Validación de tecnologías (sin sensores)

> **Estado:** código completo ✅ · compilado para ESP32-C3 ✅ · pruebas en placa: 01 y 03-07 hechas ✅, 02 y 08 **pendientes** · [prioridades](#prioridad-de-las-pruebas)

Objetivo: comprobar, **una por una y por separado**, todas las tecnologías del proyecto antes de
juntarlas. Ninguna prueba necesita las IMU (BNO055 y MPU6050): todo se hace con las ESP32-C3 del laboratorio.

| # | Prueba | Qué valida | Placas | Estado |
|---|---|---|---|---|
| 01 | [Latencia ESP-NOW](#01--latencia-esp-now) | Que ESP-NOW es lo bastante rápido para jugar | 2 | ✅ ~4 ms |
| 02 | [Emparejamiento ESP-NOW](#02--emparejamiento-esp-now-canal-automático--unicast-con-ack) | El mando encuentra solo a la base (canal y MAC) y los disparos llegan con confirmación | 2 | ⬜ |
| 03 | [MQTT](#03--mqtt-con-el-broker-de-la-universidad) | Broker de la uni, topics del grupo, retenido, LWT, configuración | 1 | ✅ |
| 04 | [FOTA](#04--fota-actualización-por-wifi) | Actualizar el programa por WiFi | 1 | ✅ |
| 05 | [FreeRTOS](#05--esqueleto-freertos) | Tareas, colas, temporizador, mutex | 1 | ✅ 10 min estable |
| 06 | [Botón BOOT](#06--botón-boot-simple-doble-y-larga) | Pulsación simple / doble / larga con antirrebote | 1 | ✅ |
| 07 | [LED RGB](#07--led-rgb-gradual) | Cambios graduales de color e intensidad | 1 | ✅ |
| 08 | [Mandos → web](#08--mandos--base--pc--node-red--web) | La cadena completa hasta una página web en Node-RED | 3 | ⬜ |

Las pruebas son independientes: cada integrante puede hacer unas distintas.

### Prioridad de las pruebas
No todas aportan lo mismo. Prioridad = las que prueban algo que **puede fallar de verdad** y que, si
falla tarde, rompe el proyecto. El resto se comprueba rápido y sin protocolo.

| Prioridad | Pruebas | Por qué |
|---|---|---|
| 🔴 **Alta** (hacerlas completas) | **02**, **03**, **08** (y 01, ya hecha) | Radio y red: el cambio de canal de la base al conectarse a la WiFi (02), el broker, la clave y los permisos del grupo (03) y la cadena de tiempo real del juego (08). Si fallan en la integración, son difíciles de diagnosticar |
| 🟡 **Baja** (comprobación rápida) | **06**, **07** | Código local, sin radio ni red. Se vuelve a montar en la **etapa 4 (firmware común)**. Basta con verlas funcionar un par de minutos en una C3 y anotar "funciona"; no hace falta el protocolo de 20 pulsaciones |
| ✅ **Hecha** | **05** | Superada en ESP32 clásica (ver *Resultados*). Se da por buena: el esqueleto se reescribe en la etapa 4 |
| ⏩ **Se pasa a la etapa 9** | **04** | En la etapa 9 se hace la FOTA con el servidor de la uni; probarla antes con un servidor en el PC duplica trabajo. Opcional si sobra tiempo |

Probar en la **ESP32 clásica** solo sirve para lo que no depende de la placa (red, MQTT, lógica):
tiene otro chip USB, otros pines y no tiene LED RGB, así que no garantiza nada de la C3.

Orden recomendado: **03** (vale con cualquier ESP32) → **02** → **08** → 06 y 07 rápidas en el laboratorio.

---

## Preparación (una sola vez)

### En el PC
1. **Arduino IDE 2** con el soporte **esp32 de Espressif 3.x** (Gestor de placas → "esp32").
2. **Librerías** (Programa → Incluir librería → Gestionar librerías):
   - `PubSubClient` de Nick O'Leary (prueba 03)
   - `ArduinoJson` de Benoit Blanchon, versión 7 (prueba 03)
3. **Python** y sus paquetes:
   ```bash
   pip install pyserial paho-mqtt
   ```
4. **Node-RED en Docker** (solo prueba 08; ver allí cómo arrancarlo).

### Al subir un programa a una ESP32-C3
- *Herramientas → Placa:* **ESP32C3 Dev Module**.
- *Herramientas → Puerto:* el COM de esa placa. Para saber cuál es: conectad las placas **de una en una** y mirad qué COM aparece. Poned cinta con el nombre en cada placa.
- Si **el monitor serie no muestra nada**: cambiad *Herramientas → USB CDC On Boot* (Enabled ↔ Disabled) y volved a subir.
- Si **la subida falla**: mantened pulsado **BOOT** al conectar el cable y volved a intentarlo.
- **Un puerto COM solo lo puede usar un programa a la vez**: cerrad el monitor serie antes de usar un script de Python con esa placa.
- Para tener varias placas abiertas a la vez: **una ventana de Arduino IDE por sketch**.

### Contraseñas: `secrets.h`
Las pruebas que usan WiFi (02 con WiFi, 03, 04) tienen un `secrets.example.h` en su carpeta:
1. Copiadlo en la **misma carpeta** con el nombre `secrets.h`.
2. Rellenadlo (WiFi y, en la 03, la clave MQTT del grupo II16 del Campus Virtual).
3. `secrets.h` está en `.gitignore`: **nunca** se sube a GitHub. No lo quitéis de ahí.

### WiFi
- **eduroam no sirve** y **infind no conectó** en nuestras pruebas → usad el **hotspot del móvil**.
- El hotspot tiene que emitir en **2,4 GHz** (iPhone: *Maximizar compatibilidad*; Android: *Banda 2,4 GHz*) y con **WPA2**.

---

## 01 · Latencia ESP-NOW
**Carpeta:** `01_latencia_espnow/` · **Hecha ✅** · Resultados y pasos en [RESULTADOS.md](01_latencia_espnow/RESULTADOS.md).

Resumen: ~1,8 ms solo radio, ~3,8 ms pasando por el PC, ~4 ms con la base conectada a la WiFi.

---

## 02 · Emparejamiento ESP-NOW (canal automático + unicast con ACK)
**Carpeta:** `02_emparejamiento_espnow/` · **Placas:** 2 (BASE y MANDO)

**Qué valida**
1. **Canal automático:** la base está en un canal que el mando no conoce (canal 6 sin WiFi, o el del router con WiFi). El mando prueba los canales 1 a 13 hasta que la base le contesta.
2. **Unicast con ACK:** después, el mando envía los disparos **a la MAC de la base** (no a todos). ESP-NOW confirma cada entrega y reintenta solo. Es como enviaremos los eventos que no se pueden perder.
3. **Recuperación:** si la base se apaga o cambia de canal, el mando la vuelve a buscar solo.

**Pasos**
1. Subid `BaseEmparejamiento` a una placa (deja `CONECTAR_WIFI 0`). Abrid su monitor serie a **115200**.
   Debe decir `BASE lista en el canal 6`.
2. Subid `MandoEmparejamiento` a otra placa. Abrid su monitor serie a **115200**.
   Veréis cómo prueba canales y se para en el 6:
   ```
   canal  5...
   canal  6... ¡RESPUESTA!
   Base encontrada: XX:XX:XX:XX:XX:XX en el canal 6
   ```
3. **Disparos:** pulsad BOOT en el mando varias veces. En el mando: `Disparo #N -> CONFIRMADO por la base`. En la base: `[EVENTO] Disparo #N ...`.
4. **Ráfaga:** en el mando escribid `r` + Enter. Envía 1000 eventos a 100 Hz y muestra cuántos se confirmaron. Después, en la base escribid `e` + Enter para ver cuántos le llegaron.
5. **Recuperación:** desconectad la base. Pulsad BOOT en el mando: tras unos fallos dirá `La base no responde` y volverá a buscarla. Conectad la base: el mando la encuentra solo.
6. **Con WiFi (la situación real):** en la base, copiad `secrets.example.h` como `secrets.h`, rellenadlo, poned `CONECTAR_WIFI 1` y subidla. La base dirá el canal del router. Reiniciad el mando (botón RST o desconectar/conectar): debe encontrarla en ese canal sin tocar nada.

**✅ Superada si:** el mando encuentra la base en cualquier canal, la ráfaga confirma **≥ 99 %**, la base recibe el mismo número que el mando confirma, y el mando se recupera solo al desconectar la base.

**Problemas típicos**
- *El mando recorre los canales sin parar:* la base no está encendida, o no ha arrancado bien (mirad su monitor serie).
- *Pocas confirmaciones en la ráfaga:* acercad las placas y repetid; probad en otro momento (mucho tráfico WiFi en el laboratorio).

---

## 03 · MQTT con el broker de la universidad
**Carpeta:** `03_mqtt/` · **Placas:** 1 · **Necesita:** WiFi con internet (hotspot) y la clave del grupo II16

**Qué valida** (con los topics del enunciado):
- `II16/esp32c3-XXXXXX/conexion` → `{"CHIPID":..,"online":true}` **retenido**; y si la placa se cae, el broker publica solo `online:false` (**LWT**, última voluntad).
- `II16/esp32c3-XXXXXX/datos` → publicación periódica (30 s en esta prueba).
- `II16/esp32c3-XXXXXX/config` → `{"envia":10}` cambia el periodo; `null` o ausente = no cambia.
- `II16/esp32c3-XXXXXX/ping` → responde en `.../pong` (para medir la latencia por MQTT).
- Reconexión automática si se cae la WiFi o el broker.

**Herramienta para ver los mensajes** (una de las dos):
- **MQTT Explorer** (programa gratuito con ventana): conexión a `iot.ac.uma.es`, puerto 1883, usuario `II16` y la clave del grupo.
- **Docker** (sin instalar nada más):
  ```bash
  docker run --rm -it eclipse-mosquitto mosquitto_sub -h iot.ac.uma.es -u II16 -P CLAVE_DEL_GRUPO -t "II16/#" -v
  ```

**Pasos**
1. En `NodoMQTT/`, copiad `secrets.example.h` como `secrets.h` y rellenad WiFi + clave MQTT.
2. Subid `NodoMQTT` y abrid el monitor serie a **115200**. Apuntad el **CHIPID** que imprime (`esp32c3-XXXXXX`).
   Debe decir `WiFi OK`, `Conectando a MQTT ... OK` y publicar en `.../datos` cada 30 s.
3. Con MQTT Explorer o `mosquitto_sub`, comprobad que llegan `conexion` (`online:true`) y `datos`.
4. **Retenido:** cerrad y volved a abrir la herramienta. Aunque la placa no haya vuelto a publicar, `conexion` aparece al momento (lo guarda el broker).
5. **Configuración:** publicad en `II16/<CHIPID>/config` el texto `{"envia":10}`.
   - MQTT Explorer: sección *Publish*.
   - Docker:
     ```bash
     docker run --rm eclipse-mosquitto mosquitto_pub -h iot.ac.uma.es -u II16 -P CLAVE_DEL_GRUPO -t "II16/esp32c3-XXXXXX/config" -m "{\"envia\":10}"
     ```
   El monitor serie dirá `nuevo periodo de envío = 10 s` y los datos llegarán cada 10 s.
   Probad también `{"envia":null}` (no debe cambiar nada) y `hola` (JSON no válido: se ignora sin colgarse).
6. **LWT:** desconectad el cable USB de golpe. En **~20-25 s** el broker publicará `{"CHIPID":..,"online":false}`.
7. **Reconexión:** quitad el hotspot del móvil 30 s y volved a ponerlo: la placa se reconecta sola y publica `online:true`.
8. **Latencia MQTT:**
   ```bash
   python medir_latencia_mqtt.py esp32c3-XXXXXX
   ```
   Pide la clave del grupo y hace 50 pings. Anotad la media (será bastante mayor que ESP-NOW porque pasa por internet; por eso el juego en tiempo real no va por MQTT).

**✅ Superada si:** se ven `conexion` (retenido) y `datos`, `config` cambia el periodo, el LWT aparece al desconectar, y se reconecta sola.

**Problemas típicos**
- `FALLO (estado 5)`: usuario o clave del grupo incorrectos.
- `FALLO (estado -2)`: no hay conexión con el broker (¿el hotspot tiene datos?).
- Con el usuario `II16` **solo** se puede publicar y suscribir en topics que empiecen por `II16/`.

---

## 04 · FOTA (actualización por WiFi)
**Carpeta:** `04_fota/` · **Placas:** 1 · **Necesita:** el PC y la placa en la **misma red** (los dos conectados al hotspot)

**Qué valida:** que la placa se actualiza sola desde un servidor, al arrancar, con pulsación larga del botón, por orden remota (aquí `u` por el monitor serie; en el proyecto será por MQTT) o periódicamente. El LED muestra la versión: **v1 verde, v2 morado**.

Para la prueba el servidor es **vuestro PC** (un servidor web de Python). El de la universidad
(`https://iot.ac.uma.es:1880/esp8266-ota`) se integrará en la etapa 9 siguiendo el ejemplo del Campus Virtual.

**Pasos**
1. Conectad el PC al hotspot del móvil y mirad su IP: en una terminal, `ipconfig` → *Dirección IPv4* del adaptador WiFi (p. ej. `192.168.43.20`).
2. En `NodoFOTA/`, copiad `secrets.example.h` como `secrets.h`: WiFi del hotspot y, en las dos URL, la IP del PC.
3. **Versión 1 por USB:** subid `NodoFOTA` tal cual (`FW_VERSION 1`). LED **verde**. El monitor serie dirá que el servidor no está disponible (aún no lo hemos arrancado): es correcto, sigue funcionando.
4. **Preparar la versión 2 (sin subirla por USB):**
   - En el código, cambiad `#define FW_VERSION 1` por `2`.
   - *Programa → Exportar binario compilado*.
   - En la carpeta del sketch aparece `build/esp32.esp32.esp32c3/NodoFOTA.ino.bin`. Copiadlo a `04_fota/servidor/` con el nombre **`firmware.bin`**.
   - Editad `04_fota/servidor/version.txt` y poned `2`.
   - Volved a poner `FW_VERSION 1` en el código (la placa sigue teniendo la 1).
5. **Arrancar el servidor** en una terminal, dentro de `04_fota/servidor/`:
   ```bash
   python -m http.server 8000
   ```
   Si Windows pregunta por el firewall, permitid el acceso en **redes privadas**. Comprobad desde el navegador del PC: `http://IP_DEL_PC:8000/version.txt` debe mostrar `2`.
6. **Actualizar:** mantened pulsado **BOOT más de 1 s** (o escribid `u` + Enter). Veréis:
   ```
   [FOTA] Versión en el servidor: 2
   [FOTA] Descargando ...  10 % ... 100 %
   ```
   La placa se reinicia sola: `firmware versión 2` y LED **morado**. En la terminal del servidor aparecen las descargas (`GET /firmware.bin 200`).
7. **Al arrancar:** preparad una versión 3 igual (LED azul), `version.txt` = 3, y pulsad **RST**: se actualiza nada más arrancar.
8. **Robustez:**
   - Parad el servidor (Ctrl+C) y pulsad BOOT largo: dice `Servidor no disponible` y sigue funcionando.
   - Poned en `version.txt` un número mayor pero borrad `firmware.bin`: da error de descarga y **sigue con la versión anterior**.

**✅ Superada si:** pasa de v1 a v2 sin cable, actualiza al arrancar, y ante un fallo sigue funcionando con la versión que tenía.

**Problemas típicos**
- `No se pudo leer ... (HTTP -1)`: IP mal puesta, PC en otra red, o el firewall de Windows lo bloquea.
- La placa no cambia de color tras actualizar: comprobad que exportasteis el binario **después** de cambiar `FW_VERSION`.

---

## 05 · Esqueleto FreeRTOS
**Carpeta:** `05_freertos/` · **Placas:** 1

**Qué valida:** la estructura del firmware real con el sensor simulado:

```
tareaSensor (100 Hz) --colaOrientacion (buzón: solo el último dato)--> tareaComunicaciones (50 Hz)
tareaBoton (cada 5 ms) --colaEventos--> tareaEventos (prioridad más alta)
temporizador "latido" (cada 2 s) --colaEventos--> tareaEventos
tareaMonitor (cada 10 s): pila libre de cada tarea y memoria
Mutex para que las tareas no mezclen sus mensajes en el monitor serie
```

**Pasos**
1. Subid `EsqueletoFreeRTOS` y abrid el monitor serie a **115200**.
2. Cada segundo: `[COMS] yaw ... pitch ... | sensor 100 Hz, envíos 50 Hz`. Los valores giran (círculo simulado) y la *edad del dato* es pequeña (< 10 ms).
3. Cada 2 s: `[EVENTO] latido (temporizador software)`.
4. Pulsad BOOT: `[EVENTO] DISPARO (latencia de reacción X ms)`, con X de unos pocos ms aunque las otras tareas estén trabajando.
5. Cada 10 s: `[MONITOR] pila libre ...`. **Apuntad estos números para la memoria**: si alguno baja de ~200, esa tarea necesita más pila.

**✅ Superada si:** sensor ~100 Hz y envíos ~50 Hz estables, latidos cada 2 s, los disparos se notan al momento, ningún mensaje sale mezclado y no se reinicia nunca (dejadlo 10 min).

**Para entenderlo (y explicarlo en la defensa)**
- Cambiad la prioridad de `tareaEventos` a 1: los disparos siguen funcionando, pero la latencia puede subir.
- La `colaOrientacion` tiene longitud 1 y se usa con `xQueueOverwrite`: a comunicaciones solo le interesa el **último** dato; los eventos, en cambio, van en una cola normal porque **no se pueden perder**.

---

## 06 · Botón BOOT: simple, doble y larga
**Carpeta:** `06_boton/` · **Placas:** 1

**Qué valida** lo que pide el enunciado:
| Pulsación | Acción |
|---|---|
| Simple | El LED alterna entre apagado y su estado anterior |
| Doble (2 pulsaciones en < 300 ms) | LED al máximo conservando el color |
| Larga (≥ 1 s) | Comprobar actualización FOTA (aquí simulado) |
Con **antirrebote** de 30 ms; muestra cuántos rebotes ha filtrado.

**Pasos**
1. Subid `BotonBoot` y abrid el monitor serie a **115200**. El LED se enciende azul tenue.
2. **Simple:** una pulsación corta → `[SIMPLE] LED APAGADO`; otra → `ENCENDIDO`. Se confirma ~0,3 s después de soltar (espera por si es doble).
3. **Doble:** dos pulsaciones rápidas → `[DOBLE] LED al máximo`.
4. **Larga:** mantened > 1 s → `[LARGA]` aparece **sin soltar**.
5. Haced **20 de cada tipo** y apuntad los fallos (una doble detectada como dos simples, etc.).
6. Si os cuesta hacer la doble, subid `DOBLE_MS` a 400; si la simple tarda demasiado, bajadlo a 250.

**✅ Superada si:** ≤ 1 fallo de cada 20 en cada tipo, y nunca una pulsación genera dos eventos (eso sería un rebote no filtrado).

---

## 07 · LED RGB gradual
**Carpeta:** `07_led_rgb/` · **Placas:** 1

**Qué valida:** el LED cambia de brillo y de color **poco a poco** (±1 % cada 10 ms por defecto, configurable) y, al terminar, se manda **un único** mensaje con el valor final (como pide el enunciado para `led/brillo/estado`).

**Pasos** (monitor serie a **115200**, con fin de línea **"Nueva línea"**)
1. Subid `LedRGB`. Se enciende al 30 % en azul claro y aparece una línea JSON con el estado.
2. `b 100` → sube al máximo en ~0,7 s (70 pasos × 10 ms). Al terminar, **una** línea: `{"LED":100,...}`.
3. `b 0` → baja hasta apagarse en ~1 s.
4. `v 50` y luego `b 100` → ahora tarda ~5 s (50 ms por cada 1 %).
5. `c 255 0 0` → pasa a rojo de forma suave. `c 0 255 0` → a verde.
6. `off` y `on` → se apaga y vuelve al brillo anterior.
7. Mientras cambia, mandad otra orden (p. ej. `b 20` a mitad de subida): debe ir hacia el nuevo valor sin saltos.

**✅ Superada si:** todos los cambios son graduales, el tiempo coincide con la velocidad configurada, y sale **un solo** mensaje por cambio.

---

## 08 · Mandos → base → PC → Node-RED → web
**Carpeta:** `08_nodered_web/` · **Placas:** 3 (o 2 con un solo mando)

**Qué valida:** la cadena de tiempo real completa del juego, con mandos simulados:
```
MandoSimulado (id 1, cazador) --ESP-NOW-->
                                          BaseSerie --USB--> puente_serie_udp.py --UDP 5005--> Node-RED --WebSocket--> página /etapa2
MandoSimulado (id 2, pájaro)  --ESP-NOW-->
```
La página dibuja el **punto de mira** del cazador (hace círculos) y el **avión** del pájaro (se inclina).

> ¿Por qué un puente en Python? Node-RED corre en **Docker** y, en Windows, un contenedor **no ve los puertos COM**. El puente lee el COM en el PC y se lo pasa a Node-RED por UDP.

**Pasos**
1. **Node-RED con el puerto UDP 5005 abierto.** El contenedor tiene que publicar ese puerto:
   ```bash
   docker run -d --name nodered -p 1880:1880 -p 5005:5005/udp -v nodered_data:/data nodered/node-red
   ```
   ⚠️ Si ya tenéis un contenedor de Node-RED **sin** `-p 5005:5005/udp`, hay que recrearlo. **Antes**, exportad vuestros flujos (*Menú → Exportar → Todos los flujos*) por si no usabais un volumen. Si usabais un volumen (`-v ...:/data`), al recrearlo con el mismo volumen se conservan.
2. **Importar el flujo:** en `http://localhost:1880` → *Menú → Importar* → seleccionad `flujo_etapa2.json` → *Importar* → **Deploy**. Aparece la pestaña "Etapa 2 - mandos a la web".
3. **Probar sin placas (en casa):** en una terminal,
   ```bash
   python simular_mandos_udp.py
   ```
   Abrid `http://localhost:1880/etapa2`: debe poner **conectado**, unos **100 msg/s**, el punto de mira girando y el avión balanceándose. Si funciona, el lado de Node-RED está bien. Paradlo con Ctrl+C.
4. **Con las placas (laboratorio):**
   - Subid `BaseSerie` a la placa base.
   - Subid `MandoSimulado` a otra placa (`ID_MANDO 1`).
   - Cambiad a `#define ID_MANDO 2` y subidlo a la tercera.
   - Alimentad los mandos (USB del PC o powerbank).
5. Con el **monitor serie de la base cerrado**, arrancad el puente con el COM de la base:
   ```bash
   python puente_serie_udp.py COMx
   ```
   Cada segundo: `reenviadas a Node-RED en el último segundo: ~100` (50 por mando). Cada 5 s, una línea `BASE:` con paquetes recibidos y perdidos de cada mando.
6. Abrid `http://localhost:1880/etapa2`: mismo resultado que con el simulador, pero con datos reales por ESP-NOW.
7. **Disparo:** pulsad BOOT en el mando 1: el punto de mira hace un destello.
8. **Pérdida de un mando:** desconectad un mando: en la web aparece `[sin datos]` y se pone gris en medio segundo. Al conectarlo vuelve solo.

**✅ Superada si:** la web se mueve con fluidez, sin tirones; ~50 msg/s por mando; pocas pérdidas (< 2 %); el disparo se ve al instante; y un mando que se desconecta se detecta.

**Problemas típicos**
- *La web dice "desconectado":* el flujo no está desplegado (Deploy) o Node-RED no está arrancado.
- *"conectado" pero 0 msg/s:* el puerto 5005/udp no está publicado en Docker, o el puente/simulador no está funcionando.
- *El puente da "Acceso denegado":* el monitor serie de la base está abierto.

---

## Reparto de tareas: antes del laboratorio (3 PCs + 1 ESP32 clásica)

Fuera del laboratorio solo tenemos **3 PCs y 1 ESP32 clásica** (no C3). Con eso se puede adelantar
casi todo. La 02 y la 08 con placas se quedan para el laboratorio porque necesitan 2-3 placas.

### Cambios para la ESP32 clásica (no subirlos a `main`)
- *Herramientas → Placa:* **ESP32 Dev Module** (no "ESP32C3 Dev Module").
- Lleva un chip USB-serie (CP2102 o CH340): si no aparece ningún COM, falta instalar su driver.
  Lo de *USB CDC On Boot* no se aplica a esta placa.
- **Botón BOOT = GPIO0** (en la C3 es el 9): en 05, 06 y 04 cambiad `PIN_BOTON = 9` por `PIN_BOTON = 0`.
  El GPIO0 también es pin de arranque: si está pulsado al enchufar o al pulsar EN, entra en modo programación.
- **No tiene LED RGB integrado:** en 06 y 04 el LED no hace nada (el código lo detecta), pero todo
  sale por el monitor serie. En la 04 la versión se ve en el mensaje `firmware versión N`.
- ⚠️ **07 (LED RGB): cambiad `PIN_RGB = 8` por `PIN_RGB = 4` ANTES de subirlo.** Sin LED integrado
  el código usa el GPIO8, que en la ESP32 clásica es de la **memoria flash**: la placa se cuelga o
  se reinicia en bucle.
- En la 04 el binario exportado sale en `build/esp32.esp32.esp32/` (no en `esp32c3`).
- En la 03 el CHIPID saldrá como `esp32c3-XXXXXX`: es solo el nombre, no importa.

Son cambios locales para probar: al terminar, deshacedlos (`git restore <archivo>`) para que el
código de `main` siga preparado para la C3.

### Iván: placa sin red (05, 06, 07)
> Actualizado según la [prioridad de las pruebas](#prioridad-de-las-pruebas): la 06 y la 07 pasan a ser
> una comprobación rápida en el laboratorio con una C3. Iván pasa la ESP32 a David para la **03**.

- [x] **05 FreeRTOS** completa, con `PIN_BOTON = 0`: 605 s sin reinicios (ver *Resultados*).
- [ ] ~~06 Botón, 20 pulsaciones de cada tipo~~ → comprobación rápida en el laboratorio con una C3.
- [ ] ~~07 LED RGB, solo la lógica~~ → comprobación rápida en el laboratorio con una C3 (con el LED de verdad).
- [x] Rellenar sus filas en la tabla de resultados y anotar los problemas.

### David: placa con red (03, 04)
Usa la ESP32 cuando Iván termine. Necesita el **hotspot del móvil** (2,4 GHz, WPA2, con datos) y la
**clave MQTT del grupo II16** (Campus Virtual).
- [ ] **03 MQTT** completa: `conexion` retenido, `datos`, `config` (`{"envia":10}`, `null`, `hola`),
      LWT al desconectar el USB y reconexión al quitar el hotspot. Miguel mira los mensajes desde su PC.
- [ ] Medir la latencia MQTT con `python medir_latencia_mqtt.py <CHIPID>` y apuntar la media.
- [ ] *(Opcional, pasa a la etapa 9)* **04 FOTA**, con `PIN_BOTON = 0`: v1 → v2 sin cable, actualización al
      arrancar y robustez (servidor parado, falta `firmware.bin`). El servidor (`python -m http.server 8000`)
      puede ser el PC de Miguel, conectado al mismo hotspot.
- [ ] Rellenar sus filas en la tabla de resultados y anotar los problemas.

### Miguel: los 3 PCs y Node-RED (preparación y 08 sin placas)
- [x] **Dejar preparados los 3 PCs** (ver *Preparación*): Arduino IDE 2 + soporte esp32 3.x,
      librerías `PubSubClient` y `ArduinoJson` v7, `pip install pyserial paho-mqtt`, y Docker.
      Probar que cada PC compila un sketch para **ESP32C3 Dev Module**, para no perder tiempo en el laboratorio.
      - [x] PC de Miguel (2026-11-23): esp32 3.3.12, `PubSubClient` 2.8, `ArduinoJson` 7.4.3, Python 3.14.10 con
            `pyserial` 3.5 y `paho-mqtt` 2.2.0, Docker con Node-RED. `NodoMQTT` (03) compila para la C3 (74 % flash, 11 % RAM)
      - [x] PC de Iván (2026-11-23, todo instalado según el grupo)
      - [x] PC de David (2026-11-23, todo instalado según el grupo)
- [x] **08, parte de Node-RED:** arrancar Node-RED con `-p 5005:5005/udp`, importar `flujo_etapa2.json`,
      ejecutar `python simular_mandos_udp.py` y comprobar en `http://localhost:1880/etapa2`:
      "conectado", ~100 msg/s, el punto de mira girando y el avión balanceándose.
- [x] Ayudar a David: en la 03, ver los mensajes con **MQTT Explorer** (`iot.ac.uma.es`, 1883, usuario
      `II16`); en la 04, hacer de **servidor FOTA**.
- [x] Anotar en la tabla los problemas de instalación (sirven para la memoria). Iván y David: sin problemas.

### Después, en el laboratorio (con las 3 C3, todos)
- [ ] **02** Emparejamiento ESP-NOW (2 placas).
- [ ] **06** y **07**, comprobación rápida en una C3 con los pines originales (unos minutos, sin protocolo).
- [ ] **08** completa: base + 2 mandos simulados → puente → Node-RED → web.

---

## Resultados (rellenad al hacer cada prueba)

| # | Fecha | Quién | Resultado | Notas |
|---|---|---|---|---|
| 01 | | | ✅ ~4 ms | Ver RESULTADOS.md |
| 02 | | | | % confirmados en ráfaga: |
| 03 | | | ✅ | Marcada como hecha |
| 04 | | | ✅ | Marcada como hecha (la FOTA definitiva, en la etapa 9) |
| 05 | 2026-11-23 | Iván | ✅ | ESP32 clásica (`PIN_BOTON = 0`). 605 s sin reinicios; sensor 100 Hz y envíos 50 Hz estables; latido cada 2 s; 6 disparos con latencia 0 ms; heap libre constante (320468 B, sin fugas). Pila libre mínima: sensor 2276, coms 2028, botón 1468, eventos 2620 (**bytes**, aunque el programa diga "palabras": en ESP32 `uxTaskGetStackHighWaterMark` devuelve bytes). 1 línea con bytes basura en el serie en t≈157 s, sin reinicio: fallo puntual de la transmisión USB, no mezcla de mensajes |
| 06 | | | ✅ | Marcada como hecha |
| 07 | | | ✅ | Marcada como hecha |
| 08 | 2026-11-23 | Miguel | 🟡 parte Node-RED ✅ | **Solo la parte sin placas** (simulador → Node-RED → web): "conectado", **~97 msg/s** (96-98) con el simulador en Python nativo, punto de mira girando y avión balanceándose. (Con el simulador dentro de Docker salían ~81 msg/s: el contenedor envía más despacio, no era pérdida en Node-RED.) **Falta la prueba completa con placas** en el laboratorio (msg/s por mando y pérdidas) |

### Problemas encontrados y soluciones
- **08 · Node-RED sin el puerto UDP** (Miguel): el Node-RED ya existente (con `docker-compose.yml`) solo publicaba el 1880, así que no le llegaban los datos de los mandos.
  Solución: añadir `- 5005:5005/udp` en `ports:` del servicio `nodered` y `docker compose up -d nodered`.
  Los flujos se conservan porque están en la carpeta montada en `/data`.
- **08 · Python no instalado** (Miguel): en Windows, `python` abría la Microsoft Store (es solo un acceso directo).
  Para la prueba sin placas se ejecutó el simulador en un contenedor:
  `docker run --rm -v <carpeta 08_nodered_web>:/w python:3.12-alpine python /w/simular_mandos_udp.py host.docker.internal 5005`.
  Para el laboratorio hace falta **instalar Python** de verdad: `puente_serie_udp.py` lee el puerto COM y un contenedor no lo ve.
  **Solución definitiva:** instalar Python desde python.org (*Windows installer (64-bit)*) marcando **"Add python.exe to PATH"**,
  abrir una terminal nueva y `pip install pyserial paho-mqtt`. Con Python nativo el simulador da ~97 msg/s (en Docker, ~81).
- **Rutas demasiado largas en Windows**: si el repositorio se clona en una carpeta muy profunda, `git clone` falla
  (`Filename too long`) y Windows PowerShell no encuentra los archivos. Solución: clonar en una ruta corta (p. ej. `C:\iot\`)
  o usar `git config --global core.longpaths true`.

## Etapa 2 terminada cuando
- [ ] Las pruebas de prioridad alta (01, 02, 03, 08) superadas y anotadas en la tabla
- [ ] 06 y 07 comprobadas en una C3 (rápido)
- [ ] Problemas encontrados y sus soluciones escritos aquí (servirán para la memoria)
