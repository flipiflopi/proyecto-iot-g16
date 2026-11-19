# Node-RED

Aquí van los flujos exportados (`Menú → Exportar → Todos los flujos → .json`).

- Cada uno ejecuta Node-RED en local con Docker.
- **Solo una instancia "oficial"** debe estar conectada a la vez al broker/MongoDB del grupo
  con el bot real de Telegram; para desarrollar, usad topics y colecciones de prueba.
- No escribáis contraseñas en los flujos: usad las credenciales de los nodos o variables de entorno.
