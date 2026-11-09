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
    #include "ciedpc_core.h"
    #include "ciedpc_task.h"
    #include "ciedpc_tsm.h"
    #include "ciedpc_fsm.h"
    #include "ciedpc_msg.h"
    #include "ciedpc_timer.h"

    /**
     * @brief Khai báo các task ID
     */
    
    #define TASK_NORM_A_ID    (0xE6) // ID của tác vụ điều khiển
    #define TASK_NORM_B_ID    (0xE7) // ID của tác vụ nháy LED

    /**
     * @brief Khai báo các tín hiệu
     */
    #define SIG_USR_START           (0x01u)
    #define SIG_USR_STOP            (0x02u)
    #define SIG_TSK_A_TO_B          (0x03u)
    #define SIG_TSK_B_TO_A          (0x04u)

  #ifdef __cplusplus
  }
  #endif

#endif