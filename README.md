# 🎮 Hola Mundo PS3

Un programa "Hola Mundo" homebrew para **PlayStation 3**, compilado con el toolchain [ps3toolchain](https://github.com/ps3dev/ps3toolchain) y el SDK [PSL1GHT](https://github.com/ps3dev/PSL1GHT).

## Descripción

Este proyecto es un ejemplo mínimo de desarrollo homebrew para PS3. Imprime `¡Hola Mundo desde PlayStation 3!` utilizando las herramientas de compilación cruzada del ecosistema ps3dev.

## Estructura del proyecto

```
.
├── main.c                      # Código fuente principal
├── Makefile                    # Sistema de compilación (usa ppu_rules de PSL1GHT)
├── Dockerfile                  # Dockerfile raíz (referencia)
└── .devcontainer/
    ├── Dockerfile              # Imagen Docker con el toolchain PS3 completo
    └── devcontainer.json       # Configuración de Dev Container para VS Code
```

## Requisitos previos

- [Docker](https://www.docker.com/) instalado
- [VS Code](https://code.visualstudio.com/) con la extensión [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)

## Inicio rápido

### 1. Construir la imagen Docker

La imagen compila todo el toolchain PS3 desde cero (puede tardar **30-60+ minutos** la primera vez):

```bash
cd .devcontainer
docker build -t ps3dev-local .
```

### 2. Abrir en Dev Container

Abre el proyecto en VS Code y usa el comando:

> **Dev Containers: Reopen in Container**

Esto levantará el entorno con todas las herramientas listas (`ppu-gcc`, `ppu-as`, `ppu-ld`, etc.).

### 3. Compilar

Dentro del contenedor:

```bash
make
```

Esto generará:

| Archivo          | Descripción                                  |
|------------------|----------------------------------------------|
| `hola_ps3.elf`   | Ejecutable ELF para PPU                      |
| `hola_ps3.self`  | Ejecutable firmado (listo para correr en PS3) |

### 4. Ejecutar en PS3

Transfiere el archivo `hola_ps3.self` a tu consola PS3 con CFW (Custom Firmware) mediante FTP o USB y ejecútalo desde un file manager como **multiMAN** o **webMAN**.

### 5. Ejecutar en emulador (RPCS3)

Si no tienes una PS3 física, puedes usar el emulador [RPCS3](https://rpcs3.net/):

1. **Descargar RPCS3** desde [rpcs3.net](https://rpcs3.net/download) (disponible para Windows, Linux y macOS).

2. **Instalar el firmware de PS3:**
   - Descarga el firmware oficial desde [PlayStation.com](https://www.playstation.com/en-us/support/hardware/ps3/system-software/).
   - En RPCS3 ve a **File → Install Firmware** y selecciona el archivo `PS3UPDAT.PUP`.

3. **Ejecutar el homebrew:**
   - Ve a **File → Boot (S)ELF / (S)SELF** y selecciona el archivo `hola_ps3.self`.
   - Alternativamente, puedes arrastrar el archivo `.self` directamente a la ventana de RPCS3.

4. **Crear un PKG instalable** *(opcional)*:
   Si prefieres instalar como aplicación en RPCS3, puedes empaquetar el `.self` en un `.pkg`:
   ```bash
   # Dentro del Dev Container (si las herramientas de empaquetado están disponibles)
   make pkg
   ```
   Luego en RPCS3: **File → Install Packages/Raps/Edats** y selecciona el `.pkg`.

> **Nota:** RPCS3 requiere un equipo con buenas prestaciones. Consulta la [guía de inicio rápido](https://rpcs3.net/quickstart) para los requisitos de sistema recomendados.

## Detalles técnicos

| Propiedad     | Valor                                      |
|---------------|---------------------------------------------|
| **Target**    | `hola_ps3`                                  |
| **Content ID**| `UP0001-PSL145310_00-0000000000000001`      |
| **App ID**    | `PSL145310`                                 |
| **Toolchain** | ps3toolchain (GCC cross-compiler PPU/SPU)   |
| **SDK**       | PSL1GHT                                     |
| **Base OS**   | Debian Bookworm (contenedor Docker)         |

## Licencia

Este proyecto es de uso libre con fines educativos.
