/**
 * @file test.c
 * @author Shang Huang
 * @brief Test case 02: TSM đơn giản cho tác vụ truyền data từ Task A sang Task B
 * @version 0.1
 * @date 2026-04-26
 * @copyright MIT License
 */
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "test.h"
#include "ciedpc_core.h"
#include "ciedpc_task.h"
#include "ciedpc_msg.h"
#include "ciedpc_timer.h"
#include "ciedpc_fsm.h"

/**
 * @brief Khai báo static message queue cho task USR (entry point), task A và task B
 * @attention Mỗi tác vụ sẽ có một hàng đợi tin nhắn riêng biệt với kích thước tối đa là 8 tin nhắn,
 *            có thể điều chỉnh tùy theo nhu cầu của bài test, nhưng cần đảm bảo không vượt quá giới hạn của hệ thống CIEDPC.
 */

sta ciedpc_msg_t* usr_q_mem[8];
sta ciedpc_msg_t* a_q_mem[8];
sta ciedpc_msg_t* b_q_mem[8];

/**
 * @brief Khai báo các chuỗi dữ liệu để truyền giữa Task A và Task B
 */

sta const char* data_a_to_b = "Hello from Task A!";
sta const char* data_b_to_a = "Hello from Task B!";

/**
 * @brief Định nghĩa các FSM cho các task
 */

sta ciedpc_fsm_t fsm_usr;
sta ciedpc_fsm_t fsm_a;
sta ciedpc_fsm_t fsm_b;

/**
 * @brief Declaration của các state handler cho FSM của task USR, task A và task B
 */

sta void usr_state_idle(ciedpc_msg_t* msg);
sta void usr_state_active(ciedpc_msg_t* msg);
sta void task_a_state_idle(ciedpc_msg_t* msg);
sta void task_a_state_active(ciedpc_msg_t* msg);
sta void task_b_state_idle(ciedpc_msg_t* msg);
sta void task_b_state_active(ciedpc_msg_t* msg);

/**
 * @brief Định nghĩa các trạng thái FSM
 */

void usr_state_idle(ciedpc_msg_t* msg) {
  switch (msg->sig) {
    case CIEDPC_FSM_SIG_INIT:
      printf("[USR] Initializing FSM. Entering IDLE state...\n");
      break;
    case CIEDPC_FSM_SIG_EXIT:
      printf("[USR] Exiting IDLE state...\n");
      break;
    case CIEDPC_FSM_SIG_ENTRY:
      printf("[USR] Entering IDLE state. Waiting for START signal...\n");
      break;
    case SIG_USR_START:
      printf("[USR] Received START signal. Transitioning to ACTIVE state...\n");
      ciedpc_fsm_go_next(&fsm_usr, usr_state_active);
      break;
    default:
      printf("[USR] Encountered unexpected signal in IDLE state: %x\n", msg->sig);
      break;
  }
}

void usr_state_active(ciedpc_msg_t* msg) {
  switch (msg->sig) {
    case CIEDPC_FSM_SIG_EXIT:
      printf("[USR] Exiting ACTIVE state...\n");
      break;
    case CIEDPC_FSM_SIG_ENTRY:
      printf("[USR] Entering ACTIVE state. System is now active.\n");
      // Thực hiện gửi SIG_USR_START tới task A để kích hoạt chuỗi hành động
      ciedpc_msg_t* msg_to_a = ciedpc_msg_alloc(TASK_NORM_A_ID, SIG_USR_START, 0);
      ciedpc_task_norm_post_msg(TASK_NORM_A_ID, msg_to_a);
      printf("[USR] Sent START signal to Task A. Waiting for further signals...\n");
      break;
    case SIG_USR_STOP:
      printf("[USR] Received STOP signal. Transitioning to IDLE state...\n");
      ciedpc_fsm_go_next(&fsm_usr, usr_state_idle); 
      /**
       * @brief Có thể dùng ciedpc_fsm_go_back(&fsm_usr) để quay lại trạng thái trước đó, 
       *        nhưng ở context này thì go_next sẽ trực quan hơn 
       *        để thể hiện rõ ràng việc chuyển đổi trạng thái từ ACTIVE về IDLE 
       *        khi nhận được tín hiệu STOP.
       */
      break;
    default:
      printf("[USR] Encountered unexpected signal in ACTIVE state: %x\n", msg->sig);
      break;
  }
}

