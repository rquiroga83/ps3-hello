/*
 * ============================================================================
 * HOLA MUNDO PS3 - Ejemplo Mínimo de Homebrew
 * ============================================================================
 * Este programa demuestra lo básico para crear un homebrew para PS3:
 * - Inicialización de gráficos RSX (Reality Synthesizer)
 * - Dibujo de primitivas (rectángulos)
 * - Manejo de entrada del controlador (pad)
 * - Loop principal de aplicación
 * 
 * Compilado con PSL1GHT SDK para PS3
 * ============================================================================
 */

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <sysutil/video.h>
#include <rsx/gcm_sys.h>
#include <rsx/rsx.h>
#include <sys/process.h>
#include <io/pad.h>

// Parámetros del proceso (prioridad y tamaño de stack)
SYS_PROCESS_PARAM(1001, 0x100000);

// Configuración de buffers RSX
#define HOSTBUFFER_SIZE (128*1024*1024)  // 128 MB para comandos RSX
#define CB_SIZE         0x100000          // Command buffer size
#define FRAME_BUFFER_COUNT 2              // Double buffering

// Variables globales para gestión de gráficos
static gcmContextData *context = NULL;
static u32 *color_buffer[FRAME_BUFFER_COUNT];
static u32 color_offset[FRAME_BUFFER_COUNT];
static u32 display_width;
static u32 display_height;
static u32 color_pitch;
static u32 curr_fb = 0;

// Colores en formato ARGB
#define COLOR_BLACK   0xFF000000
#define COLOR_WHITE   0xFFFFFFFF
#define COLOR_BLUE    0xFF0066FF
#define COLOR_GREEN   0xFF00FF00
#define COLOR_RED     0xFFFF0000
#define COLOR_YELLOW  0xFFFFFF00

// ============================================================================
// FUNCIONES GRÁFICAS
// ============================================================================

/*
 * setPixel - Dibuja un píxel individual en el framebuffer
 * @x: coordenada X
 * @y: coordenada Y
 * @color: color en formato ARGB (0xAARRGGBB)
 */
static void setPixel(u32 x, u32 y, u32 color)
{
	if(x < display_width && y < display_height)
		color_buffer[curr_fb][y * (color_pitch/4) + x] = color;
}

/*
 * fillRect - Dibuja un rectángulo relleno
 * @x, @y: coordenadas de la esquina superior izquierda
 * @w, @h: ancho y alto del rectángulo
 * @color: color de relleno
 */
static void fillRect(u32 x, u32 y, u32 w, u32 h, u32 color)
{
	for(u32 i = 0; i < h; i++)
		for(u32 j = 0; j < w; j++)
			setPixel(x + j, y + i, color);
}

/*
 * drawBorder - Dibuja un borde rectangular
 */
static void drawBorder(u32 x, u32 y, u32 w, u32 h, u32 thickness, u32 color)
{
	fillRect(x, y, w, thickness, color);                    // Top
	fillRect(x, y + h - thickness, w, thickness, color);    // Bottom
	fillRect(x, y, thickness, h, color);                    // Left
	fillRect(x + w - thickness, y, thickness, h, color);    // Right
}

/*
 * flip - Intercambia buffers (presenta el frame dibujado en pantalla)
 */
static void flip()
{
	gcmSetFlip(context, curr_fb);
	rsxFlushBuffer(context);
	gcmSetWaitFlip(context);
	curr_fb ^= 1;  // Alterna entre buffer 0 y 1
}

// ============================================================================
// INICIALIZACIÓN DE SISTEMA GRÁFICO RSX
// ============================================================================

/*
 * initScreen - Inicializa el sistema de gráficos RSX y configura los buffers
 * 
 * RSX es el chip gráfico de PS3 (basado en NVIDIA G70/G71).
 * Esta función:
 * 1. Asigna memoria para el host buffer (comandos RSX)
 * 2. Inicializa el contexto RSX
 * 3. Obtiene la resolución de video configurada
 * 4. Crea y configura los framebuffers
 */
static void initScreen()
{
	// Asignar memoria alineada para comandos RSX
	void *host_addr = memalign(1024*1024, HOSTBUFFER_SIZE);
	
	// Inicializar RSX
	rsxInit(&context, CB_SIZE, HOSTBUFFER_SIZE, host_addr);
	
	// Obtener configuración de video
	videoState state;
	videoGetState(0, 0, &state);
	
	videoResolution res;
	videoGetResolution(state.displayMode.resolution, &res);
	
	display_width = res.width;
	display_height = res.height;
	color_pitch = display_width * 4;  // 4 bytes por píxel (ARGB)
	
	// Crear framebuffers (doble buffer para evitar tearing)
	for(u32 i = 0; i < FRAME_BUFFER_COUNT; i++) {
		color_buffer[i] = (u32*)rsxMemalign(64, display_height * color_pitch);
		rsxAddressToOffset(color_buffer[i], &color_offset[i]);
		gcmSetDisplayBuffer(i, color_offset[i], color_pitch, display_width, display_height);
	}
	
	// Habilitar sincronización vertical (evita tearing)
	gcmSetFlipMode(GCM_FLIP_VSYNC);
}

