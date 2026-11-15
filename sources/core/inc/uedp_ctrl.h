/**
 * @file uedp_ctrl.h
 * @author Shang Huang
 * @brief Control and monitoring utilities for UEDP system
 * @version 0.1
 * @date 2026-11-15
 * @copyright MIT License
 */
#ifndef __CONTROL_H__
  #define __CONTROL_H__

  // ANCHOR - Khai báo các thư viện sử dụng
  #include "uedp_core.h"

  // ANCHOR - Định nghĩa các loại Pool tin nhắn (Nội bộ Core sử dụng)
  typedef enum uedp_ctrl_pnt_t {
    UEDP_CTRL_BEFORE_SCHEDULER = 0,
    UEDP_CTRL_AFTER_SCHEDULER,
    UEDP_CTRL_BEFORE_TIMER_TICK,
    UEDP_CTRL_AFTER_TIMER_TICK,
    UEDP_CTRL_BEFORE_TASK_DISPATCH,
    UEDP_CTRL_AFTER_TASK_DISPATCH,
    UEDP_CTRL_BEFORE_FSM_TRANSITION,
    UEDP_CTRL_AFTER_FSM_TRANSITION,
    UEDP_CTRL_BEFORE_TSM_TRANSITION,
    UEDP_CTRL_AFTER_TSM_TRANSITION,
    UEDP_CTRL_CUSTOM_EVENT
    // CRITICAL - Các điểm kiểm soát khác có thể được thêm vào đây nếu cần thiết
  } uedp_ctrl_pnt_t;

  /** ANCHOR - Cấu trúc quản lý sự kiện kiểm soát (Control Event) trong hệ thống
   * @param point: Điểm kiểm soát (Control Point) mà sự kiện xảy ra, xác định vị trí trong luồng thực thi
   * @param task_id: ID của tác vụ liên quan đến sự kiện, nếu có
   * @param signal: Tín hiệu liên quan đến sự kiện, nếu có
   * @param value_a: Giá trị bổ sung A, có thể được sử dụng để truyền thông tin thêm về sự kiện
   * @param value_b: Giá trị bổ sung B, có thể được sử dụng để truyền thông tin thêm về sự kiện
   * @param status: Trạng thái kết quả của sự kiện, giúp xác định kết quả của thao tác hoặc hành động liên quan
   * @param tick: Thời điểm (tick) mà sự kiện xảy ra, giúp theo dõi thời gian và thứ tự các sự kiện
   * @param subject: Con trỏ đến đối tượng liên quan đến sự kiện, có thể là tác vụ, FSM, TSM hoặc bất kỳ đối tượng nào khác trong hệ thống
	 */
  typedef struct uedp_ctrl_evt_t {
    uedp_ctrl_pnt_t point;
    ui8 task_id;
    ui8 signal;
    ui32 value_a;
    ui32 value_b;
    RETR_STAT status;
    ui32 tick;
    const void* subject;
  } uedp_ctrl_evt_t;

  /** ANCHOR - Cấu trúc quản lý một bước trong chuỗi sự kiện kiểm soát (Control Sequence Step)
   * @param point: Điểm kiểm soát (Control Point) mà sự kiện xảy ra, xác định vị trí trong luồng thực thi
   * @param task_id: ID của tác vụ liên quan đến sự kiện, nếu có
   * @param signal: Tín hiệu liên quan đến sự kiện, nếu có
	 */
  typedef struct uedp_seq_step_t {
    uedp_ctrl_pnt_t point;
    ui8             task_id;
    ui8             signal;
  } uedp_seq_step_t;

  /** ANCHOR - Định nghĩa các kiểu dữ liệu để quản lý hàm callback cho việc tiêm (inject) và theo dõi (trace) các sự kiện kiểm soát
   * @attention `uedp_ctrl_inject_fn` được sử dụng để quản lý các hàm callback cho việc tiêm các sự kiện kiểm soát vào luồng thực thi, 
   * 						giúp kiểm tra và theo dõi các điểm quan trọng trong hệ thống.
   * @attention `uedp_ctrl_trace_fn` được sử dụng để quản lý các hàm callback cho việc theo dõi các sự kiện kiểm soát, 
   * 						giúp ghi lại thông tin về các sự kiện xảy ra trong hệ thống để phục vụ cho việc phân tích và gỡ lỗi.
   */
  typedef void (*uedp_ctrl_inject_fn)(uedp_ctrl_pnt_t point, void* context);
  typedef void (*uedp_ctrl_trace_fn)(const uedp_ctrl_evt_t* event, void* context);

  /** ANCHOR - Đăng ký các hàm callback để tiêm (inject) các sự kiện kiểm soát trong hệ thống UEDP
   * @param callback Hàm callback sẽ được gọi khi một sự kiện kiểm soát xảy ra
   * @param context Con trỏ đến dữ liệu ngữ cảnh sẽ được truyền vào hàm callback
   */
  void uedp_ctrl_set_injector(uedp_ctrl_inject_fn callback, void* context);

  /** ANCHOR - Đăng ký các hàm callback để theo dõi (trace) các sự kiện kiểm soát trong hệ thống UEDP
   * @param callback Hàm callback sẽ được gọi khi một sự kiện kiểm soát xảy ra
   * @param context Con trỏ đến dữ liệu ngữ cảnh sẽ được truyền vào hàm callback
   */
  void uedp_ctrl_set_tracer(uedp_ctrl_trace_fn callback, void* context);

  /** ANCHOR - Tiêm (inject) một sự kiện kiểm soát vào luồng thực thi của hệ thống UEDP
   * @param point Điểm kiểm soát (Control Point) mà sự kiện sẽ được tiêm vào
   */
  void uedp_ctrl_inject(uedp_ctrl_pnt_t point);
  
  /** ANCHOR - Ghi lại (trace) một sự kiện kiểm soát trong hệ thống UEDP
   * @param event Con trỏ đến cấu trúc sự kiện kiểm soát sẽ được ghi lại
   */
  void uedp_ctrl_trace(const uedp_ctrl_evt_t* event);
  
  /** ANCHOR - Ghi lại (trace) một sự kiện kiểm soát tùy chỉnh trong hệ thống UEDP
   * @param custom_id ID tùy chỉnh để xác định sự kiện kiểm soát
   * @param val_a Giá trị bổ sung A, có thể được sử dụng để truyền thông tin thêm về sự kiện
   * @param val_b Giá trị bổ sung B, có thể được sử dụng để truyền thông tin thêm về sự kiện
   * @param payload Con trỏ đến dữ liệu bổ sung liên quan đến sự kiện, có thể là bất kỳ đối tượng nào trong hệ thống
   */
  void uedp_ctrl_trace_custom(ui16 custom_id, ui32 val_a, ui32 val_b, const void* payload);
  
  /** ANCHOR - Thiết lập chế độ tự động tick cho hệ thống UEDP
   * @param enabled true để bật chế độ tự động tick, false để tắt
   */
  void uedp_ctrl_set_auto_tick(bool enabled);
  
  /** ANCHOR - Kiểm tra xem chế độ tự động tick có đang được bật hay không trong hệ thống UEDP
   * @return true nếu chế độ tự động tick đang được bật, false nếu không
   */
  bool uedp_ctrl_auto_tick_enabled(void);

  /** ANCHOR - Chạy bộ lập lịch (scheduler) của hệ thống UEDP
   * @param max_steps Số lượng bước tối đa sẽ được thực hiện
   * @param executed_steps Con trỏ đến biến sẽ lưu số lượng bước đã thực hiện
   * @return Mã trạng thái của thao tác
   */
  RETR_STAT uedp_ctrl_run_scheduler(ui32 max_steps, ui32* executed_steps);
  
  /** ANCHOR - Tiến tới một số lượng tick nhất định trong hệ thống UEDP
   * @param tick_count Số lượng tick sẽ được tiến tới
   * @param run_scheduler true để chạy bộ lập lịch sau khi tiến tick, false để không chạy
   * @param executed_steps Con trỏ đến biến sẽ lưu số lượng bước đã thực hiện
   * @return Mã trạng thái của thao tác
   */
  RETR_STAT uedp_ctrl_advance_ticks(ui32 tick_count, bool run_scheduler, ui32* executed_steps);

  /** ANCHOR - Kiểm tra xem một chuỗi sự kiện có diễn ra đúng thứ tự trong lịch sử Trace không
   * @param expected_seq Mảng các bước mong đợi
   * @param seq_len Số lượng bước trong chuỗi
   * @param strict_order true: Phải diễn ra liên tiếp không có sự kiện xen ngang
   *                     false: Chỉ cần đúng thứ tự thời gian (cho phép sự kiện khác xen vào)
   * @return true nếu khớp chuỗi, false nếu sai thứ tự hoặc thiếu bước
   */
  bool uedp_ctrl_verify_sequence(const uedp_seq_step_t* expected_seq, size_t seq_len, bool strict_order);

#endif