void task_a_state_idle(ciedpc_msg_t* msg) {
  switch (msg->sig) {
    case CIEDPC_FSM_SIG_INIT:
      printf("[TSKA] Initializing FSM. Entering IDLE state...\n");
      break;
    case CIEDPC_FSM_SIG_EXIT:
      printf("[TSKA] Exiting IDLE state...\n");
      break;
    case CIEDPC_FSM_SIG_ENTRY:
      printf("[TSKA] Entering IDLE state. Waiting for SIG_USR_START signal...\n");
      break;
    case SIG_USR_START:
      printf("[TSKA] Received START signal. Transitioning to ACTIVE state...\n");
      ciedpc_fsm_go_next(&fsm_a, task_a_state_active);
      break;
    
    default:
      printf("[TSKA] Encountered unexpected signal in IDLE state: %x\n", msg->sig);
      break;
  }
}

void task_a_state_active(ciedpc_msg_t* msg) {
  switch (msg->sig) {
    case CIEDPC_FSM_SIG_EXIT:
      printf("[TSKA] Exiting ACTIVE state...\n");
      break;
    case CIEDPC_FSM_SIG_ENTRY:
      printf("[TSKA] Entering ACTIVE state. Preparing to send message...\n");
      // Thực hiện gửi tin nhắn từ Task A sang Task B
      ciedpc_msg_t* msg_to_b = ciedpc_msg_alloc(TASK_NORM_B_ID, SIG_TSK_A_TO_B, sizeof(char*));
      ciedpc_msg_set_data_ref(msg_to_b, (char*)&data_a_to_b); // Truyền địa chỉ của chuỗi dữ liệu
      ciedpc_task_norm_post_msg(TASK_NORM_B_ID, msg_to_b);
      printf("[TSKA] Message sent to Task B. Waiting for response...\n");
      break;
    case SIG_TSK_B_TO_A:
      printf("[TSKA] Received message from Task B. Transitioning to IDLE state...\n");
      // Thực hiện in nội dung tin nhắn nhận được từ Task B
      uintptr_t received_addr = (uintptr_t)(*(char**)(msg->data));
      char* final_str = *(char**)received_addr;
      printf("[TSKA] Content: %s\n", final_str);
      ciedpc_fsm_go_next(&fsm_a, task_a_state_idle);
      /**
       * @brief Ghi chú tương tự như ở trạng thái ACTIVE của task USR,
       *        việc sử dụng go_next sẽ giúp thể hiện rõ ràng hơn việc chuyển đổi trạng thái 
       *        từ ACTIVE về IDLE sau khi nhận được phản hồi từ Task B.
       */
      break;
    default:
      printf("[TSKA] Encountered unexpected signal in ACTIVE state: %x\n", msg->sig);
      break;
  }
}

void task_b_state_idle(ciedpc_msg_t* msg) {
  switch (msg->sig) {
    case CIEDPC_FSM_SIG_INIT:
      printf("[TSKB] Initializing FSM. Entering IDLE state...\n");
      break;
    case CIEDPC_FSM_SIG_EXIT:
      printf("[TSKB] Exiting IDLE state...\n");
      break;
    case CIEDPC_FSM_SIG_ENTRY:
      printf("[TSKB] Entering IDLE state...\n");
      break;
    case SIG_TSK_A_TO_B:
      printf("[TSKB] Received message from Task A. Transitioning to ACTIVE state...\n");
      // Thực hiện in nội dung tin nhắn nhận được từ Task A
      uintptr_t received_addr = (uintptr_t)(*(char**)(msg->data));
      char* final_str = *(char**)received_addr;
      printf("[TSKB] Content: %s\n", final_str);
      ciedpc_fsm_go_next(&fsm_b, task_b_state_active);
      break;
    default:
      printf("[TSKB] Encountered unexpected signal in IDLE state: %x\n", msg->sig);
      break;
  }
}

