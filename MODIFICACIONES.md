# 🎨 Modificaciones Simples

Aquí encontrarás ejemplos de cómo modificar el programa para experimentar con diferentes efectos visuales.

## 📋 Índice

1. [Cambiar Colores](#cambiar-colores)
2. [Modificar Tamaños y Posiciones](#modificar-tamaños-y-posiciones)
3. [Agregar Más Formas](#agregar-más-formas)
4. [Animaciones Simples](#animaciones-simples)
5. [Cambiar Controles](#cambiar-controles)

---

## 🎨 Cambiar Colores

### Definir Nuevos Colores

En el archivo `source/main.c`, busca la sección de definiciones de colores:

```c
// Colores en formato ARGB
#define COLOR_BLACK   0xFF000000
#define COLOR_WHITE   0xFFFFFFFF
#define COLOR_BLUE    0xFF0066FF
#define COLOR_GREEN   0xFF00FF00
#define COLOR_RED     0xFFFF0000
#define COLOR_YELLOW  0xFFFFFF00
```

**Agrega tus propios colores:**

```c
#define COLOR_ORANGE  0xFFFF8800  // Naranja
#define COLOR_PURPLE  0xFF9900FF  // Morado
#define COLOR_CYAN    0xFF00FFFF  // Cian
#define COLOR_PINK    0xFFFF00FF  // Rosa/Magenta
#define COLOR_BROWN   0xFF964B00  // Café
#define COLOR_GRAY    0xFF808080  // Gris
```

### Formato de Color ARGB

```
0xAARRGGBB
  │ │ │ └─ Azul (Blue): 00-FF
  │ │ └─── Verde (Green): 00-FF
  │ └───── Rojo (Red): 00-FF
  └─────── Alpha (Transparencia): FF=opaco, 00=transparente
```

**Ejemplos:**

| Color              | Código      | Explicación              |
|-------------------|-------------|--------------------------|
| Negro             | 0xFF000000  | Sin R, G, B              |
| Blanco            | 0xFFFFFFFF  | Máximo R, G, B           |
| Rojo puro         | 0xFFFF0000  | Solo R al máximo         |
| Verde puro        | 0xFF00FF00  | Solo G al máximo         |
| Azul puro         | 0xFF0000FF  | Solo B al máximo         |
| Amarillo          | 0xFFFFFF00  | R + G (no azul)          |
| Magenta           | 0xFFFF00FF  | R + B (no verde)         |
| Cian              | 0xFF00FFFF  | G + B (no rojo)          |
| Gris 50%          | 0xFF808080  | R=G=B medio              |
| Rosa pastel       | 0xFFFFB6C1  | Más R, menos G y B       |

### Cambiar Color del Fondo Principal

Busca en `drawMessage()`:

```c
fillRect(centerX - msgWidth/2, centerY - msgHeight/2, 
         msgWidth, msgHeight, COLOR_BLUE);
```

Cámbialo a:

```c
fillRect(centerX - msgWidth/2, centerY - msgHeight/2, 
         msgWidth, msgHeight, COLOR_PURPLE);  // Ahora morado
```

### Cambiar Color de Pantalla Completa

Para cambiar el fondo negro por otro color, busca en `main()`:

```c
memset(color_buffer[0], 0, display_height * color_pitch);
memset(color_buffer[1], 0, display_height * color_pitch);
```

Reemplázalo con:

```c
// Llenar la pantalla con un color específico
for(u32 y = 0; y < display_height; y++) {
    for(u32 x = 0; x < display_width; x++) {
        color_buffer[0][y * (color_pitch/4) + x] = COLOR_CYAN;
        color_buffer[1][y * (color_pitch/4) + x] = COLOR_CYAN;
    }
}
```

O más eficiente usando `fillRect`:

```c
// Limpiar con negro primero
memset(color_buffer[0], 0, display_height * color_pitch);
memset(color_buffer[1], 0, display_height * color_pitch);

// Dibujar fondo de color
fillRect(0, 0, display_width, display_height, COLOR_CYAN);
```

---

## 📐 Modificar Tamaños y Posiciones

### Cambiar Tamaño del Mensaje

En `drawMessage()`, busca:

```c
u32 msgWidth = 500;
u32 msgHeight = 150;
```

Prueba con:

```c
u32 msgWidth = 700;   // Más ancho
u32 msgHeight = 250;  // Más alto
```

### Mover el Mensaje

Para mover el mensaje hacia arriba:

```c
u32 centerX = display_width / 2;
u32 centerY = display_height / 2 - 100;  // 100 píxeles hacia arriba
```

Para mover hacia la esquina superior izquierda:

```c
u32 centerX = 200;  // 200 píxeles desde la izquierda
u32 centerY = 150;  // 150 píxeles desde arriba
```

### Cambiar Tamaño de los Bloques de Texto

```c
u32 blockSize = 40;   // Tamaño original
u32 spacing = 50;
```

Hazlos más grandes:

```c
u32 blockSize = 60;   // Bloques más grandes
u32 spacing = 80;     // Más espacio entre ellos
```

---

## 🎯 Agregar Más Formas

### Agregar un Círculo (Aproximado)

Agrega esta función después de `drawBorder()`:

```c
/*
 * drawCircle - Dibuja un círculo aproximado usando píxeles
 */
static void drawCircle(u32 centerX, u32 centerY, u32 radius, u32 color)
{
    for(u32 y = 0; y < radius * 2; y++) {
        for(u32 x = 0; x < radius * 2; x++) {
            // Calcular distancia desde el centro
            int dx = x - radius;
            int dy = y - radius;
            int distSq = dx * dx + dy * dy;
            int radiusSq = radius * radius;
            
            // Si está dentro del círculo
            if(distSq <= radiusSq) {
                setPixel(centerX - radius + x, centerY - radius + y, color);
            }
        }
    }
}
```

Úsalo en `drawMessage()`:

```c
// Agregar un círculo rojo en el centro
drawCircle(centerX, centerY + 80, 30, COLOR_RED);
```

### Agregar una Línea Diagonal

```c
/*
 * drawLine - Dibuja una línea recta (implementación simple)
 */
static void drawLine(u32 x1, u32 y1, u32 x2, u32 y2, u32 color)
{
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = (abs(dx) > abs(dy)) ? abs(dx) : abs(dy);
    
    float xInc = dx / (float)steps;
    float yInc = dy / (float)steps;
    
    float x = x1;
    float y = y1;
    
    for(int i = 0; i <= steps; i++) {
        setPixel((u32)x, (u32)y, color);
        x += xInc;
        y += yInc;
    }
}
```

Úsala:

```c
// Línea diagonal verde
drawLine(100, 100, 500, 400, COLOR_GREEN);
```

### Agregar un Gradiente

```c
/*
 * drawGradient - Dibuja un gradiente vertical
 */
static void drawGradient(u32 x, u32 y, u32 w, u32 h, u32 colorTop, u32 colorBottom)
{
    for(u32 row = 0; row < h; row++) {
        // Interpolar entre colores
        float t = row / (float)h;
        
        u8 r = ((colorTop >> 16) & 0xFF) * (1 - t) + ((colorBottom >> 16) & 0xFF) * t;
        u8 g = ((colorTop >> 8) & 0xFF) * (1 - t) + ((colorBottom >> 8) & 0xFF) * t;
        u8 b = (colorTop & 0xFF) * (1 - t) + (colorBottom & 0xFF) * t;
        
        u32 color = 0xFF000000 | (r << 16) | (g << 8) | b;
        
        fillRect(x, y + row, w, 1, color);
    }
}
```

Úsalo:

```c
// Gradiente de azul a rojo
drawGradient(centerX - 250, centerY - 75, 500, 150, COLOR_BLUE, COLOR_RED);
```

---

## 🎬 Animaciones Simples

### Hacer que el Mensaje Parpadee

En `main()`, después de `drawMessage()` y antes del loop:

```c
int frameCount = 0;
bool messageVisible = true;
```

En el loop principal, antes de `usleep`:

```c
frameCount++;

// Parpadear cada 30 frames (aprox. 0.5 segundos a 60 FPS)
if(frameCount % 30 == 0) {
    messageVisible = !messageVisible;
    
    // Limpiar buffer actual
    memset(color_buffer[curr_fb], 0, display_height * color_pitch);
    
    // Dibujar solo si está visible
    if(messageVisible) {
        drawMessage();
    }
    
    flip();
}
```

### Mover el Mensaje Horizontalmente

Modifica `drawMessage()` para aceptar una posición X:

```c
static void drawMessage(u32 posX)
{
    u32 centerX = posX;  // Usar posición pasada
    u32 centerY = display_height / 2;
    
    // ... resto del código igual
}
```

En `main()`:

```c
u32 messageX = 100;
int direction = 1;  // 1 = derecha, -1 = izquierda

while(1) {
    // Mover mensaje
    messageX += direction * 5;
    
    // Rebotar en los bordes
    if(messageX > display_width - 250) direction = -1;
    if(messageX < 250) direction = 1;
    
    // Redibujar
    memset(color_buffer[curr_fb], 0, display_height * color_pitch);
    drawMessage(messageX);
    flip();
    
    // ... resto del código de entrada
    
    usleep(16667);  // ~60 FPS
}
```

### Rotar Colores

```c
u32 colors[] = {COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_YELLOW, COLOR_PURPLE};
int colorIndex = 0;
int frameCount = 0;

while(1) {
    frameCount++;
    
    // Cambiar color cada 60 frames (1 segundo)
    if(frameCount % 60 == 0) {
        colorIndex = (colorIndex + 1) % 5;  // Ciclar entre los 5 colores
        
        memset(color_buffer[curr_fb], 0, display_height * color_pitch);
        
        // Dibujar con el color actual
        fillRect(centerX - 250, centerY - 75, 500, 150, colors[colorIndex]);
        
        flip();
    }
    
    // ... resto del código
}
```

---

## 🎮 Cambiar Controles

### Usar Otros Botones

En lugar de `BTN_CROSS`, prueba:

```c
if(paddata.BTN_CIRCLE) {   // Botón O (Círculo)
    printf("Presionaste Círculo!\n");
}

if(paddata.BTN_SQUARE) {   // Botón □ (Cuadrado)
    printf("Presionaste Cuadrado!\n");
}

if(paddata.BTN_TRIANGLE) { // Botón △ (Triángulo)
    printf("Presionaste Triángulo!\n");
}
```

### Detectar Dirección del D-Pad

```c
if(paddata.BTN_LEFT) {
    printf("Izquierda presionado\n");
    // Mover algo a la izquierda
}

if(paddata.BTN_RIGHT) {
    printf("Derecha presionado\n");
}

if(paddata.BTN_UP) {
    printf("Arriba presionado\n");
}

if(paddata.BTN_DOWN) {
    printf("Abajo presionado\n");
}
```

### Leer Sticks Analógicos

```c
// Stick izquierdo
int leftX = paddata.ANA_L_H;  // 0-255 (128 = centro)
int leftY = paddata.ANA_L_V;

// Stick derecho
int rightX = paddata.ANA_R_H;
int rightY = paddata.ANA_R_V;

printf("Stick izquierdo: X=%d, Y=%d\n", leftX, leftY);
```

**Ejemplo: Mover objeto con stick:**

```c
// Normalizar valores (-128 a 127)
int dx = leftX - 128;
int dy = leftY - 128;

// Aplicar deadzone (ignorar movimientos pequeños)
if(abs(dx) < 20) dx = 0;
if(abs(dy) < 20) dy = 0;

// Mover objeto
objectX += dx / 10;
objectY += dy / 10;
```

### Detectar L1, R1, L2, R2

```c
if(paddata.BTN_L1) {
    printf("L1 presionado\n");
}

if(paddata.BTN_R1) {
    printf("R1 presionado\n");
}

if(paddata.BTN_L2) {
    printf("L2 presionado\n");
}

if(paddata.BTN_R2) {
    printf("R2 presionado\n");
}
```

### Botones START y SELECT

```c
if(paddata.BTN_START) {
    printf("START presionado - Pausar juego\n");
}

if(paddata.BTN_SELECT) {
    printf("SELECT presionado - Abrir menú\n");
}
```

---

## 🔧 Ejemplo Completo: Programa Interactivo

Aquí hay un ejemplo que combina varios conceptos:

```c
int main()
{
    padInfo padinfo;
    padData paddata;
    
    initScreen();
    ioPadInit(7);
    
    // Variables de estado
    u32 boxX = display_width / 2;
    u32 boxY = display_height / 2;
    u32 boxSize = 50;
    u32 boxColor = COLOR_RED;
    
    printf("Usa el D-Pad para mover el cuadrado\n");
    printf("Usa X, O, [], △ para cambiar color\n");
    printf("Presiona START para salir\n");
    
    while(1) {
        ioPadGetInfo(&padinfo);
        
        for(int i = 0; i < MAX_PADS; i++) {
            if(padinfo.status[i]) {
                ioPadGetData(i, &paddata);
                
                // Controles de movimiento
                if(paddata.BTN_LEFT && boxX > boxSize) boxX -= 5;
                if(paddata.BTN_RIGHT && boxX < display_width - boxSize) boxX += 5;
                if(paddata.BTN_UP && boxY > boxSize) boxY -= 5;
                if(paddata.BTN_DOWN && boxY < display_height - boxSize) boxY += 5;
                
                // Cambiar colores
                if(paddata.BTN_CROSS) boxColor = COLOR_RED;
                if(paddata.BTN_CIRCLE) boxColor = COLOR_BLUE;
                if(paddata.BTN_SQUARE) boxColor = COLOR_GREEN;
                if(paddata.BTN_TRIANGLE) boxColor = COLOR_YELLOW;
                
                // Salir
                if(paddata.BTN_START) {
                    printf("Saliendo...\n");
                    return 0;
                }
            }
        }
        
        // Redibujar pantalla
        memset(color_buffer[curr_fb], 0, display_height * color_pitch);
        
        // Dibujar el cuadrado
        fillRect(boxX - boxSize/2, boxY - boxSize/2, boxSize, boxSize, boxColor);
        
        flip();
        usleep(16667);  // ~60 FPS
    }
    
    return 0;
}
```

---

## 🎯 Desafíos para Practicar

1. **Dibuja un tablero de ajedrez** (8x8 cuadrados alternados)
2. **Crea una barra de progreso** que se llene con el tiempo
3. **Implementa un efecto de arcoíris** (gradiente con todos los colores)
4. **Dibuja tu nombre** usando bloques de colores
5. **Crea un cursor** que se mueva con el stick analógico
6. **Implementa detección de colisión** (dos cuadrados que no se solapen)
7. **Anima un rebote** (cuadrado que rebota en los bordes)

---

## 📝 Notas Finales

- **Recuerda compilar** después de cada cambio: `make clean && make`
- **Revisa errores** en la salida del compilador
- **Prueba en la PS3** para ver los resultados reales
- **Experimenta** con diferentes valores para aprender

¡Diviértete modificando el código! 🎮✨
