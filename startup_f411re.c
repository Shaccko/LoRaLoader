#include <stdint.h>

uint32_t *vector_table = (uint32_t *) &_stext;
uint32_t *vtor = (uint32_t *)0xE000ED08;
*vtor = ((uint32_t) vector_table & 0xFFFFFFF8);

