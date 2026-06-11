/**
 * @file ciedpc_tsm.c
 * @author Shang Huang
 * @brief Implementation of Transition State Machine (TSM) management for CIEDPC system
 * @version 0.1
 * @date 2026-04-17
 * 
 * @copyright MIT License
 * 
 */
#include "ciedpc_tsm.h"
#include "ciedpc_msg.h"

void ciedpc_tsm_init(
	ciedpc_tsm_t* tsm_table, const tsm_state_desc_t* state_des_table, 
	ui8 state_count, tsm_state_id_t initial_state_id, tsm_on_state_f on_state_changed
) {
	// Kiểm tra tsm_table và state_des_table không phải là NULL
	if (!tsm_table || !state_des_table) {
		return; 
	}

	// Kiểm tra initial_state_id hợp lệ
	if (initial_state_id < CIEDPC_TSM_STATE_MIN || initial_state_id > CIEDPC_TSM_STATE_MAX) {
		return;
	}

	// Khởi tạo bảng với state đầu tiên
	tsm_table->cur_state = initial_state_id; // Đặt trạng thái hiện tại là trạng thái ban đầu
	tsm_table->prev_state = initial_state_id; // Đặt trạng thái trước đó cũng là trạng thái ban đầu
	tsm_table->state_table = state_des_table;
	tsm_table->state_count = state_count;
	tsm_table->on_state_changed = on_state_changed;

	const tsm_state_desc_t* desc = &tsm_table->state_table[initial_state_id - CIEDPC_TSM_STATE_MIN - CIEDPC_TSM_STATE_OFFSET];
	if (desc->on_entry) {
			ciedpc_msg_t m = { .sig = CIEDPC_TSM_SIG_ENTRY };
			desc->on_entry(&m);
	}
}

void ciedpc_tsm_trans(ciedpc_tsm_t* tsm_table, tsm_state_id_t state_id) {
	tsm_state_id_t next_state = state_id;

	// Bảo vệ critical section
	pal_enter_critical();

	if (tsm_table && (state_id >= CIEDPC_TSM_STATE_MIN && state_id <= CIEDPC_TSM_STATE_MAX)) {

		if (state_id == CIEDPC_TSM_STATE_STAY) {
			pal_exit_critical();
			return;
		}

		if (state_id == CIEDPC_TSM_STATE_BACK) {
			next_state = tsm_table->prev_state;
		}

		// Thực hiện exit trạng thái trước
		const tsm_state_desc_t* cur_desc = &tsm_table->state_table[tsm_table->cur_state - CIEDPC_TSM_STATE_MIN - CIEDPC_TSM_STATE_OFFSET];
    if (cur_desc->on_exit) {
        ciedpc_msg_t msg = { .sig = CIEDPC_TSM_SIG_EXIT };
        cur_desc->on_exit(&msg);
    }

		// Cập nhật trạng thái trước đó
		tsm_table->prev_state = tsm_table->cur_state;

		// Cập nhật trạng thái hiện tại
		tsm_table->cur_state = next_state;

		// Thực thi entry trạng thái mới
		const tsm_state_desc_t* next_desc = &tsm_table->state_table[next_state - CIEDPC_TSM_STATE_MIN - CIEDPC_TSM_STATE_OFFSET];
    if (next_desc->on_entry) {
        ciedpc_msg_t msg = { .sig = CIEDPC_TSM_SIG_ENTRY };
        next_desc->on_entry(&msg);
    }

		// Gọi callback thông báo cho App về việc trạng thái đã thay đổi
		if (tsm_table->on_state_changed) {
			tsm_table->on_state_changed(state_id);
		}
	}

	// Thoát critical section
	pal_exit_critical();
}

void ciedpc_tsm_dispatch(ciedpc_tsm_t* tsm_table, ciedpc_msg_t* msg) {
	if (tsm_table && msg) {
		const tsm_state_desc_t* desc = &tsm_table->state_table[tsm_table->cur_state - CIEDPC_TSM_STATE_MIN - CIEDPC_TSM_STATE_OFFSET];

		for (int index = 0; index < desc->trans_count; index++) {
			if (desc->transitions[index].sig == msg->sig) {
				if (desc->transitions[index].tsm_func) {
					desc->transitions[index].tsm_func(msg);
				}
				if (desc->transitions[index].next_state != CIEDPC_TSM_STATE_STAY) {
					ciedpc_tsm_trans(tsm_table, desc->transitions[index].next_state);
				}
				return; // Dispatch chỉ thực thi 1 lần
			}
		}
	}
}

