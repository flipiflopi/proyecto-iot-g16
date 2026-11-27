"""Genera la propuesta del proyecto (una carilla, PDF) para la tarea de la asignatura.

Uso:  python generar_propuesta.py      ->  Propuesta_G16_Cazador_vs_Pajaro.pdf (en esta carpeta)
Requiere reportlab (pip install reportlab). Usa Arial de Windows.
"""
import os
from reportlab.lib.pagesizes import A4
from reportlab.lib.units import cm
from reportlab.lib import colors
from reportlab.lib.styles import ParagraphStyle
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle
from reportlab.graphics.shapes import Drawing, Rect, String, Line, Polygon
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont

AQUI = os.path.dirname(os.path.abspath(__file__))
SALIDA = os.path.join(AQUI, "Propuesta_G16_Cazador_vs_Pajaro.pdf")

pdfmetrics.registerFont(TTFont("Arial", r"C:\Windows\Fonts\arial.ttf"))
pdfmetrics.registerFont(TTFont("Arial-B", r"C:\Windows\Fonts\arialbd.ttf"))
pdfmetrics.registerFont(TTFont("Arial-I", r"C:\Windows\Fonts\ariali.ttf"))
from reportlab.lib.fonts import addMapping
addMapping("Arial", 0, 0, "Arial"); addMapping("Arial", 1, 0, "Arial-B"); addMapping("Arial", 0, 1, "Arial-I")

AZUL = colors.HexColor("#1f4e79")
GRIS = colors.HexColor("#555555")
FONDO = colors.HexColor("#eef3f8")

titulo = ParagraphStyle("t", fontName="Arial-B", fontSize=15, leading=18, textColor=AZUL)
sub = ParagraphStyle("s", fontName="Arial", fontSize=8.5, leading=11, textColor=GRIS)
h = ParagraphStyle("h", fontName="Arial-B", fontSize=11, leading=14, textColor=AZUL, spaceBefore=8, spaceAfter=3)
txt = ParagraphStyle("p", fontName="Arial", fontSize=9, leading=11.8)
bul = ParagraphStyle("b", parent=txt, leftIndent=9, bulletIndent=0)
celda = ParagraphStyle("c", fontName="Arial", fontSize=8.2, leading=10)
celdaB = ParagraphStyle("cb", parent=celda, fontName="Arial-B")


# ---------------- Diagrama de bloques ----------------
def caja(d, x, y, w, hh, titulo_caja, lineas, color):
    d.add(Rect(x, y, w, hh, rx=5, ry=5, fillColor=color, strokeColor=AZUL, strokeWidth=0.8))
    yy = y + hh - 12
    d.add(String(x + w / 2, yy, titulo_caja, fontName="Arial-B", fontSize=8.5, textAnchor="middle", fillColor=AZUL))
    for l in lineas:
        yy -= 9.5
        d.add(String(x + w / 2, yy, l, fontName="Arial", fontSize=7, textAnchor="middle"))


def punta(d, x1, y1, x2, y2):
    import math
    a = math.atan2(y2 - y1, x2 - x1)
    L, A = 6, 0.45
    d.add(Polygon([x2, y2, x2 - L * math.cos(a - A), y2 - L * math.sin(a - A),
                   x2 - L * math.cos(a + A), y2 - L * math.sin(a + A)],
                  fillColor=AZUL, strokeColor=AZUL, strokeWidth=0.5))


def flecha(d, x1, y1, x2, y2, etiqueta="", doble=False, dx=0, dy=4):
    d.add(Line(x1, y1, x2, y2, strokeColor=AZUL, strokeWidth=1))
    punta(d, x1, y1, x2, y2)
    if doble:
        punta(d, x2, y2, x1, y1)
    if etiqueta:
        for i, t in enumerate(etiqueta.split("\n")):
            d.add(String((x1 + x2) / 2 + dx, (y1 + y2) / 2 + dy - i * 8.5, t, fontName="Arial", fontSize=6.8,
                         textAnchor="middle", fillColor=GRIS))


