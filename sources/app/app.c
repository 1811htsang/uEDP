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

//ANCHOR - Khai báo các biến toàn cục GDA
//NOTE - Sample `int x = 1;` or `char str[20] = "Hello";` or `float f = 3.14;`
const char* GDA_SYSTEM_STATUS = "STATUS: INITIALIZING";
int GDA_COUNTER = 0;
bool GDA_FLAG = false;

//ANCHOR - Khai báo bảng task norm
task_norm_t app_task_table[] = {
  { TASK_NORM_USR, UEDP_TASK_PRI_LEVEL_8, UEDP_TASK_PRI_LEVEL_8, false, NULL, &tsm_task_usr, tnorm_usr_nhler, {0}, tnorm_usr_msgq },
  { TASK_NORM_A, UEDP_TASK_PRI_LEVEL_7, UEDP_TASK_PRI_LEVEL_7, false, NULL, &tsm_task_a, tnorm_a_nhler, {0}, tnorm_a_msgq },
  { TASK_NORM_B, UEDP_TASK_PRI_LEVEL_6, UEDP_TASK_PRI_LEVEL_6, false, &fsm_task_b, NULL, tnorm_b_nhler, {0}, tnorm_b_msgq },
  { UEDP_TASK_NORM_EOT_ID, UEDP_TASK_PRI_LEVEL_0, UEDP_TASK_PRI_LEVEL_0, false, NULL, NULL, NULL, {0}, NULL }
};

//ANCHOR - Khai báo bảng task poll
task_poll_t app_poll_table[] = {
  { TASK_POLL_MEMRP, false, tpoll_memrp_phler },
  { TASK_POLL_BLINK, false, tpoll_blink_phler },
  { UEDP_TASK_POLL_EOT_ID, false, NULL }
};

//ANCHOR - Khai báo value tsm_task_usr_state_idle_ID
#define tsm_task_usr_state_idle_ID (UEDP_TSM_STATE_MIN + UEDP_TSM_STATE_OFFSET + 0u)

//ANCHOR - Khai báo các hàm entry, active, exit cho state tsm_task_usr_state_idle_ID
void tsm_task_usr_state_idle_ntry(uedp_msg_t* msg);
void tsm_task_usr_state_idle_onst(uedp_msg_t* msg);
void tsm_task_usr_state_idle_exit(uedp_msg_t* msg);

//ANCHOR - Khai báo value tsm_task_usr_state_running_ID
#define tsm_task_usr_state_running_ID (UEDP_TSM_STATE_MIN + UEDP_TSM_STATE_OFFSET + 1u)

//ANCHOR - Khai báo các hàm entry, active, exit cho state tsm_task_usr_state_running_ID
void tsm_task_usr_state_running_ntry(uedp_msg_t* msg);
void tsm_task_usr_state_running_onst(uedp_msg_t* msg);
void tsm_task_usr_state_running_exit(uedp_msg_t* msg);

//ANCHOR - Khai báo tsm_object
uedp_tsm_t tsm_task_usr;

tsm_trans_t tsm_task_usr_state_idle_trans[] = {
  { SIG_USR_START, (tsm_task_usr_state_running_ID), tsm_task_usr_state_idle_onst },
  { SIG_USR_STOP, (UEDP_TSM_STATE_STAY), tsm_task_usr_state_idle_onst },
};

tsm_trans_t tsm_task_usr_state_running_trans[] = {
  { SIG_USR_STOP, (tsm_task_usr_state_idle_ID), tsm_task_usr_state_running_onst },
  { SIG_USR_START, (UEDP_TSM_STATE_STAY), tsm_task_usr_state_running_onst },
};

tsm_state_desc_t tsm_task_usr_tbl[] = {
  { tsm_task_usr_state_idle_ID, tsm_task_usr_state_idle_ntry, tsm_task_usr_state_idle_exit, tsm_task_usr_state_idle_trans, (ui8)(sizeof(tsm_task_usr_state_idle_trans) / sizeof(tsm_task_usr_state_idle_trans[0])) },
  { tsm_task_usr_state_running_ID, tsm_task_usr_state_running_ntry, tsm_task_usr_state_running_exit, tsm_task_usr_state_running_trans, (ui8)(sizeof(tsm_task_usr_state_running_trans) / sizeof(tsm_task_usr_state_running_trans[0])) },
};

