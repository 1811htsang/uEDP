/**
 * @file fifo.c
 * @author Shang Huang
 * @brief Implementation of FIFO (First-In-First-Out) queue for CIEDPC system
 * @version 0.1
 * @date 2026-04-16
 * 
 * @copyright MIT License
 * 
 */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "fifo.h"

void fifo_init(fifo_t* fifo, void* buffer, uint32_t buffer_size, uint32_t element_size) {
	fifo->tail_index = 0;
	fifo->head_index = 0;
	fifo->fill_size = 0;

	fifo->buffer_size = buffer_size;
	fifo->buffer = memset(buffer, 0, buffer_size * element_size);
	fifo->element_size = element_size;
}

bool fifo_isinit(fifo_t* fifo) {
	return (fifo->buffer != NULL) ? true : false;
}

uint32_t fifo_available(fifo_t* fifo) {
	return fifo->fill_size;
}

bool fifo_is_empty(fifo_t* fifo) {
	return (fifo->fill_size == 0) ? true : false;
}

bool fifo_is_full(fifo_t* fifo) {
	return (fifo->fill_size == fifo->buffer_size) ? true : false;
}

uint32_t fifo_put(fifo_t* fifo, void* data) {
	uint32_t next_tail_index;

	if (fifo->fill_size == fifo->buffer_size) {
		return RET_FIFO_NG; // FIFO đã đầy, không thể thêm phần tử mới
	}

	if (data != NULL) {
		/* 1. Tính toán địa chỉ đích bằng Byte Pointer Math */
		// fifo->buffer là uint8_t*, cộng với offset tính bằng byte
		uint8_t* p_dest = (uint8_t*)fifo->buffer + (fifo->tail_index * fifo->element_size);

		/* 2. Thực hiện copy chính xác element_size bytes */
		// Tuyệt đối KHÔNG ép kiểu (uint32_t*) cho 'data' và 'p_dest'
		// memcpy sẽ tự động copy đúng số byte đã định nghĩa lúc fifo_init
		memcpy(p_dest, data, fifo->element_size);

		/* 3. Cập nhật quản lý FIFO */
		fifo->fill_size++;
		fifo->tail_index = (fifo->tail_index + 1) % fifo->buffer_size;
	}
	else {
		return RET_FIFO_NG;
	}

	return RET_FIFO_OK;
}

uint32_t fifo_get(fifo_t* fifo, void* data) {
	if ((fifo == NULL) || (data == NULL) || (fifo->buffer == NULL)) {
		return RET_FIFO_NG;
	}

	if (fifo_is_empty(fifo)) {
		return RET_FIFO_NG;
	}

	/* 1. Lấy dữ liệu ở vị trí head hiện tại */
	uint8_t* p_src = (uint8_t*)fifo->buffer + (fifo->head_index * fifo->element_size);
	memcpy(data, p_src, fifo->element_size);

	/* 2. Xóa hoàn toàn dữ liệu vừa lấy khỏi FIFO */
	memset(p_src, 0, fifo->element_size);

	/* 3. Cập nhật trạng thái FIFO */
	fifo->head_index = (fifo->head_index + 1) % fifo->buffer_size;
	fifo->fill_size--;

	return RET_FIFO_OK;
}

