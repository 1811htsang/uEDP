/** ANCHOR - Implementation of Control System for UEDP
 * @file uedp_ctrl.c
 * @author Shang Huang
 * @version 0.1
 * @date 2026-11-15
 * @copyright MIT License
 */
#include "uedp_ctrl.h"
#include "uedp_timer.h"
#include "uedp_task.h"

sta uedp_ctrl_inject_fn g_injector = NULL;
sta void* g_injector_context = NULL;
sta uedp_ctrl_trace_fn g_tracer = NULL;
sta void* g_tracer_context = NULL;
sta bool g_auto_tick_enabled = true;
sta uedp_ctrl_evt_t g_trace_history[UEDP_CTRL_TRACE_MAX] = {0};
sta size_t g_trace_idx = 0;

void uedp_ctrl_set_injector(uedp_ctrl_inject_fn callback, void* context) {
  g_injector = callback;
  g_injector_context = context;
}

void uedp_ctrl_set_tracer(uedp_ctrl_trace_fn callback, void* context) {
  g_tracer = callback;
  g_tracer_context = context;
}

void uedp_ctrl_inject(uedp_ctrl_pnt_t point) {
  if (g_injector) {
    g_injector(point, g_injector_context);
  }
}

void uedp_ctrl_trace(const uedp_ctrl_evt_t* event) {
  if (event == NULL) {
    return;
  }

  if (g_trace_idx < UEDP_CTRL_TRACE_MAX) {
    g_trace_history[g_trace_idx++] = *event;
  }

  if (g_tracer) {
    g_tracer(event, g_tracer_context);
  }
}

void uedp_ctrl_trace_custom(ui16 custom_id, void* payload, size_t value_size, ...) {
  va_list args;
  va_start(args, value_size);

  if (value_size >= 0 && payload == NULL) {
    // Nếu value_size > 0 nhưng payload là NULL, không thể ghi trace
    UEDP_FCR_RAISE_MSG(UEDP_FCR_CTRL_INVALID_PARAM, "Payload is NULL while value_size > 0");
    return;
  } else {
    

    // NOTE - Lưu danh sách các tham số bổ sung sau value_size vào con trỏ value trong sự kiện kiểm soát
    for (size_t i = 0; i < value_size; i++) {
      va_arg(args, ui32); // Bỏ qua các tham số bổ sung
    }
  }

  uedp_ctrl_evt_t evt = {
    .point = UEDP_CTRL_CUSTOM_EVENT,
    .task_id = 0xFF,                   // Không thuộc task cố định nào
    .signal = (ui8)(custom_id & 0xFF), // Mang ID tùy biến
    .value = va_arg(args, ui32*),
    .value_size = value_size,
    .status = STAT_OK,
    .tick = pal_sys_get_tick(),
    .subject = (void*)payload
  };

  va_end(args);
  uedp_ctrl_trace(&evt);
}

void uedp_ctrl_set_auto_tick(bool enabled) {
  g_auto_tick_enabled = enabled;
}

bool uedp_ctrl_auto_tick_enabled(void) {
  return g_auto_tick_enabled;
}

RETR_STAT uedp_ctrl_run_scheduler(ui32 max_steps, ui32* executed_steps) {
  ui32 steps = 0;
  RETR_STAT status = STAT_NRDY;

  if (executed_steps) {
    *executed_steps = 0;
  }
  if (max_steps == 0) {
    return STAT_NRDY;
  }

  while (steps < max_steps) {
    status = uedp_task_scheduler();
    if (status != STAT_OK) {
      break;
    }
    steps++;
  }

  if (executed_steps) {
    *executed_steps = steps;
  }
  return status;
}

RETR_STAT uedp_ctrl_advance_ticks(ui32 tick_count, bool run_scheduler, ui32* executed_steps) {
  ui32 steps = 0;
  RETR_STAT status = STAT_NRDY;

  if (executed_steps) {
    *executed_steps = 0;
  }

  for (ui32 tick = 0; tick < tick_count; tick++) {
    uedp_timer_tick();
    if (run_scheduler) {
      do {
        status = uedp_task_scheduler();
        if (status == STAT_OK) {
          steps++;
        }
      } while (status == STAT_OK);
    }
  }

  if (executed_steps) {
    *executed_steps = steps;
  }
  return run_scheduler ? status : STAT_OK;
}

bool uedp_ctrl_verify_sequence(const uedp_seq_step_t* expected_seq, size_t seq_len, bool strict_order) {
  if (seq_len == 0 || g_trace_idx < seq_len) return false;

  size_t match_idx = 0;

  for (size_t i = 0; i < g_trace_idx; i++) {
    const uedp_ctrl_evt_t* actual = &g_trace_history[i];
    const uedp_seq_step_t* target = &expected_seq[match_idx];

    // So khớp các tiêu chí (bỏ qua nếu wildcard 0xFF)
    bool point_match = (target->point == actual->point);
    bool task_match  = (target->task_id == 0xFF) || (target->task_id == actual->task_id);
    bool sig_match   = (target->signal == 0xFF)  || (target->signal == actual->signal);

    if (point_match && task_match && sig_match) {
      match_idx++; // Khớp được 1 bước trong chuỗi!
      if (match_idx == seq_len) {
        return true; // ĐÃ KHỚP TOÀN BỘ CHUỖI THÀNH CÔNG!
      }
    } else if (strict_order && match_idx > 0) {
      // Nếu yêu cầu khắt khe (strict) mà có sự kiện lạ xen vào -> Reset
      match_idx = 0;
    }
  }

  return false; // Không tìm đủ chuỗi
}