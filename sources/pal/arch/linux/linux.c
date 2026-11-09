//ANCHOR - Specific configuration for Linux
#define _GNU_SOURCE
#define _POSIX_C_SOURCE 199309L

//ANCHOR - Architecture-specific implementation for STM32F103 microcontroller
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <time.h>
#include "linux.h"
#include "uedp_core.h"
#include "uedp_task.h"
#include "uedp_msg.h"
#include "uedp_timer.h"
#include "uedp_itnlog.h"
#include "uedp_fcr.h"

//ANCHOR - Khai báo thư viện encrypt
#include "libcrc8.h"

//ANCHOR - Khai báo thư viện hệ thống
#include "pal_core.h"
#include "pal_logdp.h"
#include "pal_memrp.h"
#include "pal_rprintf.h"

//CRITICAL - Đảm bảo phải có ủy quyền timer_tick để gắn vào timer phần cứng
//NOTE - In Linux, using pthread to simulate periodic tick, so we need to call uedp_timer_tick() in that thread

extern void uedp_timer_tick(void);

// ANCHOR - Implementation for global mutex and tick simulation in Linux environment

sta pthread_mutex_t uedp_mutex;
sta pthread_mutexattr_t mutex_attr;
sta ui32 start_tick_ms = 0;

void internal_create_tick_thread(void);

//ANCHOR - Implementation for system action count for debug purposes

sta uint32_t i_sac = 0x0u;

//ANCHOR - Implementation cho uedp_core.h

void uedp_core_init(void) {
  pal_core_init();
  uedp_msg_pool_init();
  uedp_timer_init();
  uedp_itnlog_init();
  internal_create_tick_thread();
}

//ANCHOR - Implementation cho pal_core.h

void pal_core_init(void) {
  linux_init_env();
}

void pal_enter_critical(void) {
  pthread_mutex_lock(&uedp_mutex);
}

void pal_exit_critical(void) {
  pthread_mutex_unlock(&uedp_mutex);
}

ui8 pal_math_get_highest_bit32(ui32 mask) {
  if (mask == 0) return -1;
  /* 31 - CLZ của 32-bit mang lại vị trí bit cao nhất (0-15) */
  return (ui8)(31 - __builtin_clz((uint32_t)mask));
}

ui32 pal_sys_get_tick(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  ui32 current_ms = (ui32)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
  return current_ms - start_tick_ms;
}

void pal_sys_reset(void) {
  printf("[System] Linux Simulation: Performing System Reset...\n");
  linux_cleanup();
  exit(0); 
}

void pal_sys_fatal(const char* file, ui32 line, const char* msg) {
  fprintf(stderr, "\n[FATAL ERROR] %s\n", msg);
  fprintf(stderr, "Location: %s:%u\n", file, line);
  linux_cleanup();
  abort(); // Tạo core dump để debug
}

/** ANCHOR -  Implementation cho linux_arch.h
 * @note Có thể xóa hoặc bổ sung các hàm khác tùy theo nhu cầu của dự án
 */

void linux_init_env(void) {
  pthread_mutexattr_init(&mutex_attr);
  pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(&uedp_mutex, &mutex_attr);
  signal(SIGINT, linux_signal_handler);
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  start_tick_ms = (ui32)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

void linux_simulate_interrupt(ui8 task_id, ui8 signal) {
  uedp_task_norm_post_isr(task_id, signal);
}

//NOTE - Hàm này phải match với pthread callback
void* linux_simulate_tick_thread(void* arg) {
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 1000000; // 1ms

  while (1) {
    nanosleep(&ts, NULL);
    /* Gọi nhịp đập của Core */
    uedp_timer_tick(); 
  }
  return NULL;
}

void linux_cleanup(void) {
  pthread_mutex_destroy(&uedp_mutex);
  pthread_mutexattr_destroy(&mutex_attr);
}

void linux_signal_handler(int signum) {
  if (signum == SIGINT) {
    printf("\n[System] Caught SIGINT (Ctrl+C), exiting gracefully...\n");
    linux_cleanup();
    exit(0);
  }
}

/** ANCHOR
 * Due to the nature of Linux simulation, PPLP feature can be ignored 
 * or implemented as a no-op. The following functions are placeholders 
 * for PPLP functionality.
 */

void internal_create_tick_thread(void) {
  pthread_t tick_thread;
  pthread_create(&tick_thread, NULL, linux_simulate_tick_thread, NULL);
  pthread_detach(tick_thread);
}

void linux_get_sac_count(uint32_t count) {
  i_sac = count;
  printf("[System] Total system actions performed: %u\n", i_sac);
}