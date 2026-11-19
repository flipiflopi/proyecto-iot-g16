"""
Eco para la prueba de latencia (MODO 1 de la BASE).
Devuelve a la BASE cada línea "P,seq,t_us" que recibe por el puerto serie,
simulando el paso por el PC (donde estará Node-RED / el juego).

Requisito:  pip install pyserial
Uso:        python eco_pc.py COM5        (COM5 = puerto de la BASE)
"""
import sys
import time

import serial

if len(sys.argv) < 2:
    print("Uso: python eco_pc.py COMx   (puerto de la placa BASE)")
    sys.exit(1)

puerto = sys.argv[1]
with serial.Serial(puerto, 921600, timeout=0.1) as s:
    print(f"Eco activo en {puerto}. Lanza el ensayo desde el MANDO. Ctrl+C para salir.")
    devueltos = 0
    ultimo = time.time()
    while True:
        linea = s.readline()
        if linea.startswith(b"P,"):
            s.write(linea)          # Devolver tal cual
            devueltos += 1
        elif linea.strip():
            print("BASE dice:", linea.decode(errors="replace").strip())
        if time.time() - ultimo >= 1:  # Cada segundo, cuántos ha devuelto
            if devueltos:
                print(f"  devueltos en el último segundo: {devueltos}")
            devueltos = 0
            ultimo = time.time()
