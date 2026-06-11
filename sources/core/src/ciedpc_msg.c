/**
 * @file ciedpc_msg.c
 * @author Shang Huang
 * @brief Implementation of message management for CIEDPC system
 * @version 0.1
 * @date 2026-04-16
 * 
 * @copyright MIT License
 * 
 */
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "ciedpc_core.h"
#include "ciedpc_msg.h"
#include "pal_memrp.h"
#include "fifo.h"

/**
 * @brief Khai báo cấu trúc quản lý Pool tin nhắn
 * @param free_list Con trỏ đến đầu của Pool tin nhắn
 * @param used_count Số lượng tin nhắn đang được sử dụng trong Pool
 * @param max_used Số lượng tin nhắn tối đa đã từng được sử dụng trong Pool
 */
typedef struct ciedpc_msg_pool_header_t {
	ciedpc_msg_t* free_list;
	ui8       used_count;
	ui8       max_used;
} ciedpc_msg_pool_header_t;

/**
 * @brief Khai báo các Pool tin nhắn
 * @attention Mỗi pool có thể được setup kích thước và kiểu dữ liệu khác nhau, lưu ý rằng arr[i][j] thì j tương ứng từng cột, i là tương ứng từng hàng, 
 * 						nên khi khởi tạo pool cần đảm bảo tính toán đúng offset để tránh tràn bộ nhớ hoặc ghi đè dữ liệu
 */

/**
 * @brief Blank pool với kích thước là 8 32-bits units
 */
sta ciedpc_msg_t blank_pool[CIEDPC_MSG_BLANK_QUEUE_SIZE] = {0};
ciedpc_msg_pool_header_t g_blank_pool_ctrl = {0};

/**
 * @brief Alloc pool với kích thước là 16 [sizeof(void*) * 2u] units
 * @example
 * +-----------------------------------+-----+-----+-----+-------+----------------------+
 * | Index Queue T->B Index Data L->R  | [0] | [1] | [2] | [...] | [sizeof(void*) * 2u] |
 * +-----------------------------------+-----+-----+-----+-------+----------------------+
 * | [0]                               | ... | ... | ... |   ... |                  ... |
 * | [1]                               | ... | ... | ... |   ... |                  ... |
 * | [...]                             | ... | ... | ... |   ... |                  ... |
 * | [15]                              | ... | ... | ... |   ... |                  ... |
 * +-----------------------------------+-----+-----+-----+-------+----------------------+
 * 
 */
sta ciedpc_msg_t alloc_pool[CIEDPC_MSG_ALLOC_QUEUE_SIZE] = {0};
sta ui8 alloc_pool_data[CIEDPC_MSG_ALLOC_QUEUE_SIZE][CIEDPC_MSG_ALLOC_DATA_MAX] = {0};
ciedpc_msg_pool_header_t g_alloc_pool_ctrl = {0};


/**
 * @brief Extal pool với kích thước là 16 [sizeof(void*) * 4u] units
 * @example
 * +-----------------------------------+-----+-----+-----+-------+----------------------+
 * | Index Queue T->B Index Data L->R  | [0] | [1] | [2] | [...] | [sizeof(void*) * 4u] |
 * +-----------------------------------+-----+-----+-----+-------+----------------------+
 * | [0]                               | ... | ... | ... |   ... |                  ... |
 * | [1]                               | ... | ... | ... |   ... |                  ... |
 * | [...]                             | ... | ... | ... |   ... |                  ... |
 * | [15]                              | ... | ... | ... |   ... |                  ... |
 * +-----------------------------------+-----+-----+-----+-------+----------------------+
 * 
 */
sta ciedpc_msg_t extal_pool[CIEDPC_MSG_EXTAL_QUEUE_SIZE] = {0};
sta ui8 extal_pool_data[CIEDPC_MSG_EXTAL_QUEUE_SIZE][CIEDPC_MSG_EXTAL_DATA_MAX] = {0};
ciedpc_msg_pool_header_t g_extal_pool_ctrl = {0};

/**
 * @brief ISR pool với kích thước là 16 sizeof(ciedpc_msg_isr_t) units
 */
sta ciedpc_msg_isr_t isr_pool_buffer[CIEDPC_MSG_ISR_QUEUE_SIZE] = {0};
fifo_t isr_pool = {0};

/**
 * @brief Khai báo các hàm quản lý nội bộ
 */

