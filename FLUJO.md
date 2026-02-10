# 🔄 Flujo de Ejecución del Programa

Este documento muestra visualmente cómo fluye la ejecución del programa "Hola Mundo PS3".

## 📊 Diagrama de Flujo General

```
┌─────────────────────────────────────────────────────────────┐
│                        INICIO                                │
│                      main()                                  │
└───────────────────────┬─────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────────────┐
│          INICIALIZACIÓN DE GRÁFICOS                          │
│               initScreen()                                   │
│                                                              │
│  1. Asignar host buffer (128 MB)                            │
│  2. Inicializar contexto RSX                                │
│  3. Obtener resolución (1280x720 o 1920x1080)               │
│  4. Crear 2 framebuffers en VRAM                            │
│  5. Configurar VSYNC                                         │
└───────────────────────┬─────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────────────┐
│          INICIALIZACIÓN DE ENTRADA                           │
│             ioPadInit(7)                                     │
│                                                              │
│  Habilitar lectura de hasta 7 controladores                 │
└───────────────────────┬─────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────────────┐
│            LIMPIAR FRAMEBUFFERS                              │
│      memset(buffer[0/1], 0, size)                           │
│                                                              │
│  Llenar ambos buffers con negro (0x00000000)                │
└───────────────────────┬─────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────────────┐
│           DIBUJAR MENSAJE VISUAL                             │
│             drawMessage()                                    │
│                                                              │
│  1. Calcular posición central                               │
│  2. Dibujar rectángulo azul (fondo)                         │
│  3. Dibujar borde blanco                                    │
│  4. Dibujar barra verde (decorativa)                        │
│  5. Dibujar bloques de colores (P, S, 3)                    │
│  6. Dibujar línea inferior                                  │
└───────────────────────┬─────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────────────┐
│             PRESENTAR EN PANTALLA                            │
│                 flip()                                       │
│                                                              │
│  1. gcmSetFlip(buffer_actual)                               │
│  2. rsxFlushBuffer() - enviar comandos                      │
│  3. gcmSetWaitFlip() - esperar vsync                        │
│  4. Cambiar buffer (0 ↔ 1)                                  │
└───────────────────────┬─────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────────────┐
│              LOOP PRINCIPAL (while(1))                       │
└───────────────────────┬─────────────────────────────────────┘
                        │
                        ▼
            ┌───────────────────────┐
            │   ioPadGetInfo()      │
            │  ¿Controles activos?  │
            └───────┬───────────────┘
                    │
        ┌───────────┴───────────┐
        │                       │
        ▼                       ▼
   SÍ conectado            NO conectado
        │                       │
        ▼                       │
┌──────────────────┐            │
│ ioPadGetData()   │            │
│ Leer botones     │            │
└────────┬─────────┘            │
         │                      │
    ┌────▼────┐                 │
    │ ¿BTN_X? │                 │
    └────┬────┘                 │
         │                      │
   ┌─────┴─────┐                │
   │           │                │
  SÍ          NO                │
   │           │                │
   ▼           │                │
┌────────┐     │                │
│ Salir  │     │                │
│ return │     │                │
└────────┘     │                │
               │                │
               └────────────────┘
                        │
                        ▼
                ┌──────────────┐
                │ usleep(10ms) │
                └──────┬───────┘
                       │
                       └───────► VOLVER al inicio del loop
```

## 🎨 Flujo de Renderizado (drawMessage)

