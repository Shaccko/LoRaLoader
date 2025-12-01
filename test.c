#include <stdint.h>
#include <stdio.h>


int main(void) {
	uint32_t n = 0x0;
	n = 1 << 35;

	printf("%X\n", n);

}
