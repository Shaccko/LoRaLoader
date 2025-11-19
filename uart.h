#ifndef UART_H__
#define UART_H__

#include <stdint.h>
#include <hal.h>

#define UART1 ((struct uart *) 0x40011000)
#define UART2 ((struct uart *) 0x40004400)
#define UART6 ((struct uart *) 0x40011400)

#define UART2_TX (PIN_NUM(2))
#define UART2_RX (PIN_NUM(3))
#define UART2_PORT ('A')

struct uart {
	volatile uint32_t SR, DR, BRR, CR1, CR2, CR3, GTPR;
};

extern struct uart* uart2;

void uart_init(struct uart* uart, unsigned long baud, uint32_t uart_pins, uint8_t port);

int uart_read_ready(struct uart *uart);

uint8_t uart_read_byte(struct uart *uart);

void uart_write_buf(struct uart *uart, char *buf, size_t len);

void uart2_init(void);


#endif