sta void internal_ciedpc_msg_pool_init(
	ciedpc_msg_pool_header_t* header, 
	ciedpc_msg_t* pool, ui8* data_mem, 
	ciedpc_msg_type_t pool_type,
	ui8 units, ui16 data_size, ui16 data_max
);
sta ciedpc_msg_t* internal_ciedpc_msg_pool_pop(ciedpc_msg_pool_header_t* header);
sta void internal_ciedpc_msg_pool_push(ciedpc_msg_pool_header_t* header, ciedpc_msg_t* msg);
sta ciedpc_msg_pool_header_t* internal_ciedpc_msg_find_best_pool(ui16 size);
sta bool ciedpc_msg_is_valid_ptr(ciedpc_msg_t* msg);
sta void internal_ciedpc_msg_pool_panic(ui8 pool_id);

void ciedpc_msg_pool_init() {
	// Khởi tạo BLANK Pool
	internal_ciedpc_msg_pool_init(
		&g_blank_pool_ctrl, blank_pool, NULL, CIEDPC_MSG_TYPE_BLANK, 
		CIEDPC_MSG_BLANK_QUEUE_SIZE, 0, 0
	);

	// Khởi tạo ALLOC Pool
	internal_ciedpc_msg_pool_init(
		&g_alloc_pool_ctrl, alloc_pool, (ui8*)alloc_pool_data, CIEDPC_MSG_TYPE_ALLOC,
		CIEDPC_MSG_ALLOC_QUEUE_SIZE, CIEDPC_MSG_ALLOC_DATA_MAX, CIEDPC_MSG_ALLOC_DATA_MAX
	);
	
	// Khởi tạo EXTAL Pool
	internal_ciedpc_msg_pool_init(
		&g_extal_pool_ctrl, extal_pool, (ui8*)extal_pool_data, CIEDPC_MSG_TYPE_EXTAL,
		CIEDPC_MSG_EXTAL_QUEUE_SIZE, CIEDPC_MSG_EXTAL_DATA_MAX, CIEDPC_MSG_EXTAL_DATA_MAX
	);

	// Khởi tạo ISR Pool
	fifo_init(
		&isr_pool, (void*)isr_pool_buffer, 
		CIEDPC_MSG_ISR_QUEUE_SIZE, sizeof(ciedpc_msg_isr_t)
	);
}

ciedpc_msg_t* ciedpc_msg_alloc(ui16 des_task_id, ui16 sig, ui16 size) {
	ciedpc_msg_t *msg = NULL;
	ciedpc_msg_pool_header_t* pool_header = internal_ciedpc_msg_find_best_pool(size);

	if (pool_header == NULL) {
		return NULL;
	}

	pal_enter_critical(); // Đảm bảo an toàn khi truy cập Pool trong môi trường đa tác vụ hoặc ISR
	msg = internal_ciedpc_msg_pool_pop(pool_header);
	pal_exit_critical();

	if (msg != NULL) {
		msg->src_task_id = ciedpc_task_norm_get_current_id();
		msg->des_task_id = des_task_id;
		msg->sig = sig;
		msg->ref_count = 1; // mặc định 1 tham chiếu khi tạo mới
	} else {
		// Pool đã hết, có thể log lỗi hoặc thực hiện hành động khắc phục
		internal_ciedpc_msg_pool_panic(pool_header - &g_blank_pool_ctrl); // Tính toán pool_id dựa trên offset
	}

	return msg;
}

void ciedpc_msg_free(ciedpc_msg_t* msg) {
	if (!msg || !ciedpc_msg_is_valid_ptr(msg)) return;

	msg->next = NULL;

	ciedpc_msg_pool_header_t* header = NULL;
	
	switch (msg->type) {
		case CIEDPC_MSG_TYPE_BLANK:
			header = &g_blank_pool_ctrl;
			break;
		case CIEDPC_MSG_TYPE_ALLOC:
			header = &g_alloc_pool_ctrl;
			break;
		case CIEDPC_MSG_TYPE_EXTAL:
			header = &g_extal_pool_ctrl;
			break;
		default:
			return; // Loại tin nhắn không hợp lệ, không thực hiện giải phóng
			break;
	}

	pal_enter_critical(); // Đảm bảo an toàn khi truy cập Pool trong môi trường đa tác vụ hoặc ISR
	internal_ciedpc_msg_pool_push(header, msg);
	pal_exit_critical();
}

void ciedpc_msg_ref_inc(ciedpc_msg_t* msg) {
	msg->ref_count++;
}

void ciedpc_msg_ref_dec(ciedpc_msg_t* msg) {
	msg->ref_count--;
	if (msg->ref_count == 0) {
		ciedpc_msg_free(msg);
	}
}

