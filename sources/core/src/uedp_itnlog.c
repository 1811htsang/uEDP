/**
 * @file uedp_itnlog.c
 * @author Shang Huang
 * @brief Implementation of internal logger for UEDP
 * @version 0.1
 * @date 2026-05-14
 * @copyright MIT License
 */
#include "uedp_core.h"
#include "ring_buffer.h"
#include "uedp_task.h"
#include "uedp_msg.h"
#include "uedp_fsm.h"
#include "uedp_tsm.h"
#include "uedp_itnlog.h"

/**
 * @brief Khai báo cấu trúc để lưu trữ thông tin về trạng thái của internal logger
 * @param total_captured Tổng số log entry đã được ghi lại trong internal logger
 * @param underflow_flag Cờ hiệu cho biết ring buffer đã bị trống ít nhất một lần
 */
typedef struct uedp_itnlog_status_t {
  ui8 total_captured;
  ui8 underflow_flag;
} uedp_itnlog_status_t;

/**
 * @brief Khai báo ring buffer để lưu trữ log entry
 * @param itnlog_ring_buffer Buffer để truyền vào cho ring buffer, được khai báo với kích thước UEDP_ITNLOG_MAX_LOG_ENTRIES
 * @param itnlog_ringbuf Cấu trúc ring buffer để quản lý việc lưu trữ log entry trong itnlog_ring_buffer
 * @param itnlog_ringbuf_ctrl Cấu trúc quản lý cho ring buffer
 */

sta uedp_itnlog_entry_t itnlog_ring_buffer[UEDP_ITNLOG_MAX_LOG_ENTRIES] = {0}; 
sta ring_buffer_t itnlog_ringbuf = {0};
sta uedp_itnlog_status_t itnlog_status = {0}; 

/**
 * @brief Khai báo các biến toàn cục để lưu trữ thông tin về bộ lọc log của internal logger
 */

sta uedp_itnlog_level_t itnlog_filter_level = ITNLOG_LEVEL_DEBUG; // Mức độ log mặc định là DEBUG
sta const char* itnlog_filter_tag = NULL; // Thẻ log mặc định là NULL, có nghĩa là không lọc theo thẻ

/**
 * @brief Khai báo biến toàn cục để lưu trữ hàm output cho internal logger
 */
 
sta uedp_itnlog_output_func_t itnlog_output_func = NULL; // Hàm output mặc định là NULL, có nghĩa là không có đích đến cụ thể cho log

/**
 * @brief Khai báo các hàm quản lý nội bộ cho internal logger
 */

sta void internal_uedp_itnlog_add_entry(uedp_itnlog_entry_t* entry);
sta uedp_itnlog_entry_t internal_uedp_itnlog_remove_entry(void);
sta void internal_uedp_itnlog_calc_hash(uedp_itnlog_entry_t* entry);

void internal_uedp_itnlog_add_entry(uedp_itnlog_entry_t* entry) {
  if (itnlog_status.total_captured >= UEDP_ITNLOG_MAX_LOG_ENTRIES) {
    /**
     * @brief Nếu vượt ngưỡng thì flush toàn bộ log entry hiện có 
     *        trong ring buffer ra đích đến và reset trạng thái 
     *        của internal logger
     */
    uedp_itnlog_dump();
    memset(&itnlog_status, 0, sizeof(itnlog_status));
  }
  ring_buffer_put(&itnlog_ringbuf, entry);
  itnlog_status.total_captured++;
}

uedp_itnlog_entry_t internal_uedp_itnlog_remove_entry(void) {
  if (ring_buffer_is_empty(&itnlog_ringbuf)) {
    // Nếu ring buffer rỗng, tăng cờ hiệu underflow và trả về một log entry mặc định
    itnlog_status.underflow_flag++; 
    // Trả về một log entry mặc định nếu ring buffer rỗng
    return (uedp_itnlog_entry_t){0};
  } else {
    uedp_itnlog_entry_t entry;
    ring_buffer_get(&itnlog_ringbuf, &entry);
    return entry; // Trả về log entry đã được lấy ra từ ring buffer
  }
}

