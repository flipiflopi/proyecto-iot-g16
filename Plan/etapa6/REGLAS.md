# Reglas del juego "Cazador vs. Pájaro" (propuesta)

> **Estado: propuesta 🟡** — Hay que confirmarla jugando: los números (vidas, puntos, radios) son de partida
> y se ajustan en la etapa 6. Todos son **configurables** (no fijos en el código), ver al final.

## Idea
Partida de **60 s**. El pájaro tiene **3 vidas**. El cazador intenta quitárselas cuanto antes; el pájaro intenta aguantar.
- **Gana el cazador** si quita todas las vidas antes de que acabe el tiempo.
- **Gana el pájaro** si llega al final con al menos 1 vida.

## Puntuación
La puntuación depende del **tiempo** para los dos, como en vuestra idea:

| Quién | Cómo suma | Ejemplo |
|---|---|---|
| **Cazador** | Cada impacto vale `100 × tiempo_restante / 60`. Si lo mata: **bonus** `300 × tiempo_restante / 60` | Impactos en los s 10, 20 y 30 → 83 + 67 + 50 = 200, más un bonus de 150 = **350** |
| **Pájaro** | **10 puntos por segundo** vivo. Si sobrevive: **+100 por vida** que le quede | Muere en el s 30 → **300**. Aguanta los 60 s con 2 vidas → 600 + 200 = **800** |

Así, si el cazador mata pronto suma mucho y el pájaro poco, y al revés si mata tarde o no lo mata.

**Ronda doble:** se juegan 2 partidas cambiando los papeles y gana quien sume más entre las dos. Así se puede comparar
a los jugadores aunque cada papel puntúe distinto. El ranking de Telegram (`/ranking`) se guarda **por papel**.

## Reglas para que no sea trivial
Sin estas reglas, el cazador gana disparando sin parar:
- **Cargador de 5 balas.** Para **recargar**, se apunta la pistola **al suelo** (pitch < −60°) durante 1 s.
  Usa la IMU, queda bien en la demo y obliga a perder de vista al pájaro.
- **Cadencia:** como mucho 1 disparo cada 0,4 s.
- **Invulnerabilidad de 1,5 s** tras cada impacto: el pájaro parpadea y no puede recibir otro impacto
  (si no, una ráfaga le quitaría las 3 vidas de golpe).
- **Turbo del pájaro** (botón): durante 1 s se mueve el doble de rápido, y luego no puede volver a usarlo durante 5 s.
  Su LED muestra si el turbo está disponible (verde) o recargando (rojo).

## Qué hacen los actuadores
| Evento | Cazador | Pájaro | Base |
|---|---|---|---|
| Impacto | vibración corta | vibración larga | — |
| Pájaro muerto / fin | vibración triple | vibración triple | LED con el ganador |
| Cargador vacío | 2 pulsos cortos | — | — |
| Pájaro a menos de ~10° del punto de mira | — | vibración suave ("te están apuntando") | — |

## Quién decide el impacto
Lo decide **solo el juego** (la página web), que es quien conoce dónde está todo en pantalla.
El juego avisa a Node-RED, Node-RED le envía la orden de vibración a la base por MQTT y lo registra en MongoDB (etapa 7).

## Parámetros configurables
Se guardan con cada **escenario** en MongoDB, así cada escenario puede tener su propia dificultad:

| Parámetro | Valor inicial |
|---|---|
| `duracion_s` | 60 |
| `vidas_pajaro` | 3 |
| `balas_cargador` | 5 |
| `radio_impacto_px` | 40 |
| `invulnerable_s` | 1.5 |
| `turbo_s` / `turbo_espera_s` | 1 / 5 |