/**
 * @brief Khởi tạo Pool tin nhắn
 * 
 * @param header Chứa thông tin quản lý của Pool
 * @param pool Con trỏ đến mảng chứa các tin nhắn trong Pool
 * @param data_mem Con trỏ đến mảng chứa vùng dữ liệu cho các tin nhắn trong Pool
 * @param units Số lượng tin nhắn trong Pool, lấy đầu vào từ các define
 * @param data_size Kích thước dữ liệu tối đa cho mỗi tin nhắn trong Pool, lấy đầu vào từ các define
 * @param data_max Kích thước dữ liệu tối đa cho mỗi tin nhắn trong Pool, lấy đầu vào từ các define
 * @attention Nếu data_size là 0 hoặc kích thước phân bố dữ liệu không phù hợp với data_size thì coi như không phù hợp và trả về
 */
void internal_ciedpc_msg_pool_init(
	ciedpc_msg_pool_header_t* header, ciedpc_msg_t* pool, ui8* data_mem, ciedpc_msg_type_t pool_type, ui8 units, ui16 data_size, ui16 data_max
) {
	// Kiểm tra đầu vào
	if (!header || !pool) {
		return;
	}

	// Kiểm tra nếu init Blank Pool thì không cần xử lý thêm, chỉ có Blank Pool mới có thể có data_mem là NULL và data_size là 0, 
	// các Pool khác nếu không có vùng dữ liệu thì sẽ không khởi tạo Pool vì sẽ lãng phí bộ nhớ.
	if (pool_type != CIEDPC_MSG_TYPE_BLANK && !data_mem) {
		return;
	}

	/**
	 * @brief Kiểm tra nếu data_size là 0 hoặc kích thước phân bố dữ liệu không phù hợp với data_size
	 * 				thì coi như không phù hợp và trả về, không khởi tạo Pool vì sẽ lãng phí bộ nhớ. 
	 * 				Ví dụ giả sử norm_pool[12][8] nghĩa là có thể chứa 12 unit tin nhắn, mỗi unit tin nhắn có thể chứa tối đa 8 bytes dữ liệu, 
	 * 				nếu data_size là 0 hoặc data_size lớn hơn 8 bytes thì sẽ không khởi tạo Pool.
	 * 				Ngoài ra nếu data_size không phải là bội số của data_max thì cũng sẽ không khởi tạo Pool vì
	 * 				sẽ dẫn đến việc phân bố dữ liệu	không đều.
	 */
	if (data_size > 0 && (data_max % data_size != 0 || data_size > data_max)) {
		return;
	}

	// Lưu header
	header->free_list = &pool[0]; // có thể viết là header->free_list = pool; vì pool đã là con trỏ đến phần tử đầu tiên của mảng
	header->used_count = 0;
	header->max_used = 0;

	// Con trỏ index
	ui32 index = 0;

	// Vòng lặp khởi tạo
	for (index = 0; index < units; index++) {
		// Xóa dữ liệu rác
		memset(&pool[index], 0, sizeof(ciedpc_msg_t));

		// Thiết lập kiểu tin nhắn cho Pool
		pool[index].type = pool_type;

		// Gán vùng dữ liệu cho tin nhắn
		if (data_mem != NULL && data_size > 0) {
				// Tính toán địa chỉ: Bắt đầu + (vị trí * kích thước mỗi ô)
				/**
				 * @brief Mỗi tin nhắn sẽ sở hữu một vùng nhớ bắt đầu từ:
				 * 				Địa chỉ gốc + (số thứ tự tin nhắn * kích thước ô nhớ)
				 * @attention Lưu ý rằng [0][0-7], [1][8-15], ... Đây chính là nguyên lý trải phẳng của mảng 2 chiều thành mảng 1 chiều, 
				 * 						giúp việc quản lý bộ nhớ trở nên đơn giản và hiệu quả hơn.
				 */
				pool[index].data = (ui32*)(data_mem + (index * data_size));

				// Xóa sạch vùng dữ liệu
				memset(pool[index].data, 0, data_size);
		} else {
				pool[index].data = NULL; // Pool Blank
		}

		// Thiết lập danh sách liên kết (Linked List)
		if (index < (units - 1)) {
				pool[index].next = &pool[index + 1];
		} else {
				pool[index].next = NULL; // Phần tử cuối cùng
		}
	}
}

/**
 * @brief Lấy một tin nhắn từ Pool
 * 
 * @param header Chứa thông tin quản lý của Pool
 */
