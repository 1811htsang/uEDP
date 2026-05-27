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
#include "ciedpc_core.h"
#include "ciedpc_task.h"
#include "ciedpc_msg.h"
#include "ciedpc_timer.h"
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
 * @brief Định nghĩa bảng task 
 */

// Điền các khai báo tại đây

/**
 * @brief Các khai báo khác (nếu có)
 */

// Điền các khai báo tại đây

/**
 * @brief Khai báo các hàm khác (nếu có)
 */

// Điền các khai báo tại đây

/**
 * @brief Định nghĩa handler cho task USR, task A và task B
 */

// Điền các khai báo tại đây

/**
 * @brief Định nghĩa các hàm on-entry/exit cho các trạng thái của TSM (nếu có)
 */

// Điền các khai báo tại đây

/**
 * @brief Định nghĩa các state handler cho FSM của task USR, task A và task B (nếu có)
 */

// Điền các khai báo tại đây

/**
 * @brief Định nghĩa các hàm khác (nếu có)
 */

// Điền các khai báo tại đây