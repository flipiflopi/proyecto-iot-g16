# Proyecto IoT — Grupo 16 (Informática Industrial)

Juego IoT "**Cazador vs. Pájaro**" con 3 ESP32-C3:

| Nodo | Función |
|---|---|
| **Base / pasarela** | Recibe los mandos por ESP-NOW, los reenvía al PC (USB) y al broker MQTT; LED RGB con el estado de la partida |
| **Cazador** | Apunta con la mano (IMU BNO085) y dispara; vibración de retroceso |
| **Pájaro** | Pilota el objetivo inclinando la mano (IMU BNO085); vibración al recibir un impacto |

```
 [Cazador] --ESP-NOW-->                    --USB serie--> PC: Node-RED --> juego (web)
                         [BASE / pasarela]
 [Pájaro]  --ESP-NOW-->                    --WiFi/MQTT--> broker iot.ac.uma.es
            <--ESP-NOW-- (órdenes, FOTA)                       |
                                                Node-RED: MongoDB, dashboard, Telegram
```

- **Plan por etapas:** [docs/PLAN.md](docs/PLAN.md)
- **Lista de compra:** [docs/COMPRAS.md](docs/COMPRAS.md)
- **Prueba de latencia (superada, ~4 ms):** [firmware/pruebas/latencia_espnow](firmware/pruebas/latencia_espnow)

## Estructura
| Carpeta | Contenido |
|---|---|
| `firmware/` | Programas de los ESP32 (Arduino IDE, core ESP32 3.x). `pruebas/` = prototipos de cada tecnología |
| `nodered/` | Flujos de Node-RED exportados (`.json`) |
| `juego/` | Página web del juego |
| `hardware/` | Soportes de los mandos (diseños 3D `.stl`, fotos del montaje) |
| `docs/` | Plan, compras, memoria, manual de usuario |

## Secretos (¡importante!)
Las contraseñas (WiFi, MQTT/MongoDB del grupo II16, token del bot de Telegram) **nunca** se suben.
- En el firmware: copiad `firmware/secrets.example.h` como `secrets.h` (está en `.gitignore`).
- En Node-RED: usad credenciales de nodo / variables de entorno, no las escribáis en los flujos.

## Cómo trabajamos con Git
1. `git pull` antes de empezar.
2. Una rama por tarea: `git switch -c etapa2-mqtt`.
3. Commits pequeños con mensajes claros.
4. `git push` y abrid un **Pull Request** a `main`; otro integrante lo revisa y lo fusiona.
5. Nunca trabajéis directamente sobre `main`.
