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

  void stm32h723_get_i2c_inst(I2C_HandleTypeDef* instance);

  /** ANCHOR - USART1 API
   * @attention huart phải là handle đã được HAL_UART_Init() bởi CubeIDE (MX_USART1_UART_Init)
   * @attention DMA TX/RX được CubeIDE liên kết vào huart qua __HAL_LINKDMA, PAL không cần handle DMA riêng
   */
  void stm32h723_uart_bind(UART_HandleTypeDef* huart);

  void stm32h723_get_dma_spi1_tx_inst(DMA_HandleTypeDef* instance);

  void stm32h723_get_dma_spi1_rx_inst(DMA_HandleTypeDef* instance);

  /** ANCHOR - I2S1 (SPI1) API
   * @attention hi2s phải là handle đã được HAL_I2S_Init() bởi CubeIDE (MX_I2S1_Init)
   * @attention buf phải nằm trong RAM DMA-accessible (RAM_D1/D2/D3), không dùng DTCMRAM
   * @attention size dùng đơn vị của HAL_I2S_Receive_DMA (chưa xác minh với 24-bit, cần kiểm tra trên phần cứng)
   * @attention callback chạy trong ngữ cảnh ISR, chỉ gọi các hàm ISR-safe
   */
  typedef enum stm32h723_i2s_event {
    STM32H723_I2S_RX_DONE  = 0x00U,
    STM32H723_I2S_RX_ERROR = 0x01U
  } stm32h723_i2s_event_t;

  typedef void (*stm32h723_i2s_rx_cb_t)(stm32h723_i2s_event_t event);

  void stm32h723_i2s_bind(I2S_HandleTypeDef* hi2s);

  void stm32h723_i2s_set_rx_callback(stm32h723_i2s_rx_cb_t cb);

  RETR_STAT stm32h723_i2s_rx_start(ui32* buf, ui16 samples);

  RETR_STAT stm32h723_i2s_rx_stop(void);

  /** ANCHOR - OCTOSPI1 API
   * @attention hospi phải là handle đã được HAL_OSPI_Init() và HAL_OSPIM_Config() bởi CubeIDE (MX_OCTOSPI1_Init)
   * @attention cmd.NbData được gán tự động theo size; các trường còn lại do người gọi cấu hình
   */
  void stm32h723_ospi_bind(OSPI_HandleTypeDef* hospi);

  RETR_STAT stm32h723_ospi_command(OSPI_RegularCmdTypeDef* cmd, ui32 timeout);

  RETR_STAT stm32h723_ospi_transmit(OSPI_RegularCmdTypeDef* cmd, ui8* data, ui32 size, ui32 timeout);

  RETR_STAT stm32h723_ospi_receive(OSPI_RegularCmdTypeDef* cmd, ui8* data, ui32 size, ui32 timeout);

#endif // __STM32H723_H__
