#include <stdint.h>
#include <stdio.h>

#include <LoRa_raspi.h>


int main() {
	
	struct lora lora;
	if (new_lora(&lora)) {
		printf("LoRa detected\n");
	}

	return 0;
}
