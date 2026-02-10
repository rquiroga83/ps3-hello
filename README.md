# 🎮 Hola Mundo PS3

Un **ejemplo mínimo funcional** de homebrew para **PlayStation 3**, que muestra un mensaje gráfico en pantalla usando el sistema de renderizado RSX.

Compilado con el toolchain [ps3toolchain](https://github.com/ps3dev/ps3toolchain) y el SDK [PSL1GHT](https://github.com/ps3dev/PSL1GHT).

## 📝 Descripción

Este proyecto es un **ejemplo educativo completo** de desarrollo homebrew para PS3. El programa:

- ✅ Inicializa el sistema gráfico **RSX** (Reality Synthesizer - el chip gráfico de PS3)
- ✅ Dibuja un mensaje visual colorido **"HOLA PS3"** en el centro de la pantalla
- ✅ Maneja entrada del **controlador DualShock 3**
- ✅ Implementa un **loop de aplicación** funcional
- ✅ Usa **double buffering** para evitar tearing visual
- ✅ Imprime mensajes de depuración en la **consola TTY**

### 🎨 ¿Qué verás en pantalla?

El programa dibuja un mensaje visual compuesto por:
- Fondo **negro** (limpieza de pantalla)
- Rectángulo central **azul** con borde **blanco**
- Barra decorativa **verde** en la parte superior
- Bloques de colores (**amarillo**, **rojo**, **blanco**) simulando texto "PS3"
- Línea decorativa inferior

**Controles:**
- Presiona **✖ (X/Cross)** en el control para salir del programa

## 📂 Estructura del proyecto

```
.
├── source/
│   └── main.c                  # Programa principal con gráficos RSX
├── scripts/
│   └── create_sfo.py           # Generador de PARAM.SFO para PKG
├── build/                      # Archivos de compilación (generados)
├── pkg_temp/                   # Archivos temporales para PKG (generados)
├── Makefile                    # Build system (ppu_rules)
├── README.md                   # Este archivo
├── EXPLICACION.md              # 📚 Explicación técnica detallada del código
├── MODIFICACIONES.md           # 🎨 Guía de modificaciones y ejemplos
├── Dockerfile                  # Imagen Docker para compilación
├── DockerfileMacM              # Dockerfile para Mac M-series
└── .devcontainer/
    └── devcontainer.json       # Configuración de Dev Container para VS Code
```

## 📖 Documentación

Este proyecto incluye documentación completa:

- **[README.md](README.md)** (este archivo) - Inicio rápido y setup
- **[EXPLICACION.md](EXPLICACION.md)** - Explicación técnica detallada:
  - Arquitectura de PS3 (Cell, RSX)
  - Sistema gráfico y framebuffers
  - Explicación línea por línea del código
  - Conceptos avanzados (GCM, memory alignment, etc.)
  - Referencias y recursos externos

- **[MODIFICACIONES.md](MODIFICACIONES.md)** - Guía práctica de modificaciones:
  - Cambiar colores y tamaños
  - Agregar nuevas formas (círculos, líneas, gradientes)
  - Implementar animaciones simples
  - Personalizar controles
  - Ejemplos completos de código interactivo

- **[TROUBLESHOOTING.md](TROUBLESHOOTING.md)** - 🔧 Solución de problemas:
  - Error "datos dañados" al instalar PKG
  - Problemas comunes de instalación
  - Verificación de PKG y archivos
  - Herramientas de diagnóstico

- **[FLUJO.md](FLUJO.md)** - Diagramas de flujo de ejecución

## 💡 Requisitos previos

- [Docker](https://www.docker.com/) instalado
- [VS Code](https://code.visualstudio.com/) con la extensión [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)

## 🚀 Inicio rápido

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
make           # Compilar el ejecutable
make pkg       # Crear el paquete PKG instalable
```

Esto generará:

| Archivo           | Descripción                                    |
|-------------------|------------------------------------------------|
| `hello-ps3.elf`   | Ejecutable ELF para PPU                        |
| `hello-ps3.self`  | Ejecutable firmado para PS3                    |
| `hello-ps3.pkg`   | Paquete instalable (recomendado)               |

### 4. Instalar en PS3 (usando PKG)

**Método recomendado** - Instala como una aplicación real:

1. Transfiere `hello-ps3.pkg` a tu PS3 mediante:
   - **FTP** (usando FileZilla o similar)
   - **USB** (en la carpeta raíz del USB)
   - **Servidor web local** (desde el navegador de PS3)

2. En la PS3:
   - Ve a **Package Manager** o **Install Package Files**
   - Selecciona el archivo `hello-ps3.pkg`
   - Instala (aparecerá en el XMB)
   - Ejecuta "Hola Mundo PS3" desde el menú

### 5. Ejecutar directamente (archivo SELF)

También puedes ejecutar `hello-ps3.self` directamente con un file manager:
- **multiMAN**, **webMAN**, **IRISMAN**, etc.

## 📱 Requisitos de la PS3

### Opción A: Custom Firmware (CFW)

Cualquier PS3 compatible con CFW puede ejecutar este homebrew:
- PS3 Fat (CECH-xxxx)
- PS3 Slim hasta modelo 3000
- **Firmwares recomendados:** Rebug, Ferrox, IRISMAN CFW

### Opción B: PS3HEN (Homebrew Enabler)

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

## Licencia

Este proyecto es de uso libre con fines educativos.
