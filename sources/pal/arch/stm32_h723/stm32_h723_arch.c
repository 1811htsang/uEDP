/**
 * @file stm32_h723_arch.c
 * @author Shang Huang
 * @brief Implementation of STM32 Architecture Abstraction Layer for CIEDPC
 * @version 0.1
 * @date 2026-04-20
 * @copyright MIT License
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "stm32_h723_arch.h"
#include "ciedpc_core.h"
#include "ciedpc_task.h"
#include "ciedpc_msg.h"
#include "ciedpc_timer.h"
#include "ciedpc_itnlog.h"
#include "pal_memrp.h"

sta int is_inited = 0x0u;

/**
 * @brief Đảm bảo ciedpc_timer_tick được biết đến
 */

extern void ciedpc_timer_tick(void);

/**
 * @brief Khai báo hàm nội bộ 
 */

static void internal_hardfault_decoder(uint32_t *stack);

/**
 * @brief Định nghĩa các biểu tượng linker script để quản lý bộ nhớ
 */

extern ui32 _etext;            /* End của code section (.text) 				    */
extern ui32 _sidata;           /* Start của initialized data trong FLASH 	*/
extern ui32 _sdata, _edata;    /* RAM initialized data 						        */
extern ui32 _sbss, _ebss;      /* RAM zero-init data 							        */
extern ui32 _estack;           /* Top of Stack 										        */
extern ui32 _end;              /* Start of Heap (thường sau bss) 	        */

#define FLASH_START 0x08000000  /* STM32F1 FLASH Start Address */

/**
 * @brief Định nghĩa hàm nội bộ
 * @attention Do hàm gọi trong asm của HardFault_Handler nên bổ sung thuộc tính unused 
 *            để tránh cảnh báo từ compiler về việc không sử dụng hàm này trong code C thông thường
 */

CIEDPC_ATTR_UNUSED void internal_hardfault_decoder(uint32_t *stack) {
	
}

/**
 * @brief Implementation cho ciedpc_core.h
 */

void ciedpc_core_init(void) {
  pal_core_init();
  ciedpc_msg_pool_init();
  ciedpc_timer_init();
  ciedpc_itnlog_init();
  is_inited = 0x1u;
}

/**
 * @brief Implementation cho pal_core.h
 */

void pal_core_init(void) {
  pal_stm32_h723_init_env();
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

}

/**
 * @brief Implementation cho stm32_arch.h
 */

void pal_stm32_h723_init_env(void) {
  /**
   * @brief Việc bổ sung các triển khai
   *        tùy thuộc vào nhu cầu riêng của từng dự án, có thể là khởi tạo clock, GPIO, UART, v.v.
   */
  pal_memrp_get_sys_info(NULL, NULL, NULL); // Gọi hàm này để đảm bảo các biểu tượng linker script được sử dụng và không bị tối ưu hóa mất
  pal_stm32_h723_nvic_config();
}

void pal_stm32_h723_idle_sleep(void) {

}

void pal_stm32_h723_nvic_config(void) {
//	NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_2);
}

void pal_stm32_h723_exti_init(ui32 IRQnum) {

}

void pal_stm32_h723_check_hardfault_reason(char* retr) {
}

void pal_memrp_get_sys_info(ui32 *rom_used, ui32 *ram_used, ui32 *stack_curr) {

}

void SysTick_Handler(void) {
}

__attribute__((naked)) void HardFault_Handler(void) {

}


