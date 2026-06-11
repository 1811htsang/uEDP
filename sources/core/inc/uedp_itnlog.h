/**
 * @file uedp_itnlog.h
 * @author Shang Huang
 * @brief Internal logger for UEDP
 * @version 0.1
 * @date 2026-05-14
 * @copyright MIT License
 */
#ifndef __UEDP_ITNLOG_H__
  #define __UEDP_ITNLOG_H__

  /**
   * @brief Khai báo thư viện sử dụng
   */

  #include "uedp_core.h"
  #include "uedp_fsm.h"
  #include "uedp_tsm.h"
  #include <stdint.h>
  
  /**
   * @brief Định nghĩa các mức độ log
   * @param ITNLOG_LEVEL_DEBUG Mức độ log cho các thông tin chi tiết
   * @param ITNLOG_LEVEL_INFO Mức độ log cho các thông tin thông thường
   * @param ITNLOG_LEVEL_WARN Mức độ log cho các cảnh báo
   * @param ITNLOG_LEVEL_ERROR Mức độ log cho các lỗi
   * @param ITNLOG_LEVEL_FATAL Mức độ log cho các lỗi nghiêm trọng
   */
  typedef enum uedp_itnlog_level_t {
    ITNLOG_LEVEL_DEBUG = 0,
    ITNLOG_LEVEL_INFO,
    ITNLOG_LEVEL_WARN,
    ITNLOG_LEVEL_ERROR,
    ITNLOG_LEVEL_FATAL
  } uedp_itnlog_level_t;

  /**
   * @brief Định nghĩa các thẻ log mặc định cho các module của UEDP
   */
  #define ITNLOG_TAG_TSK  "TSK"
  #define ITNLOG_TAG_MSG  "MSG"
  #define ITNLOG_TAG_FSM  "FSM"
  #define ITNLOG_TAG_TSM  "TSM"
  #define ITNLOG_TAG_TIM  "TIM"

  /**
   * @brief Định nghĩa cấu trúc log
   * @param level Mức độ log
   * @param tag Thẻ log để phân loại log theo module
   * @param task_id ID của tác vụ liên quan đến log (nếu có)
   * @param msg_sig Sig của tin nhắn liên quan đến log (nếu có)
   * @param fsm Con trỏ đến FSM liên quan đến log (nếu có)
   * @param tsm Con trỏ đến TSM liên quan đến log (nếu có)
   * @param msg Nội dung log
   * @param tmstmp Thời gian log (đơn vị: ms)
   * @param hash Giá trị hash của log entry, 
   *        được tính toán dựa trên nội dung của log entry 
   *        để đảm bảo tính toàn vẹn của dữ liệu log.
   *        Tránh việc ghi đè hoặc mất dữ liệu log 
   *        do lỗi ghi log hoặc lỗi bộ nhớ.
   * @attention `fsm` và `tsm` là con trỏ đến các cấu trúc FSM và TSM tương ứng, 
   *            cho phép log ghi lại thông tin chi tiết về trạng thái 
   *            và chuyển đổi trạng thái của FSM và TSM liên quan đến log đó.
   *            Nếu không có sử dụng `fsm` hoặc `tsm`, có thể để con trỏ này là NULL 
   *            để biểu thị không có thông tin FSM hoặc TSM liên quan đến log đó.
   */
  typedef struct uedp_itnlog_entry_t {
    uedp_itnlog_level_t level;
    const char* tag;
    ui16 task_id;
    ui16 msg_sig;
    const char* msg;
    ui32 tmstmp;
    ui16 hash; 
  } uedp_itnlog_entry_t;

  /**
   * @brief Định nghĩa hàm output cho internal logger
   * @note Hàm này sẽ được sử dụng để xuất dữ liệu log ra đích đến, 
   *       có thể là console, file hoặc giao diện
   * @note Hàm này để người dùng có thể tùy chỉnh cách thức xuất dữ liệu log 
   *       của internal logger một cách linh hoạt
   */
  typedef void (*uedp_itnlog_output_func_t)(const char*);

  /**
   * @brief Ghi chú các hàm chức năng của internal logger
   * 1 ham init
   * 1 ham log <-> push
   * 1 ham clear <-> pop
   * 1 ham dump <-> pop all + print (nen co support for printf)
   * 1 ham get log count (optional)
   * 1 ham set log level (optional)
   * 1 ham get log level (optional)
   * 1 ham set log tag (optional)
   * 1 ham get log tag (optional)
   * 1 ham set log filter (optional)
   * 1 ham get log filter (optional)
   * 1 ham set log output (dung cho phien ban 1.0.2 sap toi)
   */

  /**
   * @brief Khởi tạo internal logger
   */
  void uedp_itnlog_init(void);

  /**
   * @brief Ghi một log entry vào internal logger
   * @param timestamp Thời gian log (đơn vị: ms), sử dụng HAL_GetTick 
   *                  hoặc tương đương để lấy timestamp hiện tại khi gọi hàm này
   * @param level Mức độ log
   * @param msg Nội dung log cần ghi
   * @param tag Thẻ log để phân loại log theo module
   * @attention Hàm này sẽ tự động lấy thông tin về task_id, msg_sig, fsm, tsm, tmstmp 
   *            và chksum dựa trên ngữ cảnh điều phối hiện tại của uedp_task_scheduler,
   *            do đó người dùng chỉ cần cung cấp nội dung log (msg) khi gọi hàm này 
   *            để ghi log một cách tiện lợi và nhanh chóng.
   * @note Sẽ bổ sung thêm rule để giới hạn độ dài của msg để tránh việc ghi log quá dài
   */
  void uedp_itnlog_log(ui32 timestamp, uedp_itnlog_level_t level, const char* tag, const char* msg);

  /**
   * @brief Xóa một log entry khỏi internal logger và trả về log entry đã xóa
   * @return itnlog_entry_t Log entry đã được xóa khỏi internal logger, 
   *         bao gồm tất cả các trường thông tin như level, tag, task_id, msg_sig, fsm, tsm, msg, tmstmp 
   *         và chksum của log entry đó.
   */
  uedp_itnlog_entry_t uedp_itnlog_clear(void);

  /**
   * @brief Dump tất cả log entry hiện có trong internal logger
   * @note Hàm này sẽ cần bổ sung thêm support cho đích đến để dữ liệu log được xuất ra, 
   *       có thể là console, file hoặc giao diện màn hình.
   */
  void uedp_itnlog_dump(void);

  /**
  * @brief Lấy số lượng log entry hiện có trong internal logger
  * @return ui16 Số lượng log entry hiện có trong internal logger, 
  *         giúp người dùng biết được số lượng log entry đã được ghi lại 
  *         và đang lưu trữ trong internal logger.
  */
  ui16 uedp_itnlog_get_log_count(void);

  /**
   * @brief Đặt mức độ log cho internal logger
   * @param level Mức độ log cần đặt cho internal logger, 
   *              giúp người dùng có thể điều chỉnh mức độ log 
   *              mà internal logger sẽ ghi lại
   */
  void uedp_itnlog_set_level(uedp_itnlog_level_t level);

  /**
   * @brief Lấy mức độ log hiện tại của internal logger
   * @return uedp_itnlog_level_t Mức độ log hiện tại của internal logger, 
   *         giúp người dùng biết được mức độ log mà internal logger đang sử dụng
   */
  uedp_itnlog_level_t uedp_itnlog_get_level(void);

  /**
   * @brief Lấy thẻ log hiện tại của internal logger
   * @return const char* Thẻ log hiện tại của internal logger
   */
  const char* uedp_itnlog_get_tag(void);

  /**
   * @brief Đặt thẻ log cho internal logger
   * @param tag Thẻ log cần đặt cho internal logger
   */
  void uedp_itnlog_set_tag(const char* tag);

  /**
   * @brief Đặt bộ lọc log cho internal logger
   * @param level Mức độ log cần lọc
   * @param tag Thẻ log cần lọc
   */
  void uedp_itnlog_set_filter(uedp_itnlog_level_t level, const char* tag);

  /**
   * @brief Lấy bộ lọc log hiện tại của internal logger
   * @param level Con trỏ đến biến lưu trữ mức độ log được lọc
   * @param tag Con trỏ đến biến lưu trữ thẻ log được lọc
   */
  void uedp_itnlog_get_filter(uedp_itnlog_level_t* level, char* tag);

  /**
   * @brief Đặt hàm đầu ra cho internal logger
   * @param output_func Hàm đầu ra cần đặt cho internal logger
   * @note Hàm này sẽ bổ trợ cho hàm dump để xuất dữ liệu log ra đích đến 
   *       đã được chỉ định thông qua output_func,
   *       giúp người dùng có thể tùy chỉnh cách thức 
   *       xuất dữ liệu log của internal logger một cách linh hoạt 
   *       và phù hợp với nhu cầu sử dụng của mình.
   */
  void uedp_itnlog_set_output(void (*output_func)(const char*));

#endif // __UEDP_ITNLOG_H__