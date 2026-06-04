# Tài liệu đặc tả yêu cầu phần mềm (SRS) cho Kernel QV trong CIEDPC/uEDP

**QV (Quantum Vanilla)** là một kernel hợp tác (cooperative kernel) đơn giản, không ưu tiên chiếm quyền (non-preemptive), được thiết kế để thực thi các Active Objects (AO) theo cơ chế vòng lặp đơn giản.

## 1. Yêu cầu về Lập lịch (Scheduling Requirements)

* **SRS_QP_QV_00: Lập lịch hợp tác (Cooperative Scheduling)**
  * *Mô tả:* Kernel QV phải là một kernel hợp tác, không ưu tiên chiếm quyền (non-preemptive). Điều này có nghĩa là một Active Object đang chạy sẽ không bị ngắt quãng bởi một Active Object khác, ngay cả khi đối tượng đó có mức ưu tiên cao hơn. Việc chuyển đổi ngữ cảnh chỉ xảy ra khi một sự kiện đã được xử lý xong hoàn toàn.
* **SRS_QP_QV_01: Ưu tiên theo mức độ cao nhất (Priority-Based Scheduling)**
  * *Mô tả:* Kernel QV phải luôn chọn để thực thi Active Object có mức ưu tiên cao nhất trong số tất cả các Active Object đang có sự kiện trong hàng đợi (không trống).
* **SRS_QP_QV_02: Ngữ nghĩa Run-to-Completion (RTC)**
  * *Mô tả:* Kernel QV phải tuân thủ nghiêm ngặt ngữ nghĩa RTC. Nó phải cho phép một Active Object xử lý xong một sự kiện duy nhất trước khi đánh giá lại mức ưu tiên để chọn Active Object tiếp theo.

## 2. Yêu cầu về Quản lý Năng lượng (Power Management)

* **SRS_QP_QV_10: Chế độ ngủ khi rảnh (Idle/Sleep Mode)**
  * *Mô tả:* Kernel QV phải cung cấp cơ chế để đưa CPU vào chế độ tiết kiệm năng lượng (sleep mode) khi tất cả các hàng đợi sự kiện của tất cả các Active Object đều trống.
* **SRS_QP_QV_11: Đánh thức từ chế độ ngủ (Wake-up from Sleep)**
  * *Mô tả:* Kernel QV phải có khả năng đánh thức CPU khỏi chế độ ngủ ngay sau khi một sự kiện được gửi (posted) vào hàng đợi của bất kỳ Active Object nào (thường là từ một trình phục vụ ngắt - ISR).

## 3. Yêu cầu về Tính an toàn và Hiệu suất

* **SRS_QP_QV_20: An toàn với ngắt (Interrupt Safety)**
  * *Mô tả:* Kernel QV phải được thiết kế để hoạt động an toàn trong môi trường có các trình phục vụ ngắt (ISR). Các ISR phải có khả năng gửi sự kiện đến các Active Object mà không làm hỏng cấu trúc dữ liệu của kernel.
* **SRS_QP_QV_21: Độ trễ ngắt tối thiểu (Minimal Interrupt Latency)**
  * *Mô tả:* Bản thân kernel QV không được vô hiệu hóa ngắt trong thời gian dài, đảm bảo rằng việc xử lý ngắt trong hệ thống luôn có độ trễ thấp nhất có thể.
* **SRS_QP_QV_22: Tính định thời trong việc chọn tác vụ (Deterministic Task Selection)**
  * *Mô tả:* Thời gian để kernel QV tìm ra Active Object tiếp theo có mức ưu tiên cao nhất để thực thi phải là hằng số hoặc có giới hạn trên xác định (deterministic), thường sử dụng các cơ chế như bảng tra cứu bit (bitmask lookup).

## 4. Yêu cầu về Triển khai và Tương thích

* **SRS_QP_QV_30: Sự đơn giản và Kích thước nhỏ (Small Footprint)**
  * *Mô tả:* Kernel QV phải được tối ưu hóa để có kích thước mã nguồn (code space) và bộ nhớ RAM nhỏ nhất, phù hợp cho các vi điều khiển cấp thấp (low-end microcontrollers).
* **SRS_QP_QV_31: Tương thích giao diện (API Compatibility)**
  * *Mô tả:* Kernel QV phải cung cấp giao diện lập trình ứng dụng (API) thống nhất với các kernel khác trong gia đình QP (như QK hoặc QXK), cho phép người dùng chuyển đổi kernel mà không cần thay đổi mã nguồn của ứng dụng Active Object.
