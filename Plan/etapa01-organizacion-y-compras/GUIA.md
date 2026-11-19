# Etapa 1 — Organización y compras

> **Estado:** en curso 🟡 (sin código: es una lista de tareas)

## Tareas
- [ ] **Pedido de AliExpress** → [COMPRAS.md](COMPRAS.md). Es lo primero: tarda en llegar.
- [x] Repositorio GitHub privado `flipiflopi/proyecto-iot-g16`
- [ ] Invitar a los otros 2 integrantes como colaboradores (GitHub → *Settings → Collaborators*)
- [ ] Cada uno: `git clone`, Arduino IDE 2 con esp32 3.x, librerías y Python (ver [etapa 2](../etapa02-validacion-de-tecnologias/GUIA.md#preparación-una-sola-vez)), Node-RED en Docker
- [ ] Credenciales del grupo **II16** (Campus Virtual → "Credenciales del grupo"): usuario/clave para MQTT y MongoDB → solo en `secrets.h`, nunca en el repo
- [ ] **Bot de Telegram**: hablar con `@BotFather` → `/newbot` → guardar el token (fuera del repo)
- [ ] **Canal de difusión** de Telegram, con el bot como administrador
- [ ] **Visto bueno del profesor por escrito** (correo) con la propuesta final; guardar la respuesta
- [ ] Preguntar si la universidad tiene **impresora 3D** (etapa 10)

## Cómo trabajamos con Git
1. `git pull` antes de empezar.
2. Una rama por tarea: `git switch -c etapa2-mqtt`.
3. Commits pequeños con mensajes claros.
4. `git push` y **Pull Request** a `main`; otro integrante lo revisa y lo fusiona.
5. Nunca trabajar directamente sobre `main`. Nunca subir `secrets.h`.

## Terminada cuando
Pedido hecho, los 3 con el entorno funcionando, bot y canal creados, y visto bueno del profesor guardado.
