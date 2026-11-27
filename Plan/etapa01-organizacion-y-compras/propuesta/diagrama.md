# Diagrama de bloques: Cazador vs. Pájaro

Es el mismo diagrama que lleva la propuesta en PDF (`Propuesta_G16_Cazador_vs_Pajaro.pdf`).
Si lo cambiáis aquí, copiad el bloque a `diagrama.mmd` y regenerad la imagen y el PDF:
```powershell
npx -p @mermaid-js/mermaid-cli mmdc -i diagrama.mmd -o diagrama.png -s 3 -b white -c mermaid-config.json
python generar_propuesta.py
```

```mermaid
flowchart LR
    CAZ["<b>CAZADOR</b> (mando)<br/>ESP32-C3 + powerbank<br/>IMU MPU6050 · gatillo<br/>vibración · LED RGB"]
    PAJ["<b>PÁJARO</b> (mando)<br/>ESP32-C3 + powerbank<br/>IMU MPU6050 · turbo<br/>vibración · LED RGB"]
    OTA["Servidor <b>FOTA</b><br/>(universidad)"]

    BASE["<b>BASE (ESP32-C3)</b><br/>lógica del juego<br/>(FreeRTOS)<br/>servidor web +<br/>WebSocket<br/>pasarela ESP-NOW<br/>↔ WiFi/MQTT<br/>LED RGB: estado"]

    NAV["<b>NAVEGADOR</b><br/>(PC / móvil)<br/>dibuja el juego"]
    BROKER["<b>BROKER MQTT</b><br/>iot.ac.uma.es<br/>topics II16/..."]
    DB[("<b>MongoDB</b> (PC)<br/>log, partidas,<br/>escenarios")]
    TG["<b>Telegram</b><br/>bot + canal"]

    NR["<b>Node-RED</b><br/>(PC, Docker)<br/>dashboard:<br/>jugadores, papeles,<br/>duración, historial"]

    CAZ <-- "ESP-NOW<br/>→ orientación,<br/>gatillo<br/>← vibración, LED" --> BASE
    PAJ <-- "ESP-NOW<br/>→ orientación,<br/>turbo<br/>← vibración, LED" --> BASE
    OTA -- "firmware" --> BASE

    BASE -- "WiFi: HTTP<br/>+ WebSocket" --> NAV
    BASE <-- "MQTT<br/>→ estado,<br/>resultados<br/>← config,<br/>órdenes" --> BROKER
    BROKER <-- "MQTT" --> NR
    DB <--> NR
    TG <-- "/estado,<br/>/ranking,<br/>/actualizar" --> NR

    classDef base fill:#eef3f8,stroke:#1f4e79,stroke-width:2px,color:#1f4e79
    class BASE,NR base
```

- Todo va en la misma WiFi (hotspot del móvil o router, 2,4 GHz). Más detalle en [../../ARQUITECTURA.md](../../ARQUITECTURA.md).
