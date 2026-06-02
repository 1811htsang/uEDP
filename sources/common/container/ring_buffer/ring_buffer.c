/**
 * @file ring_buffer.c
 * @author Shang Huang
 * @brief Implementation of ring buffer in C
 * @version 0.1
 * @date 2026-05-15
 * @copyright MIT License
 */
#include <stdbool.h>
#include <stdlib.h>
#include "ring_buffer.h"

void ring_buffer_init(ring_buffer_t* ring_buffer, void* buffer, uint32_t buffer_size, uint32_t element_size) {
	ring_buffer->tail_index = 0;
	ring_buffer->head_index = 0;
	ring_buffer->fill_size = 0;

	ring_buffer->buffer_size = buffer_size;
	ring_buffer->buffer = memset(buffer, 0, buffer_size * element_size);
	ring_buffer->element_size = element_size;
}

bool 		ring_buffer_isinit(ring_buffer_t* ring_buffer) {
	return (ring_buffer->buffer != NULL) ? true : false;
}

uint32_t ring_buffer_availble(ring_buffer_t* ring_buffer) {
	return ring_buffer->fill_size;
}

bool ring_buffer_is_empty(ring_buffer_t* ring_buffer) {
	return (ring_buffer->fill_size == 0) ? true : false;
}

bool ring_buffer_is_full(ring_buffer_t* ring_buffer) {
	return (ring_buffer->fill_size == ring_buffer->buffer_size) ? true : false;
}

uint8_t ring_buffer_put(ring_buffer_t* ring_buffer, void* data) {
	uint32_t next_tail_index;
	uint32_t next_head_index;

	if (data != NULL) {
		// Tính toán địa chỉ và tự động copy đúng số byte đã định nghĩa lúc ring_buffer_init
		uint8_t* p_dest = (uint8_t*)ring_buffer->buffer + (ring_buffer->tail_index * ring_buffer->element_size);
		memcpy(p_dest, data, ring_buffer->element_size);

		// Cập nhật chỉ số đuôi và kích thước đã điền vào vòng đệm
		next_tail_index = (++ring_buffer->tail_index) % ring_buffer->buffer_size;
		ring_buffer->tail_index = next_tail_index;

		// Nếu vòng đệm đã đầy, cập nhật chỉ số đầu để ghi đè phần tử cũ nhất, ngược lại tăng kích thước đã điền vào vòng đệm
		if (ring_buffer->fill_size == ring_buffer->buffer_size) {
			next_head_index = (++ring_buffer->head_index) % ring_buffer->buffer_size;
			ring_buffer->head_index = next_head_index;
		}
		else {
			ring_buffer->fill_size++;
		}
	}
	else {
		return RET_RING_BUFFER_NG;
	}

	return RET_RING_BUFFER_OK;
}

uint8_t ring_buffer_get(ring_buffer_t* ring_buffer, void* data) {
	uint32_t next_head_index;

	if (ring_buffer_is_empty(ring_buffer)) {
		return RET_RING_BUFFER_NG;
	}

	if (data != NULL) {
		// Tính toán địa chỉ và tự động copy đúng số byte đã định nghĩa lúc ring_buffer_init
		uint8_t* p_src = (uint8_t*)ring_buffer->buffer + (ring_buffer->head_index * ring_buffer->element_size);
		memcpy((uint8_t*)data, p_src, ring_buffer->element_size);

		// Cập nhật chỉ số đầu và giảm kích thước đã điền vào vòng đệm
		next_head_index = (++ring_buffer->head_index) % ring_buffer->buffer_size;
		ring_buffer->head_index = next_head_index;

		ring_buffer->fill_size--;
	}
	else {
		return RET_RING_BUFFER_NG;
	}

	return RET_RING_BUFFER_OK;
}
