/**
 * @file fifo.h
 * @author Shang Huang
 * @brief FIFO (First-In-First-Out) queue definitions and utilities for CIEDPC system
 * @version 0.1
 * @date 2026-04-16
 * @copyright MIT License
 */
#ifndef __FIFO_H__
	#define __FIFO_H__

	/**
	 * @brief Khai báo các thư viện sử dụng
	 */
	#include <stdint.h>
	#include <string.h>
	#include <stdbool.h>

	/**
	 * @brief Định nghĩa giá trị trả về của các hàm FIFO
	 */
	#define RET_FIFO_OK				(0x01)
	#define RET_FIFO_NG				(0x00)

	/**
	 * @brief Định nghĩa cấu trúc FIFO
	 * @param tail_index: Chỉ số đuôi của FIFO
	 * @param head_index: Chỉ số đầu của FIFO
	 * @param fill_size: Kích thước đã được điền vào FIFO
	 * @param buffer_size: Kích thước tổng của FIFO
	 * @param element_size: Kích thước của mỗi phần tử trong FIFO
	 * @param buffer: Con trỏ đến vùng nhớ chứa dữ liệu của FIFO
	 */
	typedef struct fifo_t {
		uint32_t tail_index;
		uint32_t head_index;
		uint32_t fill_size; 
		uint32_t buffer_size;
		uint32_t element_size;
		uint32_t* buffer;
	} fifo_t;

	/**
	 * @brief Khởi tạo FIFO
	 * @param fifo Con trỏ đến cấu trúc FIFO cần khởi tạo
	 * @param buffer Con trỏ đến vùng nhớ chứa dữ liệu của FIFO
	 * @param buffer_size Kích thước tổng của FIFO
	 * @param element_size Kích thước của mỗi phần tử trong FIFO
	 */
	void		fifo_init(fifo_t* fifo, void* buffer, uint32_t buffer_size, uint32_t element_size); 

	/**
	 * @brief Kiểm tra xem FIFO đã được khởi tạo hay chưa
	 * @param fifo Con trỏ đến cấu trúc FIFO cần kiểm tra
	 * @return true Nếu FIFO đã được khởi tạo
	 * @return false Nếu FIFO chưa được khởi tạo
	 */
	bool 		fifo_isinit(fifo_t* fifo);

	/**
	 * @brief Lấy số lượng phần tử có thể chứa trong FIFO
	 * @param fifo Con trỏ đến cấu trúc FIFO cần kiểm tra
	 * @return uint32_t Số lượng phần tử có thể chứa trong FIFO
	 */
	uint32_t	fifo_available(fifo_t* fifo);
	
	/**
	 * @brief Kiểm tra FIFO có rỗng hay không
	 * @param fifo Con trỏ đến cấu trúc FIFO cần kiểm tra
	 * @return true Nếu FIFO rỗng
	 * @return false Nếu FIFO không rỗng
	 */
	bool		fifo_is_empty(fifo_t* fifo);

	/**
	 * @brief Kiểm tra FIFO có đầy hay không
	 * @param fifo Con trỏ đến cấu trúc FIFO cần kiểm tra
	 * @return true Nếu FIFO đầy
	 * @return false Nếu FIFO không đầy
	 */
	bool		fifo_is_full(fifo_t* fifo);

	/**
	 * @brief Thêm phần tử vào FIFO
	 * @param fifo Con trỏ đến cấu trúc FIFO cần thêm phần tử
	 * @param data Con trỏ đến dữ liệu cần thêm vào FIFO
	 * @return uint32_t Trả về trạng thái của thao tác thêm phần tử vào FIFO
	 */
	uint32_t	fifo_put(fifo_t* fifo, void* data);

	/**
	 * @brief Lấy phần tử ra khỏi FIFO
	 * @param fifo Con trỏ đến cấu trúc FIFO cần lấy phần tử
	 * @param data Con trỏ đến vùng nhớ để lưu dữ liệu lấy ra từ FIFO
	 * @return uint32_t Trả về trạng thái của thao tác lấy phần tử ra khỏi FIFO
	 */
	uint32_t	fifo_get(fifo_t* fifo, void* data);

#endif //__FIFO_H__
