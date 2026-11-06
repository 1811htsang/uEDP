//ANCHOR - Khai báo thư viện uEDP
#include "uedp_core.h"
#include "uedp_fcr.h"
#include "uedp_fsm.h"
#include "uedp_itnlog.h"
#include "uedp_msg.h"
#include "uedp_ocesvc.h"
#include "uedp_task.h"
#include "uedp_timer.h"
#include "uedp_fsm.h"

//ANCHOR - Khai báo thư viện hệ thống
#include "pal_logdp.h"
#include "pal_memrp.h"
#include "pal_rprintf.h"

//ANCHOR - Khai báo thư viện app
#include "app_cfg.h"
#include "app_decl.h"

#define UEDP_NEG_TESTCASE 0x01

//ANCHOR - Biến debug số lượng các tác vụ đã được khởi tạo
uint32_t system_action_count = 0x0u;

//ANCHOR - Khai báo message queue cho các task
uedp_msg_t* tnorm_usr_msgq[UEDP_MSG_BLANK_QUEUE_SIZE];
uedp_msg_t* tnorm_b_msgq[UEDP_MSG_BLANK_QUEUE_SIZE];
uedp_msg_t* tnorm_c_msgq[UEDP_MSG_BLANK_QUEUE_SIZE];

//ANCHOR - Khai báo các biến toàn cục GDA
//NOTE - Sample `int x = 1;` or `char str[20] = "Hello";` or `float f = 3.14;`
const char* GDA_TC_NEG_04_NAME = "MISSING_GDA";
const char* GDA_TC_NEG_05_SOURCE = "PAYLOAD_TOO_LARGE_FOR_OUTPUT";

//ANCHOR - Khai báo bảng task norm
task_norm_t app_task_table[] = {
  { TASK_NORM_USR, UEDP_TASK_PRI_LEVEL_8, UEDP_TASK_PRI_LEVEL_8, false, NULL, NULL, tnorm_usr_nhler, {0}, tnorm_usr_msgq },
  { TASK_NORM_B, UEDP_TASK_PRI_LEVEL_7, UEDP_TASK_PRI_LEVEL_7, false, NULL, NULL, tnorm_b_nhler, {0}, tnorm_b_msgq },
  { TASK_NORM_C, UEDP_TASK_PRI_LEVEL_6, UEDP_TASK_PRI_LEVEL_6, false, NULL, NULL, tnorm_c_nhler, {0}, tnorm_c_msgq },
  { UEDP_TASK_NORM_EOT_ID, UEDP_TASK_PRI_LEVEL_0, UEDP_TASK_PRI_LEVEL_0, false, NULL, NULL, NULL, {0}, NULL }
};

//ANCHOR - Khai báo bảng task poll
task_poll_t app_poll_table[] = {
  { UEDP_TASK_POLL_EOT_ID, false, NULL }
};

void tnorm_usr_nhler(uedp_msg_t* msg) {
  if (msg->sig == SIG_TC_NEG_01) {
    const ui16 invalid_sig = UEDP_TBM_MAX_SIZE;
    task_norm_t task_list[] = {
      { TASK_NORM_B },
      { UEDP_TASK_NORM_EOT_ID }
    };
    RETR_STAT register_result =
        uedp_msg_register_topic(invalid_sig, task_list);
    RETR_STAT clear_result =
        uedp_msg_clear_topic(invalid_sig, task_list);
    RETR_STAT publish_result =
        uedp_msg_publish(invalid_sig, NULL, 0);
    printf("[TC_NEG_01] register=%s clear=%s publish=%s\n",
           register_result == STAT_ERROR ? "PASS" : "FAIL",
           clear_result == STAT_ERROR ? "PASS" : "FAIL",
           publish_result == STAT_ERROR ? "PASS" : "FAIL");

  }
  if (msg->sig == SIG_TC_NEG_02) {
    uedp_msg_t* held[UEDP_MSG_BLANK_QUEUE_SIZE] = {0};
    ui16 held_count = 0;
    for (ui16 i = 0; i < UEDP_MSG_BLANK_QUEUE_SIZE; i++) {
      held[i] = uedp_msg_alloc(
          TASK_NORM_B, SIG_TC_NEG_02, UEDP_MSG_TYPE_BLANK);
      if (held[i] != NULL) {
        held_count++;
      }
    }
    RETR_STAT publish_result =
        uedp_msg_publish(SIG_TC_NEG_02, NULL, 0);
    printf("[TC_NEG_02] held=%u publish=%s\n",
           held_count,
           publish_result == STAT_ERROR ? "PASS" : "FAIL");
    for (ui16 i = 0; i < held_count; i++) {
      uedp_msg_free(held[i]);
    }

  }
  if (msg->sig == SIG_TC_NEG_03) {
    uedp_msg_t* queued[UEDP_MSG_BLANK_QUEUE_SIZE] = {0};
    ui16 queued_count = 0;
    for (ui16 i = 0; i < UEDP_MSG_BLANK_QUEUE_SIZE; i++) {
      queued[i] = uedp_msg_alloc(
          TASK_NORM_B, SIG_TC_NEG_03, sizeof(ui8));
      if (queued[i] != NULL &&
          uedp_task_norm_post_msg(TASK_NORM_B, queued[i]) == STAT_OK) {
        queued_count++;
      }
    }
    RETR_STAT publish_result =
        uedp_msg_publish(SIG_TC_NEG_03, NULL, 0);
    printf("[TC_NEG_03] queued=%u publish=%s\n",
           queued_count,
           publish_result == STAT_ERROR ? "PASS" : "FAIL");

  }
  if (msg->sig == SIG_TC_NEG_04) {
    uedp_gdp_init();
    void* ref = uedp_gdp_get_ref(GDA_TC_NEG_04_NAME);
    printf("[TC_NEG_04] missing GDP ref: %s\n",
           ref == NULL ? "PASS" : "FAIL");

  }
  if (msg->sig == SIG_TC_NEG_05) {
    uedp_gdp_init();
    RETR_STAT register_result = uedp_gdp_register(
        "GDA_TC_NEG_05_SOURCE",
        GDA_TC_NEG_05_SOURCE,
        strlen(GDA_TC_NEG_05_SOURCE) + 1);
    char small_buffer[4] = "OK";
    RETR_STAT get_result = uedp_gdp_get_val(
        "GDA_TC_NEG_05_SOURCE",
        small_buffer,
        sizeof(small_buffer));
    printf("[TC_NEG_05] register=%s get_small=%s buffer=%s\n",
           register_result == STAT_OK ? "PASS" : "FAIL",
           get_result == STAT_ERROR ? "PASS" : "FAIL",
           small_buffer);

  }
  if (msg->sig == SIG_TC_NEG_06) {
    printf("[TC_NEG_06] SKIP: SSI module is not implemented\n");

  }
  system_action_count++;
}