```
drawMessage()
     │
     ▼
┌─────────────────────────────────────────────┐
│ 1. CALCULAR POSICIONES                      │
│    centerX = display_width / 2              │
│    centerY = display_height / 2             │
│    msgWidth = 500, msgHeight = 150          │
└─────────────┬───────────────────────────────┘
              │
              ▼
┌─────────────────────────────────────────────┐
│ 2. FONDO PRINCIPAL (Azul)                   │
│    fillRect(x, y, 500, 150, COLOR_BLUE)    │
│                                             │
│    ┌───────────────────────────────┐       │
│    │                               │       │
│    │         AZUL RECT             │       │
│    │                               │       │
│    └───────────────────────────────┘       │
└─────────────┬───────────────────────────────┘
              │
              ▼
┌─────────────────────────────────────────────┐
│ 3. BORDE BLANCO                             │
│    drawBorder(x, y, 500, 150, 6, WHITE)    │
│                                             │
│    ┌───────────────────────────────┐       │
│    │█████████████████████████████████       │
│    │█                             █       │
│    │█         AZUL                █       │
│    │█                             █       │
│    │█████████████████████████████████       │
│    └───────────────────────────────┘       │
└─────────────┬───────────────────────────────┘
              │
              ▼
┌─────────────────────────────────────────────┐
│ 4. BARRA DECORATIVA (Verde)                 │
│    fillRect(..., 30 alto, COLOR_GREEN)     │
│                                             │
│    ┌───────────────────────────────┐       │
│    │█████████████████████████████████       │
│    │█▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓█       │
│    │█         AZUL                █       │
│    │█                             █       │
│    │█████████████████████████████████       │
│    └───────────────────────────────┘       │
└─────────────┬───────────────────────────────┘
              │
              ▼
┌─────────────────────────────────────────────┐
│ 5. BLOQUES DE TEXTO "PS3"                   │
│    fillRect(..., YELLOW) # P                │
│    fillRect(..., RED)    # S                │
│    fillRect(..., WHITE)  # 3                │
│                                             │
│    ┌───────────────────────────────┐       │
│    │█████████████████████████████████       │
│    │█▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓█       │
│    │█                             █       │
│    │█      [Y]  [R]  [W]          █       │
│    │█                             █       │
│    │█████████████████████████████████       │
│    └───────────────────────────────┘       │
│         ↑     ↑     ↑                      │
│      Amarillo Rojo Blanco                  │
└─────────────┬───────────────────────────────┘
              │
              ▼
┌─────────────────────────────────────────────┐
│ 6. LÍNEA DECORATIVA INFERIOR                │
│    fillRect(..., 5 alto, WHITE)            │
│                                             │
│    ┌───────────────────────────────┐       │
│    │█████████████████████████████████       │
│    │█▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓█       │
│    │█                             █       │
│    │█      [Y]  [R]  [W]          █       │
│    │█      ───────────            █       │
│    │█████████████████████████████████       │
│    └───────────────────────────────┘       │
└─────────────┬───────────────────────────────┘
              │
              ▼
        RENDER COMPLETO
```

## 🔄 Ciclo de Double Buffering

```
Frame N                           Frame N+1
──────────────────────────────   ──────────────────────────────

BUFFER 0                         BUFFER 0
┌──────────────┐                 ┌──────────────┐
│              │                 │              │
│  MOSTRANDO   │  ──────►        │  DIBUJANDO   │
│   en TV      │   flip()        │  (invisible) │
│              │                 │              │
└──────────────┘                 └──────────────┘
                                         │
BUFFER 1                                 │
┌──────────────┐                         │
│              │                         │
│  DIBUJANDO   │  ──────►                ▼
│  (invisible) │   flip()         BUFFER 1
│              │                 ┌──────────────┐
└──────────────┘                 │              │
                                 │  MOSTRANDO   │
       curr_fb = 0               │   en TV      │
                                 │              │
                                 └──────────────┘
                                 
                                     curr_fb = 1


═══════════════════════════════════════════════════════════════

VENTAJAS:
• No se ve el proceso de dibujado (sin flickering)
• Sincronización con VSYNC (60 Hz)
• No hay screen tearing (imagen partida)
```

## 🎮 Flujo de Entrada (Input)

