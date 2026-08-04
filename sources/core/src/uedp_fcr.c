/**
 * @file uedp_fcr.c
 * @author Hai Minh
 * @brief Implementation of Fatal Code Return (FCR) - định danh và xử lý lỗi nghiêm trọng
 * @version 0.1
 * @date 2026-08-01
 * @copyright MIT License
 */
#include <stddef.h>
#include "uedp_core.h"
#include "uedp_fcr.h"
#include "uedp_itnlog.h"
#include "pal_core.h"

/**
 * @brief Bảng mã lỗi nghiêm trọng của lõi UEDP
 * @attention Đây là bảng "tĩnh" (static const), không cần khởi tạo runtime.
 *            Mỗi module trong lõi UEDP chỉ nên có tối đa 256 mã lỗi con (0x00 -> 0xFF),
 *            xem UEDP_FCR_MOD_* trong uedp_fcr.h để biết dải mã module tương ứng.
 * @attention Entry cuối cùng (UEDP_FCR_UNKNOWN) LUÔN phải tồn tại và được đặt cuối bảng,
 *            dùng làm fallback khi uedp_fcr_lookup() không tìm thấy mã lỗi tương ứng.
 */
sta const uedp_fcr_entry_t g_fcr_table[] = {
  // [MSG]
  { UEDP_FCR_MSG_POOL_EXHAUSTED,   "MSG pool exhausted",              UEDP_FCR_SEV_FATAL, UEDP_FCR_ACT_SYS_PANIC  },
  { UEDP_FCR_MSG_INVALID_PTR,      "MSG invalid pointer",             UEDP_FCR_SEV_ERROR,  UEDP_FCR_ACT_LOG_ONLY   },
  { UEDP_FCR_MSG_ISR_FIFO_FULL,    "MSG ISR fifo full",               UEDP_FCR_SEV_FATAL, UEDP_FCR_ACT_SYS_PANIC  },

  // [TASK]
  { UEDP_FCR_TASK_QUEUE_FULL,      "TASK message queue full",         UEDP_FCR_SEV_ERROR,  UEDP_FCR_ACT_RESET_TASK },
  { UEDP_FCR_TASK_INVALID_ID,      "TASK invalid ID",                 UEDP_FCR_SEV_ERROR,  UEDP_FCR_ACT_LOG_ONLY   },
  { UEDP_FCR_TASK_PRI_EXHAUSTED,   "TASK priority escalation exhausted", UEDP_FCR_SEV_WARN, UEDP_FCR_ACT_LOG_ONLY  },

  // [TIMER]
  { UEDP_FCR_TIMER_POOL_EXHAUSTED, "TIMER pool exhausted",            UEDP_FCR_SEV_ERROR,  UEDP_FCR_ACT_LOG_ONLY   },

  // [SM] (FSM/TSM)
  { UEDP_FCR_SM_INVALID_TRANS,     "SM invalid transition",           UEDP_FCR_SEV_ERROR,  UEDP_FCR_ACT_LOG_ONLY   },
  { UEDP_FCR_SM_NULL_HANDLER,      "SM null state handler",           UEDP_FCR_SEV_FATAL, UEDP_FCR_ACT_SYS_PANIC  },

  // [ITNLOG]
  { UEDP_FCR_ITNLOG_BUF_CORRUPT,   "ITNLOG buffer corrupted",         UEDP_FCR_SEV_ERROR,  UEDP_FCR_ACT_LOG_ONLY   },

  // [OCE]
  { UEDP_FCR_OCE_REGISTRY_FULL,    "OCE registry full",               UEDP_FCR_SEV_WARN,   UEDP_FCR_ACT_LOG_ONLY   },

  // [PAL]
  { UEDP_FCR_PAL_LOGDP_TABLE_FULL, "PAL logdp output table full",     UEDP_FCR_SEV_FATAL, UEDP_FCR_ACT_SYS_PANIC  },

  // Fallback - LUÔN đặt cuối cùng
  { UEDP_FCR_UNKNOWN,              "Unknown FCR code",                UEDP_FCR_SEV_FATAL, UEDP_FCR_ACT_SYS_PANIC  }
};

#define UEDP_FCR_TABLE_SIZE   (sizeof(g_fcr_table) / sizeof(g_fcr_table[0]))

const uedp_fcr_entry_t* uedp_fcr_lookup(uedp_fcr_code_t code) {
  for (ui16 i = 0; i < (ui16)UEDP_FCR_TABLE_SIZE; i++) {
    if (g_fcr_table[i].code == code) {
      return &g_fcr_table[i];
    }
  }

  // Không tìm thấy -> trả về entry fallback (luôn là phần tử cuối bảng)
  return &g_fcr_table[UEDP_FCR_TABLE_SIZE - 1];
}

/**
 * @brief Chuyển đổi mức độ nghiêm trọng FCR sang mức độ log tương ứng của itnlog
 */
sta uedp_itnlog_level_t internal_uedp_fcr_sev_to_level(uedp_fcr_severity_t severity) {
  switch (severity) {
    case UEDP_FCR_SEV_WARN:  return ITNLOG_LEVEL_WARN;
    case UEDP_FCR_SEV_ERROR: return ITNLOG_LEVEL_ERROR;
    case UEDP_FCR_SEV_FATAL:
    default:                 return ITNLOG_LEVEL_FATAL;
  }
}

void uedp_fcr_raise(uedp_fcr_code_t code, const char* file, ui32 line, const char* extra_msg) {
  const uedp_fcr_entry_t* entry = uedp_fcr_lookup(code);

  // 1. Ghi log qua itnlog trước khi xử lý hành động, để đảm bảo dấu vết lỗi
  //    luôn được lưu lại kể cả khi hành động tiếp theo là SYS_PANIC/SYS_RESET
  uedp_itnlog_log(
    pal_sys_get_tick(),
    internal_uedp_fcr_sev_to_level(entry->severity),
    ITNLOG_TAG_FCR,
    (extra_msg != NULL) ? extra_msg : entry->desc
  );

  // 2. Thực hiện hành động xử lý tương ứng với entry tra được trong bảng
  switch (entry->action) {
    case UEDP_FCR_ACT_LOG_ONLY:
      // Không can thiệp thêm, chỉ ghi log ở bước 1
      break;

    case UEDP_FCR_ACT_RESET_TASK:
      // Bản 0.1: chưa tự động khôi phục tác vụ (cần cơ chế reset TSM/FSM về IDLE
      // an toàn từ bên ngoài ngữ cảnh của chính tác vụ đó). Hiện tại chỉ ghi log
      // ở mức ERROR để tầng trên (task giám sát / OCE) tự quyết định xử lý tiếp.
      break;

    case UEDP_FCR_ACT_SYS_RESET:
      pal_sys_reset();
      break;

    case UEDP_FCR_ACT_SYS_PANIC:
    default:
      pal_sys_fatal(file, line, entry->desc);
      break;
  }
}