def diagrama():
    W, H = 510, 168
    d = Drawing(W, H)
    blanco = colors.white
    caja(d, 0, 108, 112, 56, "CAZADOR (mando)", ["ESP32-C3 + powerbank", "IMU BNO055 · gatillo", "vibración · LED RGB"], blanco)
    caja(d, 0, 6, 112, 56, "PÁJARO (mando)", ["ESP32-C3 + powerbank", "IMU MPU6050 · turbo", "vibración · LED RGB"], blanco)
    caja(d, 178, 30, 132, 108, "BASE (ESP32-C3)", ["lógica del juego (FreeRTOS)", "servidor web + WebSocket",
                                                     "pasarela ESP-NOW ↔ WiFi/MQTT", "LED RGB: estado partida",
                                                     "FOTA de los 3 nodos"], FONDO)
    caja(d, 392, 122, 118, 42, "NAVEGADOR (PC/móvil)", ["dibuja el juego (canvas)"], blanco)
    caja(d, 392, 72, 118, 38, "BROKER MQTT", ["iot.ac.uma.es · topics II16/..."], blanco)
    caja(d, 392, 0, 118, 58, "PC · Docker", ["Node-RED: dashboard", "MongoDB: log, partidas,", "ranking · bot Telegram"], blanco)
    # mandos <-> base
    flecha(d, 112, 136, 178, 112, "ESP-NOW", doble=True, dx=-4, dy=8)
    flecha(d, 112, 34, 178, 58, "ESP-NOW", doble=True, dx=-4, dy=-14)
    d.add(String(145, 84, "orientación 50 Hz,", fontName="Arial", fontSize=6.6, textAnchor="middle", fillColor=GRIS))
    d.add(String(145, 76, "botones →", fontName="Arial", fontSize=6.6, textAnchor="middle", fillColor=GRIS))
    d.add(String(145, 66, "← vibración, LED,", fontName="Arial", fontSize=6.6, textAnchor="middle", fillColor=GRIS))
    d.add(String(145, 58, "órdenes (ACK)", fontName="Arial", fontSize=6.6, textAnchor="middle", fillColor=GRIS))
    # base -> navegador / broker
    flecha(d, 310, 126, 392, 143, "WiFi: HTTP +\nWebSocket 50 Hz", dx=-14, dy=20)
    flecha(d, 310, 91, 392, 91, "MQTT", doble=True)
    d.add(String(351, 80, "→ config, órdenes", fontName="Arial", fontSize=6.6, textAnchor="middle", fillColor=GRIS))
    d.add(String(351, 71, "← estado, resultados", fontName="Arial", fontSize=6.6, textAnchor="middle", fillColor=GRIS))
    flecha(d, 451, 58, 451, 72, "", doble=True)
    d.add(String(456, 62, "MQTT", fontName="Arial", fontSize=6.5, fillColor=GRIS))
    return d


# ---------------- Documento ----------------
doc = SimpleDocTemplate(SALIDA, pagesize=A4, leftMargin=1.5 * cm, rightMargin=1.5 * cm,
                        topMargin=1.2 * cm, bottomMargin=1.0 * cm,
                        title="Propuesta de proyecto - Cazador vs. Pájaro (Grupo 16)", author="Grupo 16 (II16)")
s = []
s.append(Paragraph("Cazador vs. Pájaro: juego IoT con mandos de movimiento", titulo))
s.append(Paragraph("Propuesta de proyecto · Informática Industrial · Grupo 16 (II16): Iván, David y Miguel · noviembre de 2026", sub))
s.append(Spacer(1, 4))

s.append(Paragraph("1. Objetivo", h))
s.append(Paragraph(
    "Juego para dos jugadores con mandos inalámbricos que se manejan moviendo la mano. El <b>cazador</b> apunta con una "
    "«pistola» (IMU) y dispara con un gatillo; el <b>pájaro</b> pilota un avión inclinando su mando y tiene un turbo. "
    "La partida (60 s, 3 vidas, cargador limitado, recarga apuntando al suelo) se ve en el navegador, y los mandos "
    "vibran al disparar y al recibir un impacto. El sistema guarda las partidas, muestra un ranking, avisa por Telegram "
    "y permite actualizar el firmware por WiFi (FOTA).", txt))

