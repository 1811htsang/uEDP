/**
 * @file pal_rprintf.h
 * @author Shang Huang
 * @brief Definition of the redirect print service for the PAL layer.
 * @version 0.1
 * @date 2026-06-01
 * @copyright MIT License
 */
#ifndef __PAL_RPRINTF_H__
  #define __PAL_RPRINTF_H__

  /**
   * @brief Khai báo thư viện sử dụng
   */

  #include <stdbool.h>
  #include "uedp_core.h"
	#include "uedp_itnlog.h"
  #include "pal_core.h"

  /**
   * @brief Forward declaration cho itnlog
   */

  typedef struct uedp_itnlog_entry_t uedp_itnlog_entry_t;

  /**
   * @brief Khai báo contract cho dịch vụ redirect print của PAL layer
   * @param entry Con trỏ đến log entry được gửi từ internal logger để flush ra đích đến đã định nghĩa
   * @param init Hàm callback để khởi tạo dịch vụ redirect print
   * @param putc Hàm callback để xuất một ký tự đơn ra đích đến
   * @param write Hàm callback để xuất một chuỗi dữ liệu ra đích đến
   * @note Tùy thuộc vào BSP cụ thể mà hàm init sẽ được gọi từ trước khi Core init, do đó có thể init = NULL
   *      Nếu init = NULL, dịch vụ redirect print sẽ được coi là đã được khởi tạo và sẵn sàng để sử dụng,
   *      nhưng việc khởi tạo sẽ được thực hiện bởi người dùng hoặc BSP cụ thể trước khi sử dụng dịch vụ này.
   */
  typedef struct pal_rprintf_service_t {
    uedp_itnlog_entry_t entry;
    RETR_STAT (*init)(void);    
    void (*putc)(unsigned char c);    
    void (*write)(const uint8_t* data, uint16_t len);    
    bool (*is_ready)();
  } pal_rprintf_service_t;

  /**
   * @brief Hàm flush log entry từ internal logger và xuất ra đích đến đã định nghĩa
   * @param service Con trỏ đến dịch vụ redirect print
   */
  void pal_rprintf_flush_entry(pal_rprintf_service_t* service);
   
#endif // __PAL_RPRINTF_H__
