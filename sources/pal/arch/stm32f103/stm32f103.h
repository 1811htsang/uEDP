//ANCHOR - Architecture-specific header for STM32F103 microcontroller
#ifndef __STM32F103_H__
  #define __STM32F103_H__

  //ANCHOR - Khai báo các thư viện cần thiết cho ứng dụng

  #include "pal_core.h"

  /** ANCHOR - Khai báo custom API
   * @attention Xin đừng sửa đổi, tự động sinh bởi Kconfiglib và Jinja2
   */

  void stm32f103_init_env(void);

  void stm32f103_sleep(void);

  void stm32f103_wakeup(void);

  void stm32f103_nvic_config(void);

  void stm32f103_exti_init(ui32 IRQnum);

  void stm32f103_check_hardfault_reason(char* retr);

#endif // __STM32F103_H__
