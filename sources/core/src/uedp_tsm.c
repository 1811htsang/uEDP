/** ANCHOR - Implementation of Transition State Machine (TSM) management for UEDP system
 * @file uedp_tsm.c
 * @author Shang Huang
 * @version 0.1
 * @date 2026-08-04
 * @copyright MIT License
 */
#include "uedp_tsm.h"
#include "uedp_msg.h"
#include "uedp_fcr.h"

void uedp_tsm_init(
	uedp_tsm_t* tsm_table, const tsm_state_desc_t* state_des_table, 
	ui8 state_count, tsm_state_id_t initial_state_id, tsm_on_state_f on_state_changed
) {
	// Kiểm tra tsm_table và state_des_table không phải là NULL
	if (!tsm_table || !state_des_table) {
		UEDP_FCR_RAISE_MSG(UEDP_FCR_SM_NULL_HANDLER, "tsm_init: null table");
		return; 
	}

	// Kiểm tra initial_state_id hợp lệ
	if (initial_state_id < UEDP_TSM_STATE_MIN || initial_state_id > UEDP_TSM_STATE_MAX) {
		UEDP_FCR_RAISE_MSG(UEDP_FCR_SM_INVALID_TRANS, "tsm_init: bad initial state");
		return;
	}

	if (initial_state_id < (UEDP_TSM_STATE_MIN + UEDP_TSM_STATE_OFFSET) ||
			(int)initial_state_id - UEDP_TSM_STATE_MIN - UEDP_TSM_STATE_OFFSET >= (int)state_count) {
		UEDP_FCR_RAISE_MSG(UEDP_FCR_SM_INVALID_TRANS, "tsm_init: initial state outside state_table");
		return;
	}

	// Khởi tạo bảng với state đầu tiên
	tsm_table->cur_state = initial_state_id; // Đặt trạng thái hiện tại là trạng thái ban đầu
	tsm_table->prev_state = initial_state_id; // Đặt trạng thái trước đó cũng là trạng thái ban đầu
	tsm_table->state_table = state_des_table;
	tsm_table->state_count = state_count;
	tsm_table->on_state_changed = on_state_changed;

	const tsm_state_desc_t* desc = &tsm_table->state_table[initial_state_id - UEDP_TSM_STATE_MIN - UEDP_TSM_STATE_OFFSET];
	if (desc->on_entry) {
		uedp_msg_t m = { .sig = UEDP_TSM_SIG_ENTRY };
		desc->on_entry(&m);
	}
}

void uedp_tsm_trans(uedp_tsm_t* tsm_table, tsm_state_id_t state_id) {
	if (!tsm_table || !tsm_table->state_table || state_id < UEDP_TSM_STATE_MIN || state_id > UEDP_TSM_STATE_MAX) {
		return;
	}

	if (state_id == UEDP_TSM_STATE_STAY) {
		//NOTE - Minh: STAY là hành vi hợp lệ, thường xuyên xảy ra (state tự loop) - không phải lỗi, không raise FCR.
		return;
	}

	// Chỉ bảo vệ việc đọc/hoán đổi trạng thái; handler exit/entry/callback chạy NGOÀI critical section
	pal_enter_critical();
	tsm_state_id_t cur_state = tsm_table->cur_state;
	tsm_state_id_t next_state = (state_id == UEDP_TSM_STATE_BACK) ? tsm_table->prev_state : state_id;
	pal_exit_critical();

	int cur_index = (int)cur_state - UEDP_TSM_STATE_MIN - UEDP_TSM_STATE_OFFSET;
	int next_index = (int)next_state - UEDP_TSM_STATE_MIN - UEDP_TSM_STATE_OFFSET;
	if (cur_index < 0 || cur_index >= tsm_table->state_count || next_index < 0 || next_index >= tsm_table->state_count) {
		UEDP_FCR_RAISE_MSG(UEDP_FCR_SM_INVALID_TRANS, "tsm_trans: state id outside state_table");
		return;
	}

	// Thực hiện exit trạng thái trước
	const tsm_state_desc_t* cur_desc = &tsm_table->state_table[cur_index];
	if (cur_desc->on_exit) {
		uedp_msg_t msg = { .sig = UEDP_TSM_SIG_EXIT };
		cur_desc->on_exit(&msg);
	}

	pal_enter_critical();
	tsm_table->prev_state = cur_state;
	tsm_table->cur_state = next_state;
	pal_exit_critical();

	// Thực thi entry trạng thái mới
	const tsm_state_desc_t* next_desc = &tsm_table->state_table[next_index];
	if (next_desc->on_entry) {
		uedp_msg_t msg = { .sig = UEDP_TSM_SIG_ENTRY };
		next_desc->on_entry(&msg);
	}

	// Gọi callback thông báo cho App về việc trạng thái đã thay đổi
	if (tsm_table->on_state_changed) {
		tsm_table->on_state_changed(next_state);
	}
}

void uedp_tsm_dispatch(uedp_tsm_t* tsm_table, uedp_msg_t* msg) {
	if (tsm_table && msg && tsm_table->state_table) {
		int state_index = (int)tsm_table->cur_state - UEDP_TSM_STATE_MIN - UEDP_TSM_STATE_OFFSET;
		if (state_index < 0 || state_index >= tsm_table->state_count) {
			UEDP_FCR_RAISE_MSG(UEDP_FCR_SM_INVALID_TRANS, "tsm_dispatch: cur_state outside state_table");
			return;
		}
		const tsm_state_desc_t* desc = &tsm_table->state_table[state_index];

		for (int index = 0; index < desc->trans_count; index++) {
			if (desc->transitions[index].sig == msg->sig) {
				if (desc->transitions[index].tsm_func) {
					desc->transitions[index].tsm_func(msg);
				}
				if (desc->transitions[index].next_state != UEDP_TSM_STATE_STAY) {
					uedp_tsm_trans(tsm_table, desc->transitions[index].next_state);
				}
				return; // Dispatch chỉ thực thi 1 lần
			}
		}

		// Duyệt hết bảng transitions của state hiện tại mà không tìm thấy sig phù hợp
		UEDP_FCR_RAISE(UEDP_FCR_SM_INVALID_TRANS);
	}
}
