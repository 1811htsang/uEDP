/**
 * @file fifo.c
 * @author Shang Huang
 * @brief Implementation of FIFO (First-In-First-Out) queue for UEDP system
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
	//NOTE - Apply defensive programming: Check for NULL pointers and invalid sizes
	if (fifo == NULL) return;
	if (buffer == NULL || buffer_size == 0 || element_size == 0) {
		fifo->buffer = NULL;
		fifo->buffer_size = 0;
		fifo->fill_size = 0;
		return;
	}

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
	//NOTE - Apply defensive programming: Check for NULL pointers and invalid sizes
	bool is_fifo_null = (fifo == NULL);
	bool is_data_null = (data == NULL);
	bool is_buffer_null = (fifo != NULL && fifo->buffer == NULL);
	if (is_fifo_null || is_data_null || is_buffer_null) return RET_FIFO_NG;

	if (fifo->fill_size == fifo->buffer_size) return RET_FIFO_NG; // FIFO đã đầy, không thể thêm phần tử mới

	if (data != NULL) {
		// NOTE - Tính toán địa chỉ đích bằng Byte Pointer Math
		uint8_t* p_dest = (uint8_t*)fifo->buffer + (fifo->tail_index * fifo->element_size);

		// NOTE - Thực hiện copy chính xác element_size bytes
		memcpy(p_dest, data, fifo->element_size);

		// NOTE - Cập nhật quản lý FIFO
		fifo->fill_size++;
		fifo->tail_index = (fifo->tail_index + 1) % fifo->buffer_size;
	} else {
		return RET_FIFO_NG;
	}

	return RET_FIFO_OK;
}

uint32_t fifo_get(fifo_t* fifo, void* data) {
	bool is_fifo_null = (fifo == NULL);
	bool is_data_null = (data == NULL);
	bool is_buffer_null = (fifo != NULL && fifo->buffer == NULL);
	if (is_fifo_null || is_data_null || is_buffer_null) return RET_FIFO_NG;

	if (fifo_is_empty(fifo)) return RET_FIFO_NG;

	// NOTE - Lấy dữ liệu ở vị trí head hiện tại
	uint8_t* p_src = (uint8_t*)fifo->buffer + (fifo->head_index * fifo->element_size);
	memcpy(data, p_src, fifo->element_size);

	// NOTE - Xóa hoàn toàn dữ liệu vừa lấy khỏi FIFO
	memset(p_src, 0, fifo->element_size);

	// NOTE - Cập nhật trạng thái FIFO
	fifo->head_index = (fifo->head_index + 1) % fifo->buffer_size;
	fifo->fill_size--;

	return RET_FIFO_OK;
}

uint32_t 	fifo_put_head(fifo_t* fifo, void* data) {
	//NOTE - Apply defensive programming: Check for NULL pointers and invalid sizes
	bool is_fifo_null = (fifo == NULL);
	bool is_data_null = (data == NULL);
	bool is_buffer_null = (fifo != NULL && fifo->buffer == NULL);
	if (is_fifo_null || is_data_null || is_buffer_null) return RET_FIFO_NG;

	if (fifo->fill_size == fifo->buffer_size) return RET_FIFO_NG; // FIFO đã đầy, không thể thêm phần tử mới

	if (data != NULL) {
		// NOTE - Tính toán chỉ số mới cho head (đi ngược lại)
		uint32_t new_head = (fifo->head_index + fifo->buffer_size - 1) % fifo->buffer_size;

		// NOTE - Ghi dữ liệu vào vị trí mới của head
		uint8_t* p_dest = (uint8_t*)fifo->buffer + (new_head * fifo->element_size);
    memcpy(p_dest, data, fifo->element_size);

		// NOTE - Cập nhật head_index và fill_size
		fifo->head_index = new_head;
		fifo->fill_size++;
	} else {
		// NOTE - Dữ liệu là NULL, không thể thêm vào FIFO
		return RET_FIFO_NG;
	}

	return RET_FIFO_OK;
}