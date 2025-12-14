#include <stdint.h>
#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <gpio_raspi.h>

static volatile uint32_t* gpio;

int gpio_alloc(void) {
	/* Each FSEL has 10 pins */
	const char* gpio_dev = "/dev/gpiomem";
	static uint32_t block_size = 4*1024; /* Page size length, which is less than GPIO's peripheral size */
	int fd = open(gpio_dev, O_SYNC | O_RDWR);
	if (fd < 0) {
		perror("Error opening gpiomem");
		return -1;
	}

	/* Let's us play with gpio peripherals safely :) */
	gpio = mmap(
			NULL,
			block_size,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			fd,
			0
		);
	if (gpio == MAP_FAILED) {
		perror("Error in gpio mmap");
		close(fd);
		return -1;
	}

	close(fd);
	
	return 0;
}

void gpio_raspi_write_pin(uint32_t pins, uint8_t state) {
	/* Loop through pins via bitmasking */
	uint32_t pin_pos = 0x00U, bit_pos;
	while ((pins >> pin_pos) != 0x00U) {
		bit_pos = 0x01U << pin_pos;
		uint32_t curr_pin = pins & bit_pos; /* Pin exist? */
		if (curr_pin) {
			/* Get pin offset register */
			uint8_t gpset = (uint8_t)(7U + (pin_pos >> 5));
			uint8_t gpclr = (uint8_t)(10U + (pin_pos >> 5));
			/* Get pin offset bit */
			if (state == PIN_SET) {
				gpio[gpset] |= 1U << (pin_pos % 32);
			}
			if (state == PIN_RESET) {
				gpio[gpclr] |= 1U << (pin_pos % 32);
			}
		}
		pin_pos++;
	}
}


void gpio_raspi_set_mode(uint32_t pins, uint8_t mode) {
	/* Loop through pins via bitmasking */
	uint32_t pin_pos = 0x00U, bit_pos;
	while ((pins >> pin_pos) != 0x00U) {
		bit_pos = 0x01U << pin_pos;
		uint32_t curr_pin = pins & bit_pos; /* Pin exist? */
		if (curr_pin) {
			uint8_t fsel = (uint8_t) pin_pos / 10U;
			uint8_t pin_bit = (uint8_t) (((uint8_t)pin_pos % 10U) * 3U);

			/* Clear and set as output */
			gpio[fsel] &= ~(7U << pin_bit);
			gpio[fsel] |= ((uint32_t) mode << pin_bit);
		}
		pin_pos++;
	}
}



















