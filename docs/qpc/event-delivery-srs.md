# Tài liệu đặc tả yêu cầu phần mềm (SRS) cho Cơ chế Chuyển phát Sự kiện trong CIEDPC/uEDPz

## 1. Cơ chế Gửi sự kiện trực tiếp (Direct Event Posting)

* **SRS_QP_EDM_00: Hỗ trợ Gửi sự kiện trực tiếp (Direct Posting)**
  * *Mô tả:* Khung làm việc (framework) phải cung cấp cơ chế để gửi một sự kiện trực tiếp đến một Đối tượng hoạt động (Active Object - AO) cụ thể.
* **SRS_QP_EDM_01: Chuyển phát bất đồng bộ (Asynchronous Delivery)**
  * *Mô tả:* Việc gửi sự kiện trực tiếp phải mang tính bất đồng bộ. Điều này có nghĩa là sự kiện được đưa vào hàng đợi của AO nhận và hàm gửi sẽ trả về ngay lập tức mà không cần đợi AO đó xử lý xong sự kiện.
* **SRS_QP_EDM_02: An toàn luồng và hỗ trợ ISR (Thread-Safe & ISR-Callable)**
  * *Mô tả:* Cơ chế gửi sự kiện trực tiếp phải đảm bảo an toàn luồng (thread-safe) và có thể được gọi từ cả các luồng khác (AO khác) hoặc từ các Trình phục vụ ngắt (ISR).
* **SRS_QP_EDM_03: Hỗ trợ thứ tự FIFO và LIFO**
  * *Mô tả:* Framework phải cho phép gửi sự kiện vào cuối hàng đợi (FIFO - mặc định) hoặc đưa lên đầu hàng đợi (LIFO) để xử lý khẩn cấp.

## 2. Cơ chế Xuất bản - Đăng ký (Publish-Subscribe)

* **SRS_QP_EDM_10: Hỗ trợ mô hình Xuất bản - Đăng ký (Publish-Subscribe)**
  * *Mô tả:* Framework phải cung cấp cơ chế chuyển phát sự kiện kiểu "nhiều-đến-nhiều", trong đó bên gửi (publisher) không cần biết danh tính của bên nhận (subscribers).
* **SRS_QP_EDM_11: Đăng ký dựa trên Tín hiệu (Signal-Based Subscriptions)**
  * *Mô tả:* Các Đối tượng hoạt động (AO) phải có khả năng đăng ký nhận các sự kiện dựa trên mã tín hiệu (Signal) của sự kiện đó.
* **SRS_QP_EDM_12: Chuyển phát đa hướng tự động (Automatic Multicasting)**
  * *Mô tả:* Khi một sự kiện được "xuất bản" (published), framework phải tự động chuyển phát sự kiện đó tới tất cả các AO đã đăng ký nhận tín hiệu tương ứng.
* **SRS_QP_EDM_13: Hiệu quả thực thi (Runtime Efficiency)**
  * *Mô tả:* Cơ chế Publish-Subscribe phải được tối ưu hóa để việc chuyển phát sự kiện diễn ra nhanh chóng, lý tưởng nhất là với độ phức tạp thời gian không phụ thuộc quá lớn vào số lượng AO trong hệ thống.
* **SRS_QP_EDM_14: Xử lý khi không có người đăng ký (Zero Subscriptions)**
  * *Mô tả:* Việc xuất bản một tín hiệu mà không có bất kỳ AO nào đăng ký nhận phải được coi là hợp lệ và framework phải xử lý an toàn (thường là tự động giải phóng sự kiện đó).

## 3. Yêu cầu về độ tin cậy và quản lý

* **SRS_QP_EDM_20: Ngăn chặn tràn hàng đợi (Queue Overflow Protection)**
  * *Mô tả:* Framework phải cung cấp cơ chế để phát hiện hoặc xử lý tình huống hàng đợi sự kiện của một AO bị đầy khi có sự kiện mới gửi đến.
* **SRS_QP_EDM_30: Khả năng quan sát (Observability/Traceability)**
  * *Mô tả:* Các cơ chế chuyển phát sự kiện phải hỗ trợ việc tích hợp các công cụ phần mềm trung gian (như QSPY) để theo dõi quá trình gửi/nhận sự kiện giữa các thành phần.
