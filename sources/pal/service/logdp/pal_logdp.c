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
#include "uedp_fcr.h"
#include "pal_core.h"
#include "pal_logdp.h"

/**
 * @brief Khai báo bảng đăng ký các hàm callback để xuất dữ liệu log
 */
sta logdp_output_fn logdp_output_fns[LOGDP_MAX_OUTPUT_FN] = {0}; // Bảng đăng ký các hàm callback để xuất dữ liệu log

void pal_logdp_register(logdp_output_fn output_fn) {
  for (ui8 i = 0; i < LOGDP_MAX_OUTPUT_FN; i++) {
    if (logdp_output_fns[i] == NULL) {
      logdp_output_fns[i] = output_fn; // Đăng ký hàm callback vào bảng
      break;
    }
  }
  // Nếu bảng đã đầy, đưa qua FCR để ghi log + xử lý theo bảng hành động (SYS_PANIC)
  if (logdp_output_fns[LOGDP_MAX_OUTPUT_FN - 1] != NULL) {
    UEDP_FCR_RAISE(UEDP_FCR_PAL_LOGDP_TABLE_FULL);
  }
}

void pal_logdp_unregister(logdp_output_fn output_fn) {
  for (ui8 i = 0; i < LOGDP_MAX_OUTPUT_FN; i++) {
    if (logdp_output_fns[i] == output_fn) {
      logdp_output_fns[i] = NULL; // Hủy đăng ký hàm callback
      break;
    }
  }
  // Nếu không tìm thấy thì thôi, không cần xử lý gì thêm
}

void pal_logdp_dispatch(uedp_itnlog_entry_t* entry) {
  for (ui8 i = 0; i < LOGDP_MAX_OUTPUT_FN; i++) {
    if (logdp_output_fns[i] != NULL) {
      logdp_output_fns[i](entry); // Gọi hàm callback để xuất dữ liệu log
    }
  }
}
