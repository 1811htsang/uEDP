/**
 * @file pal_memrp.h
 * @author Shang Huang
 * @brief PAL Memory Reporter header file for UEDP project
 * @version 0.1
 * @date 2026-04-30
 * @copyright MIT License
 */
#ifndef __PAL_MEMRP_H__
  #define __PAL_MEMRP_H__

  /**
   * @brief Khai báo thư viện sử dụng
   */
  
  #include "uedp_core.h"
  #include "uedp_msg.h"

  /**
   * @brief Cấu trúc thông tin báo cáo bộ nhớ cho một target cụ thể
   * @attention `target` có thể là bất kỳ loại Pool nào (Blank, Alloc, Extal, Timer, ISR, Task Queue, v.v.), 
   * @param target Con trỏ đến mảng chứa các elements như pool, task message queue, timer pool, 
   *               sử dụng void* để có thể áp dụng cho bất kỳ loại Pool nào (Blank, Alloc, Extal, Timer, ISR, Task Queue, v.v.)
   * @param type Loại đối tượng bộ nhớ
   * @param used Số lượng elements đang được sử dụng trong Pool
   * @param max_used Số lượng elements tối đa đã từng được sử dụng trong Pool
   * @param total Tổng số elements có thể chứa trong Pool
   */
  typedef struct pal_memrp_info_t {
    void* target;
    const char* name; // Tên của target để dễ dàng nhận biết trong báo cáo, có thể là "Blank Pool", "Alloc Pool", "Extal Pool", "Timer Pool", "ISR Queue", "Task A Queue", v.v.
    uedp_msg_type_t type;
    ui8 used;
    ui8 max_used;
    ui8 total;
  } pal_memrp_info_t;

  typedef void (*pal_memrp_output_fn)(pal_memrp_info_t* info);

  /**
   * @brief Hàm báo cáo thông tin bộ nhớ của một target cụ thể
   * @param info Con trỏ đến cấu trúc chứa thông tin bộ nhớ cần báo cáo, bao gồm target, used, max_used và total
   */
  void pal_memrp_report(pal_memrp_info_t* info);

  /**
   * @brief Hàm thiết lập callback cho phép đưa memrp vào PPLP
   */
  void pal_memrp_set_output(pal_memrp_output_fn output_fn);

  /**
	 * @brief Hàm này được định nghĩa là weak để cho phép người dùng tùy chỉnh cách lấy thông tin bộ nhớ phần cứng nếu cần thiết.
	 */
	UEDP_ATTR_WEAK void pal_memrp_get_sys_info(ui32 *rom_used, ui32 *ram_used, ui32 *stack_curr);

#endif
