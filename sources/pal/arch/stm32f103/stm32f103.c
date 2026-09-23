//ANCHOR - Architecture-specific implementation for STM32F103 microcontroller
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "stm32f103.h"
#include "uedp_core.h"
#include "uedp_task.h"
#include "uedp_msg.h"
#include "uedp_timer.h"
#include "uedp_itnlog.h"
#include "uedp_fcr.h"

//ANCHOR - Khai báo thư viện encrypt
#include "libcrc8.h"

//ANCHOR - Khai báo thư viện hệ thống
#include "pal_logdp.h"
#include "pal_memrp.h"
#include "pal_rprintf.h"

//ANCHOR - STM32F103 BSP include
#include "stm32f103xb.h"
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
  stm32f103_config_exti_swisr();
}

//NOTE - not an optimal solution, but currently usable
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

//NOTE - sleep mode will exit after wake-up source finish
void stm32f103_sleep_exit(void) {
  HAL_SuspendTick();
  HAL_PWR_EnableSleepOnExit();
}

//NOTE - sleep mode will resume after wake-up source finish
void stm32f103_sleep_resume(void) {
  HAL_SuspendTick();
  HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
}

void stm32f103_wakeup(void) {
  HAL_ResumeTick();
}

//NOTE - using custom interrupt source would be nice instead of binding to specific functional interrupt

void exti_line_0_swi_callback(void) {
  stm32f103_wakeup();
}

EXTI_HandleTypeDef EXTI_L0_handler = {
  EXTI_LINE_0,
  &exti_line_0_swi_callback
};

EXTI_ConfigTypeDef EXTI_L0_config = {
  EXTI_LINE_0,
  EXTI_MODE_INTERRUPT,
  EXTI_TRIGGER_RISING,
  EXTI_GPIO //NOTE - This is mandatory but not in use
};

void stm32f103_config_exti_swisr(void) {
  HAL_EXTI_SetConfigLine(&EXTI_L0_handler, &EXTI_L0_config);
}

void stm32f103_trigger_swisr(ui32 IRQnum) {
  HAL_EXTI_GenerateSWI(&EXTI_L0_handler);
}

void SysTick_Handler(void) {
  HAL_IncTick();
  uedp_timer_tick();
}

__attribute__((naked)) void HardFault_Handler(void) {
  __asm volatile (
    " tst lr, #4                                 \n" // Kiểm tra bit 2 của LR (EXC_RETURN)
    " ite eq                                     \n"
    " mrseq r0, msp                              \n" // Bit 2 = 0: Dùng Main Stack (MSP)
    " mrsne r0, psp                              \n" // Bit 2 = 1: Dùng Process Stack (PSP)
    " ldr r1, =internal_hardfault_decoder        \n"
    " bx r1                                      \n"
  );
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
  // NOTE - Lấy thông tin các thanh ghi CPU trước khi xảy ra fault
  // NOTE - Bổ sung atttribute để tránh warning từ compiler
  UEDP_ATTR_UNUSED uint32_t r0  = stack[0];
  UEDP_ATTR_UNUSED uint32_t r1  = stack[1];
  UEDP_ATTR_UNUSED uint32_t r2  = stack[2];
  UEDP_ATTR_UNUSED uint32_t r3  = stack[3];
  UEDP_ATTR_UNUSED uint32_t r12 = stack[4];
  UEDP_ATTR_UNUSED uint32_t lr  = stack[5]; // Link Register (Địa chỉ trả về trước khi gọi hàm bị lỗi)
  UEDP_ATTR_UNUSED uint32_t pc  = stack[6]; // Program Counter (Địa chỉ chính xác của lệnh gây ra lỗi)
  UEDP_ATTR_UNUSED uint32_t psr = stack[7]; // Program Status Register

  // NOTE - Đọc các thanh ghi cấu hình/chẩn đoán System Control Block (SCB)
  volatile uint32_t cfsr = SCB->CFSR; // Configurable Fault Status Register ( gom MemManage, BusFault, UsageFault)
  volatile uint32_t hfsr = SCB->HFSR; // HardFault Status Register
  volatile uint32_t dfsr = SCB->DFSR; // Debug Fault Status Register
  volatile uint32_t afsr = SCB->AFSR; // Auxiliary Fault Status Register

  // NOTE - Lấy địa chỉ bộ nhớ gây lỗi (nếu có)
  volatile uint32_t mmfar = SCB->MMFAR; // MemManage Fault Address
  volatile uint32_t bfar  = SCB->BFAR;  // BusFault Address

  // NOTE - Giữ chân CPU tại đây để quan sát qua Debugger hoặc log ra UART
  (void)r0; (void)r1; (void)r2; (void)r3; (void)r12; (void)psr;
  (void)cfsr; (void)hfsr; (void)dfsr; (void)afsr; (void)mmfar; (void)bfar;

  // STUB - add itnlog here
  uedp_itnlog_log(HAL_GetTick(), ITNLOG_LEVEL_FATAL, ITNLOG_TAG_FCR, (const char*)hfsr);

  // STUB - add persistent log here with Backup Data Registers or external flash memory

  __asm volatile ("bkpt #0"); // Dừng chương trình nếu đang cắm Debugger
  while (1);
}

