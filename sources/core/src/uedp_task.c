/**
 * @file uedp_task.c
 * @author Shang Huang
 * @brief Implementation of task management for UEDP system
 * @version 0.1
 * @date 2026-04-16
 * @copyright MIT License
 */
#include <string.h>
#include "uedp_core.h"
#include "uedp_task.h"
#include "uedp_msg.h"
#include "fifo.h"

/**
 * @brief Khai báo các biến toàn cục quản lý thông tin của các tác vụ trong hệ thống UEDP
 */

sta task_norm_t g_current_task_norm = {0};                      // Cấu trúc quản lý thông tin của tác vụ hiện tại đang được thực thi
sta task_id_t g_active_task_norm_id = UEDP_TASK_NORM_IDLE_ID; // ID của tác vụ hiện tại đang được thực thi
sta uedp_msg_t* g_current_msg = NULL;                         // Con trỏ đến tin nhắn hiện tại đang được xử lý bởi tác vụ hiện tại
sta task_pri_t g_task_norm_ready = 0;                           // Biến bitmap quản lý trạng thái sẵn sàng của các tác vụ, mỗi bit tương ứng với một mức độ ưu tiên của tác vụ
sta task_norm_t* g_task_norm_table = NULL;                      // Bảng thông tin của các tác vụ bình thường
sta task_poll_t* g_task_poll_table = NULL;                      // Bảng thông tin của các tác vụ poll
sta ui8 g_task_norm_count = 0;                                  // Số lượng tác vụ bình thường được đăng ký trong hệ thống
sta ui8 g_task_poll_count = 0;                                  // Số lượng tác vụ poll được đăng ký trong hệ thống

/**
 * @brief Khai báo các hàm quản lý nội bộ của hệ thống tác vụ UEDP
 */

sta void          internal_uedp_task_norm_put_to_queue            (task_id_t tid, uedp_msg_t* msg);
sta uedp_msg_t* internal_uedp_task_norm_get_from_queue          (task_id_t tid);
sta void          internal_uedp_task_norm_set_ready               (task_pri_t pri);
sta void          internal_uedp_task_norm_clear_ready             (task_pri_t pri);
sta task_pri_t    internal_uedp_task_norm_find_highest_priority   (void);
sta void          internal_uedp_task_norm_dispatch                (task_norm_t* task, uedp_msg_t* msg);
sta void          internal_uedp_task_poll_exec                    (void);
sta task_norm_t*  internal_get_task_norm_by_id                      (task_id_t tid);

void uedp_task_norm_create(task_norm_t* task_table) {
  if (task_table) {
    g_task_norm_table = task_table;

    // Đếm số lượng tác vụ trong bảng cho đến khi gặp ID kết thúc
    while (task_table[g_task_norm_count].id != UEDP_TASK_NORM_EOT_ID) {
      g_task_norm_count++;
    }

    // Khởi tạo FIFO cho hàng đợi tin nhắn của mỗi tác vụ trong bảng
    for (ui8 i = 0; i < g_task_norm_count; i++) {
      fifo_init(
        &g_task_norm_table[i].msg_queue, 
        g_task_norm_table[i].msg_queue_buffer, 
        UEDP_TASK_MSG_QUEUE_SIZE, 
        sizeof(uedp_msg_t*)
      );
    }
  }
}

void uedp_task_poll_create(task_poll_t* task_table) {
  if (task_table) {
    g_task_poll_table = task_table;

    // Đếm số lượng tác vụ trong bảng cho đến khi gặp ID kết thúc
    while (task_table[g_task_poll_count].id != UEDP_TASK_POLL_EOT_ID) {
      g_task_poll_count++;
    }
  }
}

RETR_STAT uedp_task_norm_post_msg(task_id_t dest_id, uedp_msg_t* msg) {
  if (dest_id < UEDP_TASK_NORM_MIN_ID || dest_id > UEDP_TASK_NORM_MAX_ID) {
    return STAT_ERROR; // Trả về lỗi nếu dest_id không hợp lệ
  }
  if (!msg) {
    return STAT_ERROR; // Trả về lỗi nếu msg là NULL
  }
  internal_uedp_task_norm_put_to_queue(dest_id, msg);
  internal_uedp_task_norm_set_ready(internal_get_task_norm_by_id(dest_id)->base_pri); // Đặt trạng thái sẵn sàng cho tác vụ dựa trên mức độ ưu tiên của nó
  return STAT_OK;
}