void tnorm_usr_nhler(uedp_msg_t* msg) {
  uedp_tsm_dispatch(&tsm_task_usr, msg);
  system_action_count++;
}

void tsm_task_usr_state_idle_ntry(uedp_msg_t* msg) {
  system_action_count++;

  if (msg == NULL) {
    printf("[USR][IDLE][NTRY] LOGIC SEQ STRT.\n");
  }

}

void tsm_task_usr_state_idle_onst(uedp_msg_t* msg) {
  system_action_count++;

  if (msg->sig == SIG_USR_START) {
    printf("[USR][IDLE][ACTV] RECV SIG_USR_START. POST USR_START -> A. >> WAITING.\n");
  }

  if (msg->sig == SIG_USR_START) {
    uedp_msg_t* alloc_msg = uedp_msg_alloc(TASK_NORM_A, SIG_USR_START, sizeof(void*));
    uedp_task_norm_post_msg(TASK_NORM_A, alloc_msg);
  }

}

void tsm_task_usr_state_idle_exit(uedp_msg_t* msg) {
  system_action_count++;

  printf("[USR][IDLE][EXIT] INIT SEQ END.\n");
}

void tsm_task_usr_state_running_ntry(uedp_msg_t* msg) {
  system_action_count++;

  printf("[USR][RUNNING][NTRY] LOGIC SEQ STRT.\n");
}

void tsm_task_usr_state_running_onst(uedp_msg_t* msg) {
  system_action_count++;

  for (int i = 0; i < 3; i++) {
    printf("[USR][RUNNING][ACTV] Loop iteration %d.\n", i);
  }

  if (msg->sig == SIG_USR_STOP) { 
    printf("[USR][RUNNING][ACTV] RECV SIG_USR_STOP. USR IDLE, END SEQ.\n");
  }

}

void tsm_task_usr_state_running_exit(uedp_msg_t* msg) {
  system_action_count++;

  printf("[USR][RUNNING][EXIT] LOGIC SEQ END.\n");
}
//ANCHOR - Khai báo value tsm_task_a_state_idle_ID
#define tsm_task_a_state_idle_ID (UEDP_TSM_STATE_MIN + UEDP_TSM_STATE_OFFSET + 0u)

//ANCHOR - Khai báo các hàm entry, active, exit cho state tsm_task_a_state_idle_ID
void tsm_task_a_state_idle_ntry(uedp_msg_t* msg);
void tsm_task_a_state_idle_onst(uedp_msg_t* msg);
void tsm_task_a_state_idle_exit(uedp_msg_t* msg);

//ANCHOR - Khai báo value tsm_task_a_state_waiting_ID
#define tsm_task_a_state_waiting_ID (UEDP_TSM_STATE_MIN + UEDP_TSM_STATE_OFFSET + 1u)

//ANCHOR - Khai báo các hàm entry, active, exit cho state tsm_task_a_state_waiting_ID
void tsm_task_a_state_waiting_ntry(uedp_msg_t* msg);
void tsm_task_a_state_waiting_onst(uedp_msg_t* msg);
void tsm_task_a_state_waiting_exit(uedp_msg_t* msg);

//ANCHOR - Khai báo tsm_object
uedp_tsm_t tsm_task_a;

tsm_trans_t tsm_task_a_state_idle_trans[] = {
  { SIG_USR_START, (tsm_task_a_state_waiting_ID), tsm_task_a_state_idle_onst },
};

tsm_trans_t tsm_task_a_state_waiting_trans[] = {
  { SIG_USR_START, (UEDP_TSM_STATE_STAY), tsm_task_a_state_waiting_onst },
  { SIG_0X34, (UEDP_TSM_STATE_STAY), tsm_task_a_state_waiting_onst },
  { SIG_0XFF, (tsm_task_a_state_idle_ID), tsm_task_a_state_waiting_onst },
};

