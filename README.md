# Proyecto IoT — Grupo 16 (Informática Industrial)

Juego IoT "**Cazador vs. Pájaro**" con 3 ESP32-C3:

| Nodo | Función |
|---|---|
| **Base / pasarela** | Recibe los mandos por ESP-NOW, los reenvía al PC (USB) y al broker MQTT; LED RGB con el estado de la partida |
| **Cazador** | Apunta con la mano (IMU 9 ejes BNO055) y dispara; vibración de retroceso |
| **Pájaro** | Pilota el objetivo inclinando la mano (IMU 6 ejes MPU6050); vibración al recibir un impacto |

```
 [Cazador] --ESP-NOW-->                    --USB serie--> PC: Node-RED --> juego (web)
                         [BASE / pasarela]
 [Pájaro]  --ESP-NOW-->                    --WiFi/MQTT--> broker iot.ac.uma.es
            <--ESP-NOW-- (órdenes, FOTA)                       |
                                                Node-RED: MongoDB (log y partidas), Telegram
```

- **Plan por etapas:** [Plan/Plan.md](Plan/Plan.md) (cada etapa en `Plan/etapaNN-descripcion/` con su código y su GUIA.md)
- **Lista de compra:** [Plan/etapa01-organizacion-y-compras/COMPRAS.md](Plan/etapa01-organizacion-y-compras/COMPRAS.md)
- **Etapa 2, validación de tecnologías:** [Plan/etapa02-validacion-de-tecnologias/GUIA.md](Plan/etapa02-validacion-de-tecnologias/GUIA.md) (latencia ESP-NOW superada, ~4 ms)

## Estructura
| Carpeta | Contenido |
|---|---|
| `Plan/Plan.md` | Plan general por etapas |
| `Plan/etapaNN-descripcion/` | Código de esa etapa (sketches `.ino`, scripts, flujos Node-RED `.json`, diseños 3D) y su `GUIA.md` |

## Secretos (¡importante!)
Las contraseñas (WiFi, MQTT/MongoDB del grupo II16, token del bot de Telegram) **nunca** se suben.
- En el firmware: en cada sketch que lo necesite hay un `secrets.example.h`; copiadlo como `secrets.h` en la misma carpeta (está en `.gitignore`).
- En Node-RED: usad credenciales de nodo / variables de entorno, no las escribáis en los flujos.

## Cómo trabajamos con Git
1. `git pull` antes de empezar.
2. Una rama por tarea: `git switch -c etapa2-mqtt`.
3. Commits pequeños con mensajes claros.
4. `git push` y abrid un **Pull Request** a `main`; otro integrante lo revisa y lo fusiona.
5. Nunca trabajéis directamente sobre `main`.
