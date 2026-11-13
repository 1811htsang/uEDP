#ifndef __LIBCRC8_H__
  #define __LIBCRC8_H__

  // DEPRECATED - Old TASK - Bổ sung comment giới thiệu cho từng hàm

	/** ANCHOR - Hàm tính toán CRC-8 cho một mảng byte
   * @param msg: Con trỏ đến mảng byte cần tính toán CRC-8
	 * @param msg_len: Độ dài của mảng byte
	 * @param init: Giá trị khởi tạo cho CRC-8
	 */
  uint8_t crc8(uint8_t *msg, int msg_len, uint8_t init);
  
  /** ANCHOR - Hàm xây dựng bảng CRC-8 dựa trên đa thức cho trước
   * @param poly: Đa thức CRC-8 được sử dụng để xây dựng bảng
	 */
  void libcrc8_build_table(uint8_t poly);

  /** ANCHOR - Hàm lấy giá trị đa thức hiện tại đang được sử dụng trong tính toán CRC-8
   * @return Giá trị đa thức CRC-8 hiện tại
   */
  uint8_t libcrc8_get_polyfactor(void);
  
  /** ANCHOR - Hàm xuất bảng CRC-8 hiện tại ra màn hình console
   * @attention Hàm này chủ yếu dùng để kiểm tra và gỡ lỗi, không nên sử dụng trong môi trường sản xuất
   */
  void libcrc8_dump_table(void);
  
#endif // __LIBCRC8_H__