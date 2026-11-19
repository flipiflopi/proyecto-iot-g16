# Lista de compra (AliExpress) — Etapa 1

> Pedid **cuanto antes**: suele tardar 1-3 semanas. Comprad algún **repuesto** de lo barato
> (si algo llega mal no hay tiempo de pedir otra vez). Comprobad en cada anuncio el modelo
> exacto, la valoración y los comentarios.

## 0. Antes de comprar: preguntad al profesor
- ¿La asignatura os **presta 3 ESP32-C3** del curso (con LED RGB integrado)? Si sí, no compréis placas.

## 1. Placas
| Cant. | Componente | Buscar en AliExpress | Para |
|---|---|---|---|
| 3 (+1) | ESP32-C3 con LED RGB integrado | `ESP32-C3-DevKitM-1` | Solo si la uni no las presta. El enunciado usa el LED RGB_BUILTIN |

> Evitad las **ESP32-C3 SuperMini** para la base: muchas tienen la antena mal y no conectan bien a la WiFi.

## 2. Sensores
| Cant. | Componente | Buscar | Para |
|---|---|---|---|
| 1 (+1 recomendado) | IMU 9 ejes con fusión interna **BNO085** | `BNO085` / `BNO08X module` | Cazador: apuntar con poca deriva |
| 2 | IMU 6 ejes **MPU6050** | `GY-521 MPU6050` | Pájaro (inclinación) + repuesto / pruebas |

> El BNO085 es la pieza clave y la más cara: si el presupuesto lo permite, pedid 2.
> Evitad los BNO055 muy baratos (muchos clones fallan).

## 3. Actuadores e interfaz
| Cant. | Componente | Buscar | Para |
|---|---|---|---|
| 3 | Módulo motor de vibración (ya lleva transistor) | `vibration motor module 3.3V` | GPIO5: retroceso del disparo / impacto |
| 2 | Pantalla OLED 0,96" I2C SSD1306 | `SSD1306 0.96 I2C` | Marcador en la base (+1 repuesto) |
| 2 | Zumbador **pasivo** | `passive buzzer module` | Sonidos en la base |
| 1 pack | Pulsadores táctiles 12 mm | `tactile switch 12x12` | Gatillo, turbo |
| 4 | Microinterruptor con palanca (opcional) | `micro switch lever` | Gatillo más "de pistola" |

## 4. Alimentación (mandos inalámbricos)
Opción simple para el prototipo: **2 powerbanks pequeños** USB (los que ya tengáis).
Opción integrada:
| Cant. | Componente | Buscar | Para |
|---|---|---|---|
| 2 (+1) | Batería LiPo 1S 3,7 V 500-1000 mAh con conector JST | `lipo 3.7V 800mAh` | Mandos |
| 3 | Cargador **TP4056 USB-C con protección** (6 pines, con DW01) | `TP4056 type-c protection` | Cargar la LiPo |
| 1 pack | Interruptor deslizante | `slide switch SS12D00` | Encender/apagar el mando |

> ⚠️ LiPo: no cortocircuitar, no perforar, no descargar del todo; usad siempre el TP4056 **con protección**.

## 5. Montaje
| Cant. | Componente | Buscar |
|---|---|---|
| 3 | Protoboard 400 puntos | `breadboard 400` |
| 1 pack | Cables Dupont M-M, M-H, H-H | `dupont jumper wire` |
| 3 | Cables USB-C **de datos** | `usb c data cable` |
| 1 pack | Placas perforadas (montaje final) | `perfboard` |
| 1 pack | Tiras de pines macho/hembra | `pin header 2.54` |
| 1 | Velcro / correa para la muñeca o la mano | `velcro strap` |

## Resumen por nodo
| Nodo | Lleva |
|---|---|
| **Base** | ESP32-C3, OLED, zumbador, USB al PC |
| **Cazador** | ESP32-C3, BNO085, motor de vibración, gatillo, batería |
| **Pájaro** | ESP32-C3, MPU6050, motor de vibración, botón turbo, batería |
