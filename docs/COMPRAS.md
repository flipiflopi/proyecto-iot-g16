# Lista de compra mínima (AliExpress) — Etapa 1

Objetivo: **validar el prototipo** con lo mínimo. Sin repuestos ni extras.
Todo se prueba en el laboratorio, con las **3 ESP32-C3 del laboratorio** (no se compran placas).

| Cant. | Componente | Buscar en AliExpress | Para qué |
|---|---|---|---|
| 1 | IMU 9 ejes **BNO085** | `BNO085` / `BNO08X module` | Cazador: saber hacia dónde apunta, con poca deriva |
| 1 | IMU 6 ejes **MPU6050** | `GY-521 MPU6050` | Pájaro: medir la inclinación de la mano |
| 2 | Módulo motor de vibración (ya lleva transistor) | `vibration motor module 3.3V` | GPIO5: retroceso al disparar (cazador) y aviso de impacto (pájaro) |

## Solo si el laboratorio no lo tiene
| Cant. | Componente | Buscar | Para qué |
|---|---|---|---|
| 2 | Pulsador | `tactile switch 12x12` | Gatillo (cazador) y turbo (pájaro). No hace falta si el profesor deja usar el botón BOOT de la placa |
| 1 pack | Cables Dupont hembra-hembra | `dupont jumper wire female female` | Conectar los módulos a la placa |

## Lo que no se compra
- **Placas:** se usan las del laboratorio.
- **Alimentación de los mandos:** 2 powerbanks del móvil que ya tengáis + cable USB.
- **Base:** solo la placa, conectada por USB al PC.

> Los módulos suelen venir con los pines **sin soldar**: hace falta soldarlos (en el laboratorio
> suele haber soldador).

## Resumen por nodo
| Nodo | Lleva |
|---|---|
| **Base** | ESP32-C3 (laboratorio) conectada por USB al PC |
| **Cazador** | ESP32-C3 + BNO085 + motor de vibración + gatillo + powerbank |
| **Pájaro** | ESP32-C3 + MPU6050 + motor de vibración + botón turbo + powerbank |