RETR_STAT uedp_task_norm_post_isr(task_id_t dest_id, ui8 sig) {
  return internal_uedp_msg_enqueue_isr_sig(dest_id, sig);
}

RETR_STAT uedp_task_scheduler() {
  // Xả tin nhắn từ ISR pool 
  uedp_msg_drain_isr_pool();

  // Tìm mức độ ưu tiên cao nhất của các tác vụ đang ở trạng thái sẵn sàng
  task_pri_t highest_pri = internal_uedp_task_norm_find_highest_priority();

  if (highest_pri > 0) {
    // Nếu có tác vụ nào đó đang ở trạng thái sẵn sàng, tìm và thực thi tác vụ đó
    for (ui8 i = 0; i < g_task_norm_count; i++) {
      if (g_task_norm_table[i].base_pri == highest_pri) {
        uedp_msg_t* msg = internal_uedp_task_norm_get_from_queue(g_task_norm_table[i].id);
        // Nếu lấy được tin nhắn từ hàng đợi của tác vụ, thực thi tác vụ với tin nhắn đó
        if (msg) {
          internal_uedp_task_norm_dispatch(&g_task_norm_table[i], msg);
          // Sau khi thực thi tác vụ, kiểm tra lại hàng đợi của tác vụ đó, nếu hàng đợi trống thì xóa trạng thái sẵn sàng của tác vụ
          if (fifo_is_empty(&g_task_norm_table[i].msg_queue)) {
            internal_uedp_task_norm_clear_ready(highest_pri);
          }
          return STAT_OK; // Trả về OK sau khi đã thực thi tác vụ
        }
      }
    }
  }

  // Nếu không có tác vụ nào ở trạng thái sẵn sàng, có thể thực thi các tác vụ poll nếu cần thiết
  internal_uedp_task_poll_exec();

  return STAT_NRDY; // Trả về NRDY nếu không có tác vụ nào sẵn sàng để thực thi
}

task_id_t uedp_task_norm_get_current_id() {
  return g_active_task_norm_id; // Trả về ID của tác vụ hiện tại đang được thực thi
}

uedp_msg_t* uedp_task_norm_get_current_msg() {
  return g_current_msg; // Trả về con trỏ đến tin nhắn hiện tại đang được xử lý bởi tác vụ
}

bool uedp_task_norm_is_ready(task_id_t task_id) {
  if (task_id < UEDP_TASK_NORM_MIN_ID || task_id > UEDP_TASK_NORM_MAX_ID) {
    return false; // Trả về false nếu task_id không hợp lệ
  }
  task_pri_t pri = 0;
  for (ui8 i = 0; i < g_task_norm_count; i++) {
    if (g_task_norm_table[i].id == task_id) {
      pri = g_task_norm_table[i].base_pri;
      break;
    }
  }
  return (g_task_norm_ready & (1 << (pri - UEDP_TASK_PRI_LEVEL_0))) != 0; // Trả về true nếu tác vụ có mức độ ưu tiên tương ứng đang ở trạng thái sẵn sàng
}

void uedp_task_norm_get_queue_stats(task_id_t tid, ui8* used, ui8* max) {
  task_norm_t* task = internal_get_task_norm_by_id(tid);
  if (task) {
    *used = task->msg_queue.fill_size;
    *max = task->msg_queue.buffer_size;
  }
}

void uedp_task_poll_set_ability(task_id_t tid, ui8 ability) {
  if (tid < UEDP_TASK_POLL_MIN_ID || tid > UEDP_TASK_POLL_MAX_ID) {
    return; // Nếu tid không hợp lệ, không thực hiện gì
  }

  for (ui8 i = 0; i < g_task_poll_count; i++) {
    if (g_task_poll_table[i].id == tid) {
      g_task_poll_table[i].ability = ability; // Thiết lập khả năng thực thi cho tác vụ poll có ID tương ứng
      break;
    }
  }
}

/**
 * @brief Hàm nội bộ để đưa một tin nhắn vào hàng đợi của một tác vụ cụ thể
 * 
 * @param tid ID của tác vụ mà tin nhắn sẽ được đưa vào hàng đợi
 * @param msg Con trỏ đến tin nhắn cần đưa vào hàng đợi
 * @attention Khi gọi hàm này, hãy đảm bảo rằng message queue của tác vụ đã được init trước đó để tránh lỗi
 */
