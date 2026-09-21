/**
 * @file test.c
 * @author Shang Huang
 * @brief Test case 04: Kiểm thử [FCR] Fatal Code Return - bảng mã lỗi + hành động xử lý
 * @version 0.1
 * @date 2026-08-01
 * @copyright MIT License
 *
 * @note Test này chỉ kiểm thử các action KHÔNG làm dừng tiến trình
 *       (UEDP_FCR_ACT_LOG_ONLY, UEDP_FCR_ACT_RESET_TASK) một cách tự động.
 *       Các mã lỗi có action UEDP_FCR_ACT_SYS_PANIC / UEDP_FCR_ACT_SYS_RESET
 *       chỉ được kiểm thử qua uedp_fcr_lookup() (không side-effect), KHÔNG
 *       được raise thật trong test tự động vì pal_sys_fatal()/pal_sys_reset()
 *       trên Linux gọi abort()/exit() - sẽ làm chết tiến trình test.
 *       Xem hàm test_fcr_panic_demo() ở cuối file để chạy minh hoạ panic thủ công.
 */
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "test.h"

/**
 * @brief Static message queue cho task FCR_TEST
 */
sta uedp_msg_t* fcr_q_mem[8];

/**
 * @brief Bộ đếm PASS/FAIL toàn cục cho toàn bộ test case
 */
sta int g_pass_count = 0;
sta int g_fail_count = 0;

/**
 * @brief Cờ báo hiệu test đã chạy xong (để main() thoát vòng lặp scheduler)
 */
sta volatile bool g_test_done = false;

/**
 * @brief Buffer để capture lại các dòng log được itnlog xuất ra qua uedp_itnlog_dump(),
 *        dùng để kiểm tra nội dung log thực tế khớp với kỳ vọng (desc / extra_msg).
 */
#define CAPTURE_BUF_SIZE  (256)
sta char g_last_captured_log[CAPTURE_BUF_SIZE] = {0};

void test_itnlog_capture_output(const char* line) {
  strncpy(g_last_captured_log, line, CAPTURE_BUF_SIZE - 1);
  g_last_captured_log[CAPTURE_BUF_SIZE - 1] = '\0';
  printf("  [itnlog output] %s", line);
}

/**
 * @brief Macro tiện lợi để check điều kiện và cập nhật bộ đếm PASS/FAIL
 */
#define TEST_CHECK(desc, cond)                                            \
  do {                                                                    \
    if (cond) {                                                          \
      printf("  [PASS] %s\n", desc);                                     \
      g_pass_count++;                                                    \
    } else {                                                             \
      printf("  [FAIL] %s\n", desc);                                     \
      g_fail_count++;                                                    \
    }                                                                    \
  } while (0)

/**
 * @brief Case 1: uedp_fcr_lookup() với mã lỗi đã có sẵn trong g_fcr_table
 *        phải trả về đúng desc/severity/action đã khai báo.
 */
sta void test_case_lookup_known(void) {
  printf("[TEST] Case 1: uedp_fcr_lookup() cho mã lỗi đã đăng ký\n");

  const uedp_fcr_entry_t* e1 = uedp_fcr_lookup(UEDP_FCR_MSG_POOL_EXHAUSTED);
  TEST_CHECK("MSG_POOL_EXHAUSTED -> desc đúng",
    strcmp(e1->desc, "MSG pool exhausted") == 0);
  TEST_CHECK("MSG_POOL_EXHAUSTED -> severity FATAL",
    e1->severity == UEDP_FCR_SEV_FATAL);
  TEST_CHECK("MSG_POOL_EXHAUSTED -> action SYS_PANIC",
    e1->action == UEDP_FCR_ACT_SYS_PANIC);

  const uedp_fcr_entry_t* e2 = uedp_fcr_lookup(UEDP_FCR_TASK_PRI_EXHAUSTED);
  TEST_CHECK("TASK_PRI_EXHAUSTED -> severity WARN",
    e2->severity == UEDP_FCR_SEV_WARN);
  TEST_CHECK("TASK_PRI_EXHAUSTED -> action LOG_ONLY",
    e2->action == UEDP_FCR_ACT_LOG_ONLY);

  const uedp_fcr_entry_t* e3 = uedp_fcr_lookup(UEDP_FCR_TASK_QUEUE_FULL);
  TEST_CHECK("TASK_QUEUE_FULL -> action RESET_TASK",
    e3->action == UEDP_FCR_ACT_RESET_TASK);
}

