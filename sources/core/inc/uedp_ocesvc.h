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
   * @param dbugid: ID chỉ phục vụ mục đích debug/trace và sinh code PLTF - KHÔNG dùng cho bất kỳ
   *                logic quản lý/định tuyến nào của core (xoá/tìm kiếm service đều dựa trên con trỏ
   *                `svc`, không dựa trên giá trị này). Xem docs/uels-syntax.md (mục OCE) và
   *                docs/review/ocesvc-mexecjn.md để biết bối cảnh quyết định đổi tên từ `id`.
   * @param state: Trạng thái hiện tại của dịch vụ OCE
   * @param handler: Con trỏ đến hàm xử lý của dịch vụ OCE
   * @param context: Con trỏ đến dữ liệu ngữ cảnh của dịch vụ OCE
   * @param next: Con trỏ đến dịch vụ OCE tiếp theo trong danh sách liên kết đơn (dùng cho hàng đợi FCFS)
   */
  typedef struct ocesvc_t {
    uint8_t         dbugid;         
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
   * @note Hàm này đặt trạng thái của dịch vụ OCE thành READY. KHÔNG còn tự động gán/quản lý
   *       `dbugid` - core không đọc/ghi field này nữa ở bất kỳ đâu. Nếu cần, người dùng tự
   *       gán `svc->dbugid` trước khi gọi hàm này, hoàn toàn tuỳ chọn và chỉ phục vụ mục
   *       đích debug/trace cá nhân.
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
   * @attention Node đầu tiên của danh sách liên kết đơn có dbugid là UINT8_MAX để đánh dấu danh sách rỗng (sentinel).
   */
  void ocesvc_ctrl_init();

#endif // __OCESVC_H__