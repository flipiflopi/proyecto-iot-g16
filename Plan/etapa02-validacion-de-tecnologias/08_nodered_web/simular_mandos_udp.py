"""
Simulador de mandos SIN PLACAS: envía a Node-RED (UDP 5005) las mismas líneas
que enviaría la base, con 2 mandos moviéndose solos. Sirve para probar el flujo
de Node-RED y la página web en casa.

Uso:  python simular_mandos_udp.py [host] [puerto]
"""
import math
import socket
import sys
import time

host = sys.argv[1] if len(sys.argv) > 1 else "127.0.0.1"
puerto = int(sys.argv[2]) if len(sys.argv) > 2 else 5005
udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

print(f"Simulando 2 mandos -> udp://{host}:{puerto} a 50 Hz. Ctrl+C para salir.")
t = 0.0
while True:
    disparo = 1 if int(t * 2) % 6 == 0 else 0          # "dispara" de vez en cuando
    cazador = f"O,1,{25 * math.cos(t * 0.8):.2f},{15 * math.sin(t * 0.8):.2f},0.00,{disparo}"
    pajaro = f"O,2,0.00,{12 * math.sin(t * 0.5):.2f},{35 * math.sin(t * 0.9):.2f},0"
    udp.sendto(cazador.encode(), (host, puerto))
    udp.sendto(pajaro.encode(), (host, puerto))
    t += 0.02
    time.sleep(0.02)
