#include <stdint.h>
#include <rcc.h>

int main() {
	uart2_init();
	systick_init();

	for(;;) {
		printf("We are in app\r\n");
		delay(1000);
	}
}