void tnorm_b_nhler(uedp_msg_t* msg) {
  if (msg->sig == SIG_TC_NEG_03) {
    printf("[TC_NEG_03] B queue-full path reached\n");

  }
  system_action_count++;
}

void tnorm_c_nhler(uedp_msg_t* msg) {
  if (msg->sig == SIG_TC_NEG_03) {
    printf("[TC_NEG_03] C received after B queue-full\n");

  }
  system_action_count++;
}

void logic_init(void) {
  uedp_task_norm_create(app_task_table);
  uedp_task_poll_create(app_poll_table);
  { task_norm_t task_list[] = { { TASK_NORM_B }, { UEDP_TASK_NORM_EOT_ID } }; uedp_msg_register_topic(SIG_TC_NEG_02, task_list); }
  { task_norm_t task_list[] = { { TASK_NORM_B }, { TASK_NORM_C }, { UEDP_TASK_NORM_EOT_ID } }; uedp_msg_register_topic(SIG_TC_NEG_03, task_list); }
  uedp_msg_t* start_msg = uedp_msg_alloc(TASK_NORM_USR, SIG_TC_NEG_01, 0u);
  if (start_msg) {
    uedp_task_norm_post_msg(TASK_NORM_USR, start_msg); 
  }
  uedp_msg_t* testcase_msg_1 = uedp_msg_alloc(TASK_NORM_USR, SIG_TC_NEG_02, 0u);
  if (testcase_msg_1) {
    uedp_task_norm_post_msg(TASK_NORM_USR, testcase_msg_1);
  }
  uedp_msg_t* testcase_msg_2 = uedp_msg_alloc(TASK_NORM_USR, SIG_TC_NEG_03, 0u);
  if (testcase_msg_2) {
    uedp_task_norm_post_msg(TASK_NORM_USR, testcase_msg_2);
  }
  uedp_msg_t* testcase_msg_3 = uedp_msg_alloc(TASK_NORM_USR, SIG_TC_NEG_04, 0u);
  if (testcase_msg_3) {
    uedp_task_norm_post_msg(TASK_NORM_USR, testcase_msg_3);
  }
  uedp_msg_t* testcase_msg_4 = uedp_msg_alloc(TASK_NORM_USR, SIG_TC_NEG_05, 0u);
  if (testcase_msg_4) {
    uedp_task_norm_post_msg(TASK_NORM_USR, testcase_msg_4);
  }
  uedp_msg_t* testcase_msg_5 = uedp_msg_alloc(TASK_NORM_USR, SIG_TC_NEG_06, 0u);
  if (testcase_msg_5) {
    uedp_task_norm_post_msg(TASK_NORM_USR, testcase_msg_5);
  }
}

int app_main(void) {
  uedp_core_init();
  logic_init();
  while (1) {
    RETR_STAT retr =  uedp_task_scheduler();
    if (retr == STAT_NRDY) {
      break; 
    }
  }
  return system_action_count;
}
