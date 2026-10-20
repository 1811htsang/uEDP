//ANCHOR - Architecture-specific header for stm32h723 microcontroller
#ifndef __STM32H723_H__
  #define __STM32H723_H__

  //ANCHOR - Khai báo các thư viện cần thiết cho ứng dụng
  #include "pal_core.h"
  #include "stm32h7xx_hal.h"

  /** ANCHOR - Khai báo custom API
   * @attention Xin đừng sửa đổi, tự động sinh bởi Kconfiglib và Jinja2
   */

  void stm32h723_sleep_exit(void);

  void stm32h723_sleep_resume(void);

  void stm32h723_config_exti_swisr(void);

  void stm32h723_trigger_swisr(ui32 IRQnum);

  void stm32h723_wakeup(void);

  void stm32h723_check_hardfault_reason(char* retr);

  ui32 stm32h723_get_tick(void);

  void stm32h723_log_alloc(const char* param);

  void stm32h723_get_uart_inst(UART_HandleTypeDef* instance);

  void stm32h723_get_i2c_inst(I2C_HandleTypeDef* instance);

  void stm32h723_get_i2s_inst(I2S_HandleTypeDef* instance);

  void stm32h723_get_spi_1_inst(SPI_HandleTypeDef* instance);

  void stm32h723_get_spi_2_inst(SPI_HandleTypeDef* instance);

  void stm32h723_get_dma_usart1_tx_inst(DMA_HandleTypeDef* instance);

  void stm32h723_get_dma_usart1_rx_inst(DMA_HandleTypeDef* instance);

#endif // __STM32H723_H__
