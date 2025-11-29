#ifndef __GPIO_RASPI_H__
#define __GPIO_RASPI_H__

#include <stdint.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

enum { GPIO_PIN_INPUT, GPIO_PIN_OUTPUT };

int alloc_gpio(volatile uint32_t* gpio);
void gpio_raspi_set_mode(uint32_t pins, uint8_t mode);
void gpio_raspi_set_high(uint32_t pins);

#endif
