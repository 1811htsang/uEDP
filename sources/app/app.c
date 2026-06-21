/**
 * @file app.c
 * @author Shang Huang
 * @brief Application main source file
 * @version 0.1
 * @date 2026-05-07
 * @copyright MIT License
 */

/**
 * @brief Khai báo thư viện sử dụng
 */

#include "app_cfg.h"
#include "app_decl.h"
#include "uedp_core.h"
#include "uedp_task.h"
#include "uedp_msg.h"
#include "uedp_timer.h"
#include "pal_memrp.h"

/**
 * @brief Ủy quyền sử dụng biến đếm hành động của hệ thống
 */

uint32_t system_action_count = 0x0u;

/**
 * @brief Ủy quyền sử dụng message queue cho các tác vụ đã khai báo trong app_decl.h
 * @attention Các hàng đợi tin nhắn này phải được khai báo đúng tên 
 *            và kích thước trong app_decl.h để tránh lỗi
 *            multiple definition hoặc thiếu định nghĩa khi biên dịch
 */
// KCONFIG_MSG_QUEUE_START
// KCONFIG_MSG_QUEUE_END

/**
 * @brief Định nghĩa các buffer dữ liệu để truyền các tin nhắn kích thước lớn
 *        như dạng chuỗi hoặc struct phức tạp. 
 *        Việc này giúp giảm tải bộ nhớ bằng cách không sao chép dữ liệu 
 *        mà chỉ truyền địa chỉ của biến chứa dữ liệu.
 */
// KCONFIG_MSG_BUFFER_START
// KCONFIG_MSG_BUFFER_END

/**
 * @brief Định nghĩa bảng task NORM
 */
// KCONFIG_TASK_NORM_TABLE_START
// KCONFIG_TASK_NORM_TABLE_END

/**
 * @brief Định nghĩa bảng task POLL
 */
// KCONFIG_TASK_POLL_TABLE_START
// KCONFIG_TASK_POLL_TABLE_END

/**
 * @brief Định nghĩa bảng chuyển trạng thái cho state
 */
// KCONFIG_TSM_TRANS_START
// KCONFIG_TSM_TRANS_END

/**
 * @brief Định nghĩa bảng TSM
 */
// KCONFIG_TSM_TABLE_START
// KCONFIG_TSM_TABLE_END

/**
 * @brief Định nghĩa TSM cho tác vụ
 */
// KCONFIG_TSM_TASK_START
// KCONFIG_TSM_TASK_END

/**
 * @brief Định nghĩa FSM cho tác vụ
 */
// KCONFIG_FSM_TASK_START
// KCONFIG_FSM_TASK_END

/**
 * @brief Các khai báo khác (nếu có)
 */

// Điền các khai báo tại đây

/**
 * @brief Khai báo các hàm khác (nếu có)
 */

// Điền các khai báo tại đây

/**
 * @brief Khai báo handler cho task USR, task A và task B
 */

// KCONFIG_TASK_HANDLER_DECL_START
// KCONFIG_TASK_HANDLER_DECL_END

/**
 * @brief Khai báo hàm on-entry/exit cho các trạng thái của TSM (nếu có)
 */

// KCONFIG_TSM_ENTRY_EXIT_DECL_START
// KCONFIG_TSM_ENTRY_EXIT_DECL_END

/**
 * @brief Khai báo các state handler cho FSM của task USR, task A và task B (nếu có)
 */

// KCONFIG_FSM_STATE_HANDLERS_DECL_START
// KCONFIG_FSM_STATE_HANDLERS_DECL_END

/**
 * @brief Định nghĩa handler cho task USR, task A và task B
 */

// KCONFIG_TASK_HANDLER_START
// KCONFIG_TASK_HANDLER_END

/**
 * @brief Định nghĩa các hàm on-entry/exit cho các trạng thái của TSM (nếu có)
 */

// KCONFIG_TSM_ENTRY_EXIT_START
// KCONFIG_TSM_ENTRY_EXIT_END

/**
 * @brief Định nghĩa các state handler cho FSM của task USR, task A và task B (nếu có)
 */

// KCONFIG_FSM_STATE_HANDLERS_START
// KCONFIG_FSM_STATE_HANDLERS_END