// ============================================================================
// FUNCIONES DE DIBUJO DE MENSAJE
// ============================================================================

/*
 * drawMessage - Dibuja el mensaje "HOLA PS3" en el centro de la pantalla
 * 
 * Utiliza primitivas geométricas simples para crear un mensaje visual.
 * En un programa más avanzado usarías una biblioteca de fuentes o sprites.
 */
static void drawMessage()
{
	u32 centerX = display_width / 2;
	u32 centerY = display_height / 2;
	
	// Fondo del mensaje (rectángulo azul grande)
	u32 msgWidth = 500;
	u32 msgHeight = 150;
	fillRect(centerX - msgWidth/2, centerY - msgHeight/2, 
	         msgWidth, msgHeight, COLOR_BLUE);
	
	// Borde del mensaje (marco blanco)
	drawBorder(centerX - msgWidth/2, centerY - msgHeight/2, 
	           msgWidth, msgHeight, 6, COLOR_WHITE);
	
	// Título decorativo superior (barra verde)
	fillRect(centerX - msgWidth/2 + 10, centerY - msgHeight/2 + 10, 
	         msgWidth - 20, 30, COLOR_GREEN);
	
	// Texto visual "PS3" con bloques de colores
	u32 blockSize = 40;
	u32 spacing = 50;
	u32 startX = centerX - (3 * spacing) / 2;
	u32 textY = centerY;
	
	// Letra "P" (simulada con bloques)
	fillRect(startX, textY, blockSize, blockSize, COLOR_YELLOW);
	
	// Letra "S" (simulada)
	fillRect(startX + spacing, textY, blockSize, blockSize, COLOR_RED);
	
	// Letra "3" (simulada)
	fillRect(startX + spacing * 2, textY, blockSize, blockSize, COLOR_WHITE);
	
	// Mensaje adicional en la parte inferior
	fillRect(centerX - 100, centerY + 60, 200, 5, COLOR_WHITE);
}

// ============================================================================
// FUNCIÓN PRINCIPAL
// ============================================================================

int main()
{
	padInfo padinfo;
	padData paddata;
	
	printf("===========================================\n");
	printf("   HOLA PS3 - Homebrew Mínimo\n");
	printf("===========================================\n");
	printf("Inicializando sistema gráfico RSX...\n");
	
	// Inicializar sistema de gráficos
	initScreen();
	printf("Resolución: %dx%d\n", display_width, display_height);
	
	// Inicializar sistema de entrada (controladores)
	printf("Inicializando sistema de control...\n");
	ioPadInit(7);  // 7 es el número máximo de controladores
	
	// Limpiar ambos buffers (fondo negro)
	printf("Dibujando mensaje...\n");
	memset(color_buffer[0], 0, display_height * color_pitch);
	memset(color_buffer[1], 0, display_height * color_pitch);
	
	// Dibujar el mensaje visual
	drawMessage();
	
	// Presentar el frame en pantalla
	flip();
	
	printf("===========================================\n");
	printf("Programa iniciado correctamente!\n");
	printf("Presiona X en el control para salir\n");
	printf("===========================================\n");
	
	// ============================================================================
	// LOOP PRINCIPAL - Espera entrada del usuario
	// ============================================================================
	
	while(1) {
		// Obtener información de controladores conectados
		ioPadGetInfo(&padinfo);
		
		// Revisar cada controlador posible
		for(int i = 0; i < MAX_PADS; i++) {
			// Si el controlador está conectado
			if(padinfo.status[i]) {
				// Leer datos del controlador
				ioPadGetData(i, &paddata);
				
				// Si se presionó el botón X (Cross)
				if(paddata.BTN_CROSS) {
					printf("\n===========================================\n");
					printf("Botón X detectado. Saliendo...\n");
					printf("===========================================\n");
					
					// Limpiar la pantalla antes de salir
					memset(color_buffer[curr_fb], 0, display_height * color_pitch);
					flip();
					
					// Terminar programa
					return 0;
				}
			}
		}
		
		// Pequeña pausa para no saturar el CPU (10ms)
		usleep(10000);
	}
	
	return 0;
}
