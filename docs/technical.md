# Documentacion tecnica: Hola Mundo PS3

## Resumen del build system

### Toolchain

El proyecto se compila con el cross-compiler `powerpc64-ps3-elf-gcc` (GCC 7.2.0) incluido en la imagen Docker `flipacholas/ps3devextra:latest`. Este toolchain genera codigo para la arquitectura PowerPC 64-bit del PPU (PowerPC Processing Unit) del Cell Broadband Engine.

La imagen Docker provee:

| Herramienta | Ruta | Funcion |
|---|---|---|
| `powerpc64-ps3-elf-gcc` | `/usr/local/ps3dev/ppu/bin/` | Compilador C/C++ para PPU |
| `ppu-strip` | `/usr/local/ps3dev/ppu/bin/` | Elimina simbolos de debug del ELF |
| `sprxlinker` | `/usr/local/ps3dev/bin/` | Resuelve imports de modulos PRX del sistema |
| `make_self` | `/usr/local/ps3dev/bin/` | Genera SELF firmado (ejecutable para PS3 real) |
| `fself.py` | `/usr/local/ps3dev/bin/` | Genera fake SELF (ejecutable para RPCS3) |
| `make_self_npdrm` | `/usr/local/ps3dev/bin/` | Genera SELF con proteccion NPDRM (para .pkg) |
| `pkg.py` | `/usr/local/ps3dev/bin/` | Empaqueta en formato .pkg instalable |
| `sfo.py` | `/usr/local/ps3dev/bin/` | Genera PARAM.SFO con metadatos de la app |

### Pipeline de compilacion

```
main.c
  │
  ▼  ppu-gcc -c (compilar a objeto PPC64)
main.o
  │
  ▼  ppu-gcc (linkear con librerias PSL1GHT)
hello_world.elf          ← ELF crudo, no ejecutable en PS3
  │
  ├─▶ ppu-strip          ← Eliminar simbolos de debug
  │     │
  │     ▼
  │   build/hello_world.elf (stripped)
  │     │
  │     ├─▶ sprxlinker   ← Resolver imports de modulos del sistema
  │     │
  │     ├─▶ make_self    ← Firmar para PS3 real (CFW/HEN)
  │     │     ▼
  │     │   hello_world.self
  │     │
  │     └─▶ fself.py     ← Generar fake SELF para emulador
  │           ▼
  │         hello_world.fake.self
  │
  └─▶ (para .pkg)
        make_self_npdrm → EBOOT.BIN
        sfo.py          → PARAM.SFO
        pkg.py          → hello_world.pkg
```

### Makefile: ppu_rules

El Makefile usa `include $(PSL1GHT)/ppu_rules` que carga las reglas de compilacion del SDK. Este archivo (`/usr/local/ps3dev/ppu_rules`) define:

- **PREFIX**: `ppu-` (prefijo para todas las herramientas: `ppu-gcc`, `ppu-strip`, etc.)
- **MACHDEP**: `-mhard-float -fmodulo-sched -ffunction-sections -fdata-sections`
- **Regla `%.self: %.elf`**: strip → sprxlinker → make_self → fself.py
- **Regla `%.pkg: %.self`**: make_self_npdrm → sfo.py → pkg.py → package_finalize
- **LD**: por defecto `$(PREFIX)gcc` (usa `?=`), pero en Windows el PATH del host puede sobreescribirlo con el `ld` del sistema, por lo que se fuerza `LD := $(CC)` en el Makefile

`ppu_rules` a su vez incluye `base_rules` (reglas genericas de compilacion `.c → .o`, `.o → .elf`) y `data_rules` (conversion de datos binarios a objetos linkables).

### Problema del PATH en Windows

Al ejecutar `docker run -v "$PWD:/src"` desde Git Bash en Windows, el PATH del host se filtra al contenedor. Esto causa que `LD` (definido con `?=` en base_rules) tome el valor del `ld` nativo de Windows/MinGW en lugar de `ppu-gcc`. La solucion es forzar `LD := $(CC)` despues del include, o pasar `-e PATH=...` al docker run.

Adicionalmente, Git Bash convierte rutas como `/src` a `C:/Program Files/Git/src`. Se resuelve con `MSYS_NO_PATHCONV=1` antes del comando docker.

## Arquitectura del programa

### Inicializacion RSX (GPU)

El RSX (Reality Synthesizer) es el GPU de PS3, basado en NVIDIA G70. La inicializacion sigue estos pasos:

