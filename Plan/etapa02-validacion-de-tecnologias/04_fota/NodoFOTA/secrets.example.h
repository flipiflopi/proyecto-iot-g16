// Copia este archivo como "secrets.h" en esta misma carpeta y rellénalo.
// secrets.h está en .gitignore: NUNCA se sube al repositorio.
#pragma once

#define WIFI_SSID    "NOMBRE_DE_LA_WIFI"   // 2,4 GHz, WPA2 (p. ej. hotspot del móvil)
#define WIFI_PASS    "CLAVE_DE_LA_WIFI"

// Servidor de actualizaciones en vuestro PC (python -m http.server 8000 en la carpeta "servidor").
// Sustituid 192.168.X.Y por la IP del PC en la MISMA red que el ESP32 (ipconfig en Windows).
#define URL_VERSION  "http://192.168.X.Y:8000/version.txt"
#define URL_FIRMWARE "http://192.168.X.Y:8000/firmware.bin"
