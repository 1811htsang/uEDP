/**
 * @file pal_core.h
 * @author Shang Huang 
 * @brief Header file for UEDP PAL Core Definition
 * @version 0.1
 * @date 2026-04-19
 * @copyright MIT License
 */
#ifndef __PAL_CORE_H__
  #define __PAL_CORE_H__

  /**
   * @brief Khai báo thư viện sử dụng
   */
  #include <stdint.h>

  /**
   * @brief Định nghĩa cờ debug cho hệ thống UEDP
   */
  #ifndef UEDP_DEBUG_FLAG
    #define UEDP_DEBUG_FLAG				(0x00u) // Tắt chế độ debug theo mặc định
  #endif

  /**
   * @brief Định nghĩa các bit cờ debug cho hệ thống UEDP
   * 
   */

  #define DBG_MSG_TIMESTAMP_BIT          (0x01u)
  #define DBG_TASK_FLOW_BIT              (0x02u)
  #define DBG_POOL_USAGE_BIT             (0x04u)

  /**
   * @brief Định nghĩa các kiểu dữ liệu cơ bản cho hệ thống UEDP
   */

  #define u unsigned
  #define ul unsigned long
  #define ui unsigned int
  #define ui8 uint8_t
  #define ui16 uint16_t
  #define ui32 uint32_t

  /**
   * @brief Khai báo các từ khóa viết tắt cho các phạm vi truy cập 
   *        và loại liên kết của biến và hàm trong hệ thống UEDP
   * @attention Các từ khóa này được thiết kế để cải thiện tính rõ ràng 
   *            và nhất quán trong việc khai báo các biến và hàm, 
   *            giúp người dùng dễ dàng nhận biết phạm vi truy cập 
   *            và loại liên kết của chúng trong hệ thống UEDP.
   */

  #define sta static
  #define ext extern
  #define inl inline
  #define stinl static inline
  #define vol volatile

  /**
   * @brief Định nghĩa các attribute đối với compiler
   */

  #define UEDP_ATTR_PACKED					__attribute__((packed))     // Đóng gói cấu trúc dữ liệu
  #define UEDP_ATTR_ALIGNED(x)			__attribute__((aligned(x))) // Căn chỉnh dữ liệu theo kích thước x
  #define UEDP_ATTR_SECTION(x)			__attribute__((section(x))) // Lưu một section cụ thể trong bộ nhớ
  #define UEDP_ATTR_WEAK						__attribute__((weak))       // Cho phép được ghi đè bởi một định nghĩa khác
  #define UEDP_ATTR_UNUSED					__attribute__((unused))     // Tránh cảnh báo các hàm hoặc biến không được sử dụng

  /**
   * @brief Định nghĩa kiểu dữ liệu trả về cho các hàm trong hệ thống UEDP
   * @attention `RETR_STAT` được thiết kế để quản lý các trạng thái trả về của các hàm trong hệ thống UEDP, 
   *            bao gồm các trạng thái như OK, ERROR, BUSY, TIMEOUT, DONE, NRDY và RDY, 
   *            giúp người dùng dễ dàng xác định kết quả của các thao tác và xử lý lỗi một cách hiệu quả.
   */
  typedef enum RETR_STAT {
    STAT_OK       = 0x00U,
    STAT_ERROR    = 0x01U,
    STAT_BUSY     = 0x02U,
    STAT_TIMEOUT  = 0x03U,
    STAT_DONE     = 0x04U,
    STAT_NRDY     = 0x05U,
    STAT_RDY      = 0x06U
  } RETR_STAT;

  /**
   * @brief Định nghĩa các hằng số boolean và trạng thái cơ bản cho hệ thống UEDP
   */
  
  #define UEDP_TRUE						  (0x01u)
  #define UEDP_FALSE						(0x00u)
  #define UEDP_ENABLE						UEDP_TRUE
  #define UEDP_DISABLE					UEDP_FALSE
  #define UEDP_FLAG_ON					UEDP_TRUE
  #define UEDP_FLAG_OFF					UEDP_FALSE
  #define UEDP_RET_OK						UEDP_TRUE
  #define UEDP_RET_NG						UEDP_FALSE

  /**
   * @brief Định nghĩa mã lỗi cho hệ thống UEDP
   * @note Cần sửa lại cái này thành enum để dễ quản lý hơn
   */

  #define UEDP_ERR_MSG_POOL   (0x01u) // Lỗi liên quan đến quản lý Pool tin nhắn
  #define UEDP_ERR_TASK       (0x02u) // Lỗi liên quan đến quản lý tác vụ
  #define UEDP_ERR_SCHEDULER  (0x03u) // Lỗi liên quan đến bộ lập lịch tác vụ
  #define UEDP_ERR_INTERFACE  (0x04u) // Lỗi liên quan đến quản lý các interface

  /**
   * @brief Định nghĩa số lượng hàm callback tối đa có thể đăng ký để xuất dữ liệu log trong hệ thống UEDP
   */
  #define PAL_LOGDP_MAX_OUTPUT_FN  (4u) // Số lượng hàm callback tối đa có thể đăng ký để xuất dữ liệu log

  /**
   * @brief Định nghĩa kiểu dữ liệu con trỏ hàm trả về void và không nhận tham số,
   *        được sử dụng phổ biến trong hệ thống UEDP để định nghĩa các callback hoặc các hàm xử lý sự kiện.
   */
  typedef void (*uedp_void_func_t)(void);

  /**
   * @brief Khai báo hàm khởi tạo PAL Core cho nền tảng
   */
  UEDP_ATTR_WEAK void pal_core_init(void);

  /**
   * @brief Khai báo các hàm PAL Core cần thiết cho việc triển khai đa nền tảng
   */

  UEDP_ATTR_WEAK void pal_enter_critical(void);
  UEDP_ATTR_WEAK void pal_exit_critical(void);

  /**
   * @brief Tìm bit có trọng số cao nhất (Dùng cho Priority Lookup)
   * @return 0-32 (vị trí bit) hoặc 0xFF nếu mask = 0
   */
  UEDP_ATTR_WEAK ui8 pal_math_get_highest_bit32(ui32 mask);

  /**
   * @brief Lấy tick hệ thống hiện tại (ví dụ: số ms kể từ khi khởi động)
   * @return ui32 Tick hệ thống hiện tại
   */
  UEDP_ATTR_WEAK ui32 pal_sys_get_tick(void);

  /**
   * @brief Reset hệ thống (ví dụ: khởi động lại phần cứng hoặc ứng dụng)
   */
  UEDP_ATTR_WEAK void pal_sys_reset(void);

  /**
   * @brief Xử lý lỗi nghiêm trọng (Core Panic)
   * @param file Tên tệp chứa lỗi
   * @param line Số dòng chứa lỗi
   * @param msg Thông báo lỗi
   */
  UEDP_ATTR_WEAK void pal_sys_fatal(const char* file, ui32 line, const char* msg);

  /**
   * @brief Macro tiện lợi để gọi hàm panic với thông tin file và line tự động điền
   */
  #define UEDP_PANIC(m)   pal_sys_fatal(__FILE__, __LINE__, m)

#endif