```
┌─────────────────────────────────────────────────────────────┐
│                  ioPadGetInfo(&padinfo)                      │
│           Actualizar info de controladores                   │
└───────────────────────┬─────────────────────────────────────┘
                        │
                        ▼
                for i = 0 to MAX_PADS
                        │
                        ▼
            ┌───────────────────────┐
            │ ¿padinfo.status[i]?   │
            │  (¿está conectado?)   │
            └───────┬───────────────┘
                    │
        ┌───────────┴───────────┐
        │                       │
        ▼                       ▼
   Conectado               No conectado
        │                       │
        ▼                       └──► Siguiente iteración
┌──────────────────────┐
│ ioPadGetData(i, &d)  │
│  Leer estado actual  │
└────────┬─────────────┘
         │
         ▼
    ┌────────────────────┐
    │   paddata tiene:   │
    │                    │
    │ • BTN_CROSS        │
    │ • BTN_CIRCLE       │
    │ • BTN_SQUARE       │
    │ • BTN_TRIANGLE     │
    │ • BTN_L1/R1        │
    │ • BTN_L2/R2        │
    │ • BTN_START        │
    │ • BTN_SELECT       │
    │ • BTN_UP/DOWN      │
    │ • BTN_LEFT/RIGHT   │
    │ • ANA_L_H/V        │
    │ • ANA_R_H/V        │
    └────────┬───────────┘
             │
             ▼
    ┌────────────────┐
    │ Procesar input │
    │ (if statements)│
    └────────────────┘
```

## 🖼️ Flujo de Píxel a Pantalla

```
setPixel(x, y, color)
         │
         ▼
┌─────────────────────────────────────────────┐
│ 1. Verificar límites                        │
│    if (x < display_width &&                 │
│        y < display_height)                  │
└─────────────┬───────────────────────────────┘
              │
              ▼ (dentro de límites)
┌─────────────────────────────────────────────┐
│ 2. Calcular índice en framebuffer           │
│    índice = y * (color_pitch/4) + x         │
│                                             │
│    Ejemplo con 1280x720:                    │
│    píxel(100, 50)                           │
│    = 50 * 1280 + 100                        │
│    = 64,100                                 │
└─────────────┬───────────────────────────────┘
              │
              ▼
┌─────────────────────────────────────────────┐
│ 3. Escribir color en memoria                │
│    color_buffer[curr_fb][índice] = color    │
│                                             │
│    Formato: 0xAARRGGBB                      │
│    Ejemplo: 0xFF0066FF (azul)               │
└─────────────┬───────────────────────────────┘
              │
              ▼
┌─────────────────────────────────────────────┐
│ VRAM (Framebuffer)                          │
│                                             │
│ [pixel 0][pixel 1]...[pixel 64,100]...     │
│   negro    negro    AZUL (0xFF0066FF)      │
└─────────────┬───────────────────────────────┘
              │
              ▼ (después de flip())
┌─────────────────────────────────────────────┐
│         RSX lee VRAM                        │
│         Envía a TV/Monitor                  │
│         Píxel visible en pantalla           │
└─────────────────────────────────────────────┘
```

## 💾 Mapa de Memoria

```
╔═══════════════════════════════════════════════════════════╗
║                    MAIN RAM (XDR)                         ║
║                      256 MB                               ║
╠═══════════════════════════════════════════════════════════╣
║                                                           ║
║  ┌─────────────────────────────────────────┐             ║
║  │ Host Buffer (comandos RSX)              │             ║
║  │ 128 MB - memalign(1MB, 128MB)           │             ║
║  │                                         │             ║
║  │ Comandos GCM en cola:                   │             ║
║  │ • gcmSetFlip                            │             ║
║  │ • gcmSetDisplayBuffer                   │             ║
║  │ • etc.                                  │             ║
║  └─────────────────────────────────────────┘             ║
║                                                           ║
║  ┌─────────────────────────────────────────┐             ║
║  │ Stack del programa                      │             ║
║  │ 1 MB (SYS_PROCESS_PARAM)                │             ║
║  └─────────────────────────────────────────┘             ║
║                                                           ║
║  Variables locales, heap, etc.                           ║
║                                                           ║
╚═══════════════════════════════════════════════════════════╝

╔═══════════════════════════════════════════════════════════╗
║                    VRAM (GDDR3)                           ║
║                      256 MB                               ║
╠═══════════════════════════════════════════════════════════╣
║                                                           ║
║  ┌─────────────────────────────────────────┐             ║
║  │ Framebuffer 0                           │             ║
║  │ ~3.5 MB (1280x720x4 bytes)              │             ║
║  │ rsxMemalign(64, size)                   │             ║
║  │                                         │             ║
║  │ [píxel 0][píxel 1]...[píxel 921,599]    │             ║
║  │ 0xFF000000 (negro) por defecto          │             ║
║  └─────────────────────────────────────────┘             ║
║                                                           ║
║  ┌─────────────────────────────────────────┐             ║
║  │ Framebuffer 1                           │             ║
║  │ ~3.5 MB (1280x720x4 bytes)              │             ║
║  │ rsxMemalign(64, size)                   │             ║
║  │                                         │             ║
║  │ [píxel 0][píxel 1]...[píxel 921,599]    │             ║
║  │ 0xFF000000 (negro) por defecto          │             ║
║  └─────────────────────────────────────────┘             ║
║                                                           ║
║  Espacio libre: ~249 MB                                  ║
║  (para texturas, shaders, etc.)                          ║
║                                                           ║
╚═══════════════════════════════════════════════════════════╝
```

