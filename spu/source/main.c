/*
 * SPU Vector Math - Programa que corre en el Synergistic Processing Element
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

static vecmath_data_t data __attribute__((aligned(128)));

static void wait_for_dma(void)
{
    mfc_write_tag_mask(1 << TAG);
    spu_mfcstat(MFC_TAG_UPDATE_ALL);
}

int main(uint64_t ea_data, uint64_t arg2, uint64_t arg3, uint64_t arg4)
{
    (void)arg2; (void)arg3; (void)arg4;

    /* 1. DMA get: traer struct desde memoria principal */
    mfc_get(&data, ea_data, sizeof(vecmath_data_t), TAG, 0, 0);
    wait_for_dma();

    /* 2. Cargar input en registro SIMD de 128 bits */
    vector float v_input = *(vector float *)data.input;

    /* 3. Cuadrado de cada componente: output = input * input */
    vector float v_squared = spu_mul(v_input, v_input);
    *(vector float *)data.output = v_squared;

    /* 4. Producto punto: suma horizontal de los 4 elementos */
    vector float v_rot1 = (vector float)spu_rlqwbyte((vector unsigned char)v_squared, 4);
    vector float v_sum1 = spu_add(v_squared, v_rot1);
    vector float v_rot2 = (vector float)spu_rlqwbyte((vector unsigned char)v_sum1, 8);
    vector float v_dot  = spu_add(v_sum1, v_rot2);

    data.dot_product = spu_extract(v_dot, 0);

    /* 5. Magnitud: sqrt(dot) = dot * rsqrte(dot) */
    vector float v_rsqrt = spu_rsqrte(v_dot);
    vector float v_mag   = spu_mul(v_dot, v_rsqrt);
    data.magnitude = spu_extract(v_mag, 0);

    /* 6. Marcar como completado */
    data.done = 1;

    /* 7. DMA put: enviar resultados de vuelta al PPU */
    mfc_put(&data, ea_data, sizeof(vecmath_data_t), TAG, 0, 0);
    wait_for_dma();

    spu_thread_exit(0);
    return 0;
}
