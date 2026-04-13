/*
 * ring_buffer.c
 *
 *  Created on: Apr 12, 2026
 *      Author: suemi
 */

#include "ring_buffer.h"

bool rb_full(const RingBuffer *rb) {
    return ((rb->head + 1) & (RING_BUFFER_SIZE - 1)) == rb->tail;
}

bool rb_empty(const RingBuffer *rb) {
    return rb->head == rb->tail;
}

bool rb_put(RingBuffer *rb, uint8_t byte) {
    if (rb_full(rb)) {
        return false;
    }
    rb->buf[rb->head] = byte;
    /* Advance head with wrap-around */
    rb->head = (rb->head + 1) & (RING_BUFFER_SIZE - 1);
    return true;
}

bool rb_get(RingBuffer *rb, uint8_t *byte) {
    if (rb_empty(rb)) {
        return false;
    }
    *byte = rb->buf[rb->tail];
    /* Advance tail with wrap-around */
    rb->tail = (rb->tail + 1) & (RING_BUFFER_SIZE - 1);
    return true;
}
