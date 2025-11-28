#include <stdint.h>
#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>


static int alloc_gpio(volatile uint32_t* gpio) {
	/* Each FSEL has 10 pins */
	const char* gpio_dev = "dev/gpiomem";
	static uint32_t block_size = 4*1024; /* Page size length, which is less than GPIO's peripheral size */
	static uint32_t gpio_base = 0x7E200000;
	int fd;
	if (fd = open(gpio_dev, O_SYNC | O_RDWR) < 0) {
		perror("Error opening gpiomem\n");
		return -1;
	}

	/* Let's us play with gpio peripherals safely :) */
	gpio = mmap(
			NULL,
			block_size,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			fd,
			gpio_base
		);
	if (gpio == MAP_FAILED) {
		perror("Error in gpio mmap\n");
		close(fd);
		return -1;
	}

	close(fd);
}

void gpio_set_high(uint32_t pins);
	static uint8_t gpset = 7;

	volatile uint32_t* gpio;
	alloc_gpio(gpio);

	uint32_t pin_pos = 0x00U, bit_pos;
	while ((pin >> pin_pos) != 0x00U) {
		bit_pos = 0x01 << pin_pos;
		uint8_t curr_pin = pin & bit_pos;
		if (curr_pin) gpio[gpset] = 1 << pin_pos;
		
		pin_pos++;
	}
}


void gpio_set_mode(uint32_t pins, uint8_t mode) {
	static uint8_t gpset = 7;

	volatile uint32_t* gpio;
	alloc_gpio(gpio);

	uint32_t pin_pos = 0x00U, bit_pos;
	while ((pins >> pin_pos) != 0x00U) {
		bit_pos = 0x01U << pin_pos;
		uint8_t curr_pin = pin & bit_pos;
		if (curr_pin) {
			uint8_t fsel = pin_pos / 10;
			uint8_t pin_bit = (pin_pos % 10) * 3;

			/* Clear and set as output */
			gpio[fsel] &= ~(7U << pin_bit);
			gpio[fsel] |= MODE << pin_bit;
		}
		pin_pos++;
	}
}



















