#include "uedp_ctrl.h"
#include "uedp_timer.h"
#include "uedp_task.h"

static uedp_ctrl_inject_fn g_injector = NULL;
static void* g_injector_context = NULL;
static uedp_ctrl_trace_fn g_tracer = NULL;
static void* g_tracer_context = NULL;
static bool g_auto_tick_enabled = true;

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
  if (g_tracer && event) {
    g_tracer(event, g_tracer_context);
  }
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
      status = uedp_task_scheduler();
      if (status == STAT_OK) {
        steps++;
      }
    }
  }

  if (executed_steps) {
    *executed_steps = steps;
  }
  return run_scheduler ? status : STAT_OK;
}