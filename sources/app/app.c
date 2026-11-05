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

//ANCHOR - Biến debug số lượng các tác vụ đã được khởi tạo
uint32_t system_action_count = 0x0u;

//ANCHOR - Khai báo message queue cho các task
uedp_msg_t* tnorm_usr_msgq[UEDP_MSG_BLANK_QUEUE_SIZE];
uedp_msg_t* tnorm_a_msgq[UEDP_MSG_BLANK_QUEUE_SIZE];
uedp_msg_t* tnorm_b_msgq[UEDP_MSG_BLANK_QUEUE_SIZE];
uedp_msg_t* tnorm_c_msgq[UEDP_MSG_BLANK_QUEUE_SIZE];

//ANCHOR - Khai báo các biến toàn cục GDA
//NOTE - Sample `int x = 1;` or `char str[20] = "Hello";` or `float f = 3.14;`
int GDA_TC_POS_05_REF = 42;
int GDA_TC_POS_05_REF_RESULT = 0;
const char* GDA_TC_POS_06_SOURCE = "VALUE_PAYLOAD";

//ANCHOR - Khai báo bảng task norm
task_norm_t app_task_table[] = {
  { TASK_NORM_USR, UEDP_TASK_PRI_LEVEL_8, UEDP_TASK_PRI_LEVEL_8, false, NULL, NULL, tnorm_usr_nhler, {0}, tnorm_usr_msgq },
  { TASK_NORM_A, UEDP_TASK_PRI_LEVEL_7, UEDP_TASK_PRI_LEVEL_7, false, NULL, NULL, tnorm_a_nhler, {0}, tnorm_a_msgq },
  { TASK_NORM_B, UEDP_TASK_PRI_LEVEL_6, UEDP_TASK_PRI_LEVEL_6, false, NULL, NULL, tnorm_b_nhler, {0}, tnorm_b_msgq },
  { TASK_NORM_C, UEDP_TASK_PRI_LEVEL_5, UEDP_TASK_PRI_LEVEL_5, false, NULL, NULL, tnorm_c_nhler, {0}, tnorm_c_msgq },
  { UEDP_TASK_NORM_EOT_ID, UEDP_TASK_PRI_LEVEL_0, UEDP_TASK_PRI_LEVEL_0, false, NULL, NULL, NULL, {0}, NULL }
};

//ANCHOR - Khai báo bảng task poll
task_poll_t app_poll_table[] = {
  { UEDP_TASK_POLL_EOT_ID, false, NULL }
};


