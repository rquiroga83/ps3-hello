# 📚 Explicación Técnica del Código

Este documento explica en detalle cómo funciona el ejemplo "Hola Mundo PS3" y los conceptos fundamentales del desarrollo homebrew para PS3.

## 📖 Índice

1. [Arquitectura de PS3](#arquitectura-de-ps3)
2. [Sistema Gráfico RSX](#sistema-gráfico-rsx)
3. [Estructura del Código](#estructura-del-código)
4. [Paso a Paso del Programa](#paso-a-paso-del-programa)
5. [Conceptos Avanzados](#conceptos-avanzados)
6. [Referencias y Recursos](#referencias-y-recursos)

---

## 🏗️ Arquitectura de PS3

### Cell Broadband Engine (Cell BE)

La PlayStation 3 usa el procesador **Cell Broadband Engine**, que tiene:

- **1x PPU (Power Processing Unit)**: Procesador principal PowerPC de 64 bits
- **8x SPU (Synergistic Processing Unit)**: Co-procesadores especializados para procesamiento paralelo
- **RSX (Reality Synthesizer)**: GPU basada en arquitectura NVIDIA G70/G71

**Nuestro programa corre en el PPU** y envía comandos a la GPU RSX para renderizar gráficos.

### Memory Layout

```
┌─────────────────────────────────────┐
│  Main RAM (XDR): 256 MB             │ ← System y aplicaciones
├─────────────────────────────────────┤
│  VRAM (GDDR3): 256 MB               │ ← Framebuffers y texturas
└─────────────────────────────────────┘
```

---

## 🎨 Sistema Gráfico RSX

### ¿Qué es RSX?

El **RSX** (Reality Synthesizer) es el chip gráfico de PS3, desarrollado por NVIDIA y Sony. Es similar a la GeForce 7800 GTX pero personalizado para PS3.

**Características:**
- 550 MHz de frecuencia
- 256 MB de VRAM dedicada (GDDR3)
- Soporte para shaders programables
- Fixed-function pipeline para compatibilidad

### Framebuffers y Double Buffering

```c
#define FRAME_BUFFER_COUNT 2  // Usamos 2 buffers
```

**¿Por qué 2 buffers?**

```
     BUFFER 0             BUFFER 1
┌──────────────┐      ┌──────────────┐
│  Dibujando   │      │  Mostrando   │
│  frame N+1   │      │  frame N     │
└──────────────┘      └──────────────┘
        ↓                     ↑
     SWAP (flip)
        ↓                     ↑
┌──────────────┐      ┌──────────────┐
│  Mostrando   │      │  Dibujando   │
│  frame N+1   │      │  frame N+2   │
└──────────────┘      └──────────────┘
```

**Ventajas:**
- Evita **screen tearing** (rasgado de imagen)
- La GPU puede mostrar un frame mientras dibujamos el siguiente
- Sincronización con VSYNC (actualización vertical del monitor)

### Formato de Color: ARGB

Cada píxel ocupa **4 bytes** (32 bits):

```
 Byte 3    Byte 2    Byte 1    Byte 0
┌────────┬────────┬────────┬────────┐
│  Alpha │   Red  │  Green │  Blue  │
│ (0xFF) │ (0x00) │ (0x66) │ (0xFF) │
└────────┴────────┴────────┴────────┘
    ↓        ↓        ↓        ↓
  100%    Negro   ~40%    Azul
Opaco            Verde    100%

Resultado: 0xFF0066FF = Azul morado brillante
```

**Colores usados en el ejemplo:**

```c
#define COLOR_BLACK   0xFF000000  // Negro opaco
#define COLOR_WHITE   0xFFFFFFFF  // Blanco opaco
#define COLOR_BLUE    0xFF0066FF  // Azul morado
#define COLOR_GREEN   0xFF00FF00  // Verde puro
#define COLOR_RED     0xFFFF0000  // Rojo puro
#define COLOR_YELLOW  0xFFFFFF00  // Amarillo
```

---

## 🗂️ Estructura del Código

### Includes Necesarios

```c
#include <stdio.h>          // printf, etc.
#include <malloc.h>         // memalign para memoria alineada
#include <string.h>         // memset
#include <unistd.h>         // usleep
#include <sysutil/video.h>  // Funciones de video
#include <rsx/gcm_sys.h>    // GCM (Graphics Command Management)
#include <rsx/rsx.h>        // API de RSX
#include <sys/process.h>    // Parámetros de proceso
#include <io/pad.h>         // Input de controladores
```

### Macros y Constantes

```c
// Parámetros del proceso PS3
SYS_PROCESS_PARAM(1001, 0x100000);
//               ^       ^
//               |       └─ Stack size: 1 MB
//               └───────── Priority: 1001 (normal)

// Configuración de buffers RSX
#define HOSTBUFFER_SIZE (128*1024*1024)  // 128 MB
#define CB_SIZE         0x100000          // 1 MB
#define FRAME_BUFFER_COUNT 2              // Double buffer
```

### Variables Globales

```c
static gcmContextData *context;              // Contexto RSX
static u32 *color_buffer[FRAME_BUFFER_COUNT]; // Punteros a framebuffers
static u32 color_offset[FRAME_BUFFER_COUNT];  // Offsets en VRAM
static u32 display_width;                     // Ancho (ej: 1920)
static u32 display_height;                    // Alto (ej: 1080)
static u32 color_pitch;                       // Bytes por línea
static u32 curr_fb = 0;                       // Buffer actual (0 o 1)
```

---

## 🔄 Paso a Paso del Programa

### 1. Inicialización de Gráficos (`initScreen`)

```c
static void initScreen()
{
    // 1. Asignar memoria para comandos RSX (host buffer)
    void *host_addr = memalign(1024*1024, HOSTBUFFER_SIZE);
    //                        ^              ^
    //                        |              └─ Tamaño: 128 MB
    //                        └──────────────── Alineación: 1 MB
```

**¿Por qué alineación de 1 MB?**
- RSX requiere que la memoria esté alineada para acceso DMA eficiente
- Evita penalizaciones de rendimiento

```c
    // 2. Inicializar contexto RSX
    rsxInit(&context, CB_SIZE, HOSTBUFFER_SIZE, host_addr);
```

Este comando:
- Crea el contexto de comunicación con RSX
- Configura el command buffer (cola de comandos para la GPU)
- Mapea la memoria host para que RSX pueda accederla

```c
    // 3. Obtener resolución de video configurada
    videoState state;
    videoGetState(0, 0, &state);
    
    videoResolution res;
    videoGetResolution(state.displayMode.resolution, &res);
    
    display_width = res.width;    // Ejemplo: 1280 o 1920
    display_height = res.height;  // Ejemplo: 720 o 1080
```

**Resoluciones comunes:**
- **720p:** 1280 x 720
- **1080p:** 1920 x 1080
- **480p:** 720 x 480

```c
    // 4. Calcular pitch (bytes por línea)
    color_pitch = display_width * 4;  // 4 bytes = ARGB
```

**Ejemplo con 1280x720:**
```
pitch = 1280 * 4 = 5120 bytes por línea
```

```c
    // 5. Crear framebuffers en VRAM
    for(u32 i = 0; i < FRAME_BUFFER_COUNT; i++) {
        // Asignar memoria en VRAM (alineada a 64 bytes)
        color_buffer[i] = (u32*)rsxMemalign(64, display_height * color_pitch);
        
        // Convertir dirección a offset para RSX
        rsxAddressToOffset(color_buffer[i], &color_offset[i]);
        
        // Registrar el buffer en el sistema de display
        gcmSetDisplayBuffer(i, color_offset[i], color_pitch, 
                           display_width, display_height);
    }
```

**Tamaño de cada framebuffer (1280x720):**
```
Tamaño = 720 * 5120 = 3,686,400 bytes ≈ 3.5 MB por buffer
Total con 2 buffers = ~7 MB de VRAM
```

```c
    // 6. Habilitar VSYNC (sincronización vertical)
    gcmSetFlipMode(GCM_FLIP_VSYNC);
}
```

**VSYNC:**
- Sincroniza el flip con el refresco del TV/monitor (60 Hz típico)
- Evita tearing (imagen partida)
- Limita FPS a 60 (o 50 en PAL)

---

### 2. Funciones de Dibujo

#### `setPixel` - Dibuja un píxel

```c
static void setPixel(u32 x, u32 y, u32 color)
{
    if(x < display_width && y < display_height)
        color_buffer[curr_fb][y * (color_pitch/4) + x] = color;
        //                    │                      │
        //                    │                      └─ Columna (X)
        //                    └────────────────────────Fila (Y)
}
```

**Cálculo del índice:**
```
Índice = (y * ancho_en_pixels) + x

¿Por qué color_pitch/4?
  color_pitch está en BYTES
  Queremos índice en PIXELS (u32 = 4 bytes)
  División por 4 convierte bytes → pixels
```

**Ejemplo para píxel en (100, 50) con resolución 1280x720:**
```
Índice = (50 * (5120/4)) + 100
       = (50 * 1280) + 100
       = 64,000 + 100
       = 64,100

Dirección = color_buffer[curr_fb] + (64,100 * 4 bytes)
```

#### `fillRect` - Dibuja un rectángulo relleno

```c
static void fillRect(u32 x, u32 y, u32 w, u32 h, u32 color)
{
    for(u32 i = 0; i < h; i++)        // Para cada fila
        for(u32 j = 0; j < w; j++)    // Para cada columna
            setPixel(x + j, y + i, color);
}
```

**Ejemplo visual:**
```
fillRect(100, 50, 200, 100, COLOR_BLUE);

     x=100      w=200
      ↓          →
y=50→ ┌──────────────┐ 
      │              │
h=100 │   BLUE       │
  ↓   │   RECT       │
      └──────────────┘
```

#### `drawBorder` - Dibuja un borde

```c
static void drawBorder(u32 x, u32 y, u32 w, u32 h, u32 thickness, u32 color)
{
    fillRect(x, y, w, thickness, color);                    // Top
    fillRect(x, y + h - thickness, w, thickness, color);    // Bottom
    fillRect(x, y, thickness, h, color);                    // Left
    fillRect(x + w - thickness, y, thickness, h, color);    // Right
}
```

**Diagrama:**
```
     x          w
     ↓          →
  y→ ┌──────────┐  ← Top (grosor = thickness)
     │          │
     │  HUECO   │
     │          │
     └──────────┘  ← Bottom (grosor = thickness)
     ↑          ↑
   Left       Right
 (grosor)   (grosor)
```

#### `flip` - Intercambia buffers

```c
static void flip()
{
    gcmSetFlip(context, curr_fb);    // Decirle a RSX que muestre este buffer
    rsxFlushBuffer(context);         // Enviar comandos pendientes a RSX
    gcmSetWaitFlip(context);         // Esperar que se complete el flip
    curr_fb ^= 1;                    // Cambiar buffer (0→1 o 1→0)
}
```

**Operador XOR (^) para alternar:**
```c
curr_fb ^= 1;

Si curr_fb = 0:  0 XOR 1 = 1  →  curr_fb = 1
Si curr_fb = 1:  1 XOR 1 = 0  →  curr_fb = 0
```

---

### 3. Función Principal (`main`)

#### Secuencia de inicialización

```c
int main()
{
    padInfo padinfo;  // Info de controladores conectados
    padData paddata;  // Datos del controlador (botones, sticks)
```

**1. Inicializar gráficos:**
```c
    initScreen();
    printf("Resolución: %dx%d\n", display_width, display_height);
```

**2. Inicializar sistema de entrada:**
```c
    ioPadInit(7);  // Soportar hasta 7 controladores
```

**3. Limpiar framebuffers:**
```c
    memset(color_buffer[0], 0, display_height * color_pitch);
    memset(color_buffer[1], 0, display_height * color_pitch);
```

Esto llena ambos buffers con 0x00000000 (negro transparente).

**4. Dibujar mensaje:**
```c
    drawMessage();  // Llama a la función que dibuja el mensaje visual
```

**5. Mostrar en pantalla:**
```c
    flip();  // Intercambiar buffers para que se vea
```

#### Loop principal

```c
    while(1) {  // Loop infinito
        ioPadGetInfo(&padinfo);  // Actualizar info de controles
        
        for(int i = 0; i < MAX_PADS; i++) {  // Revisar cada control
            if(padinfo.status[i]) {           // ¿Está conectado?
                ioPadGetData(i, &paddata);    // Leer botones
                
                if(paddata.BTN_CROSS) {       // ¿Presionó X?
                    // Limpiar y salir
                    memset(color_buffer[curr_fb], 0, display_height * color_pitch);
                    flip();
                    return 0;
                }
            }
        }
        
        usleep(10000);  // Esperar 10ms (10,000 microsegundos)
    }
}
```

**¿Por qué usleep(10000)?**
- Sin pausa, el CPU estaría al 100% haciendo polling
- 10ms = 100 iteraciones/segundo (suficiente para entrada)
- Ahorra batería/energía

---

## 🎓 Conceptos Avanzados

### GCM (Graphics Command Management)

```c
gcmSetFlip(context, buffer_index);
```

**GCM** es la API de bajo nivel para comunicarse con RSX. Los comandos se acumulan en un **command buffer** y se envían en batch a la GPU.

**Flujo:**
```
CPU (PPU)                    GPU (RSX)
   │                            │
   ├─ gcmSetFlip() ────────────►│ [comando en cola]
   ├─ gcmSetXXX()  ────────────►│ [comando en cola]
   ├─ rsxFlushBuffer() ────────►│ ¡EJECUTAR TODO!
   │                            │
   └─ gcmSetWaitFlip() ◄────────┤ [esperar...]
```

### Memory Alignment (Alineación)

```c
memalign(1024*1024, size);  // Alinear a 1 MB
rsxMemalign(64, size);      // Alinear a 64 bytes
```

**¿Por qué es importante?**

Memoria NO alineada (lenta):
```
0x1000: [    ]
0x1001: [DATA]  ← ¡No alineado!
0x1002: [    ]
```

Memoria alineada (rápida):
```
0x1000: [DATA]  ← ¡Alineado a 0x1000!
0x1001: [    ]
```

**Beneficios:**
- Acceso DMA más rápido
- Cachelines completas
- Sin penalización de rendimiento

### RSX Memory vs Main RAM

```c
rsxMemalign()   // Asigna en VRAM (256 MB dedicados)
memalign()      // Asigna en Main RAM (256 MB compartidos)
```

**VRAM (RSX):**
- ✅ Acceso ultra rápido desde GPU
- ✅ Ideal para framebuffers, texturas
- ❌ Solo 256 MB disponibles

**Main RAM (XDR):**
- ✅ 256 MB para sistema y aplicación
- ✅ Accesible desde CPU y SPUs
- ❌ Más lento para GPU (vía bus)

---

## 📚 Referencias y Recursos

### Documentación Oficial

- **PSL1GHT SDK:** https://github.com/ps3dev/PSL1GHT
- **ps3toolchain:** https://github.com/ps3dev/ps3toolchain
- **PS3 Dev Wiki:** https://www.psdevwiki.com/

### Repositorios de Ejemplo

- **hcorion/ps3-homebrew:** https://github.com/hcorion/ps3-homebrew
  - Ejemplos de SSL, USB, networking
  - Tests de syscalls
  - Dumps de certificados

- **ps3dev/ps3-examples:** https://github.com/ps3dev/ps3-examples (si existe)
  - Ejemplos oficiales del SDK

### Comunidades

- **PSX-Place:** https://www.psx-place.com/
- **GBAtemp PS3 Scene:** https://gbatemp.net/forums/sony-ps3.90/
- **Reddit /r/ps3homebrew:** https://reddit.com/r/ps3homebrew

### Herramientas

- **pkg_tools:** Crear paquetes PKG
- **make_self:** Convertir ELF → SELF
- **ps3load:** Cargar homebrew vía red (para desarrollo)

---

## 🚀 Próximos Pasos

Si quieres expandir este ejemplo, podrías:

1. **Agregar fuentes de texto reales**
   - Usar bibliotecas como FreeType
   - Renderizar texto verdadero en lugar de bloques

2. **Implementar sprites y texturas**
   - Cargar imágenes PNG/JPG
   - Usar shaders para efectos

3. **Usar SPUs para procesamiento**
   - Offload cálculos intensivos a los SPUs
   - Física, audio processing

4. **Agregar audio**
   - Reproducir música y efectos de sonido
   - Usar librerías como SDL_mixer o PSL1GHT audio

5. **Networking**
   - Conectar a internet
   - Multiplayer simple

6. **Entrada avanzada**
   - Leer giroscopio/acelerómetro del control
   - Soportar PS Move

---

**¡Feliz desarrollo para PS3!** 🎮✨
