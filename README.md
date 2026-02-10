# 🎮 Hola Mundo PS3

Un programa "Hola Mundo" homebrew para **PlayStation 3**, compilado con el toolchain [ps3toolchain](https://github.com/ps3dev/ps3toolchain) y el SDK [PSL1GHT](https://github.com/ps3dev/PSL1GHT).

## Descripción

Este proyecto es un ejemplo mínimo de desarrollo homebrew para PS3. El programa imprime un saludo en la consola TTY y termina.

## Estructura del proyecto

```
.
├── source/
│   └── main.c                  # Programa PPU (Hello World)
├── scripts/
│   └── fix_entry_toc.sh        # Parche post-link para el TOC del entry point
├── Makefile                    # Makefile principal (ppu_rules)
├── Dockerfile                  # Dockerfile raíz (referencia)
├── DockerfileMacM              # Dockerfile alternativo para Mac M-series
└── .devcontainer/
    └── devcontainer.json       # Configuración de Dev Container para VS Code
```

## Requisitos previos

- [Docker](https://www.docker.com/) instalado
- [VS Code](https://code.visualstudio.com/) con la extensión [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)

## Inicio rápido

### 1. Construir la imagen Docker

La imagen compila todo el toolchain PS3 desde cero (puede tardar **30-60+ minutos** la primera vez):

```bash
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

| Archivo           | Descripción                                    |
|-------------------|------------------------------------------------|
| `hello-ps3.elf`   | Ejecutable ELF para PPU                        |

### 4. Ejecutar en PS3 (CFW)

Transfiere el archivo `hello-ps3.self` a tu consola PS3 con CFW (Custom Firmware) mediante FTP o USB y ejecútalo desde un file manager como **multiMAN** o **webMAN**.

### 5. Ejecutar en PS3 (HEN)

[PS3HEN](https://www.psx-place.com/threads/ps3hen.23369/) (Homebrew Enabler) permite correr homebrew en **cualquier PS3** con firmware oficial (OFW), sin necesidad de un Custom Firmware completo. Es compatible con todos los modelos, incluyendo Super Slim.

#### Instalación de HEN

1. Asegúrate de tener tu PS3 en el **firmware más reciente** (o el requerido por la versión de HEN).
2. Desde el navegador web de la PS3, accede al sitio del exploit HEN (por ejemplo, `http://ps3xploit.me/hen`).
3. Sigue las instrucciones en pantalla para habilitar HEN. Al reiniciar la consola, deberás activar HEN nuevamente desde el navegador (no es permanente).

#### Transferir y ejecutar el homebrew

1. **Vía USB:**
   - Copia `hello-ps3.self` a una memoria USB.
   - Conecta la USB a la PS3 y usa un file manager como **multiMAN** para navegar y ejecutar el archivo.

2. **Vía FTP:**
   - Instala un servidor FTP como **webMAN MOD** (disponible como `.pkg`).
   - Conéctate desde tu PC con un cliente FTP a la IP de tu PS3.
   - Sube `hello-ps3.self` a `/dev_hdd0/game/` o cualquier ubicación accesible.
   - Ejecuta desde multiMAN o el file manager.

3. **Como PKG instalable** *(recomendado)*:
   - Genera un `.pkg` con `make pkg`.
   - Copia el `.pkg` a una USB en la raíz.
   - En la PS3 ve a **Juego → Package Manager → Install Package Files → Standard**.

> **Importante:** HEN se desactiva al reiniciar la consola. Deberás habilitarlo de nuevo desde el navegador cada vez que enciendas la PS3.

### 6. Ejecutar en emulador (RPCS3)

Si no tienes una PS3 física, puedes usar el emulador [RPCS3](https://rpcs3.net/):

1. **Descargar RPCS3** desde [rpcs3.net](https://rpcs3.net/download).

2. **Instalar el firmware de PS3:**
   - Descarga el firmware oficial de Sony desde su sitio web.
   - En RPCS3: File → Install Firmware → selecciona el archivo `.PUP`

3. **Ejecutar el programa:**
   - File → Boot (S)ELF / (S)SELF
   - Selecciona `hello-ps3.self`
   - La salida "hello, ps3" aparecerá en **View → Log → TTY**

## Generación de PKG

Para crear un paquete `.pkg` instalable:

```bash
make pkg
```

Esto genera `hello-ps3.pkg` que puede instalarse en:
- PS3 con CFW usando Package Manager
- PS3 con HEN usando Package Manager  
- RPCS3 (File → Install Packages/Games)

El PKG incluye:
- `PARAM.SFO` con metadata (título, ID, versión)
- `EBOOT.BIN` (SELF ejecutable)
- Firma con Content ID: `UP0001-PSL145310_00-0000000000000001`

## Archivos Generados

| Archivo | Descripción | Tamaño aprox. |
|---------|-------------|---------------|
| `hello-ps3.elf` | Ejecutable ELF para PowerPC 64-bit | ~844 KB |
| `hello-ps3.self` | Archivo SELF firmado | ~847 KB |
| `hello-ps3.pkg` | Paquete PKG instalable | ~848 KB |

## Notas Técnicas

- **SDK:** PSL1GHT (lightweight PS3 SDK)
- **Toolchain:** ps3toolchain (GCC 7.2.0 para PPU)
- **Arquitectura:** PowerPC 64-bit Cell Broadband Engine
- **Entry Point:** Usa `SYS_PROCESS_PARAM(1001, 0x100000)` estándar
- **Salida:** TTY (Terminal output visible en debuggers/RPCS3)

2. **Instalar el firmware de PS3:**
   - Descarga el firmware oficial desde [PlayStation.com](https://www.playstation.com/en-us/support/hardware/ps3/system-software/).
   - En RPCS3 ve a **File → Install Firmware** y selecciona el archivo `PS3UPDAT.PUP`.

3. **Ejecutar el homebrew:**
   - Ve a **File → Boot (S)ELF / (S)SELF** y selecciona `hello-ps3.elf`.
   - La salida se puede ver en **View → Log → TTY**.

## Detalles técnicos

| Propiedad      | Valor                                      |
|----------------|---------------------------------------------|
| **Target**     | `hello-ps3`                                 |
| **Ejecutable** | `hello-ps3.elf` / `hello-ps3.self`          |
| **Content ID** | `UP0001-PSL145310_00-0000000000000001`      |
| **App ID**     | `PSL145310`                                 |
| **Toolchain**  | ps3toolchain (GCC cross-compiler PPU)       |
| **SDK**        | PSL1GHT                                     |
| **Base OS**    | Debian Bookworm (contenedor Docker)         |

### Nota sobre el parche de entry point

El CRT de inicio (`lv2-crt0.o`) de PSL1GHT tiene un bug donde el descriptor de función `_start` en `.rodata` no incluye correctamente el TOC (Table of Contents). El script `scripts/fix_entry_toc.sh` corrige esto automáticamente después del enlazado, parchando el campo rtoc en el entry point del ELF.

## Licencia

Este proyecto es de uso libre con fines educativos.
