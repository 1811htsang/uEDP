/**
 * @file uedp_ocesvc.h
 * @author Shang Huang
 * @brief This file contains the declaration of the UEDP OCE service class.
 * @version 0.1
 * @date 2026-06-26
 * @copyright MIT License
 */
#ifndef __OCESVC_H__
  #define __OCESVC_H__

  /**
   * @brief Khai báo các thư viện sử dụng
   */
  #include <stdint.h>
  #include <stdbool.h>
  #include "uedp_core.h"
  #include "uedp_task.h"
  #include "llist.h"

  /**
   * @brief Khai báo bộ quản lý trạng thái OCE
   */

  typedef enum ocesvc_state_t {
    OCESVC_STATE_IDLE = 0,    /* Dịch vụ đang nghỉ */
    OCESVC_STATE_READY,       /* Được trigger, đang chờ đến lượt */
    OCESVC_STATE_RUNNING,     /* Đang thực thi bước hiện tại */
    OCESVC_STATE_COMPLETED,   /* Đã hoàn thành toàn bộ công việc */
    OCESVC_STATE_ERROR        /* Gặp sự cố trong quá trình thực thi */
  } ocesvc_state_t;

  /**
   * @brief Khai báo cấu trúc dữ liệu cho dịch vụ OCE
   * @param id: ID của dịch vụ OCE
   * @param state: Trạng thái hiện tại của dịch vụ OCE
   * @param handler: Con trỏ đến hàm xử lý của dịch vụ OCE
   * @param context: Con trỏ đến dữ liệu ngữ cảnh của dịch vụ OCE
   * @param next: Con trỏ đến dịch vụ OCE tiếp theo trong danh sách liên kết đơn (dùng cho hàng đợi FCFS)
   */
  typedef struct ocesvc_t {
    uint8_t         id;             
    ocesvc_state_t  state;          
    void (*handler)(struct ocesvc_t* me); 
    void*           context;        
    struct ocesvc_t* next;          /* Danh sách liên kết đơn cho hàng đợi FCFS */
  } ocesvc_t;

  /**
   * @brief Khai báo cấu trúc dữ liệu cho bộ điều khiển dịch vụ OCE
   * @param head: Con trỏ đến dịch vụ OCE đầu tiên trong danh sách liên kết đơn
   * @param fill_size: Số lượng dịch vụ OCE hiện có trong danh sách
   */
  typedef struct ocesvc_ctrl_t {
    ocesvc_t* head;
    uint8_t   fill_size;
  } ocesvc_ctrl_t;

  /**
   * @brief Hàm đăng ký dịch vụ OCE vào bộ điều khiển dịch vụ OCE
   * @param svc Con trỏ đến dịch vụ OCE cần đăng ký
   * @note Hàm này sẽ gán ID cho dịch vụ OCE và đặt trạng thái của nó thành READY.
   * @attention ID của dịch vụ OCE sẽ được tự động tăng dần từ 0, 
   * và không được trùng lặp với các dịch vụ OCE khác đã đăng ký.
   * Nghĩa là dù người dùng đăng ký OCE với ID bất kỳ, 
   * nhưng hệ thống sẽ gán lại ID cho OCE theo thứ tự tăng dần.
   */
  void ocesvc_register(ocesvc_t* svc);

  /**
   * @brief Hàm hủy đăng ký dịch vụ OCE khỏi bộ điều khiển dịch vụ OCE
   * @param svc Con trỏ đến dịch vụ OCE cần hủy đăng ký
   */
  void ocesvc_unregister(ocesvc_t* svc);

  /**
   * @brief Hàm thực thi dịch vụ OCE theo cơ chế FCFS
   */
  void ocesvc_scheduler();

  /**
   * @brief Hàm khởi tạo bộ điều khiển dịch vụ OCE
   * @note Hàm này cần được gọi trước khi sử dụng bất kỳ dịch vụ OCE nào.
   * Thực hiện việc khởi tạo danh sách liên kết đơn và đặt fill_size về 0.
   * @attention Node đầu tiên của danh sách liên kết đơn có id là -1 để đánh dấu danh sách rỗng.
   */
  void ocesvc_ctrl_init();

#endif // __OCESVC_H__