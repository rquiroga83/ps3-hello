# 🎮 Hola Mundo PS3

Un programa "Hola Mundo" homebrew para **PlayStation 3**, compilado con el toolchain [ps3toolchain](https://github.com/ps3dev/ps3toolchain) y el SDK [PSL1GHT](https://github.com/ps3dev/PSL1GHT).

## Descripción

Este proyecto es un ejemplo de desarrollo homebrew para PS3 que demuestra el uso de los **Synergistic Processing Elements (SPEs)** del procesador Cell Broadband Engine. El programa envía un vector de 4 floats a un SPE, que realiza cálculos SIMD en paralelo (cuadrados, producto punto, magnitud) y devuelve los resultados al PPU.

### ¿Qué hace el ejemplo SPE?

1. **PPU** (PowerPC Processing Unit): Prepara un vector `(1.0, 2.0, 3.0, 4.0)` y crea un thread SPU
2. **SPE** (Synergistic Processing Element): Recibe el vector via **DMA**, calcula:
   - **Cuadrados** de cada componente usando multiplicación SIMD paralela: `(1, 4, 9, 16)`
   - **Producto punto** (norma²): `1² + 2² + 3² + 4² = 30.0`
   - **Magnitud** usando la instrucción `rsqrte` del SPE: `≈ 5.4772`
3. **PPU**: Lee los resultados transferidos por DMA y los muestra en pantalla

## Estructura del proyecto

```
.
├── source/
│   └── main.c                  # Programa PPU (gestión de threads SPU)
├── spu/
│   ├── source/
│   │   └── main.c              # Programa SPU (cálculo vectorial SIMD)
│   └── Makefile                # Compilación del programa SPU (spu_rules)
├── include/
│   └── vecmath.h               # Estructura compartida PPU ↔ SPU
├── data/                       # Binario SPU compilado (spu.bin)
├── Makefile                    # Makefile principal PPU (ppu_rules + bin2o)
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

| Archivo          | Descripción                                    |
|------------------|------------------------------------------------|
| `data/spu.bin`   | Programa SPU compilado (embebido en el ELF)    |
| `hola-ps3.elf`   | Ejecutable ELF para PPU con SPU embebido       |
| `hola-ps3.self`  | Ejecutable firmado (listo para correr en PS3)  |

> **Nota:** El Makefile primero compila el programa SPU (`spu/`), luego lo embebe en el ejecutable PPU usando `bin2o`.

### 4. Ejecutar en PS3 (CFW)

Transfiere el archivo `hola_ps3.self` a tu consola PS3 con CFW (Custom Firmware) mediante FTP o USB y ejecútalo desde un file manager como **multiMAN** o **webMAN**.

### 5. Ejecutar en PS3 (HEN)

[PS3HEN](https://www.psx-place.com/threads/ps3hen.23369/) (Homebrew Enabler) permite correr homebrew en **cualquier PS3** con firmware oficial (OFW), sin necesidad de un Custom Firmware completo. Es compatible con todos los modelos, incluyendo Super Slim.

#### Instalación de HEN

1. Asegúrate de tener tu PS3 en el **firmware más reciente** (o el requerido por la versión de HEN).
2. Desde el navegador web de la PS3, accede al sitio del exploit HEN (por ejemplo, `http://ps3xploit.me/hen`).
3. Sigue las instrucciones en pantalla para habilitar HEN. Al reiniciar la consola, deberás activar HEN nuevamente desde el navegador (no es permanente).

#### Transferir y ejecutar el homebrew

1. **Vía USB:**
   - Crea la ruta `PS3/SAVEDATA/` o simplemente copia `hola_ps3.self` a una memoria USB.
   - Conecta la USB a la PS3 y usa un file manager como **multiMAN** (instalable como `.pkg`) para navegar y ejecutar el archivo.

2. **Vía FTP:**
   - Instala un servidor FTP como **webMAN MOD** (disponible como `.pkg`).
   - Conéctate desde tu PC con un cliente FTP (FileZilla, WinSCP, etc.) a la IP de tu PS3.
   - Sube `hola_ps3.self` a `/dev_hdd0/game/` o cualquier ubicación accesible.
   - Ejecuta desde multiMAN o el file manager.

3. **Como PKG instalable** *(recomendado)*:
   - Si generas un `.pkg`, puedes instalarlo directamente desde el XMB:
     - Copia el `.pkg` a una USB en la raíz.
     - En la PS3 ve a **Juego → Package Manager → Install Package Files → Standard**.
     - La aplicación aparecerá en el XMB como cualquier juego.

> **Importante:** HEN se desactiva al reiniciar la consola. Deberás habilitarlo de nuevo desde el navegador cada vez que enciendas la PS3.

### 6. Ejecutar en emulador (RPCS3)

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
| **Target**    | `hola-ps3`                                  |
| **Content ID**| `UP0001-PSL145310_00-0000000000000001`      |
| **App ID**    | `PSL145310`                                 |
| **Toolchain** | ps3toolchain (GCC cross-compiler PPU/SPU)   |
| **SDK**       | PSL1GHT                                     |
| **Base OS**   | Debian Bookworm (contenedor Docker)         |

### Arquitectura Cell Broadband Engine

```
┌─────────────────────────────────────────────────┐
│              Cell Broadband Engine              │
│                                                 │
│  ┌─────────┐    ┌─────┐ ┌─────┐ ┌─────┐         │
│  │   PPU   │    │ SPE │ │ SPE │ │ SPE │         │
│  │ PowerPC │    │  0  │ │  1  │ │  2  │         │
│  │  64-bit │    │256KB│ │256KB│ │256KB│         │
│  └────┬────┘    └──┬──┘ └──┬──┘ └──┬──┘         │
│       │            │       │       │            │
│  ═════╪════════════╪═══════╪═══════╪═══════════ │
│       │     Element Interconnect Bus (EIB)      │
│  ═════╪════════════╪═══════╪═══════╪═══════════ │
│       │            │       │       │            │
│  ┌────┴────┐    ┌──┴──┐ ┌──┴──┐ ┌──┴──┐         │
│  │   MIC   │    │ SPE │ │ SPE │ │ SPE │         │
│  │ Memory  │    │  3  │ │  4  │ │  5  │         │
│  │Interface│    │256KB│ │256KB│ │256KB│         │
│  └─────────┘    └─────┘ └─────┘ └─────┘         │
│                                                 │
│  PPU: Controla el flujo del programa            │
│  SPE: 128-bit SIMD, cálculo masivo paralelo     │
│  DMA: Transferencia asíncrona entre RAM y SPE   │
└─────────────────────────────────────────────────┘
```

## Licencia

Este proyecto es de uso libre con fines educativos.
