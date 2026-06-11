/**
 * @file pal_logdp.h
 * @author Shang Huang
 * @brief Platform Abstraction Layer for Log Dispatching in UEDP
 * @version 0.1
 * @date 2026-05-24
 * @copyright MIT License
 */
#ifndef __PAL_LOGDP_H__
  #define __PAL_LOGDP_H__

  /**
   * @brief Khai báo thư viện sử dụng
   */

  #include "pal_core.h"
  
  /**
   * @brief Forward declaration cho itnlog
   */
  typedef struct uedp_itnlog_entry_t uedp_itnlog_entry_t;

  /**
   * @brief Định nghĩa kiểu hàm callback để xuất dữ liệu log
   */
  typedef void (*logdp_output_fn)(uedp_itnlog_entry_t* entry);

  /**
   * @brief Hàm để đăng ký hàm callback xuất dữ liệu log
   * @param output_fn Hàm callback được đăng ký để xuất dữ liệu log,
   */
  void pal_logdp_register(logdp_output_fn output_fn);

  /**
   * @brief Hàm để hủy đăng ký hàm callback xuất dữ liệu log
   * @param output_fn Hàm callback được hủy đăng ký để xuất dữ liệu log
   */
  void pal_logdp_unregister(logdp_output_fn output_fn);

  /**
   * @brief Hàm để dispatch log entry từ internal logger đến các hàm callback đã đăng ký
   * @param entry Con trỏ đến log entry được gửi từ internal logger để dispatch đến các hàm callback đã đăng ký
   */
  void pal_logdp_dispatch(uedp_itnlog_entry_t* entry);

#endif // __PAL_LOGDP_H__
