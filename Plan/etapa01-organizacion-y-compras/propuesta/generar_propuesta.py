"""Genera la propuesta del proyecto (una carilla, PDF) para la tarea de la asignatura.

Uso:  python generar_propuesta.py      ->  Propuesta_G16_Cazador_vs_Pajaro.pdf (en esta carpeta)
Requiere reportlab (pip install reportlab). Usa Arial de Windows y diagrama.png (el Mermaid de diagrama.md).
"""
import os
from reportlab.lib.pagesizes import A4
from reportlab.lib.units import cm
from reportlab.lib import colors
from reportlab.lib.styles import ParagraphStyle
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, Image
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

titulo = ParagraphStyle("t", fontName="Arial-B", fontSize=15, leading=18, textColor=colors.black)
sub = ParagraphStyle("s", fontName="Arial", fontSize=8.5, leading=11, textColor=GRIS)
h = ParagraphStyle("h", fontName="Arial-B", fontSize=11, leading=14, textColor=colors.black, spaceBefore=8, spaceAfter=3)
txt = ParagraphStyle("p", fontName="Arial", fontSize=9, leading=11.8)
bul = ParagraphStyle("b", parent=txt, leftIndent=9, bulletIndent=0)
celda = ParagraphStyle("c", fontName="Arial", fontSize=8.2, leading=10)
celdaB = ParagraphStyle("cb", parent=celda, fontName="Arial-B")


# ---------------- Diagrama de bloques ----------------
# Es la imagen del diagrama Mermaid de diagrama.md (renderizado con mermaid-cli):
#   npx -p @mermaid-js/mermaid-cli mmdc -i diagrama.mmd -o diagrama.png -s 3 -b white -c mermaid-config.json
DIAGRAMA = os.path.join(AQUI, "diagrama.png")


def diagrama(ancho):
    from reportlab.lib.utils import ImageReader
    w, hh = ImageReader(DIAGRAMA).getSize()
    return Image(DIAGRAMA, width=ancho, height=ancho * hh / w)


# ---------------- Documento ----------------
doc = SimpleDocTemplate(SALIDA, pagesize=A4, leftMargin=1.5 * cm, rightMargin=1.5 * cm,
                        topMargin=1.2 * cm, bottomMargin=1.0 * cm,
                        title="Propuesta de proyecto - Cazador vs. Pájaro (Grupo 16)", author="Grupo 16: Iván Pérez, David Sánchez y Miguel Piedrahita")
s = []
s.append(Paragraph("Cazador vs. Pájaro: juego IoT con mandos de movimiento", titulo))
s.append(Paragraph("Grupo 16: Iván Pérez, David Sánchez y Miguel Piedrahita", sub))
s.append(Spacer(1, 4))

s.append(Paragraph("1. Objetivo", h))
s.append(Paragraph(
    "Juego para dos jugadores con mandos inalámbricos que se manejan moviendo la mano. El <b>cazador</b> apunta con una "
    "pistola (IMU) y dispara con un gatillo; el <b>pájaro</b> vuela por la pantalla siguiendo los movimientos de su mando y tiene un turbo. "
    "La partida (60 s, 3 vidas, cargador limitado, recarga apuntando al suelo) se ve en el navegador, y los mandos "
    "vibran al disparar y al recibir un impacto. El sistema guarda las partidas, muestra un ranking, avisa por Telegram "
    "y permite actualizar el firmware por WiFi (FOTA).", txt))

s.append(Paragraph("2. Dispositivos", h))
filas = [
    ["Nodo", "Equipo", "Sensores / entradas", "Actuadores", "Rol"],
    ["Base", "ESP32-C3", "botón BOOT", "LED RGB (estado de la partida)",
     "Recibe los mandos, ejecuta la lógica del juego, sirve la web y hace de pasarela con MQTT"],
    ["Cazador", "ESP32-C3 + powerbank", "IMU 6 ejes MPU6050, gatillo (microinterruptor)", "motor de vibración (retroceso), LED RGB",
     "Apuntar y disparar"],
    ["Pájaro", "ESP32-C3 + powerbank", "IMU 6 ejes MPU6050, botón turbo", "motor de vibración (impacto), LED RGB (turbo)",
     "Volar y esquivar los disparos"],
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
s.append(diagrama(doc.width))
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
