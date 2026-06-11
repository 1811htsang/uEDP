/**
 * @file esp32_s3_arch.c
 * @author Shang Huang
 * @brief Implementation of ESP32-S3 Architecture Abstraction Layer for CIEDPC
 * @version 0.1
 * @date 2026-04-20
 * @copyright MIT License
 */

#include "inttypes.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "esp32_s3_arch.h"
#include "ciedpc_core.h"
#include "ciedpc_task.h"
#include "ciedpc_msg.h"
#include "ciedpc_timer.h"
#include "pal_memrp.h"

/**
 * @brief Khai báo spinlock để bảo vệ trong môi trường đa nhân
 */

// static portMUX_TYPE ciedpc_mux = portMUX_INITIALIZER_UNLOCKED;

/**
 * @brief Khai báo biến toàn cục kiểm tra hệ thống khởi động
 */

sta ui8 system_inited = 0;

/**
 * @brief Đảm bảo ciedpc_timer_tick được biết đến
 */

extern void ciedpc_timer_tick(void);

/**
 * @brief Implementation cho ciedpc_core.h
 */

void ciedpc_core_init(void) {
  pal_core_init();
  ciedpc_msg_pool_init();
  ciedpc_timer_init();
  system_inited = 1;
}

/**
 * @brief Implementation cho pal_core.h
 */

void pal_core_init(void) {
  pal_esp32_s3_init_env();
}

void pal_enter_critical(void) {

}

void pal_exit_critical(void) {

}

ui8 pal_math_get_highest_bit16(ui16 mask) {

}

ui32 pal_sys_get_tick(void) {

}

void pal_sys_reset(void) {

}

void pal_sys_fatal(const char* file, ui32 line, const char* msg) {
  printf("FATAL ERROR at %s:%" PRIu32 ": %s\n", file, line, msg);
  pal_sys_reset();
}

/**
 * @brief Implementation cho esp32_s3_arch.h
 */

void timer_callback(void* arg) {
  if (system_inited) {
    ciedpc_timer_tick();
  }
}

void pal_esp32_s3_init_env(void) {
  
}

void pal_esp32_s3_idle_sleep(void) {
	esp_light_sleep_start();
}

void pal_memrp_get_sys_info(ui32 *rom_used, ui32 *ram_used, ui32 *stack_curr) {
  
}


