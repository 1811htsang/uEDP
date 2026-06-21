/**
 * @file esp32_wr32_arch.c
 * @author Shang Huang
 * @brief Implementation of ESP32-WR32 Architecture Abstraction Layer for UEDP
 * @version 0.1
 * @date 2026-04-20
 * @copyright MIT License
 */

#include "inttypes.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "esp32_wr32_arch.h"
#include "uedp_core.h"
#include "uedp_task.h"
#include "uedp_msg.h"
#include "uedp_timer.h"
#include "pal_memrp.h"

/**
 * @brief Khai báo spinlock để bảo vệ trong môi trường đa nhân
 */

// static portMUX_TYPE uedp_mux = portMUX_INITIALIZER_UNLOCKED;

/**
 * @brief Khai báo biến toàn cục kiểm tra hệ thống khởi động
 */

sta ui8 system_inited = 0;

/**
 * @brief Đảm bảo uedp_timer_tick được biết đến
 */

extern void uedp_timer_tick(void);

/**
 * @brief Implementation cho uedp_core.h
 */

void uedp_core_init(void) {
  pal_core_init();
  uedp_msg_pool_init();
  uedp_timer_init();
  system_inited = 1;
}

/**
 * @brief Implementation cho pal_core.h
 */

void pal_core_init(void) {
  pal_esp32_wr32_init_env();
}

void pal_enter_critical(void) {

}

void pal_exit_critical(void) {

}

ui8 pal_math_get_highest_bit32(ui32 mask) {

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
 * @brief Implementation cho esp32_wr32_arch.h
 */

void timer_callback(void* arg) {
  if (system_inited) {
    uedp_timer_tick();
  }
}

void pal_esp32_wr32_init_env(void) {
  
}

void pal_esp32_wr32_idle_sleep(void) {
	esp_light_sleep_start();
}

void pal_memrp_get_sys_info(ui32 *rom_used, ui32 *ram_used, ui32 *stack_curr) {
  
}