1. **Asignar memoria host** para el command buffer del RSX: `memalign(1MB, 1MB)`
2. **Inicializar RSX**: `rsxInit(0x10000, 1MB, host_addr)` — crea el contexto GCM con 64K entradas en el command buffer
3. **Detectar resolucion**: `videoGetState()` → `videoGetResolution()` — obtiene el modo de video activo
4. **Configurar salida de video**: `videoConfigure()` con formato `VIDEO_BUFFER_FORMAT_XRGB` (32 bits por pixel)
5. **Crear framebuffers**: dos buffers en VRAM via `rsxMemalign(64, size)` para doble buffering
6. **Registrar buffers con GCM**: `rsxAddressToOffset()` + `gcmSetDisplayBuffer()` — mapea la memoria RSX a display buffers

El doble buffering funciona asi: mientras el RSX muestra el buffer A, el PPU dibuja en el buffer B. Al terminar, se hace `gcmSetFlip()` para intercambiarlos. `gcmSetFlipMode(GCM_FLIP_VSYNC)` sincroniza el flip con el refresco vertical para evitar tearing.

### Renderizado de texto

No se usa ningun sistema de fuentes del SDK. El texto se renderiza con una fuente bitmap 8x8 embebida directamente en el codigo como un array de 95 caracteres (ASCII 32-126). Cada caracter es un array de 8 bytes donde cada bit representa un pixel.

El renderizado es por software: se itera sobre cada bit del glyph y se escribe directamente al framebuffer en RAM. El factor de escala multiplica cada pixel logico en un bloque de `scale x scale` pixeles reales.

### Input del control

Se usa `libio` para leer el DualShock 3:

1. `ioPadInit(7)` — inicializa el subsistema de pads (hasta 7 controles)
2. En el loop: `ioPadGetInfo()` verifica si hay un pad conectado en el slot 0
3. `ioPadGetData(0, &paddata)` lee el estado actual de todos los botones
4. `paddata.BTN_CROSS` es un campo que indica si el boton X esta presionado

### Callback del sistema

`sysUtilRegisterCallback()` registra una funcion que el sistema llama cuando ocurren eventos como:
- `SYSUTIL_EXIT_GAME`: el usuario presiono PS button → Quit Game desde el XMB
- Otros eventos del sistema (bateria baja del control, etc.)

Se debe llamar `sysUtilCheckCallback()` en cada frame para procesar estos eventos.

## Librerias y headers

### Headers utilizados

| Header | Libreria | Contenido |
|---|---|---|
| `<ppu-types.h>` | (base) | Tipos basicos: `u8`, `u16`, `u32`, `u64`, `s32`, etc. |
| `<rsx/rsx.h>` | librsx | `rsxInit`, `rsxFlushBuffer`, `rsxFinish`, `rsxMemalign`, `rsxAddressToOffset` |
| `<rsx/gcm_sys.h>` | libgcm_sys | `gcmSetFlip`, `gcmSetDisplayBuffer`, `gcmGetFlipStatus`, `gcmResetFlipStatus`, `gcmSetFlipMode`, `gcmSetWaitFlip` |
| `<rsx/mm.h>` | librsx | Memory manager del RSX |
| `<sysutil/video.h>` | libsysutil | `videoGetState`, `videoGetResolution`, `videoConfigure`, `VIDEO_BUFFER_FORMAT_XRGB` |
| `<sysutil/sysutil.h>` | libsysutil | `sysUtilRegisterCallback`, `sysUtilCheckCallback`, `SYSUTIL_EXIT_GAME` |
| `<io/pad.h>` | libio | `ioPadInit`, `ioPadEnd`, `ioPadGetInfo`, `ioPadGetData`, `padInfo`, `padData` |
| `<lv2/process.h>` | liblv2 | `sysProcessExit` |
| `<malloc.h>` | libc (newlib) | `memalign` para alineacion de memoria |

### Orden de linkeo

```
-lrsx -lgcm_sys -lio -lsysutil -lrt -llv2
```

El orden importa porque el linker de GCC resuelve simbolos de izquierda a derecha:
- `librsx` depende de `libgcm_sys` (funciones GCM de bajo nivel)
- `libio` y `libsysutil` dependen de `liblv2` (syscalls)
- `librt` es el runtime de C que inicializa stdout/stderr sobre el TTY de lv2 — sin ella, `printf` falla con `CELL_EBADF` porque el file descriptor 1 (stdout) no existe
- `liblv2` es la base: wrappers directos sobre las syscalls del hypervisor lv2

## Formato SELF

El formato SELF (Signed ELF) es el formato ejecutable nativo de PS3. Es un wrapper criptografico alrededor de un ELF estandar:

