#ifndef __GPIO_RASPI_H__
#define __GPIO_RASPI_H__

#include <stdint.h>

enum { PIN_INPUT, PIN_OUTPUT };
enum { PIN_SET, PIN_RESET };

int gpio_alloc(void);
void gpio_raspi_set_mode(uint32_t pins, uint8_t mode);
void gpio_raspi_write_pin(uint32_t pins, uint8_t state);

#endif
