/**
 * @file test.c
 * @author Shang Huang
 * @brief Test case 01: FSM đơn giản cho Task USR và TSM cho Task Blinker
 * @version 0.1
 * @date 2026-05-03
 * @copyright MIT License
 */
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "test.h"
#include "ciedpc_core.h"
#include "ciedpc_task.h"
#include "ciedpc_tsm.h"
#include "ciedpc_fsm.h"
#include "ciedpc_msg.h"
#include "ciedpc_timer.h"

/**
 * @brief Khai báo static message queue cho task USR (entry point), task Controller và task Blinker
 */

static ciedpc_msg_t* ctrl_q_mem[8];
static ciedpc_msg_t* blink_q_mem[8];

/**
 * @brief Định nghĩa các on-entry/exit cho STATE_BLINK_ACTIVE
 */

void fn_on_active_exit(ciedpc_msg_t* msg) {
  (void)msg;
  printf("[Blinker] -> Exiting ACTIVE Mode. Stopping Timer...\n");
  // Bắt buộc phải gỡ timer để tránh nhận tin nhắn thừa trong IDLE
  ciedpc_timer_remove(TASK_NORM_BLINKER_ID, SIG_INTERNAL_TICK);
}

void fn_on_active_entry(ciedpc_msg_t* msg) {
  (void)msg;
  printf("[Blinker] -> Entered ACTIVE Mode. Starting Timer 1000ms...\n");
  ciedpc_timer_set(TASK_NORM_BLINKER_ID, SIG_INTERNAL_TICK, 1000, CIEDPC_TIMER_PERIODIC);
}

/**
 * @brief Định nghĩa hàm on-entry cho STATE_BLINK_IDLE
 */

void fn_on_idle_entry(ciedpc_msg_t* msg) { 
  (void)msg;
  printf("[Blinker] -> Entered IDLE Mode. Waiting for Start...\n");
}

/**
 * @brief Định nghĩa hàm on-state cho TSM
 */

void fn_active_logic(ciedpc_msg_t* msg) {
  (void)msg;
  printf("[Blinker] Processing Tick event. State remains ACTIVE.\n");
}

/**
 * @brief Định nghĩa bảng chuyển trạng thái khi IDLE cho TSM
 */

const tsm_trans_t blink_idle_trans[] = {
  { SIG_USR_START, STATE_BLINK_ACTIVE, NULL },
  { SIG_USR_STOP,  CIEDPC_TSM_STATE_STAY, NULL } // Đã IDLE rồi thì STOP đứng yên
};

/**
 * @brief Định nghĩa bảng chuyển trạng thái khi ACTIVE cho TSM
 */

const tsm_trans_t blink_active_trans[] = {
  { SIG_INTERNAL_TICK, CIEDPC_TSM_STATE_STAY, fn_active_logic },
  { SIG_USR_STOP,      STATE_BLINK_IDLE,      NULL },
  { SIG_USR_START,     CIEDPC_TSM_STATE_STAY, NULL } // Đã ACTIVE rồi thì START đứng yên
};

/**
 * @brief Định nghĩa bảng TSM cho task Blinker
 */

const tsm_state_desc_t blinker_tsm_table[] = {
  { STATE_BLINK_IDLE,   fn_on_idle_entry,   NULL,              blink_idle_trans,   1 },
  { STATE_BLINK_ACTIVE, fn_on_active_entry, fn_on_active_exit, blink_active_trans, 2 }
};

/**
 * @brief Định nghĩa TSM cho task Blinker
 */

static ciedpc_tsm_t blinker_tsm;

/**
 * @brief Định nghĩa các handler cho các task
 */

void task_blinker_handler(ciedpc_msg_t* msg) {
  ciedpc_tsm_dispatch(&blinker_tsm, msg);
}

void task_controller_handler(ciedpc_msg_t* msg) {
  if (msg->sig == SIG_USR_START || msg->sig == SIG_USR_STOP) {
    printf("[Controller] Relaying Signal 0x%02X to Blinker\n", msg->sig);
    ciedpc_msg_t* m = ciedpc_msg_alloc(TASK_NORM_BLINKER_ID, msg->sig, 0);
    if (m) ciedpc_task_norm_post_msg(TASK_NORM_BLINKER_ID, m);
  }
}

/**
 * @brief Định nghĩa bảng task tổng
 */

task_norm_t app_task_table[] = {
  { TASK_NORM_CONTROLLER_ID, CIEDPC_TASK_PRI_LEVEL_8, {0}, {0}, task_controller_handler, {0}, ctrl_q_mem  },
  { TASK_NORM_BLINKER_ID,    CIEDPC_TASK_PRI_LEVEL_6, {0}, {0}, task_blinker_handler,    {0}, blink_q_mem },
  { CIEDPC_TASK_NORM_EOT_ID, CIEDPC_TASK_PRI_LEVEL_0, {0}, {0}, NULL,                    {0}, NULL        }
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

  // Khởi động Core
  ciedpc_core_init();

  // Khởi tạo Message Pool, Timer và Task
  ciedpc_msg_pool_init();
  ciedpc_timer_init();
  ciedpc_task_norm_create(app_task_table);

  // Khởi tạo TSM cho task Blinker
  ciedpc_tsm_init(&blinker_tsm, blinker_tsm_table, 2, STATE_BLINK_IDLE, NULL);

  // Tạo luồng giả lập Tick
  pthread_t tick_tid;
  pthread_create(&tick_tid, NULL, linux_tick_thread, NULL);

  // Giả lập tín hiệu bắt đầu
  printf("[System] Simulating External Interrupt: Start Button Pressed...\n");
  ciedpc_task_norm_post_isr(TASK_NORM_CONTROLLER_ID, SIG_USR_START);  

  while (1) {
    ciedpc_task_scheduler();
    usleep(100); // Sleep để tránh CPU hogging
  }

  return 0;
}