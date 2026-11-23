// Página de la prueba 09: la misma de la prueba 08, pero servida por la base
// y con un panel de estadísticas de la base (ESP-NOW, WebSocket, memoria, MQTT).
// Va en la flash como texto (sin LittleFS para que la prueba sea más sencilla de subir).
#pragma once

const char PAGINA[] = R"rawliteral(<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Prueba 09 - juego servido por la base</title>
<style>
  body { margin: 0; background: #10141c; color: #e8ecf3; font-family: sans-serif; }
  header { padding: 8px 14px; display: flex; flex-wrap: wrap; gap: 18px; align-items: center; }
  .ok { color: #5fd38d; } .mal { color: #ff6b6b; }
  #stats { padding: 0 14px 8px; color: #9aa7bd; font-size: 13px; font-family: monospace; }
  #orden { padding: 0 14px 8px; color: #ffcc33; min-height: 18px; }
  canvas { display: block; margin: 0 auto; background: #1b2230; border-radius: 8px; max-width: 100%; }
</style>
</head>
<body>
<header>
  <b>Prueba 09 · mandos → base (ESP-NOW) → web servida por la base</b>
  <span id="estado" class="mal">desconectado</span>
  <span id="frecuencia">0 msg/s</span>
</header>
<div id="stats">esperando estadísticas de la base...</div>
<div id="orden"></div>
<canvas id="lienzo" width="900" height="520"></canvas>
<script>
const lienzo = document.getElementById('lienzo');
const ctx = lienzo.getContext('2d');
const mandos = {
  1: { yaw: 0, pitch: 0, roll: 0, disparo: 0, ultimo: 0, flash: 0 },
  2: { yaw: 0, pitch: 0, roll: 0, disparo: 0, ultimo: 0, flash: 0 }
};
const GRADOS_A_PIXELES = 8;
let mensajes = 0;
const $ = (id) => document.getElementById(id);

function conectar() {
  const ws = new WebSocket('ws://' + location.host + '/ws/mandos');
  ws.onopen = () => { $('estado').textContent = 'conectado'; $('estado').className = 'ok'; };
  ws.onclose = () => { $('estado').textContent = 'desconectado'; $('estado').className = 'mal'; setTimeout(conectar, 1000); };
  ws.onmessage = (ev) => {
    const d = JSON.parse(ev.data);
    if (d.tipo === 'stats') {
      const pc = (r, p) => (r + p) ? (100 * p / (r + p)).toFixed(1) : '0.0';
      $('stats').textContent =
        'mando1 ' + d.rx1 + ' paq/s (' + d.p1 + ' perd, ' + pc(d.rx1, d.p1) + ' %) · ' +
        'mando2 ' + d.rx2 + ' paq/s (' + d.p2 + ' perd, ' + pc(d.rx2, d.p2) + ' %) · ' +
        'WS ' + d.ws + ' env/s, ' + d.salt + ' saltados · ' + d.cli + ' navegador(es) · ' +
        'heap ' + d.heap + ' B (mín ' + d.heapMin + ') · canal ' + d.canal + ' · RSSI ' + d.rssi + ' dBm · ' +
        'MQTT ' + (d.mqtt ? 'OK' : 'NO');
      return;
    }
    if (d.tipo === 'orden') {
      $('orden').textContent = 'Orden MQTT recibida: "' + d.texto + '" a las ' + new Date().toLocaleTimeString();
      return;
    }
    const m = mandos[d.id];
    if (!m) return;
    if (d.disparo && !m.disparo) m.flash = 12;   // flanco de subida = nuevo disparo
    Object.assign(m, d, { ultimo: performance.now() });
    mensajes++;
  };
}
setInterval(() => { $('frecuencia').textContent = mensajes + ' msg/s'; mensajes = 0; }, 1000);

function dibujar() {
  const W = lienzo.width, H = lienzo.height, ahora = performance.now();
  ctx.clearRect(0, 0, W, H);
  ctx.strokeStyle = '#2c3648';
  ctx.lineWidth = 1;
  ctx.beginPath(); ctx.moveTo(W / 2, 0); ctx.lineTo(W / 2, H); ctx.moveTo(0, H / 2); ctx.lineTo(W, H / 2); ctx.stroke();

  // Cazador: punto de mira (yaw -> x, pitch -> y)
  const c = mandos[1];
  const x = W / 2 + c.yaw * GRADOS_A_PIXELES, y = H / 2 - c.pitch * GRADOS_A_PIXELES;
  const vivo1 = ahora - c.ultimo < 500;
  ctx.strokeStyle = vivo1 ? '#ffcc33' : '#555';
  ctx.lineWidth = 3;
  ctx.beginPath(); ctx.arc(x, y, 18 + c.flash * 2, 0, Math.PI * 2); ctx.stroke();
  ctx.beginPath(); ctx.moveTo(x - 26, y); ctx.lineTo(x + 26, y); ctx.moveTo(x, y - 26); ctx.lineTo(x, y + 26); ctx.stroke();
  if (c.flash > 0) c.flash--;

  // Pájaro: avión que se inclina (roll) y sube/baja (pitch)
  const p = mandos[2];
  const vivo2 = ahora - p.ultimo < 500;
  ctx.save();
  ctx.translate(W * 0.8, H / 2 - p.pitch * GRADOS_A_PIXELES);
  ctx.rotate(p.roll * Math.PI / 180);
  ctx.fillStyle = vivo2 ? '#5fb8ff' : '#555';
  ctx.beginPath(); ctx.moveTo(-60, 0); ctx.lineTo(60, 0); ctx.lineTo(0, -14); ctx.closePath(); ctx.fill();
  ctx.fillRect(-6, -30, 12, 30);
  ctx.restore();

  ctx.fillStyle = '#9aa7bd';
  ctx.font = '14px sans-serif';
  ctx.fillText('Cazador (id 1): yaw ' + c.yaw.toFixed(1) + '°  pitch ' + c.pitch.toFixed(1) + '°' + (vivo1 ? '' : '  [sin datos]'), 12, H - 34);
  ctx.fillText('Pájaro (id 2): roll ' + p.roll.toFixed(1) + '°  pitch ' + p.pitch.toFixed(1) + '°' + (vivo2 ? '' : '  [sin datos]'), 12, H - 14);
  requestAnimationFrame(dibujar);
}
conectar();
dibujar();
</script>
</body>
</html>
)rawliteral";
