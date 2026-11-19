"""
Mide la latencia ida y vuelta por MQTT (PC -> broker -> ESP32 -> broker -> PC)
usando los topics ping/pong del sketch NodoMQTT.

Requisito:  pip install paho-mqtt
Uso:        python medir_latencia_mqtt.py esp32c3-XXXXXX
            (el CHIPID lo imprime el ESP32 al arrancar)
La clave del grupo se pide por teclado (no se guarda en ningún sitio).
"""
import getpass
import statistics
import sys
import threading
import time

import paho.mqtt.client as mqtt

SERVIDOR = "iot.ac.uma.es"
PUERTO = 1883
USUARIO = "II16"
N_PINGS = 50

if len(sys.argv) < 2:
    print("Uso: python medir_latencia_mqtt.py esp32c3-XXXXXX")
    sys.exit(1)

chip = sys.argv[1]
t_ping = f"{USUARIO}/{chip}/ping"
t_pong = f"{USUARIO}/{chip}/pong"
clave = getpass.getpass(f"Clave MQTT del usuario {USUARIO}: ")

respuesta = threading.Event()
recibido = {}


def al_conectar(cliente, userdata, flags, codigo, properties=None):
    print("Conectado al broker" if codigo == 0 else f"Error de conexión: {codigo}")
    cliente.subscribe(t_pong)


def al_mensaje(cliente, userdata, msg):
    recibido["t"] = time.perf_counter()
    recibido["payload"] = msg.payload.decode(errors="replace")
    respuesta.set()


cliente = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
cliente.username_pw_set(USUARIO, clave)
cliente.on_connect = al_conectar
cliente.on_message = al_mensaje
cliente.connect(SERVIDOR, PUERTO, keepalive=30)
cliente.loop_start()
time.sleep(1.5)

tiempos = []
perdidos = 0
for i in range(N_PINGS):
    respuesta.clear()
    t0 = time.perf_counter()
    cliente.publish(t_ping, f"{i}")
    if respuesta.wait(timeout=2.0) and recibido["payload"] == f"{i}":
        tiempos.append((recibido["t"] - t0) * 1000)
    else:
        perdidos += 1
    time.sleep(0.1)

cliente.loop_stop()
cliente.disconnect()

if tiempos:
    tiempos.sort()
    p95 = tiempos[int(len(tiempos) * 0.95) - 1]
    print(f"\nRespuestas: {len(tiempos)}/{N_PINGS} (perdidas {perdidos})")
    print(f"RTT MQTT [ms]: min {tiempos[0]:.1f} | media {statistics.mean(tiempos):.1f} | "
          f"p95 {p95:.1f} | max {tiempos[-1]:.1f}")
else:
    print("No ha llegado ninguna respuesta. ¿CHIPID correcto? ¿El ESP32 está conectado?")