void internal_uedp_task_norm_put_to_queue(task_id_t tid, uedp_msg_t* msg) {
  // Kiểm tra xem tác vụ có tồn tại trong bảng tác vụ hay không
  task_norm_t* task = internal_get_task_norm_by_id(tid);
  if (task == NULL) {
    // Nếu tác vụ không tồn tại, có thể ghi log lỗi hoặc xử lý theo cách phù hợp
    return;
  }

  // Kiểm tra fifo của tác vụ đã được khởi tạo chưa
  if (!fifo_isinit(&task->msg_queue)) {
    // Nếu fifo chưa được khởi tạo, có thể ghi log lỗi hoặc xử lý theo cách phù hợp
    return;
  }

  // Entry critical section
  pal_enter_critical();

  // Đưa tin nhắn vào hàng đợi của tác vụ
  fifo_put(&task->msg_queue, (uedp_msg_t*)(&msg));

  // Exit critical section
  pal_exit_critical();
}

/**
 * @brief Hàm nội bộ để lấy một tin nhắn từ hàng đợi của một tác vụ cụ thể
 * 
 * @param tid ID của tác vụ mà tin nhắn sẽ được lấy từ hàng đợi
 * @attention Khi gọi hàm này, hãy đảm bảo rằng message queue của tác vụ đã được init trước đó để tránh lỗi
 *            Kết quả trả về của hàm là blank message, nếu muốn thực hiện nạp data vào msg thì gọi hàm uedp_msg_set_data
 * @return uedp_msg_t* Con trỏ đến tin nhắn được lấy từ hàng đợi, hoặc NULL nếu hàng đợi trống
 */
uedp_msg_t* internal_uedp_task_norm_get_from_queue(task_id_t tid) {
  task_norm_t* task = internal_get_task_norm_by_id(tid);
  uedp_msg_t* msg_ptr = NULL; // Chỉ cần một con trỏ để hứng giá trị từ FIFO

  if (task && fifo_isinit(&task->msg_queue)) {
    // fifo_get sẽ copy cái con trỏ msg_t* từ trong buffer ra biến msg_ptr
    if (fifo_get(&task->msg_queue, &msg_ptr) == RET_FIFO_OK) {
      return msg_ptr;
    }
  }
  return NULL;
}

/**
 * @brief Hàm nội bộ để thiết lập trạng thái sẵn sàng cho một tác vụ dựa trên mức độ ưu tiên của nó
 * 
 * @param pri Mức độ ưu tiên của tác vụ cần thiết lập trạng thái sẵn sàng
 */
void internal_uedp_task_norm_set_ready(task_pri_t pri) {
  // Kiểm tra xem mức độ ưu tiên có hợp lệ hay không (giả sử mức độ ưu tiên hợp lệ là từ 0 đến 15)
  if (pri < UEDP_TASK_PRI_LEVEL_0 || pri > UEDP_TASK_PRI_LEVEL_15) {
    // Nếu mức độ ưu tiên không hợp lệ, có thể ghi log lỗi hoặc xử lý theo cách phù hợp
    return;
  }

  // Bật entry critical section
  pal_enter_critical();

  // Thiết lập trạng thái sẵn sàng cho tác vụ bằng cách đặt bit tương ứng với mức độ ưu tiên
  g_task_norm_ready |= (1 << (pri - UEDP_TASK_PRI_LEVEL_0));

  // Tắt entry critical section
  pal_exit_critical();
}

/**
 * @brief Hàm nội bộ để xóa trạng thái sẵn sàng của một tác vụ dựa trên mức độ ưu tiên của nó
 * 
 * @param pri Mức độ ưu tiên của tác vụ cần xóa trạng thái sẵn sàng
 */
void internal_uedp_task_norm_clear_ready(task_pri_t pri) {
  // Kiểm tra xem mức độ ưu tiên có hợp lệ hay không (giả sử mức độ ưu tiên hợp lệ là từ 0 đến 15)
  if (pri < UEDP_TASK_PRI_LEVEL_0 || pri > UEDP_TASK_PRI_LEVEL_15) {
    // Nếu mức độ ưu tiên không hợp lệ, có thể ghi log lỗi hoặc xử lý theo cách phù hợp
    return;
  }

  // Bật entry critical section
  pal_enter_critical();

  // Xóa trạng thái sẵn sàng cho tác vụ bằng cách xóa bit tương ứng với mức độ ưu tiên
  g_task_norm_ready &= ~(1 << (pri - UEDP_TASK_PRI_LEVEL_0));

  // Tắt entry critical section
  pal_exit_critical();
}

