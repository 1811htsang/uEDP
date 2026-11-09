// Khai báo thư viện sử dụng
#include <stdbool.h>
#include <stdint.h>
#include "fsm.h"
#include "message.h"
#include "port.h"
#include "timer.h"
#include "app.h"
#include "task_list.h"
#include "task_if.h"

// Khai báo hàm xử lý tín hiệu dựa trên loại đích đến
static void if_des_type_handler(ak_msg_t* msg);

// Hàm xử lý tin nhắn đến tác vụ giao diện
void task_if(ak_msg_t* msg) {
	if_des_type_handler(msg);
}

// Hàm xử lý tin nhắn với loại đích đến để gửi đến các tác vụ khác
void if_des_type_handler(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_IF_PURE_MSG_IN: {
		msg_inc_ref_count(msg);
		set_msg_sig(msg, msg->if_sig);
		set_msg_src_task_id(msg, msg->if_src_task_id);
		task_post(msg->if_des_task_id, msg);
	}
		break;

	case AC_IF_COMMON_MSG_IN: {
		msg_inc_ref_count(msg);
		set_msg_sig(msg, msg->if_sig);
		set_msg_src_task_id(msg, msg->if_src_task_id);
		task_post(msg->if_des_task_id, msg);
	}
		break;

	case AC_IF_DYNAMIC_MSG_IN: {
		msg_inc_ref_count(msg);
		set_msg_sig(msg, msg->if_sig);
		set_msg_src_task_id(msg, msg->if_src_task_id);
		task_post(msg->if_des_task_id, msg);
	}
		break;

	case AC_IF_PURE_MSG_OUT: {
		msg_inc_ref_count(msg);
		set_msg_sig(msg, AC_IF_PURE_MSG_OUT);
		task_post(AC_TASK_IF_ID, msg);
	}
		break;

	case AC_IF_COMMON_MSG_OUT: {
		msg_inc_ref_count(msg);
		set_msg_sig(msg, AC_IF_COMMON_MSG_OUT);
		task_post(AC_TASK_IF_ID, msg);
	}
		break;

	case AC_IF_DYNAMIC_MSG_OUT: {
		msg_inc_ref_count(msg);
		set_msg_sig(msg, AC_IF_DYNAMIC_MSG_OUT);
		task_post(AC_TASK_IF_ID, msg);
	}
		break;

	default:
		break;
	}
}

