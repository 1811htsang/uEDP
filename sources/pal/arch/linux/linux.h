//ANCHOR - Architecture-specific header for Linux
#ifndef __LINUX_H__
  #define __LINUX_H__

  //ANCHOR - Khai báo thư viện sử dụng

  #include "pal_core.h"

  /** ANCHOR - Khai báo custom API
   * @attention Xin đừng sửa đổi, tự động sinh bởi Kconfiglib và Jinja2
   */

  void linux_init_env(void);

  void linux_simulate_interrupt(ui8 task_id, ui8 signal);

  void linux_simulate_tick(void);

  void linux_cleanup(void);

  void linux_signal_handler(int signum);

  void linux_get_sac_count(uint32_t count);

#endif