ciedpc_msg_t* internal_ciedpc_msg_pool_pop(ciedpc_msg_pool_header_t* header) {
	if (!header || !header->free_list) {
		return NULL; // Pool trống hoặc không hợp lệ
	}

	// Lấy tin nhắn từ đầu danh sách liên kết
	ciedpc_msg_t* msg = header->free_list;

	// Cập nhật con trỏ đầu của danh sách liên kết
	header->free_list = msg->next;

	// Cập nhật số lượng tin nhắn đang được sử dụng
	header->used_count++;

	// Cập nhật số lượng tin nhắn tối đa đã từng được sử dụng (dùng cho debug và tối ưu hóa)
	if (header->used_count > header->max_used) {
		header->max_used = header->used_count;
	}

	// Ngắt liên kết của tin nhắn với danh sách liên kết
	msg->next = NULL;

	return msg;
}

/**
 * @brief Trả một tin nhắn về Pool
 * 
 * @param header Chứa thông tin quản lý của Pool
 * @param msg Con trỏ đến tin nhắn cần trả về Pool
 */
void internal_ciedpc_msg_pool_push(ciedpc_msg_pool_header_t* header, ciedpc_msg_t* msg) {
	if (!header || !msg) {
		return; // Pool không hợp lệ hoặc tin nhắn không hợp lệ
	}

	// Ngắt liên kết của tin nhắn với danh sách liên kết (nếu có)
	msg->next = NULL;

	// Nạp tin nhắn về đầu danh sách liên kết
	msg->next = header->free_list;
	header->free_list = msg;

	// Cập nhật số lượng tin nhắn đang được sử dụng
	if (header->used_count > 0) {
		header->used_count--;
	}
}

/**
 * @brief Tìm Pool tin nhắn phù hợp nhất dựa trên kích thước dữ liệu yêu cầu
 * 
 * @param size Kích thước dữ liệu yêu cầu cho tin nhắn
 * @return ciedpc_msg_pool_header_t* Con trỏ đến Pool tin nhắn phù hợp nhất hoặc NULL nếu không có Pool nào phù hợp
 * @attention Pool EXTAL và ISR không được xem xét trong hàm này vì nó được thiết kế để đảm bảo signal từ ngoài vào core được
 * 						xử lý trước khi vào hàng đợi tin nhắn chính thức của task, 
 * 						do đó nó không cần phải tuân theo quy tắc lựa chọn Pool dựa trên kích thước dữ liệu như các Pool khác.
 */
ciedpc_msg_pool_header_t* internal_ciedpc_msg_find_best_pool(ui16 size) {
	if (size == 0) {
		return &g_blank_pool_ctrl; // Pool Blank
	} else if (size <= CIEDPC_MSG_ALLOC_DATA_MAX) {
		return &g_alloc_pool_ctrl; // Pool Alloc
	} else {
		return NULL; // Không có Pool nào phù hợp
	}
}

/**
 * @brief Xả hàng đợi tin nhắn trong ngữ cảnh ISR để giải phóng các tin nhắn đang bị giữ trong hàng đợi ISR
 * 
 */
void ciedpc_msg_drain_isr_pool(void) {

	pal_enter_critical(); // Đảm bảo an toàn khi truy cập hàng đợi ISR trong môi trường đa tác vụ hoặc ISR

	while (!fifo_is_empty(&isr_pool)) {
		ciedpc_msg_isr_t msg_isr;
		fifo_get(&isr_pool, &msg_isr);
		
		ciedpc_msg_t* msg = ciedpc_msg_alloc(msg_isr.des_task_id, msg_isr.sig, 0);

		if (msg) {
			ciedpc_task_norm_post_msg(msg->des_task_id, msg);
		} else {
			// Xử lý tình huống cấp phát tin nhắn thất bại, có thể log lỗi hoặc thực hiện hành động khắc phục
			internal_ciedpc_msg_pool_panic(CIEDPC_MSG_ISR_QUEUE_SIZE); // Sử dụng một mã lỗi đặc biệt cho Pool ISR
		}
	}

	pal_exit_critical();
}

/**
 * @brief Kiểm tra xem con trỏ tin nhắn có hợp lệ hay không (được cấp phát từ một trong các Pool tin nhắn)
 * 
 * @param msg Con trỏ đến tin nhắn cần kiểm tra
 * @return true nếu con trỏ tin nhắn hợp lệ (được cấp phát từ một trong các Pool tin nhắn)
 * @return false nếu con trỏ tin nhắn không hợp lệ (không được cấp phát từ bất kỳ Pool tin nhắn nào)
 */