ui32 stm32f103_get_tick(void) {
  return HAL_GetTick();
}

/** NOTE
 * Khai báo entry mặc định cho toàn bộ tính năng PPLP
 */

const uedp_itnlog_entry_t default_entry = {
  UEDP_TASK_NORM_IDLE_ID, // NOTE - IDLE task id
  0x0, // NOTE - No message sig
  0x0,
  0x0,
  (const char*)"0",
  ITNLOG_TAG_PAL,
  ITNLOG_LEVEL_INFO
};

/** NOTE
 * entry của pal_log_dispatch được set global
 * vì 1 số tham số có thể được thay đổi trong quá trình runtime,
 * do đó chỉ cần set entry 1 lần duy nhất
 */

uedp_itnlog_entry_t logdp_entry = default_entry;

void stm32f103_log_alloc(const char* param) {
  logdp_entry.msg = param;
  logdp_entry.tmstmp = stm32f103_get_tick();
  logdp_entry.hash = crc8((uint8_t*)param, strlen(param), 0xB4);
  pal_logdp_dispatch(&logdp_entry);
}

//ANCHOR - Add instance for STM32F103 peripheral handles

I2C_HandleTypeDef i_hi2c1;
SPI_HandleTypeDef i_hspi1;
UART_HandleTypeDef i_huart1;
DMA_HandleTypeDef i_hdma_usart1_tx;
DMA_HandleTypeDef i_hdma_usart1_rx;

//ANCHOR - Add functions to get peripheral instances

void stm32f103_get_uart_inst(UART_HandleTypeDef* instance) {
  i_huart1.Instance = (USART_TypeDef*)instance;
}

void stm32f103_get_i2c_inst(I2C_HandleTypeDef* instance) {
  i_hi2c1.Instance = (I2C_TypeDef*)instance;
}

void stm32f103_get_spi_inst(SPI_HandleTypeDef* instance) {
  i_hspi1.Instance = (SPI_TypeDef*)instance;
}

//DEPRECATED - Old TASK - Need checking for pointer type casting
//STATUS - correct due to i_hdma_usart1_tx reciev resolved data

void stm32f103_get_dma_usart1_tx_inst(DMA_HandleTypeDef* instance) {
  i_hdma_usart1_tx = *instance;
}

void stm32f103_get_dma_usart1_rx_inst(DMA_HandleTypeDef* instance) {
  i_hdma_usart1_rx = *instance;
}

//ANCHOR - Define buffer size for UART TX/RX

#define RX_BUF_SIZE (ui16)32u
#define TX_BUF_SIZE (ui16)32u

//ANCHOR - Define buffer for UART TX/RX communication

sta ui8 rx_buf[RX_BUF_SIZE];
sta ui8 tx_buf[TX_BUF_SIZE];

RETR_STAT stm32f103_uart_init(void) {
  HAL_UARTEx_ReceiveToIdle_DMA(&i_huart1, rx_buf, RX_BUF_SIZE);
  // __HAL_DMA_DISABLE_IT(&i_hdma_usart1_tx, DMA_IT_HT);
  // __HAL_DMA_DISABLE_IT(&i_hdma_usart1_rx, DMA_IT_HT);
  //DEPRECATED - Old TASK - Change this to adding handler for handler callback
  return STAT_OK;
}

void internal_uart_tx_dma(ui8 *data, ui16 size) {
  memcpy(tx_buf, data, size);
  HAL_UART_Transmit_DMA(&i_huart1, tx_buf, size);
}

void internal_uart_rx_dma(void) {
  // Start DMA reception in normal mode
  HAL_UART_Receive_DMA(&i_huart1, rx_buf, sizeof(rx_buf));
}

//TASK - Add detail implementation for UART output TX functions for rprintf service
//TASK - cmake build to remove errors on C/C++ Intellisense

void stm32f103_uart_putc(unsigned char c) {
  // Transmit the character using DMA
  internal_uart_tx_dma(&c, 1);
}

void stm32f103_uart_write(const uint8_t* data, ui16 len) {
  // Transmit the data using DMA
  internal_uart_tx_dma((uint8_t*)data, len);
}

bool stm32f103_uart_isready() {
  HAL_UART_StateTypeDef status = HAL_UART_GetState(&i_huart1);
  if (status != HAL_UART_STATE_READY) {
    return false;
  }
  return true;
}

const uedp_itnlog_entry_t rprintf_entry = default_entry;

/** NOTE
 * rprintf_entry có thể thay đổi runtime nên cho phép const pointer ở khai báo trước,
 * sau đó gán giá trị mới cho các trường trong struct khi cần thiết
 */

pal_rprintf_service_t svc = {
  "uart",
  rprintf_entry,
  &stm32f103_uart_init,
  &stm32f103_uart_putc,
  &stm32f103_uart_write,
  &stm32f103_uart_isready
};

// STUB - In stm32f1xx_it.c has implement IRQ for DMA so no need to reinvent

void stm32f103_check_hardfault_reason(char* retr) {
  /** NOTE
   * The result has aleady been logged in internal_hardfault_decoder, so just return a simple message
   * However, the itnlog is not bene implemented to be persisted
   * so the user can implement their own persistent log mechanism to store the hardfault reason for later analysis
   */
}
