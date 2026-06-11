/**
 * @file stm32_arch.h
 * @author Shang Huang
 * @brief Header file for STM32 Architecture Abstraction Layer in CIEDPC
 * @version 0.1
 * @date 2026-04-20
 * @copyright MIT License
 */

#ifndef __STM32_F103_ARCH_H__
  #define __STM32_F103_ARCH_H__

  /**
   * @brief Khai báo thư viện sử dụng
   * @note Tùy thuộc vào nhu cầu của ứng dụng mà có thể thêm bớt các thư viện cần thiết,
   *       nhưng cần đảm bảo không gây ra xung đột hoặc lỗi biên dịch do thiếu thư viện.
   */

  #include "pal_core.h"

  /**
   * @brief Khởi tạo môi trường làm việc trên STM32 cho Core hoạt động
   */
  void pal_stm32_f103_init_env(void);

  /**
   * @brief Hàm này sẽ được gọi khi Core không có tác vụ nào để chạy
   */
  void pal_stm32_f103_idle_sleep(void);

  /**
   * @brief Cấu hình NVIC cho phép ngắt từ các ngoại vi cần thiết
   */
  void pal_stm32_f103_nvic_config(void);

  /**
   * @brief Cấu hình EXTI cho một chân GPIO cụ thể để tạo ngắt
   * @param pin: Số chân GPIO (ví dụ: GPIO_PIN_0)
   * @param trigger_mode: Chế độ kích hoạt (ví dụ: RISING, FALLING, BOTH)
   */
  void pal_stm32_f103_exti_init(ui32 IRQnum);

  /**
   * @brief Kiểm tra nguyên nhân gây ra HardFault trên STM32 và in thông tin chi tiết để hỗ trợ debug
   */
  void pal_stm32_f103_check_hardfault_reason(char* retr);

  /**
   * @brief Các cấu hình khác sẽ được bổ sung tùy thuộc vào nhu cầu riêng của từng dự án.
   */

  // Điền các khai báo tại đây

#endif // __STM32_F103_ARCH_H__