void tnorm_usr_nhler(uedp_msg_t* msg) {
  if (msg->sig == SIG_STR) {
    printf("[USR] RECV STR -> POST STR TO A\n");

    uedp_msg_t* msg =
      uedp_msg_alloc(TASK_NORM_A, SIG_STR, UEDP_MSG_TYPE_BLANK);
    if (msg != NULL) {
      uedp_task_norm_post_msg(TASK_NORM_A, msg);
    }

  }
  if (msg->sig == SIG_0X01) {
    printf("[USR] RECV 0x01 -> STP -> END\n");
    printf("[USR] END\n");

  }
  if (msg->sig == SIG_TC_POS_03) {
    RETR_STAT result = uedp_msg_publish(SIG_TC_POS_03, NULL, 0);
    printf("[TC_POS_03] publish without subscribers: %s\n",
           result == STAT_OK ? "PASS" : "FAIL");

  }
  if (msg->sig == SIG_TC_POS_04) {
    task_norm_t removed_tasks[] = {
      { TASK_NORM_B },
      { UEDP_TASK_NORM_EOT_ID }
    };
    RETR_STAT clear_result =
        uedp_msg_clear_topic(SIG_TC_POS_04, removed_tasks);
    RETR_STAT publish_result =
        uedp_msg_publish(SIG_TC_POS_04, NULL, 0);
    printf("[TC_POS_04] clear B: %s, publish: %s\n",
           clear_result == STAT_OK ? "PASS" : "FAIL",
           publish_result == STAT_OK ? "PASS" : "FAIL");

  }
  if (msg->sig == SIG_TC_POS_05) {
    uedp_gdp_init();
    RETR_STAT register_result = uedp_gdp_register(
        "GDA_TC_POS_05_REF",
        &GDA_TC_POS_05_REF,
        sizeof(GDA_TC_POS_05_REF));
    int* ref = (int*)uedp_gdp_get_ref("GDA_TC_POS_05_REF");
    bool ref_ok = register_result == STAT_OK &&
                  ref == &GDA_TC_POS_05_REF;
    if (ref_ok) {
      *ref = 84;
      GDA_TC_POS_05_REF_RESULT = *ref;
    }
    printf("[TC_POS_05] GDP REF: %s, value=%d\n",
           ref_ok ? "PASS" : "FAIL",
           GDA_TC_POS_05_REF_RESULT);

  }
  if (msg->sig == SIG_TC_POS_06) {
    uedp_gdp_init();
    RETR_STAT register_result = uedp_gdp_register(
        "GDA_TC_POS_06_SOURCE",
        GDA_TC_POS_06_SOURCE,
        strlen(GDA_TC_POS_06_SOURCE) + 1);
    RETR_STAT publish_result = uedp_msg_publish(
        SIG_TC_POS_06,
        GDA_TC_POS_06_SOURCE,
        strlen(GDA_TC_POS_06_SOURCE) + 1);
    printf("[TC_POS_06] GDP VAL source registration: %s, publish: %s\n",
           register_result == STAT_OK ? "PASS" : "FAIL",
           publish_result == STAT_OK ? "PASS" : "FAIL");

  }
  system_action_count++;
}

void tnorm_a_nhler(uedp_msg_t* msg) {
  if (msg->sig == SIG_STR) {
    printf("[A] RECV STR -> PUBLISH 0x30 to B,C\n");
    uedp_msg_publish(SIG_0X30, NULL, 0);

  }
  if (msg->sig == SIG_0X74) {
    printf("[A] RECV 0x74 -> POST 0x04 TO A\n");

    uedp_msg_t* msg =
      uedp_msg_alloc(TASK_NORM_A, SIG_0X04, UEDP_MSG_TYPE_BLANK);
    if (msg != NULL) {
      uedp_task_norm_post_msg(TASK_NORM_A, msg);
    }

  }
  if (msg->sig == SIG_0X04) {
    printf("[A] RECV 0x04 -> STAY\n");

  }
  system_action_count++;
}

void tnorm_b_nhler(uedp_msg_t* msg) {
  if (msg->sig == SIG_0X30) {
    printf("[B] RECV 0x30 -> WAIT 0xA4\n");

  }
  if (msg->sig == SIG_0XA4) {
    printf("[B] RECV 0xA4 -> PUBLISH 0x01 to USR,A,C\n");
    uedp_msg_publish(SIG_0X01, NULL, 0);

    printf("[B] RECV 0xA4 -> POST 0x0A TO B\n");
    uedp_msg_t* msg =
      uedp_msg_alloc(TASK_NORM_B, SIG_0X0A, UEDP_MSG_TYPE_BLANK);
    if (msg != NULL) {
      uedp_task_norm_post_msg(TASK_NORM_B, msg);
    }

  }
  if (msg->sig == SIG_0X0A) {
    printf("[B] RECV 0x0A -> STAY\n");

  }
  if (msg->sig == SIG_TC_POS_04) {
    printf("[TC_POS_04] FAIL: B received message after unsubscribe\n");

  }
  if (msg->sig == SIG_TC_POS_06) {
    char copied_payload[sizeof("VALUE_PAYLOAD")];
    memcpy(copied_payload,
           msg->data,
           strlen(GDA_TC_POS_06_SOURCE) + 1);
    copied_payload[0] = 'X';
    printf("[TC_POS_06] copy=%s, source=%s, %s\n",
           copied_payload,
           GDA_TC_POS_06_SOURCE,
           GDA_TC_POS_06_SOURCE[0] == 'V' ? "PASS" : "FAIL");

  }
  system_action_count++;
}

