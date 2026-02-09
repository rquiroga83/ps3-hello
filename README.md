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
