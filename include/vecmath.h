#ifndef __VECMATH_H__
#define __VECMATH_H__

/*
 * Estructura compartida entre PPU y SPU para calculo vectorial.
 * Debe estar alineada a 128 bytes para transferencias DMA.
 *
 * El PPU llena 'input' con 4 floats, y el SPU calcula:
 *   output[i] = input[i] * input[i]
 *   dot_product = sum(input[i]^2)
 *   magnitude = sqrt(dot_product)
 */
typedef struct _vecmath_data {
    float input[4];
    float output[4];
    float dot_product;
    float magnitude;
    unsigned int done;
    unsigned int pad[21]; /* pad to exactly 128 bytes (DMA requires multiples of 16) */
} vecmath_data_t __attribute__((aligned(128)));

#endif
