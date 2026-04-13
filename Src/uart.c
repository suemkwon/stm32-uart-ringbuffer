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

static RingBuffer rx_buf;
static RingBuffer tx_buf;

void uart_init(uint32_t baud) {
    RCC->AHBENR  |= RCC_AHBENR_GPIOCEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    for (volatile int i = 0; i < 1000; i++);

    /* Configure PC4 (TX) and PC5 (RX) as Alternate Function */
    GPIOC->MODER &= ~(GPIO_MODER_MODER4 | GPIO_MODER_MODER5);
    GPIOC->MODER |=  (GPIO_MODER_MODER4_1 | GPIO_MODER_MODER5_1);

    /* AF7 = USART1 on PC4 and PC5 */
    GPIOC->AFR[0] &= ~(0xF << 16);
    GPIOC->AFR[0] |=  (7   << 16);

    GPIOC->AFR[0] &= ~(0xF << 20);
    GPIOC->AFR[0] |=  (7   << 20);

    /* Set baud rate */
    USART1->BRR = 8000000 / baud;

    /* Enable USART with TX, RX, and RX interrupt */
    USART1->CR1 = USART_CR1_UE | USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE;

    /* Let USART stabilize before enabling interrupt */
    for (volatile int i = 0; i < 1000; i++);

    /* Enable USART1 interrupt in NVIC */
    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_SetPriority(USART1_IRQn, 1);
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

void USART1_IRQHandler(void) {
    uint32_t sr = USART1->ISR;

    /* RX: byte received -> store in ring buffer */
    if (sr & USART_ISR_RXNE) {
        uint8_t byte = (uint8_t)(USART1->RDR);
        rb_put(&rx_buf, byte);
    }

    /* TX: register empty -> send next byte or disable interrupt */
    if (sr & USART_ISR_TXE) {
        uint8_t byte;
        if (rb_get(&tx_buf, &byte)) {
            USART1->TDR = byte;
        } else {
            USART1->CR1 &= ~USART_CR1_TXEIE;
        }
    }
}