tsm_state_desc_t tsm_task_a_tbl[] = {
  { tsm_task_a_state_idle_ID, tsm_task_a_state_idle_ntry, tsm_task_a_state_idle_exit, tsm_task_a_state_idle_trans, (ui8)(sizeof(tsm_task_a_state_idle_trans) / sizeof(tsm_task_a_state_idle_trans[0])) },
  { tsm_task_a_state_waiting_ID, tsm_task_a_state_waiting_ntry, tsm_task_a_state_waiting_exit, tsm_task_a_state_waiting_trans, (ui8)(sizeof(tsm_task_a_state_waiting_trans) / sizeof(tsm_task_a_state_waiting_trans[0])) },
};

void tnorm_a_nhler(uedp_msg_t* msg) {
  uedp_tsm_dispatch(&tsm_task_a, msg);
  system_action_count++;
}

void tsm_task_a_state_idle_ntry(uedp_msg_t* msg) {
  system_action_count++;

  if (msg->sig == UEDP_TSM_SIG_ENTRY) {
    printf("[A][IDLE][NTRY] RECV UEDP_TSM_SIG_ENTRY. LOGIC SEQ STRT.\n");
  }

  if (msg->sig == SIG_0XFF) {
    printf("[A][IDLE][NTRY] RECV SIG_0XFF. A IDLE, END SEQ.\n");
  }

}

void tsm_task_a_state_idle_onst(uedp_msg_t* msg) {
  system_action_count++;

  if (msg->sig == SIG_USR_START) {
    printf("[USR][IDLE][ACTV] RECV USR_START.\n");
    printf("[USR][IDLE][ACTV] POST 0X12 -> B. >> WAITING.\n");
  }

  if (msg->sig == SIG_USR_START) {
    uedp_msg_t* alloc_msg = uedp_msg_alloc(TASK_NORM_B, SIG_0X12, sizeof(void*));
    if (alloc_msg) {
      uedp_task_norm_post_msg(TASK_NORM_B, alloc_msg);
    }
  }

}

void tsm_task_a_state_idle_exit(uedp_msg_t* msg) {
  system_action_count++;

  printf("[A][IDLE][EXIT] LOGIC SEQ END.\n");
}

void tsm_task_a_state_waiting_ntry(uedp_msg_t* msg) {
  system_action_count++;

  if (msg->sig == SIG_USR_START) {
    printf("[A][WAITING][NTRY] RECV SIG_USR_START.\n");
  }

  //STUB - temporary code for later implementation if any signal is received triggered tsm_trans

}

void tsm_task_a_state_waiting_onst(uedp_msg_t* msg) {
  system_action_count++;

  if (msg->sig == SIG_0X34) {
    printf("[A][WAITING][NTRY] RECV SIG_0X34. CHECK DATA\n");
  }

  if (msg->sig == SIG_0X34) {
    printf("[A][WAITING][NTRY] RECV SIG_0X34. CHECK DATA\n");
    char* data = (char*)(msg->data);
    if (data && strcmp(data, GDA_SYSTEM_STATUS) == 0) {
      printf("[A][WAITING][NTRY] RECV SIG_0X34. DATA MATCHED: %s\n", data);
    } else {
      printf("[A][WAITING][NTRY] RECV SIG_0X34. DATA MISMATCHED. EXPECTED: %s, RECEIVED: %s\n", GDA_SYSTEM_STATUS, data ? data : "NULL");
    }
  }

  if (msg->sig == SIG_0XFF) {
    printf("[A][WAITING][NTRY] RECV SIG_0XFF.\n");
    printf("[A][WAITING][NTRY] POST SIG_USR_STOP -> USR. >> IDLE.\n");
  }

  if (msg->sig == SIG_0XFF) {
    uedp_msg_t* alloc_msg = uedp_msg_alloc(TASK_NORM_USR, SIG_USR_STOP, sizeof(void*));
    if (alloc_msg) {
      uedp_task_norm_post_msg(TASK_NORM_USR, alloc_msg);
    }
  }

}

