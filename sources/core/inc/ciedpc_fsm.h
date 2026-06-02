/**
 * @file ciedpc_fsm.h
 * @author Shang Huang
 * @brief Finite State Machine definitions and utilities for CIEDPC system
 * @version 0.1
 * @date 2026-04-16
 * 
 * @copyright MIT License
 * 
 */
#ifndef __CIEDPC_FSM_H__
	#define __CIEDPC_FSM_H__

	/**
	 * @brief Khai báo thư viện sử dụng
	 */
	#include <stdint.h>
	#include "ciedpc_core.h"

	/**
	 * @brief Khai báo kiểu dữ liệu để quản lý tin nhắn trong hệ thống CIEDPC
	 * @attention `ciedpc_msg_t` được gọi ở đây để thực thi forward declaration, 
	 * 						cho phép sử dụng con trỏ đến `ciedpc_msg_t` trong các khai báo sau này
	 */
	typedef struct ciedpc_msg_t ciedpc_msg_t;

	/**
	 * @brief Số lượng trạng thái tối đa trong lịch sử của FSM
	 */
	#define CIEDPC_FSM_HIS_MAX 			(4u) 

	/**
	 * @brief Định nghĩa kiểu hàm xử lý trạng thái trong FSM
	 * @param msg là con trỏ đến tin nhắn được gửi đến FSM, 
	 * 				cho phép hàm xử lý trạng thái truy cập và xử lý thông tin từ tin nhắn đó 
	 * 				để thực hiện các hành động tương ứng dựa trên nội dung của tin nhắn và trạng thái hiện tại của FSM.
	 */
	typedef void (*state_handler)(ciedpc_msg_t* msg);

	/**
	 * @brief Định nghĩa cấu trúc để quản lý thông tin của FSM trong hệ thống CIEDPC
	 * @attention `history` không được khai báo vượt quá CIEDPC_FSM_HIS_MAX 
	 * 						để đảm bảo an toàn bộ nhớ và tránh lỗi tràn bộ nhớ 
	 * 						khi lưu lịch sử trạng thái của FSM.
	 */
	typedef struct ciedpc_fsm_t {
		state_handler state; // Hàm xử lý trạng thái hiện tại của FSM
		state_handler history[CIEDPC_FSM_HIS_MAX]; // Mảng lưu lịch sử trạng thái 
		ui8 history_index; // Chỉ số để quản lý vị trí hiện tại trong lịch sử trạng thái
		ui8 history_count; // Số lượng trạng thái đã lưu trong lịch sử, giúp quản lý và kiểm soát việc lưu trữ lịch sử trạng thái của FSM
	} ciedpc_fsm_t;

	/**
	 * @brief Khởi tạo FSM và thực hiện hành động INIT đầu tiên
	 */
	#define ciedpc_fsm_init(me, init_func) \
	do { \
		(me)->state = (state_handler)(init_func); \
		(me)->history_index = 0; \
		memset((me)->history, 0, sizeof((me)->history)); \
		ciedpc_msg_t* m = ciedpc_msg_alloc(0, CIEDPC_FSM_SIG_INIT, 0); \
		(init_func)(m); /* Gửi tín hiệu INIT đến trạng thái khởi tạo của FSM */ \
	} while(0)

	/**
	 * @brief Khai báo hàm để xử lý tin nhắn và điều hướng trạng thái trong FSM
	 * @param me chỉ trạng thái hiện tại của FSM
	 * @param msg chỉ con trỏ đến tin nhắn được gửi đến FSM
	 */
	stinl void ciedpc_fsm_dispatch(ciedpc_fsm_t* me, ciedpc_msg_t* msg) {
		if (me && me->state && msg) {
			me->state(msg);
		}
	}

	/**
	 * @brief Hàm để chuyển đổi trạng thái của FSM
	 * 
	 * @param me chỉ trạng thái hiện tại của FSM
	 * @param target chỉ hàm xử lý trạng thái mục tiêu mà FSM sẽ chuyển đến
	 */
	void ciedpc_fsm_go_next(ciedpc_fsm_t* me, state_handler target);

	/**
	 * @brief Hàm để quay lại trạng thái trước đó của FSM dựa trên lịch sử đã lưu
	 * 
	 * @param me chỉ trạng thái hiện tại của FSM, hàm sẽ sử dụng thông tin trong `history` để quay lại trạng thái trước đó
	 */
	void ciedpc_fsm_go_back(ciedpc_fsm_t* me);

#endif //__CIEDPC_FSM_H__
