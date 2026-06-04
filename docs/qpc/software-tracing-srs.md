# Tài liệu đặc tả yêu cầu phần mềm (SRS) cho Tracing trong CIEDPC/uEDP

## 1. Yêu cầu chung về Tracing (General Requirements)

* **SRS_QP_QS_00: Cung cấp cơ chế quan sát phần mềm (Software Instrumentation)**
  * *Mô tả:* Khung làm việc phải cung cấp một cơ chế quan sát phần mềm tích hợp sẵn để theo dõi hành vi của hệ thống (cả framework và ứng dụng) mà không làm thay đổi đáng kể đặc tính thời gian thực của hệ thống đó.

## 2. Sự ảnh hưởng và Hiệu năng (Overhead & Footprint)

* **SRS_QP_QS_10: Khả năng cấu hình tại thời điểm biên dịch**
  * *Mô tả:* Cơ chế tracing phải có khả năng được bật hoặc tắt hoàn toàn tại thời điểm biên dịch.
* **SRS_QP_QS_11: Ảnh hưởng tối thiểu khi tắt (Zero Footprint)**
  * *Mô tả:* Khi tính năng tracing bị tắt, nó không được tiêu thụ bộ nhớ (ROM/RAM) hoặc chu kỳ CPU (ngoại trừ các macro rỗng).
* **SRS_QP_QS_12: Ảnh hưởng tối thiểu khi bật (Minimal Runtime Overhead)**
  * *Mô tả:* Khi được bật, việc ghi lại một dữ liệu vết (trace record) phải cực kỳ nhanh chóng để giảm thiểu hiện tượng "Probe Effect" (thay đổi hành vi hệ thống do việc đo lường).

## 3. Lọc dữ liệu Vết (Trace Filtering)

* **SRS_QP_QS_20: Khả năng lọc tại nguồn (Source-level Filtering)**
  * *Mô tả:* Hệ thống phải cho phép chọn lọc các loại dữ liệu vết nào được phép gửi đi để tiết kiệm băng thông và giảm tải cho CPU.
* **SRS_QP_QS_21: Lọc toàn cục (Global Filtering)**
  * *Mô tả:* Hỗ trợ lọc dựa trên loại bản ghi (Record ID). Ví dụ: Chỉ cho phép ghi lại các sự kiện máy trạng thái mà bỏ qua các sự kiện hàng đợi.
* **SRS_QP_QS_22: Lọc cục bộ (Local/Object-level Filtering)**
  * *Mô tả:* Hỗ trợ lọc dựa trên từng đối tượng cụ thể. Ví dụ: Chỉ theo dõi hành vi của một Active Object cụ thể mà bỏ qua các đối tượng khác.

## 4. Thu thập và Định dạng Dữ liệu (Data Collection & Formatting)

* **SRS_QP_QS_30: Sử dụng bộ đệm vòng (Circular Buffer)**
  * *Mô tả:* Dữ liệu vết phải được lưu trữ tạm thời trong một bộ đệm vòng an toàn để xử lý việc chênh lệch tốc độ giữa việc tạo dữ liệu và việc truyền dữ liệu.
* **SRS_QP_QS_31: Định dạng dữ liệu bên ngoài mục tiêu (Offline Formatting)**
  * *Mô tả:* Hệ thống mục tiêu (Target) chỉ gửi dữ liệu thô (binary). Việc chuyển đổi dữ liệu thành văn bản (human-readable) phải được thực hiện ở máy chủ (Host) để giảm tải cho vi xử lý nhúng.
* **SRS_QP_QS_32: Giao thức truyền tin hiệu quả**
  * *Mô tả:* Dữ liệu phải được đóng gói theo một giao thức nhẹ (như giao thức QS của QP) để đảm bảo tính toàn vẹn và dễ dàng tách khung dữ liệu (framing).

## 5. Nội dung bản ghi Vết (Trace Records)

* **SRS_QP_QS_40: Hỗ trợ các bản ghi hệ thống và ứng dụng**
  * *Mô tả:* Framework phải cung cấp các bản ghi có sẵn cho các sự kiện nội bộ (chuyển trạng thái, gửi sự kiện, v.v.) và cho phép người dùng định nghĩa các bản ghi riêng cho ứng dụng.
* **SRS_QP_QS_41: Dấu thời gian (Timestamps)**
  * *Mô tả:* Mỗi bản ghi vết phải có khả năng đính kèm một dấu thời gian có độ phân giải cao để phục vụ việc phân tích trình tự thời gian.
* **SRS_QP_QS_42: Cơ chế từ điển (Dictionary Records)**
  * *Mô tả:* Để tối ưu băng thông, hệ thống phải hỗ trợ "bản ghi từ điển" để ánh xạ các địa chỉ bộ nhớ hoặc hằng số thành tên (chuỗi văn bản) trên máy chủ.

## 6. Tương tác hai chiều (Bi-directional Interaction)

* **SRS_QP_QS_50: Hỗ trợ nhận lệnh từ Host (Target Input)**
  * *Mô tả:* Cơ chế QS không chỉ là một chiều (gửi đi) mà còn phải hỗ trợ nhận các lệnh hoặc dữ liệu từ máy chủ để tác động ngược lại vào hệ thống đang chạy (phục vụ Unit Testing hoặc cấu hình trực tiếp).
