/**
 * @file uedp_task.h
 * @author Shang Huang
 * @brief Task management definitions and utilities for UEDP system
 * @version 0.1
 * @date 2026-04-16
 * @copyright MIT License
 */
#ifndef __TASK_H__
	#define __TASK_H__

	/**
	 * @brief Khai báo các hằng số và macro cho hệ thống UEDP
	 */
	#include <stdint.h>
	#include <stdbool.h>
	#include "uedp_core.h"
	#include "uedp_fsm.h"
	#include "uedp_tsm.h"
	#include "fifo.h" 

	/**
	 * @brief Khai báo kiểu dữ liệu để quản lý tin nhắn trong hệ thống UEDP
	 * @attention `uedp_msg_t` được gọi ở đây để thực thi forward declaration, 
	 * 						cho phép sử dụng con trỏ đến `uedp_msg_t` trong các khai báo sau này 
	 * 						mà không cần phải định nghĩa chi tiết của `uedp_msg_t` tại thời điểm này, 
	 * 						giúp tránh các vấn đề về phụ thuộc lẫn nhau giữa các cấu trúc dữ liệu trong hệ thống UEDP.
	 */
	typedef struct uedp_msg_t uedp_msg_t;

	/**
	 * @brief Định nghĩa các hằng số để quản lý kích thước 
	 * 				của hàng đợi đối tượng tác vụ và hàng đợi ngắt
	 * @attention Trong thiết kế hệ thống thì bắt buộc phải có hàng đợi dành cho ISR 
	 * 						nhằm đảm bảo tính đồng bộ, an toàn và độc lập phần cứng khi xử lý các sự kiện ngắt
	 */
	#define QUEUE_OBJ_SIZE		(512) // Kích thước của hàng đợi đối tượng
	#define QUEUE_ISR_SIZE		(128) // Kích thước của hàng đợi ngắt

	/**
	 * @brief Định nghĩa các kiểu dữ liệu để quản lý ID 
	 * 				và mức độ ưu tiên của tác vụ trong hệ thống UEDP
	 */
	typedef ui32	task_pri_t; // Mức ưu tiên của tác vụ
	typedef ui16	task_id_t; 	// ID của tác vụ

	/**
	 * @brief Định nghĩa các kiểu dữ liệu để quản lý hàm thực thi của tác vụ
	 * @attention `pf_task_poll` được sử dụng để quản lý các hàm thực thi của tác vụ theo cơ chế poll-driven, 
	 * 						nơi mỗi tác vụ sẽ được kích hoạt và thực thi liên tục hoặc theo một lịch trình nhất định, 
	 * 						không phụ thuộc vào việc nhận tin nhắn.
	 */
	typedef void (*pf_task_norm)(uedp_msg_t*);
	typedef void (*pf_task_poll)(); 

	/**
	 * @brief Định nghĩa cấu trúc để quản lý thông tin của tác vụ message-driven
	 * @param id: ID của tác vụ message-driven
	 * @param base_pri: Mức độ ưu tiên cơ bản của tác vụ message-driven
	 * @param cur_pri: Mức độ ưu tiên hiện tại của tác vụ message-driven, 
	 * 								 có thể được điều chỉnh dựa trên các tín hiệu khẩn cấp hoặc các yếu tố khác
	 * @param urgent_pending: Cờ để chỉ ra liệu có tín hiệu khẩn cấp đang chờ xử lý cho tác vụ này hay không,
	 * @param fsm: Cấu trúc FSM để quản lý trạng thái của tác vụ message-driven
	 * @param tsm: Cấu trúc TSM để quản lý thời gian và sự kiện của tác vụ message-driven
	 * @param task_norm: Hàm thực thi của tác vụ message-driven, được gọi khi tác vụ nhận được tin nhắn để xử lý
	 * @param msg_queue: Hàng đợi tin nhắn của tác vụ message-driven
	 * @param msg_queue_buffer: Bộ nhớ đệm cho hàng đợi tin nhắn của tác vụ message-driven
	 */
	typedef struct task_norm_t {
		task_id_t id;
		task_pri_t base_pri;
		task_pri_t cur_pri;
		bool urgent_pending;
		pf_task_norm task_norm;
		fifo_t msg_queue; 
		uedp_msg_t** msg_queue_buffer;
	} task_norm_t;

	/**
	 * @brief Định nghĩa cấu trúc để quản lý thông tin của tác vụ poll-driven
	 * @attention `ability` được sử dụng để quản lý khả năng của tác vụ poll, 
	 *            cho phép hệ thống UEDP xác định và điều phối việc thực thi của các tác vụ poll 
	 * 						dựa trên khả năng của chúng.
	 */
	typedef struct task_poll_t {
		task_id_t id;									// ID của tác vụ poll
		ui8 ability;									// Khả năng của tác vụ poll
		pf_task_poll task_poll;	// Hàm thực thi của tác vụ poll
	} task_poll_t;

	/**
	 * @brief Hàm tạo tác vụ message-driven trong hệ thống UEDP
	 * @param task_table: Con trỏ đến bảng chứa thông tin của các tác vụ message-driven cần tạo     
	 */
	void uedp_task_norm_create(task_norm_t* task_table);

	/**
	 * @brief Hàm tạo tác vụ poll-driven trong hệ thống UEDP
	 * @param task_table: Con trỏ đến bảng chứa thông tin của các tác vụ poll-driven cần tạo     
	 */
	void uedp_task_poll_create(task_poll_t* task_table);

	/**
	 * @brief Hàm gửi tin nhắn từ một tác vụ đến một tác vụ khác trong hệ thống UEDP
	 * @param dest_id: ID của tác vụ đích mà tin nhắn sẽ được gửi đến
	 * @param msg: Con trỏ đến cấu trúc tin nhắn cần gửi
	 * @return RETR_STAT: Trả về trạng thái của việc gửi tin nhắn
	 */
	RETR_STAT uedp_task_norm_post_msg(task_id_t dest_id, uedp_msg_t* msg);

	/**
	 * @brief Hàm đăng ký tín hiệu từ ISR cho tác vụ trong hệ thống UEDP
	 * @param dest_id: ID của tác vụ đích mà tín hiệu sẽ được đăng ký
	 * @param sig: Giá trị của tín hiệu cần đăng ký
	 * @return RETR_STAT: Trả về trạng thái của việc đăng ký tín hiệu
	 * @attention Hàm này được thiết kế tách biệt dành cho việc xử lý với ISR
	 */
	RETR_STAT uedp_task_norm_post_isr(task_id_t dest_id, ui8 sig);

	/**
	 * @brief Hàm lập lịch và thực thi các tác vụ trong hệ thống UEDP
	 * @return RETR_STAT: Trả về trạng thái của việc lập lịch và thực thi các tác vụ, 
	 * 				 bao gồm các trạng thái như OK, ERROR, BUSY, TIMEOUT, DONE, NRDY và RDY, giúp người dùng dễ dàng xác định kết quả của việc lập lịch và thực thi các tác vụ trong hệ thống UEDP.
	 */
	RETR_STAT uedp_task_scheduler(); 

	/**
	 * @brief Hàm lấy ID của tác vụ hiện tại đang được thực thi
	 * @return task_id_t 
	 */
	task_id_t uedp_task_norm_get_current_id();

	/**
	 * @brief Hàm lấy tin nhắn hiện tại đang được xử lý bởi tác vụ
	 * @return uedp_msg_t* 
	 */
	uedp_msg_t* uedp_task_norm_get_current_msg();

	/**
	 * @brief Hàm kiểm tra xem tác vụ có sẵn sàng để thực thi hay không
	 * 
	 * @param task_id ID của tác vụ cần kiểm tra
	 * @return true nếu tác vụ sẵn sàng để thực thi
	 * @return false nếu tác vụ không sẵn sàng để thực thi
	 */
	bool uedp_task_norm_is_ready(task_id_t task_id);

	/**
	 * @brief Lấy thông tin hàng đợi của một Task
	 * @param tid ID của Task cần lấy thông tin
	 * @param used Con trỏ đến biến sẽ nhận số lượng tin nhắn đang có trong hàng đợi của Task
	 * @param max Con trỏ đến biến sẽ nhận kích thước tối đa của hàng đợi của Task
	 */
	void uedp_task_norm_get_queue_stats(task_id_t tid, ui8* used, ui8* max);

	/**
	 * @brief Thiết lập khả năng thực thi cho tác vụ poll
	 * @param tid ID của tác vụ poll cần thiết lập khả năng
	 * @param ability Khả năng thực thi của tác vụ poll
	 */
	void uedp_task_poll_set_ability(task_id_t tid, ui8 ability);

		/**
	 * @brief Thiết lập một tín hiệu khẩn cấp cho một tác vụ message-driven,
	 * 				cho phép tác vụ tăng mức ưu tiên thực thi cao hơn mức tín hiệu cao nhất hiện tại,
	 * 				để đảm bảo rằng tác vụ sẽ được thực thi ngay lập tức sau khi nhận được tín hiệu khẩn cấp,
	 * 				mà không bị chậm trễ bởi các tác vụ khác đang ở trạng thái sẵn sàng với mức độ ưu tiên thấp hơn.
	 * @param tid ID của tác vụ message-driven cần thiết lập tín hiệu khẩn cấp
	 */
	void uedp_task_norm_set_urgent(task_id_t tid);

#endif //__TASK_H__

