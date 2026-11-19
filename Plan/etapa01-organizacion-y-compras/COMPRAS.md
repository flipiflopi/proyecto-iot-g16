# Lista de compra mínima (AliExpress) — Etapa 1

Objetivo: **validar el prototipo** con lo mínimo. Sin repuestos ni extras.
Todo se prueba en el laboratorio, con las **3 ESP32-C3 del laboratorio** (no se compran placas).

| Cant. | Componente | Buscar en AliExpress | Para qué |
|---|---|---|---|
| 2 | IMU 9 ejes **BNO085** (los anuncios "BNO08X" valen si el chip es BNO085 o BNO086) | `BNO085` / `BNO08X` | Cazador: hacia dónde apunta. Pájaro: inclinación de la mano |
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

## Resumen por nodo
| Nodo | Lleva |
|---|---|
| **Base** | ESP32-C3 (laboratorio) conectada por USB al PC |
| **Cazador** | ESP32-C3 + BNO085 + motor de vibración + gatillo + powerbank |
| **Pájaro** | ESP32-C3 + BNO085 + motor de vibración + botón turbo + powerbank |
