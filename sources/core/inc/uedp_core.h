/**
 * @file uedp_core.h
 * @author Shang Huang
 * @brief Core definitions and utilities for UEDP system
 * @version 0.1
 * @date 2026-04-16
 * 
 * @copyright MIT License
 * 
 */
#ifndef __UEDP_CORE_H__
  #define __UEDP_CORE_H__

  /**
   * @brief Khai báo thư viện sử dụng cho hệ thống UEDP
   */
  #include <string.h>
  #include "core_cfg.h"
  #include "pal_core.h"

  /**
   * @brief Định nghĩa các hằng số cho ID của tác vụ bình thường
   * @attention ID của tác vụ được thiết kế tuân thủ theo encoding `0xEx`, 
   *            trong đó `x` là một giá trị từ 0 đến 15 (0x0 đến 0xF),
   *            cho phép hệ thống UEDP quản lý tối đa 16 tác vụ khác nhau, 
   *            bao gồm các tác vụ timer, giao tiếp, hệ thống, debug, người dùng và trống.
   */

  #define UEDP_TASK_NORM_MAX_SIZE						(16u) 	// 16 tác vụ, từ 0 đến 15
  #define UEDP_TASK_NORM_TIM_ID							(0xE0) // Tác vụ timer
  #define UEDP_TASK_NORM_IF_ID		    			(0xE1) // Tác vụ giao tiếp
  #define UEDP_TASK_NORM_SYS_ID							(0xE2) // Tác vụ hệ thống (info + memrp)
  #define UEDP_TASK_NORM_DBG_ID							(0xE3) // Tác vụ debug
  #define UEDP_TASK_NORM_USR_ID							(0xE4) // Tác vụ người dùng (dùng để entry)
  #define UEDP_TASK_NORM_IDLE_ID						(0xE5) // Tác vụ rảnh
  #define UEDP_TASK_NORM_EOT_ID							(0xEF) // Kết thúc danh sách tác vụ
  #define UEDP_TASK_NORM_MIN_ID 						(0xE0) // ID đầu tiên
  #define UEDP_TASK_NORM_MAX_ID							(0xEF) // ID cuối cùng
  #define UEDP_TASK_NORM_OFFSET						  (0x06) // Offset để tránh trùng với các tác vụ khác

  /**
   * @brief Định nghĩa các hằng số cho ID của tác vụ poll
   * @attention ID của tác vụ được thiết kế tuân thủ theo encoding `0xDx`,
   *            trong đó `x` là một giá trị từ 0 đến 15 (0x0 đến 0xF)
   */

  #define UEDP_TASK_POLL_MAX_SIZE					  (8u) // 8 tác vụ, từ 0 đến 7
  #define UEDP_TASK_POLL_WDG_ID             (0xD0) // Tác vụ "đá" Watchdog để reset chip nếu treo
  #define UEDP_TASK_POLL_SYSLF_ID           (0xD1) // Tác vụ nháy LED Heartbeat (Nhịp tim hệ thống)
  #define UEDP_TASK_POLL_MEMRP_ID           (0xD2) // Tác vụ giám sát rò rỉ RAM hoặc tràn Stack
  #define UEDP_TASK_POLL_IDLE_ID            (0xD3) // Tác vụ nhàn rỗi 
  #define UEDP_TASK_POLL_EOT_ID         	  (0xDF) // Kết thúc danh sách tác vụ poll
  #define UEDP_TASK_POLL_MIN_ID         	  (0xD0) // ID đầu tiên
  #define UEDP_TASK_POLL_MAX_ID         	  (0xDF) // ID cuối cùng
  #define UEDP_TASK_POLL_OFFSET             (0x04) // Offset để tránh trùng với các tác vụ khác

  /**
   * @brief Định nghĩa các hằng số cho mức độ ưu tiên của tác vụ trong hệ thống UEDP
   * @attention Mức độ ưu tiên của tác vụ được thiết kế tuân thủ theo encoding `0xCx`,
   *            trong đó `x` là một giá trị từ 0 đến 15 (0x0 đến 0xF),
   *            cho phép hệ thống UEDP quản lý tối đa 16 mức độ ưu tiên khác nhau, 
   *            từ mức 0 (ưu tiên thấp nhất) đến mức 15 (ưu tiên cao nhất).
   */

  #define UEDP_TASK_PRI_MAX_SIZE			(24u)   // 24 mức ưu tiên, từ 0 đến 23
  #define UEDP_TASK_PRI_LEVEL_0				(0xCA0u) // Ưu tiên thấp nhất
  #define UEDP_TASK_PRI_LEVEL_1				(0xCA1u)
  #define UEDP_TASK_PRI_LEVEL_2				(0xCA2u)
  #define UEDP_TASK_PRI_LEVEL_3				(0xCA3u)
  #define UEDP_TASK_PRI_LEVEL_4				(0xCA4u)
  #define UEDP_TASK_PRI_LEVEL_5				(0xCA5u)
  #define UEDP_TASK_PRI_LEVEL_6				(0xCA6u)
  #define UEDP_TASK_PRI_LEVEL_7				(0xCA7u)
  #define UEDP_TASK_PRI_LEVEL_8				(0xCA8u)
  #define UEDP_TASK_PRI_LEVEL_9				(0xCA9u)
  #define UEDP_TASK_PRI_LEVEL_10			(0xCAAu)
  #define UEDP_TASK_PRI_LEVEL_11			(0xCABu)
  #define UEDP_TASK_PRI_LEVEL_12			(0xCACu)
  #define UEDP_TASK_PRI_LEVEL_13			(0xCADu)
  #define UEDP_TASK_PRI_LEVEL_14			(0xCAEu)
  #define UEDP_TASK_PRI_LEVEL_15			(0xCAFu)
  #define UEDP_TASK_PRI_LEVEL_16			(0xCB0u)
  #define UEDP_TASK_PRI_LEVEL_17			(0xCB1u)
  #define UEDP_TASK_PRI_LEVEL_18			(0xCB2u)
  #define UEDP_TASK_PRI_LEVEL_19			(0xCB3u)
  #define UEDP_TASK_PRI_LEVEL_20			(0xCB4u)
  #define UEDP_TASK_PRI_LEVEL_21			(0xCB5u)
  #define UEDP_TASK_PRI_LEVEL_22			(0xCB6u)
  #define UEDP_TASK_PRI_LEVEL_23			(0xCB7u)
  #define UEDP_TASK_PRI_MIN_LEVEL			(0xCA0u) // Mức ưu tiên thấp nhất
  #define UEDP_TASK_PRI_MAX_LEVEL			(0xCB7u) // Mức ưu tiên cao nhất
  
  /**
   * @brief Các tín hiệu đặc biệt dành cho quản lý vòng đời trạng thái
   * @attention Các tín hiệu này được thiết kế tuân thủ theo encoding `0xBx`,
   *            trong đó `x` là một giá trị từ 0 đến 15 (0x0 đến 0xF),
   *            cho phép hệ thống UEDP quản lý tối đa 16 tín hiệu đặc biệt 
   * 						khác nhau để điều khiển vòng đời của trạng thái trong FSM,
   */
  #define UEDP_FSM_SIG_ENTRY    (0xB0u)
  #define UEDP_FSM_SIG_EXIT     (0xB1u)
  #define UEDP_FSM_SIG_INIT     (0xB2u)
  #define UEDP_FSM_SIG_LOOP     (0xB3u) // Tín hiệu để xử lý trạng thái tự trỏ chính nó.
  #define UEDP_FSM_SIG_MIN      (0xB0u) // ID thấp nhất
  #define UEDP_FSM_SIG_MAX      (0xBFu) // ID cao nhất
  #define UEDP_FSM_SIG_OFFSET   (0x04u) // Offset để tránh trùng với các tín hiệu khác

  /**
   * @brief Khai báo dải tín hiệu TSM
   * @attention Các tín hiệu này được thiết kế tuân thủ theo encoding `0xAx`,
   * 						trong đó `x` là một giá trị từ 0 đến 15 (0x0 đến 0xF),
   * 						cho phép hệ thống UEDP quản lý tối đa 16 tín hiệu
   */
  #define UEDP_TSM_SIG_ENTRY    (0xA0u)
  #define UEDP_TSM_SIG_EXIT     (0xA1u)
  #define UEDP_TSM_SIG_INIT     (0xA2u)
  #define UEDP_TSM_SIG_MIN      (0xA0u) // ID thấp nhất
  #define UEDP_TSM_SIG_MAX      (0xAFu) // ID cao nhất
  #define UEDP_TSM_SIG_OFFSET   (0x03u) // Offset để tránh trùng với các tín hiệu khác

  /**
   * @brief Khai báo dải trạng thái TSM
   * @attention Các tín hiệu này được thiết kế tuân thủ theo encoding `0xAFx`,
   * 						trong đó `x` là một giá trị từ 0 đến 15 (0x0 đến 0xF),
   * 						cho phép hệ thống UEDP quản lý tối đa 16 tín hiệu
   * @attention Do tồn tại các trạng thái đặc biệt như BACK, STAY và RESET, 
   *            nên dải trạng thái TSM được thiết kế có offset là 0x003 
   *            để tránh trùng lặp với các trạng thái đặc biệt này.
   */
  #define UEDP_TSM_STATE_BACK   (0xAF0u) // Quay lại trạng thái cũ
  #define UEDP_TSM_STATE_STAY   (0xAF1u) // Giữ nguyên trạng thái hiện tại
  #define UEDP_TSM_STATE_RESET  (0xAF2u) // Đặt lại về trạng thái ban đầu
  #define UEDP_TSM_STATE_MIN		(0xAF0u) // ID thấp nhất
  #define UEDP_TSM_STATE_MAX 		(0xAFFu) // ID cao nhất
  #define UEDP_TSM_STATE_OFFSET (0x003u) // Offset để tránh trùng với các trạng thái đặc biệt

  /**
   * @brief Khai báo các hằng số cho kích thước của các Pool tin nhắn
   * @attention 1 pool tin nhắn được thiết kế để chứa một số lượng tin nhắn nhất định, với kích thước dữ liệu tối đa được xác định trước,
   *            cho phép hệ thống UEDP quản lý bộ nhớ một cách hiệu quả. Ví dụ giả sử norm_pool[12][8] nghĩa là có thể chứa unit tin nhắn,
   * 						mỗi tin nhắn có thể chứa tối đa 8 bytes dữ liệu, mỗi bytes được chia ra như thế nào phụ thuộc vào đầu vào data_size khi khởi tạo pool
   * @attention sizeof(uedp_msg_t) phụ thuộc vào kiến trúc và trình biên dịch, nhưng giả sử là 32 bits.
   * @attention Kích thước nên được khai báo ở dạng bytes, không phải bit để tránh nhầm lẫn
   */

  #ifndef UEDP_MSG_BLANK_QUEUE_SIZE
    #define UEDP_MSG_BLANK_QUEUE_SIZE  (8u) 	// units
  #endif // -> equipvalent to 8 units * 4 bytes = 32 bytes

  #ifndef UEDP_MSG_ALLOC_QUEUE_SIZE
    #define UEDP_MSG_ALLOC_QUEUE_SIZE (16u)  // units
  #endif
  #ifndef UEDP_MSG_ALLOC_DATA_MAX
    #define UEDP_MSG_ALLOC_DATA_MAX   (sizeof(void*) * 2u) // auto arrange depended on architecture
  #endif // -> equipvalent to 16 units * [(sizeof(void*) * 2u) + sizeof(uedp_msg_t)]

  #ifndef UEDP_MSG_EXTAL_QUEUE_SIZE
    #define UEDP_MSG_EXTAL_QUEUE_SIZE  (16u) 	// units
  #endif
  #ifndef UEDP_MSG_EXTAL_DATA_MAX
    #define UEDP_MSG_EXTAL_DATA_MAX   (sizeof(void*) * 4u) // auto arrange depended on architecture
  #endif // -> equipvalent to 16 units * [(sizeof(void*) * 4u) + sizeof(uedp_msg_t)]

  #ifndef UEDP_MSG_ISR_QUEUE_SIZE
    #define UEDP_MSG_ISR_QUEUE_SIZE   (16u) 	// units
  #endif

  /**
   * @brief Define các kích thước của các hàng đợi tin nhắn cho các tác vụ trong hệ thống UEDP
   */
  #ifndef UEDP_TASK_MSG_QUEUE_SIZE
    #define UEDP_TASK_MSG_QUEUE_SIZE  (8u) // Số lượng tin nhắn tối đa trong hàng đợi của mỗi tác vụ
  #endif

  /**
   * @brief Khai báo các hằng số cho số lượng timer tối đa có thể chạy cùng lúc
   */
  #ifndef UEDP_TIMER_MAX_NODES
    #define UEDP_TIMER_MAX_NODES    (4u) // units
  #endif

  /**
   * @brief Khai báo hằng số cho kích thước của ring buffer log entry
   */

  #ifndef UEDP_ITNLOG_MAX_LOG_ENTRIES
    #define UEDP_ITNLOG_MAX_LOG_ENTRIES  (32u) // units
  #endif // -> equipvalent to 32 units * sizeof(itnlog_entry_t) bytes

  /**
   * @brief Khai báo hằng số cho ngưỡng flush log entry của internal logger
   */

  #ifndef UEDP_ITNLOG_FLUSH_THRESHOLD
    #define UEDP_ITNLOG_FLUSH_THRESHOLD  (28u) // units, ngưỡng để tự động flush log entry ra đích đến khi số lượng log entry trong ring buffer đạt đến ngưỡng này
  #endif

  /**
   * @brief Khởi tạo toàn bộ lõi UEDP (Pools, Timers, Task Manager)
   */
  UEDP_ATTR_WEAK void uedp_core_init(void);

#endif // __UEDP_CORE_H__

