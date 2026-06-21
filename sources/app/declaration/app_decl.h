/**
 * @file app_decl.h
 * @author Shang Huang
 * @brief Application declaration header file
 * @version 0.1
 * @date 2026-05-07
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef __APP_DECL_H__
	#define __APP_DECL_H__

	/**
	 * @brief Khai báo thư viện sử dụng
	 */

	#include "uedp_core.h"
	#include "uedp_task.h"
	#include "uedp_tsm.h"
	#include "uedp_fsm.h"
	#include "uedp_msg.h"
	#include "uedp_timer.h"

	/**
	 * @brief Khai báo tác vụ
	 * @attention Tác vụ phải khai báo đúng định dạng `0xEx`
	 * 						x bắt đầu từ 6 trở đi. Giới hạn tối đa là 0xEF (15 tác vụ)
	 * @example
	 * #define TASK_NORM_A_ID (0xE6u)
	 * #define TASK_NORM_B_ID (0xE7u)
	 */

	// Điền các khai báo tại đây

	/**
	 * @brief Khai báo tín hiệu giao tiếp giữa các tác vụ
	 * @attention Tác vụ phải khai báo đúng định dạng `0x0x`
	 * 						x bắt đầu từ 1 trở đi. Không nên vượt quá 0x7F 
	 * 						để tránh trùng với tín hiệu nội bộ của hệ thống UEDP
	 * @example
	 * #define SIG_USR_START     (0x01u)
	 * #define SIG_USR_STOP      (0x02u)
	 * #define SIG_TSK_A_TO_B    (0x03u)
	 * #define SIG_TSK_B_TO_A    (0x04u)
	 */

	// Điền các khai báo tại đây

	/**
	 * @brief Khai báo message queue cho các tác vụ
	 * @attention Mỗi tác vụ sẽ có một hàng đợi tin nhắn riêng biệt
	 * 						Tùy thuộc vào nhu cầu của ứng dụng để điều chỉnh kích thước của hàng đợi, 
	 * 						nhưng cần đảm bảo không vượt quá giới hạn của hệ thống UEDP
	 * @example 
	 * extern uedp_msg_t* usr_q_mem[8];
	 * extern uedp_msg_t* a_q_mem[8];
	 * extern uedp_msg_t* b_q_mem[8];
	 */

	// Điền các khai báo tại đây

	/**
	 * @brief Khai báo biến đếm hoạt động của hệ thống
	 * @attention Nên khuyến khích sử dụng biến này để theo dõi số lượng hành động 
	 * 						đã thực hiện trong hệ thống khi task_scheduler được gọi,
	 * 						đặc biệt hữu ích trong các bài test để xác nhận rằng hệ thống đang hoạt động như mong đợi 
	 * 						và để phát hiện các vấn đề tiềm ẩn như vòng lặp vô hạn hoặc tắc nghẽn trong scheduler.
	 */

	extern uint32_t system_action_count;

	/**
	 * @brief Khai báo các hàm handler cho các task
	 * @attention Mỗi tác vụ phải có một hàm handler tương ứng 
	 * 						để xử lý các tin nhắn nhận được. 
	 * 						Với các task_norm thì hàm handler 
	 * 							có định dạng `void task_norm_x_handler(uedp_msg_t* msg)`,
	 * 							trong đó x là tên tác vụ.
	 * 						Với các task_poll thì hàm handler 
	 * 							có định dạng `void task_poll_x_handler()`,
	 * 							trong đó x là tên tác vụ polling.
	 */

	// Điền các khai báo tại đây

	/**
	 * @brief Khai báo các hàm on-entry/exit cho các trạng thái FSM (nếu có)
	 */

	// Điền các khai báo tại đây

	/**
	 * @brief Khai báo các hàm on-state cho các trạng thái TSM (nếu có)
	 */

	// Điền các khai báo tại đây

	/**
	 * @brief Khai báo các state_handler cho các trạng thái FSM (nếu có)
	 */

	// Điền các khai báo tại đây

	/**
	 * @brief Khai báo khác (nếu có)
	 */

#endif //__APP_DECL_H__

