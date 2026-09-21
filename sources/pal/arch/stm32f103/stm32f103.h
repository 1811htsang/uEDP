//ANCHOR - Architecture-specific header for STM32F103 microcontroller
#ifndef __STM32F103_H__
  #define __STM32F103_H__

  //ANCHOR - Khai báo các thư viện cần thiết cho ứng dụng
  #include "pal_core.h"

  /** ANCHOR - Khai báo custom API
   * @attention Xin đừng sửa đổi, tự động sinh bởi Kconfiglib và Jinja2
   */

  void stm32f103_sleep_exit(void);

  void stm32f103_sleep_resume(void);

  void stm32f103_config_exti_swisr(void);

  void stm32f103_trigger_swisr(ui32 IRQnum);

  void stm32f103_wakeup(void);

  void stm32f103_check_hardfault_reason(char* retr);

  ui32 stm32f103_get_tick(void);

  void stm32f103_log_alloc(const char* param);

#endif // __STM32F103_H__
