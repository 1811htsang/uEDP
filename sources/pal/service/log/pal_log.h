/**
 * @file pal_log.h
 * @author Shang Huang
 * @brief Platform Abstraction Layer for Logging in CIEDPC
 * @version 0.1
 * @date 2026-05-24
 * @copyright MIT License
 */
#ifndef __PAL_LOG_H__
  #define __PAL_LOG_H__

  /**
   * @brief Khai báo thư viện sử dụng
   */

  #include "pal_core.h"
  #include "ciedpc_itnlog.h" // Để lấy cấu trúc ciedpc_itnlog_entry_t

  /**
   * @brief Hàm để lưu log entry vào phần cứng bền vững, 
   *        được gọi bởi internal logger khi cần lưu trữ log
   * @attention Hàm này cần được triển khai bởi người dùng để phù hợp với phần cứng tùy nền tảng
   * @param entry Con trỏ đến log entry cần lưu vào phần cứng bền vững
   */
  CIEDPC_ATTR_WEAK void pal_log_save(ciedpc_itnlog_entry_t* entry);

  /**
   * @brief Hàm để tải log entry từ phần cứng bền vững, 
   *        được gọi bởi internal logger khi cần truy xuất log
   * @attention Hàm này cần được triển khai bởi người dùng để phù hợp với phần cứng tùy nền tảng
   * @param entry Con trỏ đến log entry cần tải từ phần cứng bền vững
   */
  CIEDPC_ATTR_WEAK void pal_log_load(ciedpc_itnlog_entry_t* entry);

#endif // __PAL_LOG_H__