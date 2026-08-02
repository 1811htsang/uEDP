#ifndef __UEDP_FCR_H__
  #define __UEDP_FCR_H__

  /**
   * @brief Khai báo thư viện sử dụng
   */
  #include "uedp_core.h"

  /**
   * @brief Định nghĩa kiểu dữ liệu cho mã lỗi FCR
   * @attention Mã lỗi FCR được thiết kế theo encoding tương tự các dải tín hiệu khác
   *            trong UEDP (xem [HES] Heximal Encoding Signals ở arch-design.md):
   *            byte cao (bit 15-8) là mã MODULE (module nào phát sinh lỗi),
   *            byte thấp (bit 7-0) là mã SUB-CODE (lỗi cụ thể trong module đó).
   *            Nhờ vậy 1 module có thể có tối đa 256 mã lỗi con khác nhau.
   */
  typedef ui16 uedp_fcr_code_t;

  /**
   * @brief Khai báo dải mã MODULE cho FCR
   * @attention Dải `0x9x` được chọn vì các dải `0xAx` -> `0xFx` đã được
   *            sử dụng cho TASK_NORM/TASK_POLL/TASK_PRI/FSM_SIG/TSM_SIG/TSM_STATE
   *            (xem uedp_core.h), tránh xung đột định danh trong toàn hệ thống.
   */
  #define UEDP_FCR_MOD_MSG      (0x90u) // Module quản lý tin nhắn (uedp_msg)
  #define UEDP_FCR_MOD_TASK     (0x91u) // Module quản lý tác vụ (uedp_task)
  #define UEDP_FCR_MOD_TIMER    (0x92u) // Module quản lý timer (uedp_timer)
  #define UEDP_FCR_MOD_SM       (0x93u) // Module máy trạng thái (uedp_fsm / uedp_tsm)
  #define UEDP_FCR_MOD_ITNLOG   (0x94u) // Module logger nội bộ (uedp_itnlog)
  #define UEDP_FCR_MOD_OCE      (0x95u) // Module Out-Context Execution (uedp_ocesvc)
  #define UEDP_FCR_MOD_PAL      (0x96u) // Module PAL / dịch vụ phần cứng (logdp, rprintf, memrp, arch...)
  #define UEDP_FCR_MOD_APP      (0x9Eu) // Dành riêng cho tầng ứng dụng tự khai báo mã lỗi
  #define UEDP_FCR_MOD_UNK      (0x9Fu) // Module không xác định / fallback

  /**
   * @brief Macro tiện lợi để ghép mã MODULE và SUB-CODE thành 1 uedp_fcr_code_t
   * @param mod Mã module, xem các hằng số UEDP_FCR_MOD_*
   * @param sub Mã lỗi con trong module đó (0x00 -> 0xFF)
   */
  #define UEDP_FCR_CODE(mod, sub)   ((uedp_fcr_code_t)(((ui16)(mod) << 8) | (ui16)(sub)))

  /**
   * @brief Bảng mã lỗi nghiêm trọng của lõi UEDP
   * @attention Đây KHÔNG phải danh sách đầy đủ - người dùng có thể bổ sung thêm
   *            mã lỗi riêng cho tầng ứng dụng bằng cách dùng UEDP_FCR_CODE(UEDP_FCR_MOD_APP, x)
   *            và tự đăng ký entry tương ứng nếu cần (xem uedp_fcr_raise()).
   */

  // [MSG] - 0x90xx
  #define UEDP_FCR_MSG_POOL_EXHAUSTED     UEDP_FCR_CODE(UEDP_FCR_MOD_MSG, 0x00) // Pool tin nhắn (BLANK/ALLOC/EXTAL/ISR) đã hết chỗ trống
  #define UEDP_FCR_MSG_INVALID_PTR        UEDP_FCR_CODE(UEDP_FCR_MOD_MSG, 0x01) // Thao tác trên con trỏ tin nhắn không hợp lệ (không thuộc Pool nào)
  #define UEDP_FCR_MSG_ISR_FIFO_FULL      UEDP_FCR_CODE(UEDP_FCR_MOD_MSG, 0x02) // Hàng đợi FIFO nhận tín hiệu ISR đã đầy

  // [TASK] - 0x91xx
  #define UEDP_FCR_TASK_QUEUE_FULL        UEDP_FCR_CODE(UEDP_FCR_MOD_TASK, 0x00) // Hàng đợi tin nhắn nội bộ của 1 tác vụ đã đầy
  #define UEDP_FCR_TASK_INVALID_ID        UEDP_FCR_CODE(UEDP_FCR_MOD_TASK, 0x01) // ID tác vụ không tồn tại trong bảng tác vụ
  #define UEDP_FCR_TASK_PRI_EXHAUSTED     UEDP_FCR_CODE(UEDP_FCR_MOD_TASK, 0x02) // Hết mức ưu tiên tạm thời khi thực hiện Priority Escalation ([APE])

  // [TIMER] - 0x92xx
  #define UEDP_FCR_TIMER_POOL_EXHAUSTED   UEDP_FCR_CODE(UEDP_FCR_MOD_TIMER, 0x00) // Hết node trống trong pool timer (UEDP_TIMER_MAX_NODES)

  // [SM] (FSM/TSM) - 0x93xx
  #define UEDP_FCR_SM_INVALID_TRANS       UEDP_FCR_CODE(UEDP_FCR_MOD_SM, 0x00) // Không tìm thấy transition hợp lệ cho tín hiệu hiện tại trong TSM
  #define UEDP_FCR_SM_NULL_HANDLER        UEDP_FCR_CODE(UEDP_FCR_MOD_SM, 0x01) // Con trỏ hàm state hiện tại của FSM là NULL

  // [ITNLOG] - 0x94xx
  #define UEDP_FCR_ITNLOG_BUF_CORRUPT     UEDP_FCR_CODE(UEDP_FCR_MOD_ITNLOG, 0x00) // Dữ liệu ring buffer log nội bộ không toàn vẹn (hash mismatch)

  // [OCE] - 0x95xx
  #define UEDP_FCR_OCE_REGISTRY_FULL      UEDP_FCR_CODE(UEDP_FCR_MOD_OCE, 0x00) // Không thể đăng ký thêm dịch vụ OCE mới

  // [PAL] - 0x96xx
  #define UEDP_FCR_PAL_LOGDP_TABLE_FULL   UEDP_FCR_CODE(UEDP_FCR_MOD_PAL, 0x00) // Bảng đăng ký callback của logdp đã đầy

  // Fallback
  #define UEDP_FCR_UNKNOWN                UEDP_FCR_CODE(UEDP_FCR_MOD_UNK, 0xFF) // Mã lỗi không tra được trong bảng (không có entry tương ứng)

  /**
   * @brief Định nghĩa mức độ nghiêm trọng của một mã lỗi FCR
   * @param UEDP_FCR_SEV_WARN Chỉ cảnh báo, hệ thống vẫn tiếp tục hoạt động bình thường
   * @param UEDP_FCR_SEV_ERROR Lỗi có ảnh hưởng cục bộ, cần can thiệp ở mức tác vụ/module
   * @param UEDP_FCR_SEV_FATAL Lỗi nghiêm trọng, có thể ảnh hưởng tới toàn bộ hệ thống
   */
  typedef enum uedp_fcr_severity_t {
    UEDP_FCR_SEV_WARN = 0,
    UEDP_FCR_SEV_ERROR,
    UEDP_FCR_SEV_FATAL
  } uedp_fcr_severity_t;

  /**
   * @brief Định nghĩa hành động xử lý tương ứng khi một mã lỗi FCR được raise
   * @param UEDP_FCR_ACT_LOG_ONLY Chỉ ghi log qua itnlog, không can thiệp vào luồng chạy
   * @param UEDP_FCR_ACT_RESET_TASK Đánh dấu để tầng trên (task/app) tự khôi phục lại tác vụ liên quan
   *        (bản thân uedp_fcr KHÔNG tự ý reset trạng thái TSM/FSM của tác vụ khác, chỉ ghi log + trả về)
   * @param UEDP_FCR_ACT_SYS_RESET Gọi pal_sys_reset() để khởi động lại toàn bộ hệ thống
   * @param UEDP_FCR_ACT_SYS_PANIC Gọi pal_sys_fatal() (UEDP_PANIC) để dừng hệ thống ngay lập tức
   */
  typedef enum uedp_fcr_action_t {
    UEDP_FCR_ACT_LOG_ONLY = 0,
    UEDP_FCR_ACT_RESET_TASK,
    UEDP_FCR_ACT_SYS_RESET,
    UEDP_FCR_ACT_SYS_PANIC
  } uedp_fcr_action_t;

  /**
   * @brief Khai báo 1 dòng trong bảng mã lỗi FCR
   * @param code Mã lỗi FCR (xem UEDP_FCR_CODE)
   * @param desc Mô tả ngắn gọn về lỗi, dùng khi ghi log hoặc panic
   * @param severity Mức độ nghiêm trọng của lỗi
   * @param action Hành động xử lý tương ứng khi lỗi này được raise
   */
  typedef struct uedp_fcr_entry_t {
    uedp_fcr_code_t      code;
    const char*           desc;
    uedp_fcr_severity_t   severity;
    uedp_fcr_action_t     action;
  } uedp_fcr_entry_t;

  /**
   * @brief Tra bảng mã lỗi FCR để lấy thông tin (mô tả, mức độ, hành động) tương ứng
   * @param code Mã lỗi FCR cần tra
   * @return const uedp_fcr_entry_t* Con trỏ đến entry tương ứng trong bảng,
   *         hoặc entry UEDP_FCR_UNKNOWN nếu không tìm thấy code trong bảng
   */
  const uedp_fcr_entry_t* uedp_fcr_lookup(uedp_fcr_code_t code);

  /**
   * @brief Raise (báo cáo) một lỗi nghiêm trọng FCR
   * @param code Mã lỗi FCR cần raise (xem các hằng số UEDP_FCR_*)
   * @param file Tên tệp phát sinh lỗi (thường truyền __FILE__)
   * @param line Số dòng phát sinh lỗi (thường truyền __LINE__)
   * @param extra_msg Thông tin bổ sung (có thể NULL), sẽ được nối thêm vào log
   * @note Hàm này sẽ:
   *       1. Tra bảng qua uedp_fcr_lookup() để lấy mô tả, mức độ và hành động.
   *       2. Ghi log qua uedp_itnlog_log() với tag ITNLOG_TAG_FCR, mức độ log
   *          tương ứng với severity (WARN/ERROR -> ITNLOG_LEVEL_WARN/ERROR,
   *          FATAL -> ITNLOG_LEVEL_FATAL).
   *       3. Thực hiện hành động xử lý tương ứng (action) - xem uedp_fcr_action_t.
   * @attention Với action = UEDP_FCR_ACT_SYS_PANIC, hàm này KHÔNG return
   *            (pal_sys_fatal thường sẽ abort()/reset hệ thống).
   */
  void uedp_fcr_raise(uedp_fcr_code_t code, const char* file, ui32 line, const char* extra_msg);

  /**
   * @brief Macro tiện lợi để gọi uedp_fcr_raise() với thông tin file/line tự động điền
   */
  #define UEDP_FCR_RAISE(code)              uedp_fcr_raise((code), __FILE__, __LINE__, NULL)
  #define UEDP_FCR_RAISE_MSG(code, extra)   uedp_fcr_raise((code), __FILE__, __LINE__, (extra))

#endif // __UEDP_FCR_H__