/**
 * @brief Hàm nội bộ để tìm mức độ ưu tiên cao nhất của các tác vụ đang ở trạng thái sẵn sàng
 * 
 * @return task_pri_t Mức độ ưu tiên cao nhất của các tác vụ đang ở trạng thái sẵn sàng, hoặc 0 nếu không có tác vụ nào sẵn sàng
 */
task_pri_t internal_uedp_task_norm_find_highest_priority(void) {
  // Kiểm tra nếu không có tác vụ nào sẵn sàng
  if (g_task_norm_ready == 0) {
    return 0; // Trả về 0 hoặc một giá trị đặc biệt để biểu thị không có tác vụ nào sẵn sàng
  }

  return (task_pri_t)(pal_math_get_highest_bit16(g_task_norm_ready) + UEDP_TASK_PRI_LEVEL_0);
}

/**
 * @brief Hàm nội bộ để điều phối việc thực thi của một tác vụ cụ thể với một tin nhắn cụ thể
 * 
 * @param task Con trỏ đến cấu trúc thông tin của tác vụ cần điều phối
 * @param msg Con trỏ đến tin nhắn cần được xử lý bởi tác vụ
 * @attention Việc kiểm tra priority để xác định tác vụ có được dispatch hay không
 *            sẽ phụ thuộc vào logic của hàm uedp_task_scheduler
 */
void internal_uedp_task_norm_dispatch(task_norm_t* task, uedp_msg_t* msg) {
  // Kiểm tra hợp lệ của task và msg
  if (task == NULL || msg == NULL) {
    // Nếu task hoặc msg không hợp lệ, có thể ghi log lỗi hoặc xử lý theo cách phù hợp
    return;
  }

  // Cập nhật thông tin về tác vụ hiện tại và tin nhắn hiện tại đang được xử lý
  g_current_task_norm = *task; // Sao chép thông tin của tác vụ vào biến toàn cục quản lý tác vụ hiện tại
  g_current_msg = msg; // Cập nhật con trỏ đến tin nhắn hiện tại đang được xử lý
  g_active_task_norm_id = task->id; // Cập nhật ID của tác vụ hiện tại đang được thực thi

  // Thực thi hàm xử lý của tác vụ với tin nhắn hiện tại
  task->task_norm(msg); 

  // Sau khi thực thi xong, có thể thực hiện các bước dọn dẹp hoặc cập nhật trạng thái nếu cần thiết
  uedp_msg_free(msg); // Giải phóng tin nhắn sau khi đã xử lý xong
  g_current_msg = NULL; // Đặt con trỏ tin nhắn hiện tại về NULL
  g_active_task_norm_id = UEDP_TASK_NORM_IDLE_ID; // Đặt ID của tác vụ hiện tại về ID của tác vụ nhàn rỗi
  g_current_task_norm = (task_norm_t){0}; // Đặt thông tin của tác vụ hiện tại về giá trị mặc định
}

/**
 * @brief Hàm nội bộ để thực thi các tác vụ poll có khả năng được bật
 */
void internal_uedp_task_poll_exec(void) {
  // Duyệt qua bảng tác vụ poll để tìm các tác vụ có khả năng được bật
  for (ui8 i = 0; i < g_task_poll_count; i++) {
    task_poll_t* task = &g_task_poll_table[i];

    // Kiểm tra nếu tác vụ có khả năng được bật
    if (task->ability == true) {
      // Thực thi hàm thực thi của tác vụ poll
      task->task_poll();
    }
  }
}

/**
 * @brief Hàm nội bộ để tìm một tác vụ trong bảng tác vụ dựa trên ID của nó
 * 
 * @param tid ID của tác vụ cần tìm kiếm
 * @return task_norm_t* Con trỏ đến cấu trúc thông tin của tác vụ nếu tìm thấy, hoặc NULL nếu không tìm thấy
 */
task_norm_t* internal_get_task_norm_by_id(task_id_t tid) {
  // Kiểm tra tid hợp lệ
  if (tid < UEDP_TASK_NORM_MIN_ID || tid > UEDP_TASK_NORM_MAX_ID) {
    return NULL; // Trả về NULL nếu tid không hợp lệ
  }

  // Duyệt qua bảng tác vụ để tìm tác vụ có ID tương ứng
  for (ui8 i = 0; i < g_task_norm_count; i++) {
    if (g_task_norm_table[i].id == tid) {
      return &g_task_norm_table[i]; // Trả về con trỏ đến tác vụ nếu tìm thấy
    }
  }

  return NULL; // Trả về NULL nếu không tìm thấy tác vụ nào có ID tương ứng
}
