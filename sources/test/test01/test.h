#ifndef __TEST_H__
  #define __TEST_H__

  #ifdef __cplusplus
  extern "C"
  {
  #endif

    /**
     * @brief Khai báo thư viện sử dụng
     */
    #include <stdint.h>
    #include <stdbool.h>
    #include "uedp_core.h"
    #include "uedp_task.h"
    #include "uedp_tsm.h"
    #include "uedp_fsm.h"
    #include "uedp_msg.h"
    #include "uedp_timer.h"

    /**
     * @brief Khai báo các task ID
     */
    #define TASK_NORM_CONTROLLER_ID  (0xE6) // ID của tác vụ điều khiển
    #define TASK_NORM_BLINKER_ID     (0xE7) // ID của tác vụ nháy LED

     /**
     * @brief Khai báo các tín hiệu
     */
    #define SIG_USR_START           (0x01u)
    #define SIG_USR_STOP            (0x02u)
    #define SIG_INTERNAL_TICK       (0x03u)

    /**
     * @brief Khai báo trạng thái TSM cho tác vụ nháy LED
     */
    #define STATE_BLINK_IDLE        (0xAF3u)
    #define STATE_BLINK_ACTIVE      (0xAF4u)

  #ifdef __cplusplus
  }
  #endif

#endif