## ⚡ Timeline de Ejecución

```
Tiempo  │ Evento
────────┼─────────────────────────────────────────────────────
0.00s   │ ▶ Inicio del programa (main)
        │
0.01s   │ ● Asignar host buffer (128 MB)
        │ ● rsxInit() - inicializar RSX
        │
0.02s   │ ● videoGetState() - obtener configuración
        │ ● videoGetResolution() - 1280x720
        │
0.03s   │ ● Crear framebuffer 0 en VRAM
        │ ● Crear framebuffer 1 en VRAM
        │ ● gcmSetFlipMode(VSYNC)
        │
0.04s   │ ● ioPadInit(7) - inicializar controles
        │
0.05s   │ ● memset(buffer 0, 0) - limpiar
        │ ● memset(buffer 1, 0) - limpiar
        │
0.06s   │ ● drawMessage() - dibujar todo
        │   ├─ fillRect (fondo azul)
        │   ├─ drawBorder (borde blanco)
        │   ├─ fillRect (barra verde)
        │   ├─ fillRect x3 (bloques P,S,3)
        │   └─ fillRect (línea inferior)
        │
0.07s   │ ● flip() - mostrar en pantalla
        │   ├─ gcmSetFlip()
        │   ├─ rsxFlushBuffer()
        │   └─ gcmSetWaitFlip() [espera VSYNC]
        │
0.09s   │ ◆ IMAGEN VISIBLE EN TV
        │
0.10s   │ ▶ Entrada al LOOP PRINCIPAL
        │ 
~~~~~~~   ┊ ┌──────────────────────────────┐
Cada      ┊ │ while(1) - Loop infinito     │
10ms      ┊ │                              │
~~~~~~~   ┊ │ • ioPadGetInfo()             │
        ┊ │ • ioPadGetData()             │
        ┊ │ • Verificar BTN_CROSS        │
        ┊ │ • usleep(10000)              │
        ┊ │                              │
        ┊ └───────────┬──────────────────┘
        ┊            │
        ┊            └──► Repetir hasta presionar X
        │
Presiona X
        │
Final   │ ● Limpiar buffer actual
        │ ● flip()
        │ ● return 0
        │
        │ ▣ FIN DEL PROGRAMA
```

---

## 🎓 Notas Finales

Este flujo de ejecución muestra:

1. **Inicialización** - Setup de hardware y memoria
2. **Renderizado** - Dibujo de gráficos en framebuffer
3. **Presentación** - Mostrar en pantalla con flip
4. **Loop interactivo** - Esperar entrada del usuario

El programa usa patrones comunes en desarrollo de videojuegos:
- **Double buffering** para fluidez visual
- **VSYNC** para sincronización
- **Input polling** en el loop principal
- **Separación de lógica y renderizado**

Para entender cada función en detalle, consulta [EXPLICACION.md](EXPLICACION.md).
