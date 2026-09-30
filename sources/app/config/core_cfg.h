//ANCHOR - Core configuration header
#ifndef __CORE_CFG_H__
  #define __CORE_CFG_H__

  /** ANCHOR - Khai báo các cấu hình core
   * @example
   *    #define UEDP_MSG_BLANK_QUEUE_SIZE  (16u) 	// units
   *    #define UEDP_MSG_ALLOC_DATA_MAX   (sizeof(void*) * 8u) // auto arrange depended on architecture
   * @attention Xin đừng sửa đổi, tự động sinh bởi Kconfiglib và jinja2
   */

  #define UEDP_MSG_BLANK_QUEUE_SIZE      (8u)
  #define UEDP_MSG_ISR_QUEUE_SIZE        (16u)
  #define UEDP_MSG_ALLOC_QUEUE_SIZE      (8u)
  #define UEDP_MSG_ALLOC_N_VALUE         (2u)
  #define UEDP_MSG_EXTAL_QUEUE_SIZE      (8u)
  #define UEDP_MSG_EXTAL_N_VALUE         (2u)
  #define UEDP_TASK_MSG_QUEUE_SIZE       (8u)
  #define UEDP_TIMER_MAX_NODES           (8u)
  #define UEDP_ITNLOG_MAX_LOG_ENTRIES    (32u)
  #define UEDP_ITNLOG_FLUSH_THRESHOLD    (28u)
  
#endif //__CORE_CFG_H__