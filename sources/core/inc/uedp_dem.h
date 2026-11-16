/**
 * @file uedp_dem.h
 * @brief Diagnostic Event Management (DEM): ECS/DBS tĩnh và cờ điều kiện OPC/APC/IOC
 * @version 0.1
 * @copyright MIT License
 */
#ifndef __UEDP_DEM_H__
  #define __UEDP_DEM_H__

  #include <stdbool.h>
  #include "uedp_fcr.h"

  #ifndef UEDP_DEM_MAX_EVENTS
    #define UEDP_DEM_MAX_EVENTS (8u)
  #endif

  /** ANCHOR - Loại cờ điều kiện chặn
   * @param UEDP_COND_OPC Operation Preventing Condition: chặn thực hiện một operation
   * @param UEDP_COND_APC Activation Preventing Condition: chặn kích hoạt một chức năng/service
   * @param UEDP_COND_IOC Inhibition Preventing Condition: ức chế một đường tín hiệu/đầu ra
   */
  typedef enum uedp_cond_kind_t {
    UEDP_COND_OPC = 0,
    UEDP_COND_APC,
    UEDP_COND_IOC,
    UEDP_COND_KIND_COUNT
  } uedp_cond_kind_t;

  /** ANCHOR - Cấu hình tĩnh của một event DEM
   * @param code Mã FCR đóng vai trò khoá định danh event
   * @param enabled ECS: false thì báo cáo bị bỏ qua, FCR action không được thực hiện
   * @param debounce_cnt DBS: số lần báo cáo liên tiếp để event được confirmed (0 hoặc 1: confirm ngay)
   * @param cond_kind Loại cờ điều kiện được set khi event confirmed
   * @param cond_bit Bit (0..31) của cờ trong loại cond_kind
   */
  typedef struct uedp_dem_cfg_t {
    uedp_fcr_code_t   code;
    bool              enabled;
    ui8               debounce_cnt;
    uedp_cond_kind_t  cond_kind;
    ui8               cond_bit;
  } uedp_dem_cfg_t;

  /** ANCHOR - Trạng thái của event sau DBS */
  typedef enum uedp_dem_status_t {
    UEDP_DEM_STATUS_IDLE = 0,
    UEDP_DEM_STATUS_DEBOUNCING,
    UEDP_DEM_STATUS_CONFIRMED
  } uedp_dem_status_t;

  /** ANCHOR - Nạp bảng cấu hình DEM tĩnh, gọi một lần khi khởi tạo hệ thống
   * @param table Bảng cấu hình, phải tồn tại suốt vòng đời chương trình
   * @param count Số entry trong bảng, không vượt quá UEDP_DEM_MAX_EVENTS
   */
  void uedp_dem_set_table(const uedp_dem_cfg_t* table, ui16 count);

  /** ANCHOR - Báo cáo một lần lỗi cho event DEM tương ứng với mã FCR
   * @param code Mã FCR vừa được raise
   * @return true nếu FCR action được phép thực hiện (mã không có trong bảng, hoặc event vừa confirmed),
   *         false nếu event bị ECS chặn hoặc đang debounce
   */
  bool uedp_dem_report(uedp_fcr_code_t code);

  /** ANCHOR - Đưa event về IDLE, xoá bộ đếm DBS và gỡ cờ điều kiện do event đó set
   * @param code Mã FCR của event cần heal
   */
  void uedp_dem_heal(uedp_fcr_code_t code);

  /** ANCHOR - Lấy trạng thái hiện tại của event DEM
   * @param code Mã FCR của event
   * @return Trạng thái event, IDLE nếu mã không có trong bảng
   */
  uedp_dem_status_t uedp_dem_get_status(uedp_fcr_code_t code);

  /** ANCHOR - Kiểm tra cờ điều kiện chặn (OPC/APC/IOC) có đang được set hay không
   * @param kind Loại cờ cần kiểm tra
   * @param bit Bit cần kiểm tra (0..31)
   * @return true nếu có event confirmed đang set cờ này
   */
  bool uedp_cond_is_set(uedp_cond_kind_t kind, ui8 bit);

#endif // __UEDP_DEM_H__
