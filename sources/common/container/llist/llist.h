/**
 * @file llist.h
 * @author Shang Huang
 * @brief Linked list definitions and utilities for UEDP system
 * @version 0.1
 * @date 2026-07-07
 * @copyright MIT License
 */
#ifndef __LLIST_H__
  #define __LLIST_H__

  /**
   * @brief Khai báo các thư viện sử dụng
   */
  #include <stdint.h>
  #include <stdbool.h>
  #include <string.h>

  /**
	 * @brief Định nghĩa giá trị trả về của các hàm FIFO
	 */
	#define RET_FIFO_OK				(0x01)
	#define RET_FIFO_NG				(0x00)

  /**
   * @brief Định nghĩa cấu trúc node của linked list
   * @param data: Con trỏ đến dữ liệu của node
   * @param next: Con trỏ đến node tiếp theo trong linked list
   */
  typedef struct llist_node_t {
    void* data;
    struct llist_node_t* next;
  } llist_node_t;

  /**
   * @brief Định nghĩa cấu trúc linked list
   * @param head: Con trỏ đến node đầu tiên của linked list
   * @param tail: Con trỏ đến node cuối cùng của linked list
   * @param size: Kích thước hiện tại của linked list (số lượng node)
   */
  typedef struct llist_t {
    llist_node_t* head;
    llist_node_t* tail;
    uint32_t size;
  } llist_t;

  /**
   * @brief Khởi tạo linked list
   * @param list Con trỏ đến cấu trúc linked list cần khởi tạo
   */
  void llist_init(llist_t* list);

  /**
   * @brief Kiểm tra xem linked list có rỗng hay không
   * @param list Con trỏ đến cấu trúc linked list cần kiểm tra
   * @return true Nếu linked list rỗng
   * @return false Nếu linked list không rỗng
   */
  bool llist_is_empty(llist_t* list);

  /**
   * @brief Thêm một node vào cuối linked list
   * @param list Con trỏ đến cấu trúc linked list cần thêm node
   * @param data Con trỏ đến dữ liệu của node cần thêm
   */
  void llist_append(llist_t* list, void* data);

  /**
   * @brief Xóa một node khỏi linked list dựa trên dữ liệu của nó
   * @param list Con trỏ đến cấu trúc linked list cần xóa node
   * @param data Con trỏ đến dữ liệu của node cần xóa
   * @return true Nếu xóa thành công
   * @return false Nếu không tìm thấy node với dữ liệu tương ứng
   */
  bool llist_remove(llist_t* list, void* data);

#endif