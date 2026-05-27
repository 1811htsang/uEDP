/**
 * @file app_cfg.h
 * @author Shang Huang
 * @brief Application configuration header file
 * @version 0.1
 * @date 2026-05-12
 * @copyright MIT License
 */
#ifndef __APP_CFG_H__
	#define __APP_CFG_H__

	/**
	 * @brief Khai báo thư viện sử dụng
	 */

	// Điền các thư viện cần thiết cho ứng dụng tại đây

	/**
	 * @brief Khai báo danh sách tác vụ và tác vụ polling của ứng dụng
	 * @attention Với n tác vụ thì có n entry trong bảng tác vụ và tác vụ polling
	 * @attention Lưu ý rằng CIEDPC_TASK_NORM_USR_ID và CIEDPC_TASK_NORM_EOT_ID là 2 ID đặc biệt bắt buộc phải có
	 * @note Người dùng tự định nghĩa bảng tác vụ và tác vụ polling nhưng vẫn giữ nguyên
	 * 			 khai báo tên bảng tác vụ và tác vụ polling. Phần implementation nội bộ
	 * 			 do người dùng tự triển khai hoặc có thể giữ nguyên nếu không sử dụng tác vụ polling.
	 * @example 
	 * task_norm_t app_task_table[] = {
	 * 	{ CIEDPC_TASK_NORM_USR_ID,  CIEDPC_TASK_PRI_LEVEL_8, task_norm_usr_handler, {0}, usr_q_mem  },
	 * 	{ TASK_NORM_A_ID,           CIEDPC_TASK_PRI_LEVEL_7, task_norm_a_handler,   {0}, a_q_mem    },
	 * 	{ TASK_NORM_B_ID,           CIEDPC_TASK_PRI_LEVEL_6, task_norm_b_handler,   {0}, b_q_mem    },
	 * 	{ CIEDPC_TASK_NORM_EOT_ID,  CIEDPC_TASK_PRI_LEVEL_0, NULL,                  {0}, NULL       }
	 * };
	 *
	 * task_poll_t app_poll_table[] = {
	 * 	{ CIEDPC_TASK_POLL_MEMRP_ID , 0, task_poll_memrp_handler },
	 * 	{ CIEDPC_TASK_POLL_EOT_ID, 0, NULL }
	 * };
	 */

	extern task_norm_t app_task_table[];
	extern task_poll_t app_poll_table[];

	/**
	 * @brief Định nghĩa bảng chuyển trạng thái cho state
	 * @attention Với n state thì có n bảng chuyển trạng thái
	 * @note Người dùng tự định nghĩa bảng, có thể xóa dòng 47 và thay thế bằng
	 * 			 triển khai của người dùng hoặc có thể giữ nguyên nếu không sử dụng TSM
	 * @example
	 * const tsm_trans_t blink_active_trans[] = {
	 * { SIG_INTERNAL_TICK, CIEDPC_TSM_STATE_STAY, fn_active_logic },
	 * { SIG_USR_STOP,      STATE_BLINK_IDLE,      NULL },
	 * { SIG_USR_START,     CIEDPC_TSM_STATE_STAY, NULL } // Đã ACTIVE rồi thì START đứng yên
	 * };
	 */

	extern const tsm_trans_t state_trans_table[];

	/**
	 * @brief Định nghĩa bảng TSM cho task Blinker
	 * @attention Với n state thì có n entry trong bảng TSM
	 * @note Người dùng tự định nghĩa bảng, có thể xóa dòng 61 và thay thế bằng
	 * 			 triển khai của người dùng hoặc có thể giữ nguyên nếu không sử dụng TSM
	 * @example
	 * const tsm_state_desc_t blinker_tsm_table[] = {
	 * 	{ STATE_BLINK_IDLE,   fn_on_idle_entry,   NULL,              blink_idle_trans,   1 },
	 * 	{ STATE_BLINK_ACTIVE, fn_on_active_entry, fn_on_active_exit, blink_active_trans, 2 }
	 * };
	 */

	extern const tsm_state_desc_t tsm_table[];

	/**
	 * @brief Định nghĩa TSM cho tác vụ
	 * @attention Với n tác vụ sử dụng TSM thì có n định nghĩa TSM.
	 * 						Tuy nhiên mỗi task không nhất thiết phải sử dụng TSM
	 * @note Người dùng tự định nghĩa TSM, có thể xóa dòng 71 và thay thế bằng
	 * 			 triển khai của người dùng hoặc có thể giữ nguyên nếu không sử dụng TSM cho tác vụ
	 */

	extern ciedpc_tsm_t app_tsm;

	/**
	 * @brief Định nghĩa FSM cho tác vụ
	 * @attention Với n tác vụ sử dụng FSM thì có n định nghĩa FSM.
	 * 						Tuy nhiên mỗi task không nhất thiết phải sử dụng FSM
	 * @note Người dùng tự định nghĩa FSM, có thể xóa dòng 81 và thay thế bằng
	 * 			 triển khai của người dùng hoặc có thể giữ nguyên nếu không sử dụng FSM cho tác vụ
	 */

	extern ciedpc_fsm_t app_fsm;

#endif //__APP_CFG_H__

