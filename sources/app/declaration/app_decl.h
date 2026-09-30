//ANCHOR - Application declaration header file
#ifndef __APP_DECL_H__
  #define __APP_DECL_H__

  //ANCHOR - Khai báo các thư viện cần thiết cho ứng dụng

  #include "uedp_core.h"
  #include "uedp_task.h"
  #include "uedp_tsm.h"
  #include "uedp_fsm.h"
  #include "uedp_msg.h"
  #include "uedp_timer.h"

  /** ANCHOR - Khai báo tác vụ norm
   * @note Tác vụ phải khai báo đúng định dạng `0xEx`
   *   x bắt đầu từ 6 trở đi. Giới hạn tối đa là 0xEF (15 tác vụ)
   * @example
   *   #define TASK_NORM_A_ID (0xE6u)
   *   #define TASK_NORM_B_ID (0xE7u)
   * @attention Xin đừng sửa đổi, tự động sinh bởi Kconfiglib và Jinja2
   */
  
  #define TASK_NORM_USR        (0xE3u)
  #define TASK_NORM_A          (0xE4u)
  #define TASK_NORM_B          (0xE5u)
  
  /** ANCHOR - Khai báo tác vụ poll
   * @note Tác vụ phải khai báo đúng định dạng `0xDx`
   *   x bắt đầu từ 4 trở đi. Giới hạn tối đa là 0xDF (8 tác vụ)
   * @example
   *   #define TASK_POLL_A_ID (0xD4u)
   *   #define TASK_POLL_B_ID (0xD5u)
   * @attention Xin đừng sửa đổi, tự động sinh bởi Kconfiglib và Jinja2
   */

  #define TASK_POLL_MEMRP      (0xD0u)
  #define TASK_POLL_BLINK      (0xD1u)
  
  /** ANCHOR - Khai báo tín hiệu giao tiếp giữa các tác vụ
   * @attention Tác vụ phải khai báo đúng định dạng `0x0x`
   *   x bắt đầu từ 1 trở đi. Không nên vượt quá 0x7F 
   * để tránh trùng với tín hiệu nội bộ của hệ thống UEDP
   * @example
   *   #define SIG_USR_START     (0x01u)
   *   #define SIG_USR_STOP      (0x02u)
   *   #define SIG_TSK_A_TO_B    (0x03u)
   *   #define SIG_TSK_B_TO_A    (0x04u)
   * @attention Xin đừng sửa đổi, tự động sinh bởi Kconfiglib và Jinja2
   */
  
  #define SIG_USR_START        (0x01u)
  #define SIG_USR_STOP         (0x02u)
  #define SIG_0X34             (0x03u)
  #define SIG_0XFF             (0x04u)
  #define SIG_0X12             (0x05u)
  #define SIG_0XAA             (0x06u)
  #define SIG_HALT_NOW         (0x07u)
  
  /** ANCHOR - Khai báo message queue cho các tác vụ
   * @note Mỗi tác vụ sẽ có một hàng đợi tin nhắn riêng biệt
   *   Tùy thuộc vào nhu cầu của ứng dụng để điều chỉnh kích thước của hàng đợi, 
   *   nhưng cần đảm bảo không vượt quá giới hạn của hệ thống UEDP
   * @example 
   *   extern uedp_msg_t* usr_msgq[8];
   *   extern uedp_msg_t* a_q_msgq[8];
   *   extern uedp_msg_t* b_q_msgq[8];
   * @attention Xin đừng sửa đổi, tự động sinh bởi Kconfiglib và Jinja2
   */
  
  extern uedp_msg_t* tnorm_usr_msgq[UEDP_MSG_BLANK_QUEUE_SIZE];
  extern uedp_msg_t* tnorm_a_msgq[UEDP_MSG_BLANK_QUEUE_SIZE];
  extern uedp_msg_t* tnorm_b_msgq[UEDP_MSG_BLANK_QUEUE_SIZE];
  
  /** ANCHOR - Khai báo biến đếm hoạt động của hệ thống
   * @attention Nên khuyến khích sử dụng biến này để theo dõi số lượng hành động 
   *   đã thực hiện trong hệ thống khi task_scheduler được gọi,
   *   đặc biệt hữu ích trong các bài test để xác nhận rằng hệ thống đang hoạt động như mong đợi 
   *   và để phát hiện các vấn đề tiềm ẩn như vòng lặp vô hạn hoặc tắc nghẽn trong scheduler.
   */

  extern uint32_t system_action_count;

  /** ANCHOR - Khai báo các hàm handler cho các task norm
   * @attention Xin đừng sửa đổi, tự động sinh bởi Kconfiglib và Jinja2
   */

  void tnorm_usr_nhler(uedp_msg_t* msg);
  void tnorm_a_nhler(uedp_msg_t* msg);
  void tnorm_b_nhler(uedp_msg_t* msg);
  
  /** ANCHOR - Khai báo các hàm handler cho các task poll
   * @attention Xin đừng sửa đổi, tự động sinh bởi Kconfiglib và Jinja2
   */
  
  void tpoll_memrp_phler();
  void tpoll_blink_phler();
  
  /** ANCHOR - Khai báo các hàm on-entry/exit (tsmio) cho các trạng thái TSM (nếu có)
   * @attention Xin đừng sửa đổi, tự động sinh bởi Kconfiglib và Jinja2
   */
  
  void tsm_task_a_state_idle_ntry(uedp_msg_t* msg);
  void tsm_task_a_state_idle_exit(uedp_msg_t* msg);
  
  void tsm_task_a_state_waiting_ntry(uedp_msg_t* msg);
  void tsm_task_a_state_waiting_exit(uedp_msg_t* msg);
  
  void tsm_task_usr_state_idle_ntry(uedp_msg_t* msg);
  void tsm_task_usr_state_idle_exit(uedp_msg_t* msg);
  
  void tsm_task_usr_state_running_ntry(uedp_msg_t* msg);
  void tsm_task_usr_state_running_exit(uedp_msg_t* msg);
  
  /** ANCHOR - Khai báo các hàm on-state (aka on_actv) cho các trạng thái TSM (nếu có)
   * @attention Xin đừng sửa đổi, tự động sinh bởi Kconfiglib và Jinja2
   */
  
  void tsm_task_a_state_idle_onst(uedp_msg_t* msg);
  void tsm_task_a_state_waiting_onst(uedp_msg_t* msg);
  void tsm_task_usr_state_idle_onst(uedp_msg_t* msg);
  void tsm_task_usr_state_running_onst(uedp_msg_t* msg);
  
  /** ANCHOR - Khai báo các state_handler cho các trạng thái FSM (nếu có)
   * @attention Xin đừng sửa đổi, tự động sinh bởi Kconfiglib và Jinja2
   */

  void fsm_task_b_state_busy_onst(uedp_msg_t* msg);
  void fsm_task_b_state_idle_onst(uedp_msg_t* msg);
  
  // ANCHOR - Khai báo khác (nếu có)

  int app_main(void);

#endif //__APP_DECL_H__