1. **ELF crudo** → `ppu-strip` elimina simbolos de debug
2. **sprxlinker** → resuelve los imports de modulos PRX del sistema operativo (libsre, libfs, etc.)
3. **make_self** → comprime cada segmento del ELF con deflate y lo encripta con claves de retail. El output es un SELF firmado que puede ejecutarse en PS3 con CFW
4. **fself.py** → genera un "fake SELF" sin firma criptografica real, solo con la estructura minima. RPCS3 acepta estos porque no verifica firmas

## Fuentes y referencias

### Documentacion del SDK

- [PSL1GHT repository](https://github.com/ps3dev/PSL1GHT) — SDK principal. Los archivos `ppu_rules`, `base_rules` y `data_rules` definen todo el build system
- [PSL1GHT ppu_rules](https://github.com/ps3dev/PSL1GHT/blob/master/ppu_rules) — Reglas de compilacion PPU, definiciones de SELF/PKG
- [PSL1GHT base_rules](https://github.com/ps3dev/PSL1GHT/blob/master/base_rules) — Reglas genericas de compilacion (%.c → %.o, %.o → %.elf)
- [PSL1GHT templates/trivial](https://github.com/ps3dev/PSL1GHT/tree/master/templates/trivial) — Template oficial minimo de hello world. Nuestro Makefile se basa en este
- [PSL1GHT DeepWiki](https://deepwiki.com/ps3dev/PSL1GHT) — Documentacion generada del SDK con detalles del build system y estructura de librerias

### Docker toolchain

- [flipacholas/Docker-PS3DevExtra](https://github.com/flipacholas/Docker-PS3DevExtra) — Imagen Docker con ps3toolchain precompilado. Issue #2 documenta problemas de compilacion de samples
- [ps3toolchain](https://github.com/ps3dev/ps3toolchain) — Scripts para compilar GCC, binutils y newlib para PPC64/SPU

### Ejemplos y samples

- [M0S4YSH3LL0/helloworld-ps3](https://github.com/M0S4YSH3LL0/helloworld-ps3) — Hello world con SimpleRSX (mas complejo, usa graficos RSX completos con FreeType)
- [PSL1GHT samples](https://github.com/ps3dev/PSL1GHT/tree/master/samples) — Samples oficiales organizados por categoria (audio, graphics, input, network, spu, sys)

### Foros y comunidad

- [RPCS3 Forums — Hello World using PSL1GHT SDK](https://forums.rpcs3.net/showthread.php?tid=149980) — Discusion sobre compilacion y ejecucion de homebrew PSL1GHT en RPCS3
- [Scognito's blog — Sconsole](https://scognito.wordpress.com/2010/11/07/sconsole-a-simple-function-for-printing-strings-on-ps3/) — Funcion para imprimir texto en PS3 (referencia historica de renderizado de texto en framebuffer)

### Documentacion del hardware

- [PS3 Developer Wiki — PSL1GHT](https://www.psdevwiki.com/ps3/PSL1GHT) — Referencia del SDK en la wiki de desarrollo PS3
- El procesador Cell Broadband Engine fue documentado extensamente por IBM. La informacion sobre SPEs, Local Store de 256KB, registros SIMD de 128 bits y el EIB proviene de la documentacion publica de IBM sobre Cell BE

### Problemas encontrados y soluciones

| Problema | Causa | Solucion |
|---|---|---|
| `ppu-lv2-gcc: Command not found` | El compilador en esta imagen se llama `powerpc64-ps3-elf-gcc`, no `ppu-lv2-gcc` | Usar el nombre correcto o confiar en `ppu_rules` que define `PREFIX := ppu-` |
| `the input device is not a TTY` | Docker en Windows con Git Bash no soporta `-it` | Usar `docker run --rm` sin `-it` |
| Git Bash convierte `/src` a `C:/Program Files/Git/src` | MSYS path conversion | Prefijar con `MSYS_NO_PATHCONV=1` |
| `ld: unrecognized option '-Wl,-N'` | `base_rules` usa `$(LD)` que por defecto es `ppu-gcc`, pero en Windows se filtra `ld` del host | Forzar `LD := $(CC)` despues del include |
| `File in wrong format` al linkear | Object files `.o` de una compilacion anterior con toolchain incorrecto | `make clean` antes de recompilar |
| `sys_fs_fstat failed with CELL_EBADF [1]` | stdout (fd 1) no inicializado porque faltaba `-lrt` | Agregar `-lrt` a LIBS (runtime que inicializa stdio sobre lv2 TTY) |
| `cannot find -lpsl1ght` | No existe esa libreria en esta version del SDK | Eliminar de LIBS, usar solo `-lrt -llv2` (o las librerias especificas necesarias) |
| `psl1ght/lv2.h: No such file` | Header inexistente en este SDK; los headers estan en `/usr/local/ps3dev/ppu/include/` directamente | Usar `<lv2/process.h>` u otros headers correctos del SDK |
