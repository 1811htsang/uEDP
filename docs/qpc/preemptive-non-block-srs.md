# Tài liệu đặc tả yêu cầu phần mềm (SRS) cho Lập lịch Chiếm quyền và Không chặn trong CIEDPC/uEDP

## 1. Yêu cầu về Lập lịch và Chiếm quyền (Scheduling & Preemption)

* **SRS_QP_QK_00: Lập lịch dựa trên mức ưu tiên (Priority-Based Scheduling)**
  * *Mô tả:* Kernel QK phải luôn thực thi Active Object (AO) có mức ưu tiên cao nhất đang sẵn sàng (có sự kiện trong hàng đợi).
* **SRS_QP_QK_01: Chiếm quyền hoàn toàn (Fully Preemptive)**
  * *Mô tả:* QK phải cho phép một AO có mức ưu tiên cao hơn chiếm quyền thực thi của một AO có mức ưu tiên thấp hơn ngay lập tức khi có sự kiện gửi đến AO cao hơn đó.
* **SRS_QP_QK_02: Ngữ nghĩa Run-to-Completion (RTC)**
  * *Mô tả:* QK phải đảm bảo rằng mặc dù có sự chiếm quyền, mỗi sự kiện vẫn được xử lý theo nguyên tắc RTC. Một AO bị chiếm quyền sẽ tiếp tục xử lý sự kiện đang dang dở ngay tại điểm bị ngắt sau khi AO ưu tiên cao hơn đã hoàn thành công việc của nó.
* **SRS_QP_QK_03: Không chặn (Non-Blocking)**
  * *Mô tả:* QK không được phép hỗ trợ các cơ chế chặn (blocking) như trì hoãn luồng (với hàm delay) hoặc chờ đợi các tài nguyên (như semaphore). Các AO phải xử lý sự kiện và trả về quyền điều khiển cho kernel.

## 2. Yêu cầu về Quản lý Ngăn xếp (Stack Management)

* **SRS_QP_QK_10: Cấu trúc Ngăn xếp đơn (Single-Stack Architecture)**
  * *Mô tả:* Tất cả các Active Objects và các Trình phục vụ ngắt (ISR) trong hệ thống phải chạy trên cùng một ngăn xếp (stack) duy nhất.
* **SRS_QP_QK_11: Xử lý lồng nhau (Nested Execution)**
  * *Mô tả:* QK phải quản lý việc chiếm quyền thông qua cơ chế gọi hàm lồng nhau trên ngăn xếp. Khi một AO ưu tiên cao hơn chiếm quyền, khung ngữ cảnh của AO cũ được đẩy vào ngăn xếp và AO mới bắt đầu thực thi như một "lớp" bên trên.

## 3. Yêu cầu về Ngắt (Interrupt Handling)

* **SRS_QP_QK_20: Ngắt chiếm quyền (Preemptive Interrupts)**
  * *Mô tả:* Các ISR phải có khả năng chiếm quyền của bất kỳ AO nào đang chạy.
* **SRS_QP_QK_21: Ngắt lồng nhau (Nested Interrupts)**
  * *Mô tả:* QK phải hỗ trợ cơ chế ngắt lồng nhau nếu phần cứng vi điều khiển cho phép.
* **SRS_QP_QK_22: Chuyển đổi ngữ cảnh sau ngắt (Context Switch on Exit)**
  * *Mô tả:* Khi thoát khỏi một ISR, nếu có một AO ưu tiên cao hơn đã sẵn sàng (do ISR vừa gửi sự kiện đến), kernel QK phải thực hiện chuyển đổi ngữ cảnh để chạy AO đó ngay lập tức thay vì quay lại AO bị ngắt ban đầu.

## 4. Yêu cầu về Hiệu suất và Định thời (Efficiency & Determinism)

* **SRS_QP_QK_30: Độ trễ tối thiểu (Minimal Overhead)**
  * *Mô tả:* Việc lập lịch và chuyển đổi ngữ cảnh trong QK phải được tối ưu hóa để tiêu tốn ít chu kỳ CPU nhất có thể (vì thực tế nó chỉ là các lần đẩy/rút ngăn xếp thông thường).
* **SRS_QP_QK_31: Tính định thời (Determinism)**
  * *Mô tả:* Thời gian cần thiết để xác định AO ưu tiên cao nhất tiếp theo phải là hằng số (O(1)) hoặc có giới hạn trên xác định, không phụ thuộc vào số lượng AO trong hệ thống.

## 5. Yêu cầu về Quản lý Năng lượng

* **SRS_QP_QK_40: Vòng lặp rảnh và Chế độ ngủ (Idle Loop & Low-Power)**
  * *Mô tả:* QK phải cung cấp một "vòng lặp rảnh" (Idle loop) được thực thi khi không có AO nào sẵn sàng. Vòng lặp này phải cho phép người dùng đặt CPU vào chế độ tiết kiệm năng lượng (Low-power mode).
