/*
 * uart.c
 *
 *  Created on: Apr 12, 2026
 *      Author: suemi
 */

#include "uart.h"
#include "ring_buffer.h"
#include "stm32f3xx.h"
#include <string.h>

/* Two ring buffers: one for incoming bytes and one for outgoing bytes */
static RingBuffer rx_buf;
static RingBuffer tx_buf;

void uart_init(uint32_t baud) {
    /* RCC_AHBENR  = AHB bus clock enable register (controls GPIO clocks)
     * RCC_APB2ENR = APB2 bus clock enable register (controls USART1 clock) */
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN; /* enable GPIOA clock */
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN; /* enable USART1 clock */

    /* Each GPIO pin has two control registers:
     * MODER:  mode (input / output / alternate function / analog)
     * AFR[]:  which alternate function (0-15) when mode = AF
     *
     * PA9 is UART1 TX, PA10 is UART1 RX
     * Bits for PA9 are at position 18-19 in MODER; PA10 at 20-21
     * Alternate function 7 (0b0111) = USART1 on these pins */

    /* Clear mode bits for PA9 and PA10 then set to AF mode (0b10) */
    GPIOA->MODER &= ~(GPIO_MODER_MODER9 | GPIO_MODER_MODER10);
    GPIOA->MODER |= (GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1);

    /* Select AF7 for PA9 (high nibble in AFRH byte 1) */
    GPIOA->AFR[1] &= ~(0xF << 4); /* clear PA9 AF bits (AFRH bits 7:4) */
    GPIOA->AFR[1] |=  (7   << 4); /* set AF7 */

    /* Select AF7 for PA10 (high nibble in AFRH byte 2) */
    GPIOA->AFR[1] &= ~(0xF << 8); /* clear PA10 AF bits (AFRH bits 11:8) */
    GPIOA->AFR[1] |=  (7   << 8); /* set AF7 */

    /* USART_BRR = FCLK / baud
     * FCLK (USART1 clock) = 8 MHz after reset (HSI, no PLL) */
    USART1->BRR = SystemCoreClock / baud;

    /* CR1 bits:
     * UE (bit 0) = USART enable
     * RE (bit 2) = Receiver enable
     * TE (bit 3) = Transmitter enable
     * RXNEIE (bit 5) = RX interrupt enable (fires when byte received)
     * TXEIE  (bit 7) = TX interrupt enable (fires when TX register empty) */
    USART1->CR1 = USART_CR1_UE | USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE;

    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_SetPriority(USART1_IRQn, 1); /* priority 1 = fairly high */
}

void uart_send_byte(uint8_t byte) {
    __disable_irq();
    rb_put(&tx_buf, byte);
    USART1->CR1 |= USART_CR1_TXEIE;
    __enable_irq();
}

void uart_send_string(const char *str) {
    while (*str) {
        uart_send_byte((uint8_t)*str++);
    }
}

int uart_data_available(void) {
    return !rb_empty(&rx_buf);
}

uint8_t uart_read_byte(void) {
    uint8_t b = 0;
    rb_get(&rx_buf, &b);
    return b;
}

/* USART1 Interrupt Service Routine */
void USART1_IRQHandler(void) {
    uint32_t sr = USART1->ISR;

    /* RX: a new byte arrived */
    if (sr & USART_ISR_RXNE) {
        uint8_t byte = (uint8_t)(USART1->RDR);
        rb_put(&rx_buf, byte);
    }

    /* TX: transmit data register is empty, feed the next byte */
    if (sr & USART_ISR_TXE) {
        uint8_t byte;
        if (rb_get(&tx_buf, &byte)) {
            USART1->TDR = byte;
        } else {
            USART1->CR1 &= ~USART_CR1_TXEIE;
        }
    }
}
