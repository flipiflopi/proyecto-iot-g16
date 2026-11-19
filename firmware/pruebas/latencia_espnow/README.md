# Prueba de latencia ESP-NOW ✅ (superada)

Mide si ESP-NOW es lo bastante rápido para que los mandos controlen el juego en tiempo real.

- `Mando/Mando.ino`: envía 1000 paquetes a 100 Hz y mide cuánto tardan en volver.
- `Base/Base.ino`: los devuelve. `MODO 0` = solo radio; `MODO 1` = pasando por el PC
  (`eco_pc.py`); `CONECTAR_WIFI 1` = la base además conectada a una WiFi.
- `eco_pc.py`: devuelve los paquetes desde el PC (`pip install pyserial`, `python eco_pc.py COMx`).

## Resultados (ESP32-C3, 1 m, canal 1)

| Prueba | Pérdidas | Ida media | Ida p99 |
|---|---|---|---|
| A: solo ESP-NOW | 0 % | 1,8 ms | 5 ms |
| B: ESP-NOW + USB + PC | 2,0 % | 3,8 ms | 8,8 ms |
| C: B + base conectada a WiFi (hotspot móvil) | 1,3-2,2 % | 3,7-4,1 ms | 6-16 ms |

**Conclusión:** el sistema añade ~4 ms de media; con pantalla y navegador, ~15-40 ms en total.
Imperceptible al jugar. Las pérdidas (~2 %) no importan para la posición (llega otra a los
10 ms), pero los eventos importantes (disparo, órdenes) se enviarán en **unicast con ACK**.

## Problemas encontrados
- La red `infind` no conectó (errores `wifi:Set status to INIT`). Con el hotspot del móvil
  (2,4 GHz, WPA2) sí. Se añadió diagnóstico de redes y potencia reducida (`WIFI_POWER_8_5dBm`).
- eduroam no sirve para el ESP32.