void tnorm_c_nhler(uedp_msg_t* msg) {
  if (msg->sig == SIG_0X30) {
    printf("[C] RECV 0x30 -> PUBLISH 0x74 to A\n");
    uedp_msg_publish(SIG_0X74, NULL, 0);

    printf("[C] RECV 0x30 -> PUBLISH 0xA4 to B\n");
    uedp_msg_publish(SIG_0XA4, NULL, 0);

  }
  if (msg->sig == SIG_TC_POS_04) {
    printf("[TC_POS_04] PASS: C received message after B unsubscribe\n");
  }
  system_action_count++;
}

void logic_init(void) {
  uedp_task_norm_create(app_task_table);
  uedp_task_poll_create(app_poll_table);
  { task_norm_t task_list[] = { { TASK_NORM_B }, { TASK_NORM_C }, { UEDP_TASK_NORM_EOT_ID } }; uedp_msg_register_topic(SIG_0X30, task_list); }
  { task_norm_t task_list[] = { { TASK_NORM_A }, { UEDP_TASK_NORM_EOT_ID } }; uedp_msg_register_topic(SIG_0X74, task_list); }
  { task_norm_t task_list[] = { { TASK_NORM_B }, { UEDP_TASK_NORM_EOT_ID } }; uedp_msg_register_topic(SIG_0XA4, task_list); }
  { task_norm_t task_list[] = { { TASK_NORM_B }, { UEDP_TASK_NORM_EOT_ID } }; uedp_msg_register_topic(SIG_0X0A, task_list); }
  { task_norm_t task_list[] = { { TASK_NORM_USR }, { TASK_NORM_A }, { TASK_NORM_C }, { UEDP_TASK_NORM_EOT_ID } }; uedp_msg_register_topic(SIG_0X01, task_list); }
  { task_norm_t task_list[] = { { TASK_NORM_B }, { TASK_NORM_C }, { UEDP_TASK_NORM_EOT_ID } }; uedp_msg_register_topic(SIG_TC_POS_04, task_list); }
  { task_norm_t task_list[] = { { TASK_NORM_B }, { UEDP_TASK_NORM_EOT_ID } }; uedp_msg_register_topic(SIG_TC_POS_06, task_list); }
  uedp_msg_t* start_msg = uedp_msg_alloc(TASK_NORM_USR, SIG_STR, 0u);
  if (start_msg) {
    uedp_task_norm_post_msg(TASK_NORM_USR, start_msg); 
  }
  uedp_msg_t* testcase_msg_1 = uedp_msg_alloc(TASK_NORM_USR, SIG_TC_POS_03, 0u);
  if (testcase_msg_1) {
    uedp_task_norm_post_msg(TASK_NORM_USR, testcase_msg_1);
  }
  uedp_msg_t* testcase_msg_2 = uedp_msg_alloc(TASK_NORM_USR, SIG_TC_POS_04, 0u);
  if (testcase_msg_2) {
    uedp_task_norm_post_msg(TASK_NORM_USR, testcase_msg_2);
  }
  uedp_msg_t* testcase_msg_3 = uedp_msg_alloc(TASK_NORM_USR, SIG_TC_POS_05, 0u);
  if (testcase_msg_3) {
    uedp_task_norm_post_msg(TASK_NORM_USR, testcase_msg_3);
  }
  uedp_msg_t* testcase_msg_4 = uedp_msg_alloc(TASK_NORM_USR, SIG_TC_POS_06, 0u);
  if (testcase_msg_4) {
    uedp_task_norm_post_msg(TASK_NORM_USR, testcase_msg_4);
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
