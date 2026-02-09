/* SPU Vector Math - Programa que corre en el Synergistic Processing Element
 *
 * Recibe un vector de 4 floats via DMA, calcula:
 *   - El cuadrado de cada componente (SIMD paralelo)
 *   - El producto punto (norma al cuadrado)
 *   - La magnitud aproximada (usando rsqrte del SPU)
 * Y devuelve los resultados al PPU via DMA.
 */
#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include <sys/spu_thread.h>

#include "vecmath.h"

#define TAG 1

/* Buffer local alineado en la Local Store del SPE */
static vecmath_data_t data __attribute__((aligned(128)));

/* Espera a que la transferencia DMA con el tag dado termine */
static void wait_for_dma(void)
{
    mfc_write_tag_mask(1 << TAG);
    spu_mfcstat(MFC_TAG_UPDATE_ALL);
}

int main(uint64_t ea_data, uint64_t arg2, uint64_t arg3, uint64_t arg4)
{
    /* 1. Traer la estructura de datos desde memoria principal (PPU) a la Local Store */
    mfc_get(&data, ea_data, sizeof(vecmath_data_t), TAG, 0, 0);
    wait_for_dma();

    /* 2. Cargar los 4 floats de entrada en un registro vectorial SIMD de 128 bits.
     *    Esto carga los 4 valores en paralelo en un solo ciclo. */
    vector float v_input = *(vector float *)data.input;

    /* 3. Calcular el cuadrado de cada componente en paralelo:
     *    output = {x*x, y*y, z*z, w*w}
     *    Una sola instrucción SIMD multiplica los 4 floats simultáneamente. */
    vector float v_squared = spu_mul(v_input, v_input);

    /* Guardar resultado en la estructura */
    *(vector float *)data.output = v_squared;

    /* 4. Calcular el producto punto (dot product = x² + y² + z² + w²)
     *    Necesitamos sumar los 4 elementos del vector.
     *    Usamos rotaciones y sumas para hacer una reducción horizontal. */

    /* v_squared = {a, b, c, d} donde a=x², b=y², c=z², d=w² */
    /* Rotar 4 bytes (1 float) y sumar: {a+b, b+c, c+d, d+a} */
    vector float v_rot1 = (vector float)spu_rlqwbyte((vector unsigned char)v_squared, 4);
    vector float v_sum1 = spu_add(v_squared, v_rot1);

    /* Rotar 8 bytes (2 floats) y sumar: obtiene la suma total en cada elemento */
    vector float v_rot2 = (vector float)spu_rlqwbyte((vector unsigned char)v_sum1, 8);
    vector float v_dot = spu_add(v_sum1, v_rot2);

    /* Extraer el producto punto (está replicado en los 4 elementos) */
    data.dot_product = spu_extract(v_dot, 0);

    /* 5. Calcular magnitud aproximada usando rsqrte del SPU
     *    rsqrte da 1/√x, luego multiplicamos por x para obtener √x
     *    Esto usa la unidad de estimación rápida del SPE (~12 bits de precisión) */
    vector float v_rsqrt_est = spu_rsqrte(v_dot);

    /* √x = x * (1/√x) */
    vector float v_magnitude = spu_mul(v_dot, v_rsqrt_est);
    data.magnitude = spu_extract(v_magnitude, 0);

    /* 6. Marcar como completado */
    data.done = 1;

    /* 7. Enviar los resultados de vuelta al PPU via DMA */
    mfc_put(&data, ea_data, sizeof(vecmath_data_t), TAG, 0, 0);
    wait_for_dma();

    /* Salir limpiamente del thread SPU */
    spu_thread_exit(0);
    return 0;
}
