/**
 * @file ciedpc_itnlog.c
 * @author Shang Huang
 * @brief Implementation of internal logger for CIEDPC
 * @version 0.1
 * @date 2026-05-14
 * @copyright MIT License
 */
#include "ciedpc_core.h"
#include "ring_buffer.h"
#include "ciedpc_task.h"
#include "ciedpc_msg.h"
#include "ciedpc_fsm.h"
#include "ciedpc_tsm.h"
#include "ciedpc_itnlog.h"

/**
 * @brief Khai báo hằng số cho kích thước của ring buffer log entry
 */

#ifndef CIEDPC_ITNLOG_MAX_LOG_ENTRIES
  #define CIEDPC_ITNLOG_MAX_LOG_ENTRIES  (32u) // units
#endif // -> equipvalent to 32 units * sizeof(itnlog_entry_t) bytes

/**
 * @brief Khai báo cấu trúc để lưu trữ thông tin về trạng thái của internal logger
 * @param total_captured Tổng số log entry đã được ghi lại trong internal logger
 * @param total_dropped Tổng số log entry đã bị bỏ qua do ring buffer đã đầy
 * @param overflow_flag Cờ hiệu cho biết ring buffer đã bị tràn ít nhất một lần
 */

typedef struct {
  ui8 total_captured;
  ui8 total_dropped;
  bool overflow_flag;
} ciedpc_itnlog_status_t;

/**
 * @brief Khai báo ring buffer để lưu trữ log entry
 * @param itnlog_ring_buffer Buffer để truyền vào cho ring buffer, được khai báo với kích thước CIEDPC_ITNLOG_MAX_LOG_ENTRIES
 * @param itnlog_ringbuf Cấu trúc ring buffer để quản lý việc lưu trữ log entry trong itnlog_ring_buffer
 * @param itnlog_ringbuf_ctrl Cấu trúc quản lý cho ring buffer
 */

CIEDPC_ATTR_SECTION(".ciedpc_itnlog_ring_buffer") sta ciedpc_itnlog_entry_t itnlog_ring_buffer[CIEDPC_ITNLOG_MAX_LOG_ENTRIES];
CIEDPC_ATTR_SECTION(".ciedpc_itnlog_ring_buffer") sta ring_buffer_t itnlog_ringbuf;
CIEDPC_ATTR_SECTION(".ciedpc_itnlog_status") sta ciedpc_itnlog_status_t itnlog_status = {0}; 

/**
 * @brief Khai báo các biến toàn cục để lưu trữ thông tin về bộ lọc log của internal logger
 */

CIEDPC_ATTR_SECTION(".ciedpc_itnlog_ring_buffer") sta ciedpc_itnlog_level_t itnlog_filter_level = ITNLOG_LEVEL_DEBUG; // Mức độ log mặc định là DEBUG
CIEDPC_ATTR_SECTION(".ciedpc_itnlog_ring_buffer") sta const char* itnlog_filter_tag = NULL; // Thẻ log mặc định là NULL, có nghĩa là không lọc theo thẻ

/**
 * @brief Khai báo các hàm quản lý nội bộ cho internal logger
 */

sta void internal_ciedpc_itnlog_add_entry(ciedpc_itnlog_entry_t* entry);
sta ciedpc_itnlog_entry_t internal_ciedpc_itnlog_remove_entry(void);
sta void internal_ciedpc_itnlog_calc_hash(ciedpc_itnlog_entry_t* entry);

void internal_ciedpc_itnlog_add_entry(ciedpc_itnlog_entry_t* entry) {
  if (ring_buffer_is_full(&itnlog_ringbuf)) {
    // Cập nhật trạng thái
    itnlog_status.overflow_flag = true;
    itnlog_status.total_dropped++;
    return; // Bỏ qua log entry mới nếu ring buffer đã đầy
  } else {
    ring_buffer_put(&itnlog_ringbuf, entry);
    itnlog_status.total_captured++;
  }
}

ciedpc_itnlog_entry_t internal_ciedpc_itnlog_remove_entry(void) {
  if (ring_buffer_is_empty(&itnlog_ringbuf)) {
    // Trả về một log entry mặc định nếu ring buffer rỗng
    return (ciedpc_itnlog_entry_t){0};
  } else {
    ciedpc_itnlog_entry_t entry;
    ring_buffer_get(&itnlog_ringbuf, &entry);
    return entry; // Trả về log entry đã được lấy ra từ ring buffer
  }
}

void internal_ciedpc_itnlog_calc_hash(ciedpc_itnlog_entry_t* entry) {
  ui8 divisor = 31; // Một số nguyên dương nhỏ để tính hash
  ui8 dividend = entry->level + strlen(entry->tag) + entry->task_id + entry->msg_sig + strlen(entry->msg) + (entry->tmstmp % 1000);
  entry->hash = (dividend * divisor) % 65536; // Tính hash và đảm bảo nó nằm trong phạm vi của ui16
}

void ciedpc_itnlog_init(void) {
  memset(itnlog_ring_buffer, 0, sizeof(itnlog_ring_buffer));
  ring_buffer_init(
    &itnlog_ringbuf, (ciedpc_itnlog_entry_t*)itnlog_ring_buffer, 
    sizeof(itnlog_ring_buffer), sizeof(ciedpc_itnlog_entry_t)
  );
}

void ciedpc_itnlog_log(ui32 timestamp, ciedpc_itnlog_level_t level, const char* tag, const char* msg) {
  ciedpc_itnlog_entry_t entry = {
    .level = level,
    .tag = tag,
    .task_id = ciedpc_task_norm_get_current_id(), 
    .msg_sig = ciedpc_task_norm_get_current_msg()->sig,
    .msg = msg,
    .tmstmp = timestamp,
    .hash = 0  // Cần bổ sung logic để tính toán checksum của log entry
  };
  internal_ciedpc_itnlog_calc_hash(&entry);
  internal_ciedpc_itnlog_add_entry(&entry);
}

ciedpc_itnlog_entry_t ciedpc_itnlog_clear(void) {
  return internal_ciedpc_itnlog_remove_entry();
}

void ciedpc_itnlog_dump(void) {
  while (!ring_buffer_is_empty(&itnlog_ringbuf)) {
    ciedpc_itnlog_entry_t entry = internal_ciedpc_itnlog_remove_entry();
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
    }
  }
}

ui16 ciedpc_itnlog_get_log_count(void) {
  return ring_buffer_get_count(&itnlog_ringbuf);
}

void ciedpc_itnlog_set_level(ciedpc_itnlog_level_t level) {
  itnlog_filter_level = level;
}

ciedpc_itnlog_level_t ciedpc_itnlog_get_level(void) {
  return itnlog_filter_level;
}

const char* ciedpc_itnlog_get_tag(void) {
  return itnlog_filter_tag;
}

void ciedpc_itnlog_set_tag(const char* tag) {
  itnlog_filter_tag = tag;
}

void ciedpc_itnlog_set_filter(ciedpc_itnlog_level_t level, const char* tag) {
  ciedpc_itnlog_set_level(level);
  ciedpc_itnlog_set_tag(tag);
}

void ciedpc_itnlog_get_filter(ciedpc_itnlog_level_t* level, char* tag) {
  *level = ciedpc_itnlog_get_level();
  *tag = (char*)ciedpc_itnlog_get_tag();
}

void ciedpc_itnlog_set_output(void (*output_func)(const char*)) {

}