# Tài liệu đặc tả yêu cầu phần mềm (SRS) cho mô hình Máy trạng thái trong CIEDPC/uEDP

## 1. Yêu cầu chung về Máy trạng thái (General Requirements)

* **SRS_QP_SM_00 (Hierarchical State Machines):** Khung làm việc phải hỗ trợ máy trạng thái phân tầng (HSM) cho cả các Đối tượng hoạt động (Active Objects) và các đối tượng có trạng thái thụ động (passive stateful objects).
* **SRS_QP_SM_10 (Implementation Strategies):** Phải hỗ trợ nhiều chiến lược triển khai máy trạng thái khác nhau và có thể hoán đổi cho nhau tại thời điểm thực thi.

## 2. Các chiến lược triển khai (Implementation Strategies)

* **SRS_QP_SM_20 (Manual Coding Optimization):** Cung cấp chiến lược triển khai tối ưu cho việc "lập trình thủ công". Việc thay đổi một yếu tố thiết kế (ví dụ: cấp bậc trạng thái) chỉ yêu cầu thay đổi một thành phần mã nguồn tương ứng.
* **SRS_QP_SM_21 (Automatic Code Generation):** Cung cấp chiến lược tối ưu cho việc "tạo mã tự động". Chiến lược này có thể chứa dữ liệu dư thừa (như bảng chuyển trạng thái - transition tables) để tăng hiệu suất thực thi.
* **SRS_QP_SM_22 (Bidirectional Traceability):** Mọi chiến lược triển khai phải đảm bảo khả năng truy xuất nguồn gốc hai chiều giữa thiết kế (biểu đồ) và mã nguồn (một-đối-một).
* **SRS_QP_SM_23 (Event Immutability):** Đảm bảo sự kiện hiện tại không thay đổi và có thể truy cập được trong suốt toàn bộ bước Run-to-Completion (RTC).
* **SRS_QP_SM_24 (Access to Instance Variables):** Cho phép các máy trạng thái truy cập dễ dàng và hiệu quả vào các biến thực thể (instance variables) của đối tượng sở hữu chúng.
* **SRS_QP_SM_25 (Is-In State):** Cung cấp phương thức kiểm tra xem máy trạng thái có đang ở một trạng thái cụ thể hoặc trạng thái con của nó hay không.
* **SRS_QP_SM_26 (Current State):** Cung cấp phương thức để lấy định danh của trạng thái hiện tại (phục vụ mục đích gỡ lỗi).

## 3. Các tính năng của Máy trạng thái phân tầng (HSM Features)

* **SRS_QP_SM_31 (Nested Substates):** Hỗ trợ các trạng thái lồng nhau (trạng thái phức hợp và trạng thái lá).
* **SRS_QP_SM_32 (Entry Actions):** Hỗ trợ các hành động "Entry" (thực hiện khi đi vào một trạng thái). Các hành động của trạng thái cha phải được thực hiện trước trạng thái con.
* **SRS_QP_SM_33 (Exit Actions):** Hỗ trợ các hành động "Exit" (thực hiện khi thoát khỏi một trạng thái). Các hành động của trạng thái con phải được thực hiện trước trạng thái cha.
* **SRS_QP_SM_34 (Nested Initial Transitions):** Hỗ trợ các chuyển đổi khởi tạo lồng nhau bên trong các trạng thái phức hợp.
* **SRS_QP_SM_35 (Transitions):** Hỗ trợ chuyển đổi giữa các trạng thái ở bất kỳ cấp độ lồng nhau nào, bao gồm cả chuyển đổi cục bộ (Local Transitions) và tự chuyển đổi (Self-Transitions).
* **SRS_QP_SM_36 (Internal Transitions):** Hỗ trợ chuyển đổi nội bộ (Internal Transitions) - thực hiện hành động nhưng không gây ra thay đổi trạng thái hoặc thực hiện entry/exit.
* **SRS_QP_SM_37 (Guards):** Hỗ trợ các điều kiện bảo vệ (Guard Conditions) cho các chuyển đổi. Nếu Guard sai, sự kiện sẽ được chuyển tiếp lên trạng thái cha.
* **SRS_QP_SM_38 (Top-most Initial Transition):** Hỗ trợ chuyển đổi khởi tạo cao nhất, được kích hoạt độc lập với việc khởi tạo đối tượng.
* **SRS_QP_SM_39 (History):** Hỗ trợ chuyển đổi về trạng thái lịch sử (History Transitions), bao gồm cả lịch sử nông (shallow) và lịch sử sâu (deep).

## 4. Thành phần cấu trúc

* **SRS_QP_SM_40 (Top State):** Có thể cung cấp một "trạng thái gốc" (Top-state) là gốc của toàn bộ hệ thống phân tầng trạng thái, mặc định sẽ bỏ qua các sự kiện không được xử lý.
