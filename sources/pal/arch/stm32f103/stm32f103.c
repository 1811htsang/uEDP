//ANCHOR - Architecture-specific implementation for STM32F103 microcontroller
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "stm32f103.h"
#include "uedp_core.h"
#include "uedp_task.h"
#include "uedp_msg.h"
#include "uedp_timer.h"
#include "uedp_itnlog.h"
#include "uedp_fcr.h"

//ANCHOR - STM32F103 BSP include
#include "core_cm3.h"
#include "stm32f1xx.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_exti.h"

//CRITICAL - Đảm bảo phải có ủy quyền timer_tick để gắn vào timer phần cứng

extern void uedp_timer_tick(void);

//ANCHOR - Implementation cho uedp_core.h

sta ui8 is_inited = 0x0u;

void uedp_core_init(void) {
  pal_core_init();
  uedp_msg_pool_init();
  uedp_timer_init();
  uedp_itnlog_init();
  is_inited = 0x1u;
}

//ANCHOR - Implementation cho pal_core.h

//ANCHOR - GVI for PRIMASK register

sta ui32 primask_gvi = 0x0u;

void pal_core_init(void) {
  stm32f103_init_env();
}

void pal_enter_critical(void) {
  __disable_irq();
  primask_gvi = __get_PRIMASK();
}

void pal_exit_critical(void) {
  __enable_irq();
  __set_PRIMASK(primask_gvi);
}

ui8 pal_math_get_highest_bit32(ui32 mask) {
  if (mask == 0) {
    return -1;
  }
  return 31 - __CLZ(mask);
}

ui32 pal_sys_get_tick(void) {
  return HAL_GetTick();
}

void pal_sys_reset(void) {
  NVIC_SystemReset();
}

void pal_sys_fatal(const char* file, ui32 line, const char* msg) {
  uedp_fcr_raise(UEDP_FCR_PAL_FATAL_API_CALLED, file, line, msg);
}

//ANCHOR - Implementation custom API cho {{arch_name}}.h
/** CRITICAL - 
  * Các parameter <return_type> và <parameters> cần được thay thế bằng kiểu dữ liệu thực tế 
  * theo nhu cầu của người dùng
  */

void stm32f103_init_env(void) {
  stm32f103_nvic_config();
}

void stm32f103_sleep(void) {
  HAL_SuspendTick();
  HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
}

void stm32f103_wakeup(void) {
  HAL_ResumeTick();
}

void stm32f103_nvic_config(void) {
	NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_2);
}

void stm32f103_exti_init(ui32 IRQnum) {

}

void stm32f103_check_hardfault_reason(char* retr) {

}

void SysTick_Handler(void) {
  
}

__attribute__((naked)) void HardFault_Handler(void) {

}

//ANCHOR - Implementation cho internal API handling

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

UEDP_ATTR_UNUSED void internal_hardfault_decoder(uint32_t *stack) {
	
}
