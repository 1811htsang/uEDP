/**
 * @file pal_rprintf.c
 * @author Shang Huang
 * @brief Implementation of the redirect print service for the PAL layer.
 * @version 0.1
 * @date 2026-06-01
 * @copyright MIT License
 */
#include "uedp_core.h"
#include "uedp_itnlog.h"
#include "xprintf.h"
#include "pal_rprintf.h"
#include <string.h>

/**
 * @brief Khai báo các biến nội bộ để hỗ trợ cho việc định dạng log entry
 */

sta char* internal_format_buffer = NULL;
sta size_t internal_format_buffer_size = 0;
sta size_t internal_format_buffer_length = 0;

/**
 * @brief Khai báo các hàm nội bộ hỗ trợ cho dịch vụ redirect print
 */

sta void internal_buffer_putc(int chr);
sta void internal_validate_entry(uedp_itnlog_entry_t* entry);
sta void internal_format_entry(uedp_itnlog_entry_t* entry, char* buffer, size_t buf_size);

/**
 * @brief Triển khai các hàm của dịch vụ redirect print cho PAL layer
 */

void pal_rprintf_flush_entry(pal_rprintf_service_t* service) {
  if (service == NULL) {
    return;
  }
  // Kiểm tra tính hợp lệ của log entry trước khi xử lý
  internal_validate_entry(&service->entry);
  // Buffer để lưu trữ log đã được định dạng
  char formatted_log[256] = {0}; 
  // Định dạng log entry thành chuỗi có thể xuất ra
  internal_format_entry(&service->entry, formatted_log, sizeof(formatted_log)); 
  // Kiểm tra xem dịch vụ redirect print đã sẵn sàng để sử dụng chưa
  if (service->is_ready != NULL && service->is_ready()) {
    // Nếu dịch vụ đã sẵn sàng, sử dụng hàm write để xuất log đã được định dạng ra đích đến
    if (service->write != NULL) {
      service->write((const uint8_t*)formatted_log, strlen(formatted_log)); 
    } else if (service->putc != NULL) {
      // Nếu hàm write không có sẵn nhưng hàm putc có sẵn, sử dụng hàm putc để xuất từng ký tự của log đã được định dạng ra đích đến
      for (size_t i = 0; i < strlen(formatted_log); i++) {
        service->putc((unsigned char)formatted_log[i]); 
      }
    }
  }
}

/**
 * @brief Triển khai các hàm nội bộ hỗ trợ cho dịch vụ redirect print
 */

 /**
  * @note Tùy thuộc vào nhu cầu kiểm tra trong tương lai nên cần 
  *       tách hàm này thành nhiều hàm nhỏ hơn để kiểm tra từng phần của log entry một cách chi tiết hơn,
  */
void internal_validate_entry(uedp_itnlog_entry_t* entry) {
  if (entry == NULL) {
    return; // Nếu entry là NULL, không cần xử lý gì
  }
}

void internal_format_entry(uedp_itnlog_entry_t* entry, char* buffer, size_t buf_size) {
  if (entry == NULL || buffer == NULL || buf_size == 0) {
    return;
  }
  internal_format_buffer = buffer;
  internal_format_buffer_size = buf_size;
  internal_format_buffer_length = 0;
  internal_format_buffer[0] = '\0';
  // Sử dụng xfprintf với specifier mà xprintf hỗ trợ để tránh in literal #X
  xfprintf(internal_buffer_putc, "[%lu tick] [TSK:%02X] [SIG:%02X] %s\n", (unsigned long)entry->tmstmp, (unsigned int)entry->task_id, (unsigned int)entry->msg_sig, entry->msg != NULL ? entry->msg : "");

  internal_format_buffer = NULL;
  internal_format_buffer_size = 0;
  internal_format_buffer_length = 0;
}

static void internal_buffer_putc(int chr) {
  if (internal_format_buffer == NULL || internal_format_buffer_size == 0) {
    return;
  }

  if (internal_format_buffer_length + 1 >= internal_format_buffer_size) {
    internal_format_buffer[internal_format_buffer_size - 1] = '\0';
    return;
  }

  internal_format_buffer[internal_format_buffer_length++] = (char)chr;
  internal_format_buffer[internal_format_buffer_length] = '\0';
}
