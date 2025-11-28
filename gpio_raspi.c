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
	int fd = open(gpio_dev, O_SYNC | O_RDWR);
	if (fd < 0) {
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
			(long int) gpio_base
		);
	if (gpio == MAP_FAILED) {
		perror("Error in gpio mmap\n");
		close(fd);
		return -1;
	}

	close(fd);
	
	return 0;
}

void gpio_set_high(uint32_t pins) {

	volatile uint32_t* gpio;
	if (alloc_gpio(gpio) < 0) {
		perror("Error obtaining GPIO mmap\n");
		return;
	}

	uint32_t pin_pos = 0x00U, bit_pos;
	while ((pins >> pin_pos) != 0x00U) {
		bit_pos = 0x01U << pin_pos;
		uint32_t curr_pin = pins & bit_pos;
		if (curr_pin){
			static uint8_t gpset = 7;
			gpio[gpset] = 1 << pin_pos;
		}
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
		uint32_t curr_pin = pins & bit_pos;
		if (curr_pin) {
			uint8_t fsel = (uint8_t) pin_pos / 10U;
			uint8_t pin_bit = (uint8_t) (((uint8_t)pin_pos % 10U) * 3U);

			/* Clear and set as output */
			gpio[fsel] &= ~(7U << pin_bit);
			gpio[fsel] |= mode << pin_bit;
		}
		pin_pos++;
	}
}



















