# Carpeta del servidor de actualizaciones

Aquí se ponen los archivos que descarga el ESP32:
- `version.txt`: un número con la última versión disponible (p. ej. `2`).
- `firmware.bin`: el binario de esa versión (Programa → Exportar binario compilado).

Se sirve con `python -m http.server 8000` desde esta carpeta. Ver la guía de la etapa 2.
Los `.bin` no se suben a Git (están en `.gitignore`).
