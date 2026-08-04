/**
 * @file core_cfg.h
 * @author Shang Huang
 * @brief Core configuration header file
 * @version 0.1
 * @date 2026-07-18
 * @copyright Copyright (c) 2026
 */
#ifndef __CORE_CFG_H__
	#define __CORE_CFG_H__

	/**
	 * @brief Khai báo các cấu hình core
   * @example
   * #define UEDP_MSG_BLANK_QUEUE_SIZE  (16u) 	// units
   * #define UEDP_MSG_ALLOC_DATA_MAX   (sizeof(void*) * 8u) // auto arrange depended on architecture
   * @attention Xin đừng sửa đổi, tự động sinh bởi Kconfiglib
	 */
	// KCONFIG_CORECFG_START
  // KCONFIG_CORECFG_END

#endif //__CORE_CFG_H__