void internal_uedp_itnlog_calc_hash(uedp_itnlog_entry_t* entry) {
  ui8 divisor = 31; // Một số nguyên dương nhỏ để tính hash
  ui8 dividend = entry->level + strlen(entry->tag) + entry->task_id + entry->msg_sig + strlen(entry->msg) + (entry->tmstmp % 1000);
  entry->hash = (dividend * divisor) % 65536; // Tính hash và đảm bảo nó nằm trong phạm vi của ui16
}

void uedp_itnlog_init(void) {
  memset(itnlog_ring_buffer, 0, sizeof(itnlog_ring_buffer));
  ring_buffer_init(
    &itnlog_ringbuf, (uedp_itnlog_entry_t*)itnlog_ring_buffer, 
    UEDP_ITNLOG_MAX_LOG_ENTRIES, sizeof(uedp_itnlog_entry_t)
  );
}

void uedp_itnlog_log(ui32 timestamp, uedp_itnlog_level_t level, const char* tag, const char* msg) {
  uedp_itnlog_entry_t entry = {
    .level = level,
    .tag = tag,
    .task_id = uedp_task_norm_get_current_id(), 
    .msg_sig = uedp_task_norm_get_current_msg()->sig,
    .msg = msg,
    .tmstmp = timestamp,
    .hash = 0  // Cần bổ sung logic để tính toán checksum của log entry
  };
  internal_uedp_itnlog_calc_hash(&entry);
  internal_uedp_itnlog_add_entry(&entry);
}

uedp_itnlog_entry_t uedp_itnlog_clear(void) {
  return internal_uedp_itnlog_remove_entry();
}

void uedp_itnlog_dump(void) {
  while (!ring_buffer_is_empty(&itnlog_ringbuf)) {
    uedp_itnlog_entry_t entry = internal_uedp_itnlog_remove_entry();
    /**
     * @brief Logic kiểm tra bộ lọc log trước khi xuất log entry ra đích đến
     * @note 
     * Điều kiện kiểm tra là:
     * - Mức độ log của log entry phải lớn hơn hoặc bằng mức độ log được lọc (itnlog_filter_level)
     * - Thẻ log của log entry phải khớp với thẻ log được lọc (itnlog_filter_tag), 
     *   hoặc nếu itnlog_filter_tag là NULL thì không lọc theo thẻ và chấp nhận tất cả các thẻ log.
     */
    if (
      entry.level >= itnlog_filter_level && 
      (itnlog_filter_tag == NULL || strcmp(entry.tag, itnlog_filter_tag) == 0)
    ) {
      // Logic để xuất log entry ra đích đến, có thể là console, file hoặc giao diện
      if (itnlog_output_func != NULL) {
        char formatted_entry[128];
        snprintf(
          formatted_entry,
          sizeof(formatted_entry),
          "[ITNLOG] %u 0x%02X 0x%02X %s\n",
          (unsigned int)entry.tmstmp,
          (unsigned int)entry.task_id,
          (unsigned int)entry.msg_sig,
          (entry.msg != NULL) ? entry.msg : ""
        );
        itnlog_output_func(formatted_entry);
      }
    }
  }
  // Sau khi dump xong, reset trạng thái của internal logger
  memset(&itnlog_status, 0, sizeof(itnlog_status));
}

ui16 uedp_itnlog_get_log_count(void) {
  return ring_buffer_get_count(&itnlog_ringbuf);
}

void uedp_itnlog_set_level(uedp_itnlog_level_t level) {
  itnlog_filter_level = level;
}

uedp_itnlog_level_t uedp_itnlog_get_level(void) {
  return itnlog_filter_level;
}

const char* uedp_itnlog_get_tag(void) {
  return itnlog_filter_tag;
}

void uedp_itnlog_set_tag(const char* tag) {
  itnlog_filter_tag = tag;
}

void uedp_itnlog_set_filter(uedp_itnlog_level_t level, const char* tag) {
  uedp_itnlog_set_level(level);
  uedp_itnlog_set_tag(tag);
}

void uedp_itnlog_get_filter(uedp_itnlog_level_t* level, char* tag) {
  *level = uedp_itnlog_get_level();
  *tag = (char)(uintptr_t)uedp_itnlog_get_tag();
}

void uedp_itnlog_set_output(void (*output_func)(const char*)) {
  itnlog_output_func = output_func;
}

uint16_t ring_buffer_get_count(ring_buffer_t* ring_buffer) {
  return ring_buffer->fill_size;
}