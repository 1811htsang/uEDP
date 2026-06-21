/**
 * @file pal_memrp.c
 * @author Shang Huang
 * @brief Implementation of PAL Memory Reporter for UEDP project
 * @version 0.1
 * @date 2026-04-30
 * @copyright MIT License
 */

#include <string.h>
#include "uedp_core.h"
#include "pal_memrp.h"
#include "uedp_task.h"
#include "uedp_msg.h"
#include "pal_rprintf.h"
#include "fifo.h"

/**
 * @brief Khai báo biến nội bộ để lưu trữ thông tin về dịch vụ redirect print của PAL layer
 */
sta pal_rprintf_service_t internal_rprintf_service = {0};

void pal_memrp_report(pal_memrp_info_t* info) {
  if (info == NULL) {
    return; // Nếu thông tin không hợp lệ, không thực hiện báo cáo
  }

  internal_uedp_msg_pool_get_info(info->type, info);
}

void pal_memrp_set_output(pal_memrp_output_fn output_fn) {
  if (output_fn == NULL) {
    return; // Nếu hàm callback không hợp lệ, không thực hiện thiết lập
  }

  // Thiết lập hàm callback để xuất thông tin bộ nhớ ra đích đến đã định nghĩa
  internal_rprintf_service.write = (void (*)(const uint8_t*, uint16_t))output_fn;
}