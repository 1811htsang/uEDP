/**
 * @file ciedpc_core.h
 * @author Shang Huang
 * @brief Core definitions and utilities for CIEDPC system
 * @version 0.1
 * @date 2026-04-16
 * 
 * @copyright MIT License
 * 
 */
#ifndef __CIEDPC_CORE_H__
  #define __CIEDPC_CORE_H__

  /**
   * @brief Khai báo thư viện sử dụng cho hệ thống CIEDPC
   */
  #include <string.h>
  #include "pal_core.h"

  /**
   * @brief Định nghĩa phiên bản của hệ thống CIEDPC
   */

  #define CIEDPC_VERSION_MAJOR    (1)
  #define CIEDPC_VERSION_MINOR    (0)
  #define CIEDPC_CORE_NAME        "CIEDPC"

  /**
   * @brief Định nghĩa các hằng số cho ID của tác vụ bình thường
   * @attention ID của tác vụ được thiết kế tuân thủ theo encoding `0xEx`, 
   *            trong đó `x` là một giá trị từ 0 đến 15 (0x0 đến 0xF),
   *            cho phép hệ thống CIEDPC quản lý tối đa 16 tác vụ khác nhau, 
   *            bao gồm các tác vụ timer, giao tiếp, hệ thống, debug, người dùng và trống.
   */
  #define CIEDPC_TASK_NORM_MAX_SIZE						(16u) 	// 16 tác vụ, từ 0 đến 15
  #define CIEDPC_TASK_NORM_TIM_ID							(0xE0) // Tác vụ timer
  #define CIEDPC_TASK_NORM_IF_ID		    			(0xE1) // Tác vụ giao tiếp
  #define CIEDPC_TASK_NORM_SYS_ID							(0xE2) // Tác vụ hệ thống (info + memrp)
  #define CIEDPC_TASK_NORM_DBG_ID							(0xE3) // Tác vụ debug
  #define CIEDPC_TASK_NORM_USR_ID							(0xE4) // Tác vụ người dùng (dùng để entry)
  #define CIEDPC_TASK_NORM_IDLE_ID						(0xE5) // Tác vụ rảnh
  #define CIEDPC_TASK_NORM_EOT_ID							(0xEF) // Kết thúc danh sách tác vụ
  #define CIEDPC_TASK_NORM_MIN_ID 						(0xE0) // ID đầu tiên
  #define CIEDPC_TASK_NORM_MAX_ID							(0xEF) // ID cuối cùng
  #define CIEDPC_TASK_NORM_OFFSET						  (0x06) // Offset để tránh trùng với các tác vụ khác

  /**
   * @brief Định nghĩa các hằng số cho ID của tác vụ poll
   * @attention ID của tác vụ được thiết kế tuân thủ theo encoding `0xDx`,
   *            trong đó `x` là một giá trị từ 0 đến 15 (0x0 đến 0xF)
   */
  #define CIEDPC_TASK_POLL_MAX_SIZE					  (8u) // 8 tác vụ, từ 0 đến 7
  #define CIEDPC_TASK_POLL_WDG_ID             (0xD0) // Tác vụ "đá" Watchdog để reset chip nếu treo
  #define CIEDPC_TASK_POLL_SYSLF_ID           (0xD1) // Tác vụ nháy LED Heartbeat (Nhịp tim hệ thống)
  #define CIEDPC_TASK_POLL_MEMRP_ID           (0xD2) // Tác vụ giám sát rò rỉ RAM hoặc tràn Stack
  #define CIEDPC_TASK_POLL_IDLE_ID            (0xD3) // Tác vụ nhàn rỗi 
  #define CIEDPC_TASK_POLL_EOT_ID         	  (0xDF) // Kết thúc danh sách tác vụ poll
  #define CIEDPC_TASK_POLL_MIN_ID         	  (0xD0) // ID đầu tiên
  #define CIEDPC_TASK_POLL_MAX_ID         	  (0xDF) // ID cuối cùng
  #define CIEDPC_TASK_POLL_OFFSET             (0x04) // Offset để tránh trùng với các tác vụ khác

  /**
   * @brief Định nghĩa các hằng số cho mức độ ưu tiên của tác vụ trong hệ thống CIEDPC
   * @attention Mức độ ưu tiên của tác vụ được thiết kế tuân thủ theo encoding `0xCx`,
   *            trong đó `x` là một giá trị từ 0 đến 15 (0x0 đến 0xF),
   *            cho phép hệ thống CIEDPC quản lý tối đa 16 mức độ ưu tiên khác nhau, 
   *            từ mức 0 (ưu tiên thấp nhất) đến mức 15 (ưu tiên cao nhất).
   */
  #define CIEDPC_TASK_PRI_MAX_SIZE				(16u)   // 16 mức ưu tiên, từ 0 đến 15
  #define CIEDPC_TASK_PRI_LEVEL_0				(0xC0u) // Ưu tiên thấp nhất
  #define CIEDPC_TASK_PRI_LEVEL_1				(0xC1u)
  #define CIEDPC_TASK_PRI_LEVEL_2				(0xC2u)
  #define CIEDPC_TASK_PRI_LEVEL_3				(0xC3u)
  #define CIEDPC_TASK_PRI_LEVEL_4				(0xC4u)
  #define CIEDPC_TASK_PRI_LEVEL_5				(0xC5u)
  #define CIEDPC_TASK_PRI_LEVEL_6				(0xC6u)
  #define CIEDPC_TASK_PRI_LEVEL_7				(0xC7u)
  #define CIEDPC_TASK_PRI_LEVEL_8				(0xC8u)
  #define CIEDPC_TASK_PRI_LEVEL_9				(0xC9u)
  #define CIEDPC_TASK_PRI_LEVEL_10			(0xCAu)
  #define CIEDPC_TASK_PRI_LEVEL_11			(0xCBu)
  #define CIEDPC_TASK_PRI_LEVEL_12			(0xCCu)
  #define CIEDPC_TASK_PRI_LEVEL_13			(0xCDu)
  #define CIEDPC_TASK_PRI_LEVEL_14			(0xCEu)
  #define CIEDPC_TASK_PRI_LEVEL_15			(0xCFu) // Ưu tiên cao nhất

  /**
   * @brief Các tín hiệu đặc biệt dành cho quản lý vòng đời trạng thái
   * @attention Các tín hiệu này được thiết kế tuân thủ theo encoding `0xBx`,
   *            trong đó `x` là một giá trị từ 0 đến 15 (0x0 đến 0xF),
   *            cho phép hệ thống CIEDPC quản lý tối đa 16 tín hiệu đặc biệt 
   * 						khác nhau để điều khiển vòng đời của trạng thái trong FSM,
   */
  #define CIEDPC_FSM_SIG_ENTRY    (0xB0u)
  #define CIEDPC_FSM_SIG_EXIT     (0xB1u)
  #define CIEDPC_FSM_SIG_INIT     (0xB2u)
  #define CIEDPC_FSM_SIG_LOOP     (0xB3u) // Tín hiệu để xử lý trạng thái tự trỏ chính nó.
  #define CIEDPC_FSM_SIG_MIN      (0xB0u) // ID thấp nhất
  #define CIEDPC_FSM_SIG_MAX      (0xBFu) // ID cao nhất
  #define CIEDPC_FSM_SIG_OFFSET   (0x04u) // Offset để tránh trùng với các tín hiệu khác

  /**
   * @brief Khai báo dải tín hiệu TSM
   * @attention Các tín hiệu này được thiết kế tuân thủ theo encoding `0xAx`,
   * 						trong đó `x` là một giá trị từ 0 đến 15 (0x0 đến 0xF),
   * 						cho phép hệ thống CIEDPC quản lý tối đa 16 tín hiệu
   */
  #define CIEDPC_TSM_SIG_ENTRY    (0xA0u)
  #define CIEDPC_TSM_SIG_EXIT     (0xA1u)
  #define CIEDPC_TSM_SIG_INIT     (0xA2u)
  #define CIEDPC_TSM_SIG_MIN      (0xA0u) // ID thấp nhất
  #define CIEDPC_TSM_SIG_MAX      (0xAFu) // ID cao nhất
  #define CIEDPC_TSM_SIG_OFFSET   (0x03u) // Offset để tránh trùng với các tín hiệu khác

  /**
   * @brief Khai báo dải trạng thái TSM
   * @attention Các tín hiệu này được thiết kế tuân thủ theo encoding `0xAFx`,
   * 						trong đó `x` là một giá trị từ 0 đến 15 (0x0 đến 0xF),
   * 						cho phép hệ thống CIEDPC quản lý tối đa 16 tín hiệu
   * @attention Do tồn tại các trạng thái đặc biệt như BACK, STAY và RESET, 
   *            nên dải trạng thái TSM được thiết kế có offset là 0x003 
   *            để tránh trùng lặp với các trạng thái đặc biệt này.
   */
  #define CIEDPC_TSM_STATE_BACK   (0xAF0u) // Quay lại trạng thái cũ
  #define CIEDPC_TSM_STATE_STAY   (0xAF1u) // Giữ nguyên trạng thái hiện tại
  #define CIEDPC_TSM_STATE_RESET  (0xAF2u) // Đặt lại về trạng thái ban đầu
  #define CIEDPC_TSM_STATE_MIN		(0xAF0u) // ID thấp nhất
  #define CIEDPC_TSM_STATE_MAX 		(0xAFFu) // ID cao nhất
  #define CIEDPC_TSM_STATE_OFFSET (0x003u) // Offset để tránh trùng với các trạng thái đặc biệt

  /**
   * @brief Khai báo các hằng số cho kích thước của các Pool tin nhắn
   * @attention 1 pool tin nhắn được thiết kế để chứa một số lượng tin nhắn nhất định, với kích thước dữ liệu tối đa được xác định trước,
   *            cho phép hệ thống CIEDPC quản lý bộ nhớ một cách hiệu quả. Ví dụ giả sử norm_pool[12][8] nghĩa là có thể chứa unit tin nhắn,
   * 						mỗi tin nhắn có thể chứa tối đa 8 bytes dữ liệu, mỗi bytes được chia ra như thế nào phụ thuộc vào đầu vào data_size khi khởi tạo pool
   * @attention sizeof(ciedpc_msg_t) phụ thuộc vào kiến trúc và trình biên dịch, nhưng giả sử là 32 bits.
   * @attention Kích thước nên được khai báo ở dạng bytes, không phải bit để tránh nhầm lẫn
   */

  #ifndef CIEDPC_MSG_BLANK_QUEUE_SIZE
    #define CIEDPC_MSG_BLANK_QUEUE_SIZE  (8u) 	// units
  #endif // -> equipvalent to 8 units * 4 bytes = 32 bytes

  #ifndef CIEDPC_MSG_ALLOC_QUEUE_SIZE
    #define CIEDPC_MSG_ALLOC_QUEUE_SIZE (16u)  // units
  #endif
  #ifndef CIEDPC_MSG_ALLOC_DATA_MAX
    #define CIEDPC_MSG_ALLOC_DATA_MAX   (sizeof(void*) * 2u) // auto arrange depended on architecture
  #endif // -> equipvalent to 16 units * [(sizeof(void*) * 2u) + sizeof(ciedpc_msg_t)]

  #ifndef CIEDPC_MSG_EXTAL_QUEUE_SIZE
    #define CIEDPC_MSG_EXTAL_QUEUE_SIZE  (16u) 	// units
  #endif
  #ifndef CIEDPC_MSG_EXTAL_DATA_MAX
    #define CIEDPC_MSG_EXTAL_DATA_MAX   (sizeof(void*) * 4u) // auto arrange depended on architecture
  #endif // -> equipvalent to 16 units * [(sizeof(void*) * 4u) + sizeof(ciedpc_msg_t)]

  #ifndef CIEDPC_MSG_ISR_QUEUE_SIZE
    #define CIEDPC_MSG_ISR_QUEUE_SIZE   (16u) 	// units
  #endif

  /**
   * @brief Define các kích thước của các hàng đợi tin nhắn cho các tác vụ trong hệ thống CIEDPC
   */
  #ifndef CIEDPC_TASK_MSG_QUEUE_SIZE
    #define CIEDPC_TASK_MSG_QUEUE_SIZE  (8u) // Số lượng tin nhắn tối đa trong hàng đợi của mỗi tác vụ
  #endif

  /**
   * @brief Khai báo các hằng số cho số lượng timer tối đa có thể chạy cùng lúc
   */
  #ifndef CIEDPC_TIMER_MAX_NODES
    #define CIEDPC_TIMER_MAX_NODES    (4u) // units
  #endif

  /**
   * @brief Khởi tạo toàn bộ lõi CIEDPC (Pools, Timers, Task Manager)
   */
  CIEDPC_ATTR_WEAK void ciedpc_core_init(void);

#endif // __CIEDPC_CORE_H__

