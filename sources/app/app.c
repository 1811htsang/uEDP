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
// Điền các khai báo tại đây

/**
 * @brief Định nghĩa các buffer dữ liệu để truyền các tin nhắn kích thước lớn
 *        như dạng chuỗi hoặc struct phức tạp. 
 *        Việc này giúp giảm tải bộ nhớ bằng cách không sao chép dữ liệu 
 *        mà chỉ truyền địa chỉ của biến chứa dữ liệu.
 */
// Điền các khai báo tại đây

/**
 * @brief Định nghĩa bảng task NORM
 */
// Điền các khai báo tại đây

/**
 * @brief Định nghĩa bảng task POLL
 */
// Điền các khai báo tại đây

/**
 * @brief Định nghĩa bảng chuyển trạng thái cho state (trans_desc)
 */
// Điền các khai báo tại đây

/**
 * @brief Định nghĩa bảng TSM (state_desc)
 */
// Điền các khai báo tại đây

/**
 * @brief Định nghĩa TSM cho tác vụ (obj)
 */
// Điền các khai báo tại đây

/**
 * @brief Định nghĩa FSM cho tác vụ (obj)
 */
// Điền các khai báo tại đây

/**
 * @brief Các khai báo tham số khác (nếu có)
 */

// Điền các khai báo tại đây

/**
 * @brief Khai báo các hàm khác (nếu có)
 */

// Điền các khai báo tại đây

/**
 * @brief Định nghĩa handler cho task norm
 */

// Điền các khai báo tại đây

/**
 * @brief Định nghĩa handler cho task poll
 */

// Điền các khai báo tại đây

/**
 * @brief Định nghĩa hàm on-entry/exit cho các trạng thái của TSM
 */

// Điền các khai báo tại đây

/**
 * @brief Định nghĩa hàm on-state cho các trạng thái của TSM
 */

// Điền các khai báo tại đây

/**
 * @brief Định nghĩa các state handler cho FSM
 */

// Điền các khai báo tại đây

/**
 * @brief Định nghĩa hàm khác
 */

// Điền các khai báo tại đây
