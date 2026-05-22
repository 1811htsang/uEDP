/**
 * @file ciedpc_timer.c
 * @author Shang Huang
 * @brief Implementation of timer management for CIEDPC system
 * @version 0.1
 * @date 2026-04-18
 * 
 * @copyright MIT License
 * 
 */
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "unistd.h"
#include "ciedpc_core.h"
#include "ciedpc_timer.h"
#include "ciedpc_task.h"

/**
 * @brief Cấu trúc quản lý toàn bộ hệ thống Timer
 * @param head: Con trỏ đến đầu danh sách liên kết các timer đang chạy
 * @param free_list: Con trỏ đến danh sách các nút timer đang rảnh trong Pool
 * @param active_count: Số lượng timer đang chạy, dùng để quản lý và giới hạn số lượng timer hoạt động cùng lúc
 */
typedef struct {
	ciedpc_timer_t* head;           /* Đầu danh sách liên kết các timer đang chạy */
	ciedpc_timer_t* free_list;      /* Danh sách các nút timer đang rảnh trong Pool */
	ui8             active_count;   /* Số lượng timer đang chạy */
} ciedpc_timer_ctrl_t;

/**
 * @brief Khai báo Pool timer
 */
sta ciedpc_timer_t timer_pool[CIEDPC_TIMER_MAX_NODES] = {0};
sta ciedpc_timer_ctrl_t timer_ctrl = {0};

sta ciedpc_timer_t* internal_ciedpc_timer_find(ui8 tid, ui8 sig) {
	ciedpc_timer_t* curr = timer_ctrl.head;
	while (curr) {
		if (curr->des_task_id == tid && curr->sig == sig) return curr;
		curr = curr->next;
	}
	return NULL;
}

void ciedpc_timer_init(void) {
	// Khởi tạo Pool timer
	for (ui8 i = 0; i < CIEDPC_TIMER_MAX_NODES; i++) {
		timer_pool[i].next = (i < (CIEDPC_TIMER_MAX_NODES - 1)) ? &timer_pool[i + 1] : NULL;
	}
	timer_ctrl.free_list = &timer_pool[0];
	timer_ctrl.head = NULL;
	timer_ctrl.active_count = 0;
}

RETR_STAT ciedpc_timer_set(ui16 tid, ui8 sig, ui32 ms, ciedpc_timer_type_t type) {
	if (ms == 0 || type > CIEDPC_TIMER_PERIODIC) {
		return STAT_ERROR; // Tham số không hợp lệ
	}

	// Entry critical section
	pal_enter_critical();

	ciedpc_timer_t* existing_timer = internal_ciedpc_timer_find(tid, sig);
	if (existing_timer) {
		// Nếu đã tồn tại timer với cặp Task ID và Signal này, cập nhật lại thời gian và loại timer
		existing_timer->period = ms / CIEDPC_TIMER_TICK;
		existing_timer->counter = existing_timer->period;
		existing_timer->type = type;
		existing_timer->is_active = true;
		pal_exit_critical();
		return STAT_OK; // Timer đã được cập nhật thành công
	}

	if (timer_ctrl.active_count >= CIEDPC_TIMER_MAX_NODES) {
		pal_exit_critical();
		return STAT_BUSY; // Đã đạt đến giới hạn số lượng timer hoạt động
	}

	if (!timer_ctrl.free_list) {
		pal_exit_critical();
		return STAT_BUSY; // Không còn nút timer nào rảnh trong Pool
	}

	// Lấy một nút timer từ free list
	ciedpc_timer_t* new_timer = timer_ctrl.free_list;
	timer_ctrl.free_list = new_timer->next;

	// Khởi tạo thông tin cho timer mới
	new_timer->des_task_id = tid;
	new_timer->sig = sig;
	new_timer->type = type;
	new_timer->period = ms / CIEDPC_TIMER_TICK;
	new_timer->counter = new_timer->period;
	new_timer->is_active = true;

	// Thêm timer mới vào đầu danh sách liên kết các timer đang chạy
	new_timer->next = timer_ctrl.head;
	timer_ctrl.head = new_timer;

	timer_ctrl.active_count++;

	// Exit critical section
	pal_exit_critical();

	return STAT_OK; // Timer đã được tạo thành công
}

RETR_STAT ciedpc_timer_remove(ui16 tid, ui8 sig) {
	// Entry critical section
	pal_enter_critical();

	ciedpc_timer_t* check = internal_ciedpc_timer_find(tid, sig);
	if (!check) {
		// Exit critical section
		pal_exit_critical();
		return STAT_NRDY; // Không tìm thấy timer với cặp Task ID và Signal này
	}

	// Gỡ timer khỏi danh sách liên kết các timer đang chạy
	ciedpc_timer_t* curr = timer_ctrl.head;
	ciedpc_timer_t* prev = NULL;

	while (curr) {
		if (curr == check) {
			if (prev) {
				prev->next = curr->next;
			} else {
				timer_ctrl.head = curr->next;
			}

			// Trả nút timer về Pool
			curr->next = timer_ctrl.free_list;
			timer_ctrl.free_list = curr;

			// Giảm số lượng timer đang chạy
			if (timer_ctrl.active_count > 0) {
				timer_ctrl.active_count--;
			}
			
			// Exit critical section
			pal_exit_critical();

			return STAT_OK; // Timer được xóa thành công
		}
		prev = curr;
		curr = curr->next;
	}

	// Exit critical section
	pal_exit_critical();

	return STAT_ERROR; // Không tìm thấy timer với cặp Task ID và Signal đã cho
}

/* Hàm này được gọi từ System Tick ISR (ví dụ mỗi 1ms) */
void ciedpc_timer_tick(void) {
	ciedpc_timer_t* curr = timer_ctrl.head;
	ciedpc_timer_t* prev = NULL;

	while (curr) {
		if (curr->is_active) {
			if (curr->counter > 0) {
				curr->counter--;
			}

			if (curr->counter == 0) {
				#ifdef CIEDPC_PLATFORM_LINUX
					printf("[Timer] Timer expired: Task ID=%u, Signal=0x%02X\n", curr->des_task_id, curr->sig);
					usleep(curr->period * 10); // Sleep for 3 * timer period to simulate processing delay and test timer accuracy under load
				#endif

				/* 1. Đã hết hạn -> Publish Event tới Task đích */
				// Ở đây dùng ciedpc_task_norm_post_isr vì chúng ta đang ở ngữ cảnh ngắt tick
				ciedpc_task_norm_post_isr(curr->des_task_id, curr->sig);

				/* 2. Xử lý loại Timer */
				if (curr->type == CIEDPC_TIMER_PERIODIC) {
					curr->counter = curr->period; // Nạp lại
					prev = curr;
					curr = curr->next;
				} else {
					/* ONE SHOT: Gỡ khỏi danh sách head và trả về free_list */
					ciedpc_timer_t* expired = curr;
					curr = curr->next;
					
					if (prev) prev->next = curr;
					else timer_ctrl.head = curr;

					// Trả về free list
					expired->is_active = false;
					expired->next = timer_ctrl.free_list;
					timer_ctrl.free_list = expired;
					timer_ctrl.active_count--;
					continue; // Bỏ qua việc gán prev
				}
			} else {
				prev = curr;
				curr = curr->next;
			}
		}
	}
}

void ciedpc_timer_get_stats(ui8* active, ui8* max_capacity) {
	*active = timer_ctrl.active_count;
	*max_capacity = CIEDPC_TIMER_MAX_NODES;
}