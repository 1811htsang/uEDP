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
    #include "uedp_msg.h"
    #include "uedp_timer.h"
    #include "uedp_itnlog.h"
    #include "uedp_fcr.h"

    /**
     * @brief Khai báo task ID dùng riêng cho test04
     */
    #define TASK_NORM_FCR_ID   (0xE6u)

    /**
     * @brief Khai báo signal dùng để kích hoạt việc chạy toàn bộ case kiểm thử FCR
     *        bên trong ngữ cảnh của TASK_NORM_FCR_ID (để g_current_msg khác NULL
     *        khi uedp_fcr_raise() gọi vào uedp_itnlog_log()).
     */
    #define SIG_TEST_RUN       (0x01u)

    /**
     * @brief Chạy toàn bộ các case kiểm thử cho FCR
     * @return int 0 nếu tất cả case PASS, khác 0 nếu có case FAIL
     */
    int test_fcr_run_all(void);

  #ifdef __cplusplus
  }
  #endif

#endif // __TEST_H__
