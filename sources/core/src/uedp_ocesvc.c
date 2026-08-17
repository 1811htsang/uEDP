/**
 * @file uedp_ocesvc.c
 * @author Shang Huang
 * @brief Implementation of OCE Service (OCE)
 * @version 0.1
 * @date 2026-08-04
 * @copyright Copyright (c) 2026
 */
#include <stdint.h>
#include <stdbool.h>
#include "uedp_core.h"
#include "uedp_task.h"
#include "llist.h"
#include "uedp_ocesvc.h"
#include "uedp_fcr.h"

sta ocesvc_ctrl_t ocesvc_ctrl; // Bộ điều khiển dịch vụ OCE
sta ocesvc_t head = {
  .context = NULL,
  .handler = NULL,
  .dbugid = UINT8_MAX, // dbugid sentinel được giữ riêng cho node đầu danh sách
  .next = NULL,
  .state = OCESVC_STATE_IDLE
}; // Node đầu tiên của danh sách liên kết đơn, dùng làm sentinel cho danh sách rỗng
sta llist_t ocesvc_list = {NULL}; // Danh sách liên kết đơn cho các dịch vụ OCE

static void ocesvc_sync_fill_size(void) {
  if (ocesvc_list.size == 0U) {
    ocesvc_ctrl.fill_size = 0U;
    return;
  }

  ocesvc_ctrl.fill_size = (uint8_t)(ocesvc_list.size - 1U);
}

void ocesvc_register(ocesvc_t* svc) {
  if (svc == NULL) {
    UEDP_FCR_RAISE_MSG(UEDP_FCR_OCE_INVALID_SVC, "register: null svc");
    return;
  }

  if (svc == &head) {
    UEDP_FCR_RAISE_MSG(UEDP_FCR_OCE_INVALID_SVC, "register: svc is sentinel head");
    return;
  }

  uint32_t previous_size = ocesvc_list.size;
  llist_append(&ocesvc_list, svc);
  if (ocesvc_list.size == previous_size) {
    UEDP_FCR_RAISE(UEDP_FCR_OCE_APPEND_FAILED);
    return;
  }

  svc->state = OCESVC_STATE_READY;
  svc->next = NULL;
  ocesvc_sync_fill_size();
}

void ocesvc_unregister(ocesvc_t* svc) {
  if (svc == NULL) {
    UEDP_FCR_RAISE_MSG(UEDP_FCR_OCE_INVALID_SVC, "unregister: null svc");
    return;
  }

  if (svc == &head) {
    UEDP_FCR_RAISE_MSG(UEDP_FCR_OCE_INVALID_SVC, "unregister: svc is sentinel head");
    return;
  }

  // Xóa dịch vụ OCE khỏi danh sách liên kết đơn
  if (llist_remove(&ocesvc_list, svc)) {
    svc->state = OCESVC_STATE_IDLE;
    svc->next = NULL;
    ocesvc_sync_fill_size();
  }
}

void ocesvc_scheduler() {
  // Lấy head của danh sách liên kết đơn và duyệt qua từng dịch vụ OCE
  llist_node_t* current = ocesvc_list.head;
  if (current == NULL) {
    //NOTE - Minh: Chỉ xảy ra nếu gọi trước ocesvc_ctrl_init() - hiếm gặp, không cần raise FCR ở hot-path.
    //FIXME - Sang: Chỗ này vẫn cần raise với message để đảm bảo cover. -done
    UEDP_FCR_RAISE_MSG(UEDP_FCR_OCE_NOT_INIT, "scheduler: list is null before init");
    return; // Nếu danh sách rỗng, không làm gì cả
  }
  // Chỉ service đầu tiên trong danh sách có trạng thái READY mới được thực thi
  while (current != NULL) {
    ocesvc_t* svc = (ocesvc_t*)current->data;
    if (svc != NULL && svc->state == OCESVC_STATE_READY && svc->handler != NULL) {
      svc->state = OCESVC_STATE_RUNNING;
      svc->handler(svc);
      // Sau khi thực thi xong, chuyển trạng thái sang COMPLETED
      svc->state = OCESVC_STATE_COMPLETED;
      return;
    }
    current = current->next;
  }
}

void ocesvc_ctrl_init() {
  ocesvc_ctrl.head = &head;
  ocesvc_ctrl.fill_size = 0;
  head.context = NULL;
  head.handler = NULL;
  head.dbugid = UINT8_MAX;
  head.next = NULL;
  head.state = OCESVC_STATE_IDLE;
  llist_init(&ocesvc_list);
  llist_append(&ocesvc_list, &head); // Thêm node đầu tiên vào danh sách liên kết đơn
}
