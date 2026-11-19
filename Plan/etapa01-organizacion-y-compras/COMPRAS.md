# Lista de compra mínima (AliExpress) — Etapa 1

Objetivo: **validar el prototipo** con lo mínimo. Sin repuestos ni extras.
Todo se prueba en el laboratorio, con las **3 ESP32-C3 del laboratorio** (no se compran placas).

| Cant. | Componente | Buscar en AliExpress | Para qué |
|---|---|---|---|
| 1 | IMU 9 ejes **BNO055** (GY-BNO055), variante **"1PCS"** — [anuncio](https://es.aliexpress.com/item/1005005499172228.html) (9,69 €, 4.4★, 1.000+ vendidos, llega 26 nov - 2 dic) | `GY-BNO055` | **Cazador**: hacia dónde apunta (necesita el giro izquierda/derecha, por eso 9 ejes) |
| 1 | IMU 6 ejes **MPU6050** (GY-521), variante **"1PCS"** — [anuncio](https://es.aliexpress.com/item/1005010057794277.html) (1,75 €, 4.7★, 4.000+ vendidos, llega 26 nov - 2 dic) | `MPU6050 GY-521` | **Pájaro**: inclinación de la mano (con 6 ejes basta) |
| 2 | Módulo motor de vibración (ya lleva transistor) | `vibration motor module 3.3V` | GPIO5: retroceso al disparar (cazador) y aviso de impacto (pájaro) |
| 1 pack (10 uds) | Microinterruptor **KW11-3Z**, variante **"3P with handle"** (palanca recta, 3 pines) — [anuncio](https://es.aliexpress.com/item/1005006863861405.html) (1,79 €) | `micro switch KW11-3Z` | **Los dos mandos**: gatillo del cazador y turbo del pájaro. Se usan 2 y sobran 8 |

## Lo que no se compra
- **Placas:** se usan las del laboratorio.
- **Alimentación de los mandos:** 2 powerbanks de móvil que ya tengáis + cable USB a la placa.
- **Base:** solo la placa, conectada por USB al PC.

> **Por qué no el botón BOOT de la placa:** es diminuto y está en GPIO9, un pin de arranque: si se
> mantiene pulsado al encender, la placa entra en modo programación. Vale para pruebas, no para jugar.
>
> **Conexión del microinterruptor:** COM → GND, NO → GPIO libre (`INPUT_PULLUP`), NC sin conectar.

> Los módulos suelen venir con los pines **sin soldar**: hace falta soldarlos (en el laboratorio
> suele haber soldador).

## Por qué estas IMU (y no el BNO085)
Al principio se pensó en 2 × BNO085 (~23 € cada uno). Se cambió para abaratar (~11,4 € las dos):
- **Pájaro** solo necesita la **inclinación** (alabeo y cabeceo). Eso lo da bien cualquier IMU de
  **6 ejes** (acelerómetro + giroscopio): el acelerómetro sabe dónde está "abajo" y corrige la deriva.
  El MPU6050 además calcula la orientación dentro del chip (**DMP**).
- **Cazador** necesita también el **giro izquierda/derecha (yaw)**. Con 6 ejes el yaw deriva poco a
  poco porque nada lo corrige; el **BNO055** (9 ejes, con magnetómetro) da la orientación completa
  calculada en el chip. Igualmente se mantiene el **botón de recentrar**.

> **Ojo:** el MPU6050 está descatalogado y abundan las copias. Al llegar, leer `WHO_AM_I`:
> `0x68` = MPU6050 auténtico; `0x70` = MPU6500 (también vale, pero algunas librerías lo rechazan).
> Ambos módulos aceptan **3,3 V** (llevan regulador) y van por **I2C**.

## Resumen por nodo
| Nodo | Lleva |
|---|---|
| **Base** | ESP32-C3 (laboratorio) conectada por USB al PC |
| **Cazador** | ESP32-C3 + **BNO055** (9 ejes) + motor de vibración + gatillo + powerbank |
| **Pájaro** | ESP32-C3 + **MPU6050** (6 ejes) + motor de vibración + botón turbo + powerbank |
