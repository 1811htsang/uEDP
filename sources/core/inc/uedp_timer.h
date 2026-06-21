/**
 * @file uedp_timer.h
 * @author Shang Huang
 * @brief Timer management definitions and utilities for UEDP system
 * @version 0.1
 * @date 2026-04-18
 * 
 * @copyright MIT License
 * 
 */
#ifndef __UEDP_TIMER_H__
	#define __UEDP_TIMER_H__

	/**
	 * @brief Khai báo các thư viện sử dụng
	 */
	#include <stdint.h>
	#include "uedp_core.h"
	#include "uedp_task.h"

	/**
	 * @brief Khai báo đơn vị thời gian cho timer tick
	 */
	#define UEDP_TIMER_TICK					(0x01u)

	/**
	 * @brief Định nghĩa loại Timer
	 * @param UEDP_TIMER_ONE_SHOT: Timer chạy một lần rồi tự xóa
	 * @param UEDP_TIMER_PERIODIC: Timer tự động nạp lại sau khi hết hạn
	 */
	typedef enum {
		UEDP_TIMER_ONE_SHOT = 0,  
		UEDP_TIMER_PERIODIC       
	} uedp_timer_type_t;

	/**
	 * @brief Cấu trúc một nút Timer (Timer Node)
	 * @param next: Con trỏ đến nút Timer tiếp theo trong danh sách liên kết
	 * @param des_task_id: ID của tác vụ đích sẽ nhận tin nhắn khi timer hết hạn
	 * @param sig: Tín hiệu sẽ được gửi đến tác vụ đích khi timer hết hạn
	 * @param type: Loại timer (One-shot hoặc Periodic)
	 * @param period: Chu kỳ nạp lại của timer (dùng cho timer định kỳ)
	 */
	typedef struct uedp_timer_t {
		/* Quản lý danh sách liên kết */
		struct uedp_timer_t* next;

		/* Thông tin điều phối */
		ui8             des_task_id;    /* Task sẽ nhận tin nhắn khi hết hạn */
		ui8             sig;            /* Tín hiệu sẽ gửi đi */
		uedp_timer_type_t type;       /* Loại timer */

		/* Quản lý thời gian (tính theo nhịp Tick) */
		ui32            period;         /* Chu kỳ nạp lại (dùng cho Periodic) */
		ui32            counter;        /* Bộ đếm lùi hiện tại */

		/* Trạng thái nội bộ */
		bool            is_active;
	} UEDP_ATTR_PACKED uedp_timer_t;

	/**
	 * @brief Khởi tạo hệ thống Timer (gọi trong uedp_init)
	 * @attention Mặc định các timer type là One-shot, 
	 * 						người dùng sẽ thiết lập lại loại timer khi gọi uedp_timer_set
	 */
	void uedp_timer_init(void);

	/**
	 * @brief Thiết lập một timer mới
	 * @param tid Task đích nhận tin nhắn
	 * @param sig Tín hiệu gửi đi
	 * @param ms Thời gian (mili giây)
	 * @param type Loại timer
	 * @return RETR_STAT STAT_OK nếu tạo thành công
	 */
	RETR_STAT uedp_timer_set(task_id_t tid, ui8 sig, ui32 ms, uedp_timer_type_t type);
	
	/**
	 * @brief Xóa một timer dựa trên cặp Task ID và Signal
	 */
	RETR_STAT uedp_timer_remove(task_id_t tid, ui8 sig);

	/**
	 * @brief Hàm xử lý tick của timer
	 * @attention Hàm này sẽ được gọi trong ngữ cảnh ngắt
	 */
	void uedp_timer_tick(void);

	/**
	 * @brief Lấy thông tin sử dụng của Timer Pool
	 * @param active Số lượng timer đang hoạt động
	 * @param max_capacity Số lượng timer tối đa có thể hoạt động cùng lúc
	 */
	void uedp_timer_get_stats(ui8* active, ui8* max_capacity);

	/**
	 * @brief Lấy giá trị tick hệ thống hiện tại
	 * @return ui32 Giá trị tick hệ thống hiện tại, giúp người dùng có thể theo dõi thời gian đã trôi qua kể từ khi hệ thống khởi động
	 */
	ui32 uedp_timer_get_systick(void);

#endif //__UEDP_TIMER_H__