/**
 * @brief Case 2: uedp_fcr_lookup() với mã lỗi KHÔNG có trong bảng (ví dụ mã tự
 *        khai báo ở tầng app qua UEDP_FCR_CODE(UEDP_FCR_MOD_APP, x) nhưng chưa
 *        được thêm entry) phải rơi về fallback UEDP_FCR_UNKNOWN.
 */
sta void test_case_lookup_unknown_fallback(void) {
  printf("[TEST] Case 2: uedp_fcr_lookup() fallback cho mã lỗi chưa đăng ký\n");

  uedp_fcr_code_t custom_app_code = UEDP_FCR_CODE(UEDP_FCR_MOD_APP, 0x05);
  const uedp_fcr_entry_t* e = uedp_fcr_lookup(custom_app_code);

  TEST_CHECK("mã app chưa đăng ký -> trả về entry UNKNOWN",
    e->code == UEDP_FCR_UNKNOWN);
  TEST_CHECK("entry UNKNOWN -> severity FATAL",
    e->severity == UEDP_FCR_SEV_FATAL);
}

/**
 * @brief Case 3: uedp_fcr_raise() với action LOG_ONLY phải ghi log đúng nội dung
 *        mô tả mặc định (entry->desc) khi không truyền extra_msg.
 */
sta void test_case_raise_log_only(void) {
  printf("[TEST] Case 3: uedp_fcr_raise() với action LOG_ONLY (dùng desc mặc định)\n");

  g_last_captured_log[0] = '\0';
  UEDP_FCR_RAISE(UEDP_FCR_TASK_INVALID_ID); // action = LOG_ONLY theo bảng
  uedp_itnlog_dump(); // Flush ring buffer ra output callback ngay để so sánh

  TEST_CHECK("log output chứa đúng mô tả lỗi mặc định",
    strstr(g_last_captured_log, "TASK invalid ID") != NULL);
}

/**
 * @brief Case 4: uedp_fcr_raise() khi có extra_msg thì log phải ưu tiên
 *        dùng extra_msg thay vì entry->desc.
 */
sta void test_case_raise_with_extra_msg(void) {
  printf("[TEST] Case 4: uedp_fcr_raise() với extra_msg tuỳ chỉnh\n");

  g_last_captured_log[0] = '\0';
  UEDP_FCR_RAISE_MSG(UEDP_FCR_SM_INVALID_TRANS, "no transition for SIG_0x99 at STATE_IDLE");
  uedp_itnlog_dump();

  TEST_CHECK("log output chứa extra_msg thay vì desc mặc định",
    strstr(g_last_captured_log, "no transition for SIG_0x99 at STATE_IDLE") != NULL);
  TEST_CHECK("log output KHÔNG chứa desc mặc định khi có extra_msg",
    strstr(g_last_captured_log, "SM invalid transition") == NULL);
}

/**
 * @brief Case 5: uedp_fcr_raise() với action RESET_TASK - ở bản 0.1 hành động này
 *        hiện chỉ dừng lại ở việc ghi log (chưa tự động khôi phục tác vụ), nên
 *        test chỉ xác nhận rằng tiến trình KHÔNG bị dừng và log vẫn được ghi.
 */
sta void test_case_raise_reset_task(void) {
  printf("[TEST] Case 5: uedp_fcr_raise() với action RESET_TASK (chưa auto-recover ở v0.1)\n");

  g_last_captured_log[0] = '\0';
  UEDP_FCR_RAISE(UEDP_FCR_TASK_QUEUE_FULL);
  uedp_itnlog_dump();

  TEST_CHECK("tiến trình vẫn sống sau RESET_TASK + log được ghi",
    strstr(g_last_captured_log, "TASK message queue full") != NULL);
}

/**
 * @brief Case 6: xác nhận các mã lỗi được xếp loại UEDP_FCR_ACT_SYS_PANIC /
 *        UEDP_FCR_ACT_SYS_RESET đúng như thiết kế - CHỈ qua lookup, KHÔNG raise
 *        thật (raise thật sẽ abort()/exit() tiến trình test).
 */
sta void test_case_panic_reset_classification(void) {
  printf("[TEST] Case 6: phân loại các mã lỗi SYS_PANIC / SYS_RESET (chỉ lookup)\n");

  TEST_CHECK("MSG_ISR_FIFO_FULL -> SYS_PANIC",
    uedp_fcr_lookup(UEDP_FCR_MSG_ISR_FIFO_FULL)->action == UEDP_FCR_ACT_SYS_PANIC);
  TEST_CHECK("SM_NULL_HANDLER -> SYS_PANIC",
    uedp_fcr_lookup(UEDP_FCR_SM_NULL_HANDLER)->action == UEDP_FCR_ACT_SYS_PANIC);
  TEST_CHECK("PAL_LOGDP_TABLE_FULL -> SYS_PANIC",
    uedp_fcr_lookup(UEDP_FCR_PAL_LOGDP_TABLE_FULL)->action == UEDP_FCR_ACT_SYS_PANIC);
  TEST_CHECK("UEDP_FCR_UNKNOWN -> SYS_PANIC (fallback nguy hiểm nhất)",
    uedp_fcr_lookup(UEDP_FCR_UNKNOWN)->action == UEDP_FCR_ACT_SYS_PANIC);
}

