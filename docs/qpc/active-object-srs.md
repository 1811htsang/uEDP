# Tài liệu đặc tả yêu cầu phần mềm (SRS) cho mô hình Active Object trong CIEDPC/uEDP

## 1. Các yêu cầu về mô hình Active Object (Core Concepts)

* **SRS_QP_AO_01 (Encapsulation):** Mỗi Active Object phải đóng gói các dữ liệu trạng thái (state variables) và các hàm xử lý sự kiện. Dữ liệu bên trong AO không được để các thực thể khác truy cập trực tiếp.
* **SRS_QP_AO_02 (Thread of Control):** Mỗi Active Object phải sở hữu một luồng thực thi (thread of control) riêng biệt.
* **SRS_QP_AO_03 (Event Queue):** Mỗi Active Object phải có một hàng đợi sự kiện (event queue) riêng để nhận các sự kiện đến một cách bất đồng bộ.
* **SRS_QP_AO_04 (Run-to-Completion - RTC):** Việc xử lý mỗi sự kiện bởi Active Object phải tuân thủ ngữ nghĩa Run-to-Completion. Nghĩa là AO phải xử lý xong hoàn toàn một sự kiện trước khi bắt đầu xử lý sự kiện tiếp theo từ hàng đợi.

## 2. Các yêu cầu về Quản lý Active Object

* **SRS_QP_AO_20 (Initialization):** Khung làm việc (framework) phải cung cấp cơ chế để khởi tạo một Active Object, bao gồm việc thực hiện hành động khởi tạo trạng thái ban đầu.
* **SRS_QP_AO_21 (Execution/Start):** Khung làm việc phải cung cấp phương thức để bắt đầu thực thi một Active Object, bao gồm việc gán mức ưu tiên (priority), hàng đợi sự kiện và bộ nhớ stack (nếu cần).
* **SRS_QP_AO_22 (Priority):** Mỗi Active Object phải được gán một mức ưu tiên duy nhất trong hệ thống.
* **SRS_QP_AO_23 (Termination):** Khung làm việc nên cung cấp cơ chế để dừng (terminate) việc thực thi của một Active Object một cách an toàn (tùy chọn tùy vào kernel).

## 3. Các yêu cầu về Chuyển phát Sự kiện (Event Delivery)

* **SRS_QP_AO_30 (Direct Post):** Hệ thống phải hỗ trợ gửi sự kiện trực tiếp (Direct Event Posting) từ một thực thể (AO hoặc ISR) tới một Active Object cụ thể.
* **SRS_QP_AO_31 (Publish-Subscribe):** Hệ thống phải hỗ trợ mô hình Xuất bản-Đăng ký (Publish-Subscribe), cho phép gửi sự kiện tới tất cả các Active Object đã đăng ký nhận loại sự kiện đó mà không cần biết danh tính của chúng.
* **SRS_QP_AO_32 (FIFO/LIFO):** Mặc định sự kiện phải được đưa vào hàng đợi theo cơ chế FIFO (vào trước ra trước), nhưng khung làm việc cũng có thể hỗ trợ LIFO (vào sau ra trước) cho các trường hợp đặc biệt (như xử lý lỗi).

## 4. Các yêu cầu về Quản lý Bộ nhớ Sự kiện

* **SRS_QP_AO_40 (Event Allocation):** Hệ thống phải cung cấp cơ chế cấp phát sự kiện động (dynamic event allocation) từ các vùng nhớ chung (event pools).
* **SRS_QP_AO_41 (Event Recycling):** Khung làm việc phải tự động giải phóng (recycle) các sự kiện động sau khi chúng đã được xử lý bởi tất cả các Active Object nhận được chúng, để tránh rò rỉ bộ nhớ.
* **SRS_QP_AO_42 (Immutability):** Các sự kiện sau khi được tạo và gửi đi không được phép thay đổi nội dung (read-only) để đảm bảo an toàn luồng (thread-safety) mà không cần dùng mutex.

## 5. Các yêu cầu về Thời gian (Time Management)

* **SRS_QP_AO_50 (Time Events):** Khung làm việc phải cung cấp cơ chế "Sự kiện thời gian" (Time Events) để gửi một sự kiện tới Active Object sau một khoảng thời gian nhất định hoặc theo chu kỳ định sẵn.
