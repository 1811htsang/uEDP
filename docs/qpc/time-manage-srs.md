# Tài liệu đặc tả yêu cầu phần mềm (SRS) cho Quản lý Thời gian trong CIEDPC/uEDP

## 1. Yêu cầu về Thiết lập Sự kiện Thời gian (Arming)

* **SRS_QP_TM_00: Hỗ trợ Sự kiện thời gian một lần (One-shot Time Events)**
  * *Mô tả:* Khung làm việc phải cung cấp cơ chế để một Sự kiện thời gian được thiết lập (armed) để hết hạn sau một khoảng thời gian xác định (số lượng tích tắc - ticks). Sau khi hết hạn, sự kiện sẽ được gửi đến Đối tượng hoạt động (AO) và tự động dừng lại.
* **SRS_QP_TM_01: Hỗ trợ Sự kiện thời gian chu kỳ (Periodic Time Events)**
  * *Mô tả:* Khung làm việc phải cung cấp cơ chế để một Sự kiện thời gian tự động thiết lập lại sau mỗi khoảng chu kỳ nhất định. Sự kiện sẽ được gửi định kỳ đến AO cho đến khi bị dừng lại một cách tường minh.
* **SRS_QP_TM_02: Cho phép thiết lập thông số thời gian khi Arming**
  * *Mô tả:* Khi thiết lập một Sự kiện thời gian, ứng dụng phải có khả năng chỉ định số lượng tích tắc (ticks) cho lần hết hạn đầu tiên và số lượng tích tắc cho các lần lặp lại (nếu là sự kiện chu kỳ).

## 2. Yêu cầu về Hủy bỏ và Thiết lập lại (Disarming & Re-arming)

* **SRS_QP_TM_10: Hỗ trợ dừng Sự kiện thời gian (Disarming)**
  * *Mô tả:* Khung làm việc phải cung cấp phương thức để dừng (disarm) một Sự kiện thời gian đã được thiết lập trước đó nhưng chưa hết hạn. Sau khi dừng, sự kiện đó không được phép gửi đến AO nữa.
* **SRS_QP_TM_11: Hỗ trợ thiết lập lại (Re-arming) hiệu quả**
  * *Mô tả:* Khung làm việc nên cung cấp cơ chế để thiết lập lại một Sự kiện thời gian đang chạy với các thông số thời gian mới mà không cần phải gọi lệnh dừng trước đó một cách thủ công.

## 3. Yêu cầu về Đa cơ sở thời gian (Multiple Time Bases)

* **SRS_QP_TM_20: Hỗ trợ nhiều cơ sở thời gian (Multiple Time Bases)**
  * *Mô tả:* Framework phải hỗ trợ nhiều "Tích tắc hệ thống" (System Ticks) độc lập. Điều này cho phép các Sự kiện thời gian hoạt động trên các thang đo thời gian khác nhau (ví dụ: một cơ sở thời gian 10ms cho các tác vụ nhanh và một cơ sở thời gian 1s cho các tác vụ chậm).
* **SRS_QP_TM_21: Chỉ định Cơ sở thời gian khi khởi tạo**
  * *Mô tả:* Mỗi Sự kiện thời gian phải có khả năng liên kết với một cơ sở thời gian cụ thể trong số các cơ sở thời gian có sẵn của hệ thống.

## 4. Yêu cầu về Xử lý Tích tắc (Tick Processing)

* **SRS_QP_TM_30: Xử lý tích tắc an toàn trong ISR (ISR-Safe Tick Processing)**
  * *Mô tả:* Hàm xử lý tích tắc (Tick Function) của framework phải được thiết kế để có thể gọi an toàn từ bên trong một Trình phục vụ ngắt (ISR) hoặc một luồng hệ thống định kỳ.
* **SRS_QP_TM_31: Hiệu suất xử lý tích tắc**
  * *Mô tả:* Việc xử lý tích tắc phải diễn ra nhanh chóng và mang tính định thời (deterministic). Thời gian xử lý tích tắc không nên phụ thuộc tuyến tính vào tổng số lượng Sự kiện thời gian có trong hệ thống, mà chỉ phụ thuộc vào số lượng sự kiện thực sự hết hạn tại thời điểm đó.

## 5. Yêu cầu về Chuyển phát Sự kiện (Delivery)

* **SRS_QP_TM_40: Liên kết với một Đối tượng hoạt động (Active Object Association)**
  * *Mô tả:* Mỗi Sự kiện thời gian khi được tạo ra phải được liên kết với duy nhất một Đối tượng hoạt động (AO) nhận. Khi sự kiện hết hạn, framework sẽ tự động gửi (post) sự kiện đó vào hàng đợi của AO đã đăng ký.