/**
 * @brief Handler chính của task FCR_TEST - chạy toàn bộ 6 case kiểm thử
 *        khi nhận được SIG_TEST_RUN (tại đây g_current_msg khác NULL nên
 *        uedp_fcr_raise() -> uedp_itnlog_log() gọi an toàn).
 */
void task_norm_fcr_handler(uedp_msg_t* msg) {
  if (msg->sig != SIG_TEST_RUN) {
    return;
  }

  printf("=== [FCR TEST] Bắt đầu chạy các case kiểm thử ===\n\n");

  test_case_lookup_known();
  test_case_lookup_unknown_fallback();
  test_case_raise_log_only();
  test_case_raise_with_extra_msg();
  test_case_raise_reset_task();
  test_case_panic_reset_classification();

  printf("\n=== [FCR TEST] Kết quả: %d PASS / %d FAIL ===\n", g_pass_count, g_fail_count);
  g_test_done = true;
}

/**
 * @brief Bảng task tổng - chỉ có 1 task duy nhất phục vụ test này
 */
task_norm_t app_task_table[] = {
  { TASK_NORM_FCR_ID,     UEDP_TASK_PRI_LEVEL_8, {0}, {0}, task_norm_fcr_handler, {0}, fcr_q_mem },
  { UEDP_TASK_NORM_EOT_ID, UEDP_TASK_PRI_LEVEL_0, {0}, {0}, NULL,                 {0}, NULL      }
};

/**
 * @brief Hàm giả lập Tick hệ thống (không dùng timer trong test này nhưng
 *        uedp_timer_init() vẫn cần được gọi để đảm bảo core khởi tạo đầy đủ)
 */
void* linux_tick_thread(void* arg) {
  while (!g_test_done) {
    usleep(1000); // 1ms
    uedp_timer_tick();
  }
  return NULL;
}

int test_fcr_run_all(void) {
  printf("=== UEDP [FCR] LINUX UNIT TEST ===\n");

  uedp_core_init();
  uedp_msg_pool_init();
  uedp_timer_init();
  uedp_itnlog_init();
  uedp_task_norm_create(app_task_table);

  /**
   * @attention WORKAROUND (không thuộc phạm vi FCR): tại thời điểm viết test này,
   *            uedp_task_norm_create() chưa gán cur_pri = base_pri cho từng task
   *            trong bảng - cur_pri bị bỏ mặc định {0} khi khai báo table theo
   *            đúng convention của test01/test02/test03. Vì scheduler chỉ dispatch
   *            khi có priority > UEDP_TASK_PRI_LEVEL_0 (= 0), task sẽ KHÔNG BAO GIỜ
   *            được chọn chạy nếu thiếu dòng gán dưới đây. Đây là một gap có sẵn ở
   *            uedp_task_norm_create() (ảnh hưởng cả test01-03), không phải lỗi của
   *            FCR - nên tạm workaround tại đây thay vì sửa trong core.
   */
  app_task_table[0].cur_pri = UEDP_TASK_PRI_LEVEL_8;

  uedp_itnlog_set_output(test_itnlog_capture_output);
  uedp_itnlog_set_filter(ITNLOG_LEVEL_DEBUG, NULL); // Không lọc, nhận mọi mức độ/tag

  pthread_t tick_tid;
  pthread_create(&tick_tid, NULL, linux_tick_thread, NULL);

  uedp_msg_t* start_msg = uedp_msg_alloc(TASK_NORM_FCR_ID, SIG_TEST_RUN, 0);
  uedp_task_norm_post_msg(TASK_NORM_FCR_ID, start_msg);

  // Chạy scheduler tới khi test xong hoặc chạm ngưỡng vòng lặp an toàn (tránh treo)
  for (int i = 0; i < 100000 && !g_test_done; i++) {
    uedp_task_scheduler();
    usleep(100);
  }

  pthread_join(tick_tid, NULL);

  if (!g_test_done) {
    printf("[FATAL] Test không hoàn tất trong giới hạn vòng lặp cho phép.\n");
    return 1;
  }

  return (g_fail_count == 0) ? 0 : 1;
}

int main(void) {
  return test_fcr_run_all();
}
