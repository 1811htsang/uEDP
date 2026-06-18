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
#include "fifo.h"

void pal_memrp_report(pal_memrp_info_t* info) {
  if (info == NULL) {
    return; // Nếu thông tin không hợp lệ, không thực hiện báo cáo
  }

  internal_uedp_msg_pool_get_info(info->type, info);

  // Sau khi điền đầy đủ thông tin vào cấu trúc info, in báo cáo ra console hoặc gửi đến hệ thống giám sát
  printf("[MEMRP] Name: %s, Used: %d, Max Used: %d, Total: %d\n",
         info->name, info->used, info->max_used, info->total);
}