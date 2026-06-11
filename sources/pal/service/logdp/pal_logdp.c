/**
 * @file pal_logdp.c
 * @author Shang Huang
 * @brief Implementation of the Log Dispatching service for the PAL layer in UEDP
 * @version 0.1
 * @date 2026-06-01
 * @copyright MIT License
 */
#include "uedp_core.h"
#include "uedp_itnlog.h"
#include "pal_core.h"
#include "pal_logdp.h"

/**
 * @brief Khai báo bảng đăng ký các hàm callback để xuất dữ liệu log
 */
sta logdp_output_fn logdp_output_fns[PAL_LOGDP_MAX_OUTPUT_FN] = {0}; // Bảng đăng ký các hàm callback để xuất dữ liệu log

void pal_logdp_register(logdp_output_fn output_fn) {
  for (ui8 i = 0; i < PAL_LOGDP_MAX_OUTPUT_FN; i++) {
    if (logdp_output_fns[i] == NULL) {
      logdp_output_fns[i] = output_fn; // Đăng ký hàm callback vào bảng
      break;
    }
  }
  // Nếu bảng đã đầy, có thể thêm xử lý lỗi hoặc ghi log cảnh báo ở đây
  if (logdp_output_fns[PAL_LOGDP_MAX_OUTPUT_FN - 1] != NULL) {
    pal_sys_fatal(__FILE__, __LINE__, "[LOGDP] Output function table is full."); // Ghi log lỗi nếu đã đạt đến giới hạn đăng ký
  }
}

void pal_logdp_unregister(logdp_output_fn output_fn) {
  for (ui8 i = 0; i < PAL_LOGDP_MAX_OUTPUT_FN; i++) {
    if (logdp_output_fns[i] == output_fn) {
      logdp_output_fns[i] = NULL; // Hủy đăng ký hàm callback
      break;
    }
  }
  // Nếu không tìm thấy thì thôi, không cần xử lý gì thêm
}

void pal_logdp_dispatch(uedp_itnlog_entry_t* entry) {
  for (ui8 i = 0; i < PAL_LOGDP_MAX_OUTPUT_FN; i++) {
    if (logdp_output_fns[i] != NULL) {
      logdp_output_fns[i](entry); // Gọi hàm callback để xuất dữ liệu log
    }
  }
}
