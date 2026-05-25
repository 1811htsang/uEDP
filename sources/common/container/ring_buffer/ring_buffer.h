/**
 * @file ring_buffer.h
 * @author Shang Huang
 * @brief Header file for ring buffer implementation in C
 * @version 0.1
 * @date 2026-05-15
 * @copyright MIT License
 */
#ifndef __RING_BUFFER_H__
	#define __RING_BUFFER_H__

	/**
   * @brief Khai báo thư viện sử dụng
   */
	#include <stdint.h>
	#include <string.h>
	#include <stdbool.h>

	/**
	 * @brief Định nghĩa giá trị trả về của các hàm Ring Buffer
	 */
	#define RET_RING_BUFFER_OK				(0x01u)
	#define RET_RING_BUFFER_NG				(0x00u)

	/**
	 * @brief Định nghĩa cấu trúc Ring Buffer
	 * @param tail_index: Chỉ số đuôi của vòng đệm
	 * @param head_index: Chỉ số đầu của vòng đệm
	 * @param fill_size: Kích thước đã được điền vào vòng đệm
	 * @param buffer_size: Kích thước tổng của vòng đệm
	 * @param element_size: Kích thước của mỗi phần tử trong vòng đệm
	 * @param buffer: Con trỏ đến vùng nhớ chứa dữ liệu của vòng đệm
	 */
	typedef struct {
		uint32_t tail_index;
		uint32_t head_index;
		uint32_t fill_size;
		uint32_t buffer_size;
		uint32_t element_size;
		uint32_t* buffer;
	} ring_buffer_t;

	/**
	 * @brief Khởi tạo vòng đệm
	 * @param ring_buffer Con trỏ đến cấu trúc vòng đệm cần khởi tạo
	 * @param buffer Con trỏ đến vùng nhớ chứa dữ liệu của vòng đệm
	 * @param buffer_size Kích thước tổng của vòng đệm
	 * @param element_size Kích thước của mỗi phần tử trong vòng đệm
	 */
	void		ring_buffer_init(ring_buffer_t* ring_buffer, void* buffer, uint32_t buffer_size, uint32_t element_size);

	/**
	 * @brief Kiểm tra xem Ring  đã được khởi tạo hay chưa
	 * 
	 * @param ring_buffer Con trỏ đến cấu trúc Ring Buffer cần kiểm tra
	 * @return true Nếu Ring Buffer đã được khởi tạo
	 * @return false Nếu Ring Buffer chưa được khởi tạo
	 */
	bool 		ring_buffer_isinit(ring_buffer_t* ring_buffer);

	/**
	 * @brief Kiểm tra số slot còn trống trong vòng đệm
	 * @param ring_buffer Con trỏ đến cấu trúc vòng đệm
	 * @return uint32_t Số lượng phần tử có thể chứa trong vòng đệm
	 */
	uint32_t	ring_buffer_availble(ring_buffer_t* ring_buffer);

	/**
	 * @brief Kiểm tra xem vòng đệm có rỗng hay không
	 * @param ring_buffer Con trỏ đến cấu trúc vòng đệm
	 * @return true Nếu vòng đệm rỗng
	 * @return false Nếu vòng đệm không rỗng
	 */
	bool		ring_buffer_is_empty(ring_buffer_t* ring_buffer);

	/**
	 * @brief Kiểm tra xem vòng đệm có đầy hay không
	 * @param ring_buffer Con trỏ đến cấu trúc vòng đệm
	 * @return true Nếu vòng đệm đầy
	 * @return false Nếu vòng đệm không đầy
	 */
	bool		ring_buffer_is_full(ring_buffer_t* ring_buffer);

	/**
	 * @brief Thêm một phần tử vào vòng đệm
	 * @param ring_buffer Con trỏ đến cấu trúc vòng đệm
	 * @param data Con trỏ đến dữ liệu cần thêm, sử dụng void* 
	 * 				để type casting tới đúng kiểu dữ liệu đã định nghĩa lúc khởi tạo vòng đệm
	 * @return uint8_t RET_RING_BUFFER_OK nếu thêm thành công, ngược lại trả về RET_RING_BUFFER_NG
	 */
	uint8_t	ring_buffer_put(ring_buffer_t* ring_buffer, void* data);

	/**
	 * @brief Lấy một phần tử ra khỏi vòng đệm
	 * @param ring_buffer Con trỏ đến cấu trúc vòng đệm
	 * @param data Con trỏ đến vùng nhớ để lưu dữ liệu lấy ra, sử dụng void*
	 * 				để type casting tới đúng kiểu dữ liệu đã định nghĩa lúc khởi tạo vòng đệm
	 * @return uint8_t RET_RING_BUFFER_OK nếu lấy thành công, ngược lại trả về RET_RING_BUFFER_NG
	 */
	uint8_t	ring_buffer_get(ring_buffer_t* ring_buffer, void* data);

	/**
	 * @brief Lấy số lượng phần tử hiện có trong vòng đệm
	 * @param ring_buffer Con trỏ đến cấu trúc vòng đệm
	 * @return uint16_t Số lượng phần tử hiện có trong vòng đệm
	 */
	uint16_t ring_buffer_get_count(ring_buffer_t* ring_buffer);

#endif //__RING_BUFFER_H__