bool ciedpc_msg_is_valid_ptr(ciedpc_msg_t* msg) {
	if (!msg) {
		return false; // Con trỏ NULL không hợp lệ
	}

	// Kiểm tra xem con trỏ tin nhắn có thuộc về bất kỳ Pool nào không
	if ((msg >= &blank_pool[0] && msg < &blank_pool[CIEDPC_MSG_BLANK_QUEUE_SIZE]) ||
			(msg >= &alloc_pool[0] && msg < &alloc_pool[CIEDPC_MSG_ALLOC_QUEUE_SIZE]) ||
			(msg >= &extal_pool[0] && msg < &extal_pool[CIEDPC_MSG_EXTAL_QUEUE_SIZE])) {
		return true; // Con trỏ tin nhắn hợp lệ
	}

	return false; // Con trỏ tin nhắn không hợp lệ
}

/**
 * @brief Xử lý tình huống khẩn cấp khi Pool tin nhắn xảy ra vấn đề
 * 
 * @param pool_id ID của Pool tin nhắn bị lỗi, có thể là CIEDPC_MSG_TYPE_BLANK, CIEDPC_MSG_TYPE_NORM, CIEDPC_MSG_TYPE_ALLOC hoặc CIEDPC_MSG_TYPE_EXTAL
 */
void internal_ciedpc_msg_pool_panic(ui8 pool_id) {

}

RETR_STAT internal_ciedpc_msg_enqueue_isr_sig(task_id_t tid, ui16 sig) {
	ciedpc_msg_isr_t raw_sig;
	raw_sig.des_task_id = tid;
	raw_sig.sig = sig;
	RETR_STAT result = STAT_ERROR;

	pal_enter_critical(); // Đảm bảo an toàn khi truy cập hàng đợi ISR trong môi trường đa tác vụ hoặc ISR
	/* 
	* Vì hàm này được gọi từ ISR, fifo_put PHẢI là lock-free 
	* hoặc chúng ta bọc bảo vệ tối thiểu nếu cần 
	*/
	if (fifo_put(&isr_pool, (ciedpc_msg_isr_t*)&raw_sig) == RET_FIFO_OK) {
		result = STAT_OK;
	}
	
	pal_exit_critical();
	return result; // Hàng đợi ISR đầy
}

void internal_ciedpc_msg_pool_get_info(ciedpc_msg_type_t pool_id, pal_memrp_info_t* info) {
	if (!info) return;

	switch (pool_id) {
		case CIEDPC_MSG_TYPE_BLANK:
			info->target = (void*)blank_pool;
			info->name = "BLANK";
			info->type = CIEDPC_MSG_TYPE_BLANK;
			info->used = g_blank_pool_ctrl.used_count;
			info->max_used = g_blank_pool_ctrl.max_used;
			info->total = CIEDPC_MSG_BLANK_QUEUE_SIZE;
			break;
		case CIEDPC_MSG_TYPE_ALLOC:
			info->target = (void*)alloc_pool;
			info->name = "ALLOC";
			info->type = CIEDPC_MSG_TYPE_ALLOC;
			info->used = g_alloc_pool_ctrl.used_count;
			info->max_used = g_alloc_pool_ctrl.max_used;
			info->total = CIEDPC_MSG_ALLOC_QUEUE_SIZE;
			break;
		case CIEDPC_MSG_TYPE_EXTAL:
			info->target = (void*)extal_pool;
			info->name = "EXTAL";
			info->type = CIEDPC_MSG_TYPE_EXTAL;
			info->used = g_extal_pool_ctrl.used_count;
			info->max_used = g_extal_pool_ctrl.max_used;
			info->total = CIEDPC_MSG_EXTAL_QUEUE_SIZE;
			break;
		case CIEDPC_MSG_TYPE_ISR:
			info->target = (void*)isr_pool_buffer;
			info->name = "ISR";
			info->type = CIEDPC_MSG_TYPE_ISR; // ISR Pool không có kiểu tin nhắn cụ thể, có thể coi là BLANK hoặc định nghĩa một kiểu mới nếu cần
			// FIFO không cung cấp trực tiếp số lượng phần tử đang sử dụng, có thể cần theo dõi riêng nếu cần
			info->used = 0; // Không có thông tin cụ thể về số lượng phần tử đang sử dụng trong FIFO
			info->max_used = 0; // Không có thông tin cụ thể về số lượng phần tử tối đa đã từng được sử dụng trong FIFO
			info->total = CIEDPC_MSG_ISR_QUEUE_SIZE; // Tổng số phần tử có thể chứa trong FIFO
			break;
		default:
			printf("Invalid pool ID: %d\n", pool_id);
			break;
	}
}