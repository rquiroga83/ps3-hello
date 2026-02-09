#ifndef __VECMATH_H__
#define __VECMATH_H__

/* Estructura compartida entre PPU y SPU para el ejemplo de cálculo vectorial.
 * Debe estar alineada a 16 bytes para transferencias DMA.
 *
 * El PPU llena 'input' con 4 floats, y el SPU calcula:
 *   output[i] = input[i] * input[i]  (cuadrado de cada componente)
 *   dot_product = sum(input[i] * input[i])  (norma al cuadrado del vector)
 */
typedef struct _vecmath_data {
    float input[4];        /* Vector de entrada (4 floats = 128 bits = 1 registro SIMD) */
    float output[4];       /* Vector de salida con los cuadrados */
    float dot_product;     /* Producto punto consigo mismo (norma²) */
    float magnitude;       /* Magnitud del vector (√dot_product) */
    unsigned int done;     /* Flag de sincronización */
    unsigned int pad[3];   /* Padding para alineación a 128 bits */
} vecmath_data_t __attribute__((aligned(128)));

#endif
