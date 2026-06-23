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
	 * @brief Khai báo tác vụ norm
	 * @attention Tác vụ phải khai báo đúng định dạng `0xEx`
	 * 						x bắt đầu từ 6 trở đi. Giới hạn tối đa là 0xEF (15 tác vụ)
	 * @example
	 * #define TASK_NORM_A_ID (0xE6u)
	 * #define TASK_NORM_B_ID (0xE7u)
	 */
	// KCONFIG_DECL_TASK_NORM_START
	// KCONFIG_DECL_TASK_NORM_END

	/**
	 * @brief Khai báo tác vụ poll
	 * @attention Tác vụ phải khai báo đúng định dạng `0xDx`
	 * 						x bắt đầu từ 4 trở đi. Giới hạn tối đa là 0xDF (8 tác vụ)
	 * @example
	 * #define TASK_POLL_A_ID (0xD4u)
	 * #define TASK_POLL_B_ID (0xD5u)
	 */
	// KCONFIG_DECL_TASK_POLL_START
	// KCONFIG_DECL_TASK_POLL_END

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
	// KCONFIG_DECL_SIGNAL_START
	// KCONFIG_DECL_SIGNAL_END

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
	// KCONFIG_DECL_MSG_QUEUE_START
	// KCONFIG_DECL_MSG_QUEUE_END

	/**
	 * @brief Khai báo biến đếm hoạt động của hệ thống
	 * @attention Nên khuyến khích sử dụng biến này để theo dõi số lượng hành động 
	 * 						đã thực hiện trong hệ thống khi task_scheduler được gọi,
	 * 						đặc biệt hữu ích trong các bài test để xác nhận rằng hệ thống đang hoạt động như mong đợi 
	 * 						và để phát hiện các vấn đề tiềm ẩn như vòng lặp vô hạn hoặc tắc nghẽn trong scheduler.
	 */

	extern uint32_t system_action_count;

	/**
	 * @brief Khai báo các hàm handler cho các task norm
	 */
	// KCONFIG_DECL_TASK_NORM_HANDLER_START
	// KCONFIG_DECL_TASK_NORM_HANDLER_END

	/**
	 * @brief Khai báo các hàm handler cho các task poll
	 */
	// KCONFIG_DECL_TASK_POLL_HANDLER_START
	// KCONFIG_DECL_TASK_POLL_HANDLER_END

	/**
	 * @brief Khai báo các hàm on-entry/exit cho các trạng thái TSM (nếu có)
	 */
	// KCONFIG_DECL_TSM_ENTRY_EXIT_START
	// KCONFIG_DECL_TSM_ENTRY_EXIT_END

	/**
	 * @brief Khai báo các hàm on-state cho các trạng thái TSM (nếu có)
	 */
	// KCONFIG_DECL_TSM_STATE_START
	// KCONFIG_DECL_TSM_STATE_END

	/**
	 * @brief Khai báo các state_handler cho các trạng thái FSM (nếu có)
	 */
	// KCONFIG_DECL_FSM_STATE_HANDLERS_START
	// KCONFIG_DECL_FSM_STATE_HANDLERS_END

	/**
	 * @brief Khai báo khác (nếu có)
	 */

#endif //__APP_DECL_H__

