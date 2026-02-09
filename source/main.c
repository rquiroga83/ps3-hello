/* hola_ps3 - Ejemplo de PS3 Homebrew con cálculo vectorial en SPE
 *
 * Este programa demuestra cómo usar los Synergistic Processing Elements (SPEs)
 * del procesador Cell Broadband Engine para realizar cálculos SIMD en paralelo.
 *
 * Flujo del programa:
 *   1. PPU prepara un vector de 4 floats
 *   2. PPU crea un thread SPU y le pasa la dirección del vector
 *   3. SPU recibe el vector via DMA, calcula cuadrados/producto punto/magnitud
 *   4. SPU envía los resultados de vuelta via DMA
 *   5. PPU lee los resultados y los muestra en pantalla
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ppu-lv2.h>
#include <sys/spu.h>
#include <sys/thread.h>

#include "vecmath.h"

/* Binario SPU embebido via bin2o (generado por el Makefile) */
extern const unsigned int spu_bin[];
extern const unsigned int spu_bin_size;

/* Datos compartidos - DEBEN estar alineados a 128 bytes para DMA del SPE */
static vecmath_data_t spe_data __attribute__((aligned(128)));

int main(int argc, const char *argv[])
{
    sysSpuImage image;
    u32 group_id;
    u32 thread_id;
    sysSpuThreadArgument arg;
    sysSpuThreadGroupAttribute grpattr;
    sysSpuThreadAttribute attr;
    int cause, status;

    printf("==============================================\n");
    printf("  Hola Mundo desde PlayStation 3!\n");
    printf("  Ejemplo de calculo vectorial SIMD en SPE\n");
    printf("==============================================\n\n");

    /* ---- Preparar los datos de entrada ---- */
    spe_data.input[0] = 1.0f;
    spe_data.input[1] = 2.0f;
    spe_data.input[2] = 3.0f;
    spe_data.input[3] = 4.0f;
    spe_data.done = 0;

    printf("[PPU] Vector de entrada: (%.1f, %.1f, %.1f, %.1f)\n",
           spe_data.input[0], spe_data.input[1],
           spe_data.input[2], spe_data.input[3]);
    printf("[PPU] Enviando al SPE para calculo SIMD...\n\n");

    /* ---- Inicializar el subsistema SPU ---- */
    printf("[PPU] Inicializando SPUs...\n");
    sysSpuInitialize(6, 0);

    /* Cargar la imagen del programa SPU (embebido en el ELF) */
    sysSpuImageImport(&image, (const void *)spu_bin, 0);

    /* ---- Crear grupo de threads SPU ---- */
    memset(&grpattr, 0, sizeof(sysSpuThreadGroupAttribute));
    grpattr.nameSize = 18;
    grpattr.nameAddress = (u64)(uintptr_t)"vecmath_spu_group";

    sysSpuThreadGroupCreate(&group_id, 1, 100, &grpattr);

    /* ---- Crear el thread SPU ---- */
    /* arg.arg0 = effective address de nuestra estructura de datos
     * El SPU usará esta dirección para hacer DMA get/put */
    memset(&arg, 0, sizeof(sysSpuThreadArgument));
    arg.arg0 = (u64)(uintptr_t)&spe_data;

    memset(&attr, 0, sizeof(sysSpuThreadAttribute));
    attr.nameSize = 19;
    attr.nameAddress = (u64)(uintptr_t)"vecmath_spu_thread";

    sysSpuThreadInitialize(&thread_id, group_id, 0, &image, &attr, &arg);
    printf("[PPU] Thread SPU creado (group=%u, thread=%u)\n", group_id, thread_id);

    /* ---- Ejecutar el SPU ---- */
    printf("[PPU] Iniciando ejecucion del SPE...\n");
    sysSpuThreadGroupStart(group_id);

    /* Esperar a que el thread SPU termine */
    sysSpuThreadGroupJoin(group_id, &cause, &status);
    printf("[PPU] SPE finalizado (cause=%d, status=%d)\n\n", cause, status);

    /* ---- Mostrar resultados ---- */
    printf("============== RESULTADOS ==============\n");
    printf("[SPE] Cuadrados:        (%.1f, %.1f, %.1f, %.1f)\n",
           spe_data.output[0], spe_data.output[1],
           spe_data.output[2], spe_data.output[3]);
    printf("[SPE] Producto punto:   %.1f\n", spe_data.dot_product);
    printf("[SPE] Magnitud:         %.4f\n", spe_data.magnitude);
    printf("[SPE] Sincronizacion:   %s\n",
           spe_data.done ? "OK" : "ERROR");
    printf("========================================\n\n");

    /* Verificación */
    printf("[PPU] Verificacion: 1^2 + 2^2 + 3^2 + 4^2 = %.1f (esperado: 30.0)\n",
           spe_data.dot_product);

    /* Limpiar */
    sysSpuThreadGroupDestroy(group_id);
    sysSpuImageClose(&image);

    printf("\n[PPU] Programa completado exitosamente.\n");
    return 0;
}
