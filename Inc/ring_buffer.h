/*
 * ring_buffer.h
 *
 *  Created on: Apr 12, 2026
 *      Author: suemi
 */

#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_

#include <stdint.h>
#include <stdbool.h>

#define RING_BUFFER_SIZE 128

typedef struct {
    volatile uint8_t  buf[RING_BUFFER_SIZE];
    volatile uint32_t head;   /* write index */
    volatile uint32_t tail;   /* read index */
} RingBuffer;

/* Put one byte into the buffer. Returns false if full */
bool rb_put(RingBuffer *rb, uint8_t byte);

/* Get one byte from the buffer. Returns false if empty */
bool rb_get(RingBuffer *rb, uint8_t *byte);

/* Returns true if the buffer is empty */
bool rb_empty(const RingBuffer *rb);

/* Returns true if the buffer is full */
bool rb_full(const RingBuffer *rb);

#endif /* RING_BUFFER_H_ */
