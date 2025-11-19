#include <stdint.h>
#include <uart.h>
#include <hal.h>

struct uart* uart2 = UART2;

void uart_init(struct uart* uart, unsigned long baud, uint32_t uart_pins, uint8_t port) {

	uint8_t af = 7; /* uart AF num */
	uint32_t pins = uart_pins;

	if (uart == UART1) RCC->APB2ENR |= BIT(4); /* Hard coded UART1 */
	if (uart == UART2) RCC->APB1ENR |= BIT(17);

	gpio_set_mode(pins, GPIO_MODE_AF, port);
	gpio_set_af(pins, af, port);

	uart->CR1 = 0;
	uart->BRR = CLK_FREQ / baud;
	uart->CR1 = BIT(13) | BIT(2) | BIT(3);
}

int uart_read_ready(struct uart *uart) {
	return uart->SR & BIT(5);
}

uint8_t uart_read_byte(struct uart *uart) {
	return (uint8_t) (uart->DR & 255);
}

static inline void spin(uint32_t delay) {
	while(delay--) (void) 0;
}

void uart_write_byte(struct uart *uart, uint8_t byte) {
	uart->DR = byte;
	while ((uart->SR & BIT(7)) == 0) spin(1);
}

void uart_write_buf(struct uart *uart, char *buf, size_t len) {
	while (len-- > 0) uart_write_byte(uart, *(uint8_t *) buf++);
}

void uart2_init(void) {
	uint32_t uart2_pins = UART2_TX | UART2_RX;
	uint8_t uart2_port = UART2_PORT;

	uart_init(uart2, 9600, uart2_pins, uart2_port);
}
