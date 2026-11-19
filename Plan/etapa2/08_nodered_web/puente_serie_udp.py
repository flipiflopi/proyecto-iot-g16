"""
Puente PUERTO SERIE -> UDP.
Lee las líneas "O,id,yaw,pitch,roll,botones" que escribe la placa BaseSerie y
las reenvía por UDP a Node-RED (nodo "udp in" en el puerto 5005).

¿Por qué no lee Node-RED el puerto serie directamente? Porque Node-RED corre en
Docker y, en Windows, un contenedor no ve los puertos COM. Este puente es la
forma más sencilla de pasar los datos.

Requisito:  pip install pyserial
Uso:        python puente_serie_udp.py COM5 [host] [puerto]
            (por defecto host 127.0.0.1 y puerto 5005)
"""
import socket
import sys
import time

import serial

if len(sys.argv) < 2:
    print("Uso: python puente_serie_udp.py COMx [host] [puerto]")
    sys.exit(1)

puerto_serie = sys.argv[1]
host = sys.argv[2] if len(sys.argv) > 2 else "127.0.0.1"
puerto_udp = int(sys.argv[3]) if len(sys.argv) > 3 else 5005

udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

with serial.Serial(puerto_serie, 921600, timeout=0.1) as s:
    print(f"Puente activo: {puerto_serie} -> udp://{host}:{puerto_udp}. Ctrl+C para salir.")
    enviados = 0
    ultimo = time.time()
    while True:
        linea = s.readline()
        if linea.startswith(b"O,"):
            udp.sendto(linea.strip(), (host, puerto_udp))
            enviados += 1
        elif linea.startswith(b"#"):
            print("BASE:", linea.decode(errors="replace").strip())
        if time.time() - ultimo >= 1:
            print(f"  reenviadas a Node-RED en el último segundo: {enviados}")
            enviados = 0
            ultimo = time.time()
