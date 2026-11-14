#ifndef __UEDP_CONTROL_H__
  #define __UEDP_CONTROL_H__

  #include "uedp_core.h"

  typedef enum {
    UEDP_CTRL_BEFORE_SCHEDULER = 0,
    UEDP_CTRL_AFTER_SCHEDULER,
    UEDP_CTRL_BEFORE_TIMER_TICK,
    UEDP_CTRL_AFTER_TIMER_TICK,
    UEDP_CTRL_BEFORE_TASK_DISPATCH,
    UEDP_CTRL_AFTER_TASK_DISPATCH,
    UEDP_CTRL_BEFORE_FSM_TRANSITION,
    UEDP_CTRL_AFTER_FSM_TRANSITION,
    UEDP_CTRL_BEFORE_TSM_TRANSITION,
    UEDP_CTRL_AFTER_TSM_TRANSITION
  } uedp_ctrl_pnt_t;

  typedef struct {
    uedp_ctrl_pnt_t point;
    ui8 task_id;
    ui8 signal;
    ui32 value_a;
    ui32 value_b;
    RETR_STAT status;
    ui32 tick;
    const void* subject;
  } uedp_ctrl_evt_t;

  typedef void (*uedp_ctrl_inject_fn)(uedp_ctrl_pnt_t point, void* context);
  typedef void (*uedp_ctrl_trace_fn)(const uedp_ctrl_evt_t* event, void* context);

  void uedp_ctrl_set_injector(uedp_ctrl_inject_fn callback, void* context);
  void uedp_ctrl_set_tracer(uedp_ctrl_trace_fn callback, void* context);
  void uedp_ctrl_inject(uedp_ctrl_pnt_t point);
  void uedp_ctrl_trace(const uedp_ctrl_evt_t* event);
  void uedp_ctrl_set_auto_tick(bool enabled);
  bool uedp_ctrl_auto_tick_enabled(void);

  RETR_STAT uedp_ctrl_run_scheduler(ui32 max_steps, ui32* executed_steps);
  RETR_STAT uedp_ctrl_advance_ticks(ui32 tick_count, bool run_scheduler, ui32* executed_steps);

#endif