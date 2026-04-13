/*
 * uart.h
 *
 *  Created on: Apr 12, 2026
 *      Author: suemi
 */

#ifndef UART_H_
#define UART_H_

#include <stdint.h>

/* Initialize UART1 at the requested baud rate (valid baud rates: 9600, 115200)
 * Call this once before any send/receive */
void uart_init(uint32_t baud);

/* Queue one byte for transmission and returns immediately (non-blocking) — the ISR drains the TX buffer */
void uart_send_byte(uint8_t byte);

/* Queue a null-terminated string for transmission */
void uart_send_string(const char *str);

/* Returns 1 if a received byte is available in the RX buffer */
int uart_data_available(void);

/* Read one byte from the RX ring buffer and check uart_data_available() first */
uint8_t uart_read_byte(void);

#endif /* UART_H_ */
