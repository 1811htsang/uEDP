/**
 * @file esp32_s3_arch.h
 * @author Shang Huang
 * @brief Header file for ESP32-S3 Architecture Abstraction Layer in UEDP
 * @version 0.1
 * @date 2026-04-20
 * @copyright MIT License
 */

#ifndef __ESP32_S3_ARCH_H__
  #define __ESP32_S3_ARCH_H__

  /**
   * @brief Khai báo thư viện sử dụng
   * @note Tùy thuộc vào nhu cầu của ứng dụng mà có thể thêm bớt các thư viện cần thiết,
   *       nhưng cần đảm bảo không gây ra xung đột hoặc lỗi biên dịch do thiếu thư viện.
   */

  #include "pal_core.h"

  /**
   * @brief Khởi tạo môi trường làm việc trên ESP32-S3 cho Core hoạt động
   */
  void pal_esp32_s3_init_env(void);

  /**
   * @brief Hàm này sẽ được gọi khi Core không có tác vụ nào để chạy
   */
  void pal_esp32_s3_idle_sleep(void);

  /**
   * @brief Các cấu hình khác sẽ được bổ sung tùy thuộc vào nhu cầu riêng của từng dự án.
   */

  // Điền các khai báo tại đây

#endif // __ESP32_S3_ARCH_H__