void task_b_state_active(ciedpc_msg_t* msg) {
  switch (msg->sig) {
    case CIEDPC_FSM_SIG_EXIT:
      printf("[TSKB] Exiting ACTIVE state...\n");
      break;
    case CIEDPC_FSM_SIG_ENTRY:
      printf("[TSKB] Entering ACTIVE state. Preparing to send message back to Task A...\n");
      // Thực hiện gửi tin nhắn phản hồi từ Task B về Task A
      ciedpc_msg_t* msg_to_a = ciedpc_msg_alloc(TASK_NORM_A_ID, SIG_TSK_B_TO_A, sizeof(char*));
      ciedpc_msg_set_data_ref(msg_to_a, (char*)&data_b_to_a);
      ciedpc_task_norm_post_msg(TASK_NORM_A_ID, msg_to_a);
      printf("[TSKB] Message sent back to Task A. Return to IDLE state...\n");
      ciedpc_fsm_go_next(&fsm_b, task_b_state_idle);
       /**
       * @brief Sau khi gửi phản hồi về Task A, Task B sẽ quay lại trạng thái IDLE 
       *        để sẵn sàng nhận các tin nhắn tiếp theo từ Task A,
       *        việc sử dụng go_next sẽ giúp thể hiện rõ ràng hơn việc chuyển đổi trạng thái 
       *        từ ACTIVE về IDLE sau khi hoàn thành nhiệm vụ gửi phản hồi.
       */
      break;
    default:
      printf("[TSKB] Encountered unexpected signal in ACTIVE state: %x\n", msg->sig);
      break;
  }
}

/**
 * @brief Định nghĩa handler cho task USR, task A và task B
 */

void task_usr_handler(ciedpc_msg_t* msg) {
  ciedpc_fsm_dispatch(&fsm_usr, msg);
}

void task_a_handler(ciedpc_msg_t* msg) {
  ciedpc_fsm_dispatch(&fsm_a, msg);
}

void task_b_handler(ciedpc_msg_t* msg) {
  ciedpc_fsm_dispatch(&fsm_b, msg);
}

/**
 * @brief Định nghĩa bảng task tổng
 * @attention TASK_USR được sử dụng làm entry point để kích hoạt hệ thống, 
 *            trong khi TASK_A và TASK_B sẽ thực hiện các chức năng chính của bài test
 */

task_norm_t app_task_table[] = {
  { CIEDPC_TASK_NORM_USR_ID,  CIEDPC_TASK_PRI_LEVEL_8, {0}, {0}, task_usr_handler,  {0}, usr_q_mem  },
  { TASK_NORM_A_ID,           CIEDPC_TASK_PRI_LEVEL_7, {0}, {0}, task_a_handler,    {0}, a_q_mem    },
  { TASK_NORM_B_ID,           CIEDPC_TASK_PRI_LEVEL_6, {0}, {0}, task_b_handler,    {0}, b_q_mem    },
  { CIEDPC_TASK_NORM_EOT_ID,  CIEDPC_TASK_PRI_LEVEL_0, {0}, {0}, NULL,              {0}, NULL       }
};

/**
 * @brief Hàm giả lập Tick của hệ thống, được chạy trong một luồng riêng biệt 
 *        để mô phỏng hoạt động của timer phần cứng trong môi trường Linux
 */

void* linux_tick_thread(void* arg) {
  while (1) {
    usleep(1000); // 1ms
    ciedpc_timer_tick();
  }
  return NULL;
}

/**
 * @brief Hàm main để khởi chạy bài test
 */

int main() {
  printf("=== CIEDPC LINUX INTEGRATION TEST ===\n");

  ciedpc_core_init();

  ciedpc_msg_pool_init();
  ciedpc_timer_init();
  ciedpc_task_norm_create(app_task_table);

  pthread_t tick_tid;
  pthread_create(&tick_tid, NULL, linux_tick_thread, NULL);

  ciedpc_fsm_init(&fsm_usr, usr_state_idle);
  ciedpc_fsm_init(&fsm_a, task_a_state_idle);
  ciedpc_fsm_init(&fsm_b, task_b_state_idle);

  ciedpc_msg_t* start_msg = ciedpc_msg_alloc(CIEDPC_TASK_NORM_USR_ID, SIG_USR_START, 0);
  ciedpc_task_norm_post_msg(CIEDPC_TASK_NORM_USR_ID, start_msg);

  while (1) {
    ciedpc_task_scheduler();
    usleep(100); // Sleep để tránh CPU hogging
  }

  return 0;
}