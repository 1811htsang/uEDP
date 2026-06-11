/**
 * @file linux_arch.h
 * @author Shang Huang
 * @brief Header file for Linux Architecture Abstraction Layer in CIEDPC
 * @version 0.1
 * @date 2026-04-20
 * @copyright MIT License
 */

#ifndef __LINUX_ARCH_H__
  #define __LINUX_ARCH_H__

  /**
   * @brief Khai báo thư viện sử dụng
   * @note Việc định nghĩa _POSIX_C_SOURCE phải được thực hiện trước khi bao gồm bất kỳ header nào để đảm bảo các hàm 
   *       và kiểu dữ liệu cần thiết được khai báo đúng cách
   */

  #include "pal_core.h"

  /**
   * @brief Khởi tạo môi trường mô phỏng trên Linux
   * Bao gồm: Khởi tạo Mutex, tạo luồng Tick, bắt tín hiệu SIGINT
   */
  void pal_linux_init_env(void);

  /**
   * @brief Giả lập một ngắt phần cứng (Ví dụ: phím nhấn từ Terminal)
   * Để gọi ciedpc_task_norm_post_isr từ ngoài vào Core
   */
  void pal_linux_simulate_interrupt(ui8 task_id, ui8 signal);

  /**
   * @brief Giả lập tick hệ thống trên Linux (Ví dụ: sử dụng timer hoặc luồng riêng)
   * Để gọi ciedpc_timer_tick() định kỳ
   */
  void pal_linux_simulate_tick(void);

  /**
   * @brief Dọn dẹp tài nguyên khi kết thúc chương trình (Ví dụ: hủy luồng, giải phóng mutex)
   */
  void pal_linux_cleanup(void);

  /**
   * @brief Xử lý tín hiệu từ hệ thống (Ví dụ: SIGINT để dọn dẹp và thoát)
   */
  void pal_signal_handler(int signum);

#endif