void tsm_task_a_state_waiting_exit(uedp_msg_t* msg) {
  system_action_count++;

  printf("[A][WAITING][EXIT] LOGIC SEQ END.\n");
}

void fsm_task_b_state_idle_onst(uedp_msg_t* msg);

void fsm_task_b_state_busy_onst(uedp_msg_t* msg);

//ANCHOR - Khai báo fsm_object
uedp_fsm_t fsm_task_b;

void tnorm_b_nhler(uedp_msg_t* msg) {
  uedp_fsm_dispatch(&fsm_task_b, msg);
  system_action_count++;
}

void fsm_task_b_state_idle_onst(uedp_msg_t* msg) {
  system_action_count++;
  if (msg->sig == SIG_0X12) {
  
    printf("[B][IDLE] RECV SIG_0X12. >> SEND SIG_0X34 + GDA_SYSTEM_STATUS.\n");
    uedp_msg_t* alloc_msg_1 = uedp_msg_alloc(TASK_NORM_A, SIG_0X34, sizeof(typeof(GDA_SYSTEM_STATUS)));
    if (alloc_msg_1) {
      uedp_msg_set_data_ref(alloc_msg_1, (const char*)GDA_SYSTEM_STATUS);
    }
    uedp_task_norm_post_msg(TASK_NORM_A, alloc_msg_1);

    printf("[B][IDLE] SEND SIG_0XFF.\n");
    uedp_msg_t* alloc_msg_2 = uedp_msg_alloc(TASK_NORM_A, SIG_0XFF, UEDP_MSG_TYPE_BLANK);
    uedp_task_norm_post_msg(TASK_NORM_A, alloc_msg_2);

    printf("[B][IDLE] SEND SIG_0XAA -> SELF. >> BUSY.\n");
    uedp_msg_t* alloc_msg_3 = uedp_msg_alloc(TASK_NORM_B, SIG_0XAA, UEDP_MSG_TYPE_BLANK);
    uedp_task_norm_post_msg(TASK_NORM_B, alloc_msg_3);

    uedp_fsm_go_next(&fsm_task_b, fsm_task_b_state_busy_onst);
  }
  if (msg->sig == SIG_0XAA) {
  
    printf("[B][IDLE] RECV SIG_0XAA. STOP\n");
    uedp_fsm_go_next(&fsm_task_b, fsm_task_b_state_idle_onst);
  }
}

void fsm_task_b_state_busy_onst(uedp_msg_t* msg) {
  system_action_count++;
  if (msg->sig == SIG_0XAA) {
  
    printf("[B][BUSY] RECV SIG_0XAA. RETR IDLE\n");
    uedp_fsm_go_next(&fsm_task_b, fsm_task_b_state_idle_onst);
  }
}

void logic_init(void) {
  uedp_task_norm_create(app_task_table);
  uedp_task_poll_create(app_poll_table);
  uedp_tsm_init(&tsm_task_usr, tsm_task_usr_tbl, (ui8)2, tsm_task_usr_state_idle_ID, NULL);
  uedp_tsm_init(&tsm_task_a, tsm_task_a_tbl, (ui8)2, tsm_task_a_state_idle_ID, NULL);
  uedp_fsm_init(&fsm_task_b, fsm_task_b_state_idle_onst);
  uedp_msg_t* start_msg = uedp_msg_alloc(TASK_NORM_USR, SIG_USR_START, 0u);
  if (start_msg) {
    uedp_task_norm_post_msg(TASK_NORM_USR, start_msg); 
  }
}

void tpoll_memrp_phler(void) {
  system_action_count++;
}

void tpoll_blink_phler(void) {
  system_action_count++;
}

void itnlog_dump_handler(void) {
  /* OUTEXEC OCE_ITNLOG_DUMP, context: NULL, state: READY. */
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