s.append(Paragraph("2. Dispositivos", h))
filas = [
    ["Nodo", "Equipo", "Sensores / entradas", "Actuadores", "Rol"],
    ["Base", "ESP32-C3", "botón BOOT", "LED RGB (estado de la partida)",
     "Recibe los mandos, ejecuta la lógica del juego, sirve la web y hace de pasarela con MQTT"],
    ["Cazador", "ESP32-C3 + powerbank", "IMU 9 ejes BNO055, gatillo (microinterruptor)", "motor de vibración (retroceso), LED RGB",
     "Apuntar y disparar"],
    ["Pájaro", "ESP32-C3 + powerbank", "IMU 6 ejes MPU6050, botón turbo", "motor de vibración (impacto), LED RGB (turbo)",
     "Pilotar el objetivo"],
    ["PC", "Docker", "—", "—", "Node-RED (dashboard, Telegram) y MongoDB; navegador con el juego"],
]
t = Table([[Paragraph(c, celdaB if i == 0 or j == 0 else celda) for j, c in enumerate(f)] for i, f in enumerate(filas)],
          colWidths=[1.75 * cm, 2.6 * cm, 4.3 * cm, 4.1 * cm, 5.25 * cm])
t.setStyle(TableStyle([
    ("BACKGROUND", (0, 0), (-1, 0), FONDO), ("GRID", (0, 0), (-1, -1), 0.4, colors.HexColor("#b0bccc")),
    ("VALIGN", (0, 0), (-1, -1), "MIDDLE"), ("TOPPADDING", (0, 0), (-1, -1), 2), ("BOTTOMPADDING", (0, 0), (-1, -1), 2),
]))
s.append(t)

s.append(Paragraph("3. Comunicaciones", h))
s.append(diagrama())
s.append(Spacer(1, 3))
for b in [
    "<b>Mandos ↔ base: ESP-NOW.</b> Mandos → base: orientación a 50 Hz y botones (latencia medida ≈ 4 ms, pérdidas ≤ 0,4 %). "
    "Base → mandos: vibración, LED, configuración y orden de FOTA, en unicast con confirmación (ACK). "
    "Los mandos buscan solos el canal de la base.",
    "<b>Base → navegador: WiFi</b> (hotspot o router, 2,4 GHz). La base sirve la página por HTTP y le envía posiciones "
    "y estado por WebSocket a 50 Hz.",
    "<b>Base ↔ Node-RED: MQTT</b> (broker de la universidad, topics <font face='Arial-I'>II16/…</font>). "
    "Hacia la base: configuración y órdenes (iniciar, parar, LED, vibración). Hacia Node-RED: conexión (LWT/latido), estado, "
    "eventos y resultados.",
    "<b>Node-RED ↔ Telegram</b> (bot y canal) y <b>FOTA</b> desde el servidor OTA de la universidad.",
]:
    s.append(Paragraph(b, bul, bulletText="•"))

s.append(Paragraph("4. Datos y lógica", h))
for b in [
    "<b>Lógica principal en la base</b> (FreeRTOS: tareas y colas): estado de la partida, tiempo, detección de impactos, "
    "puntos y órdenes de vibración. Decide al instante y la partida sigue aunque falle el PC. El navegador solo dibuja.",
    "<b>Mandos</b>: un firmware común con el rol configurable. Leen la IMU y los botones, envían y ejecutan las órdenes; "
    "la configuración se guarda en flash.",
    "<b>Node-RED + MongoDB</b>: dashboard para elegir jugadores, papeles y duración e iniciar la partida. MongoDB guarda "
    "con fecha el <b>log de eventos</b> (conexiones, disparos, impactos, órdenes, actualizaciones), las <b>partidas</b> "
    "(jugadores, duración, puntuación) y los <b>escenarios</b> con sus parámetros, y sirve para el historial y el ranking.",
    "<b>Eventos y actualizaciones</b>: los eventos discretos (disparo, impacto, fin) van con confirmación (ESP-NOW ACK / MQTT); "
    "la desconexión de un mando se detecta por latido. Telegram: <font face='Arial-I'>/estado</font>, "
    "<font face='Arial-I'>/ranking</font>, <font face='Arial-I'>/actualizar</font> y un canal con resultados y avisos. "
    "La FOTA se lanza por orden y nunca durante una partida.",
]:
    s.append(Paragraph(b, bul, bulletText="•"))

doc.build(s)
print("PDF generado:", SALIDA)
