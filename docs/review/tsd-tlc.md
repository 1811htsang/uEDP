# TSD & TLC: Khung Kiểm thử Tự động cho $\mu$E-OS (Đề xuất Kiến trúc)

Tài liệu này phác thảo thiết kế cho TSD (Test Scenario Descriptor) và TLC (Test Level Coverager). Hai thành phần này là mảnh ghép cuối cùng của lộ trình v1.2.x, đóng vai trò chuyển đổi quá trình kiểm thử từ thủ công sang tự động (Model-Based Testing).

## 1. TSD (Test Scenario Descriptor) - Đặc tả Kịch bản Kiểm thử

Thay vì là một biến thể của khai báo tác vụ (`tlist`), TSD được thiết kế như một thực thể ngang hàng (Peer Entity). Nó không định nghĩa logic hệ thống, mà đóng vai trò là một "người lái" (Driver) tác động vào các Task/Signal đã được định nghĩa và đánh giá kết quả trả về.

### Cú pháp YAML Đề xuất (Top-level block: `tsd`)

```yaml
tsd:
  - name: "usr_start_to_waiting"
    level: "ut" # Tích hợp trực tiếp với TLC (xem phần dưới)
    
    # 1. Khởi tạo môi trường (Pre-conditions)
    setup:
      gda_overrides: [] # (Tùy chọn) Ghi đè giá trị khởi tạo của GLOBAL_VAR
      
    # 2. Bơm kích thích (Stimulus)
    steps:
      - post_isr: 
          to: TASK_USR
          sig: KID_SIG_USR_START
      - advance_tick: 5 # Đẩy nhanh thời gian mô phỏng (tương đương gọi uedp_timer_tick() 5 lần)
      
    # 3. Đánh giá kết quả (Oracle)
    expect:
      - tsm_state: 
          task: TASK_USR
          state: STATE_USR_WAITING
      - gda_value: 
          name: GLOBAL_COUNTER
          equals: "1"
```

### Lộ trình Tích hợp TSD vào Pipeline (`pycdscriptor`)

Quy trình xử lý TSD sẽ phản chiếu (mirror) hoàn toàn luồng xử lý `C_tnorm_obj` hiện có:

1. Pydantic Models (`pltf/pycdscriptor/lstaxer/pydantic_model/test.py`): Bổ sung các class `C_tsd_step_obj`, `C_tsd_expect_obj`, `C_tsd_scenario_obj`, `C_tsd_list_obj`.
2. Parser Function: Thêm hàm `lukupmodel_tsd_logic(yaml_text)` chạy song song với các hàm `lukupmodel_*` khác.
3. Generator Context (`kre8.py:17-24`): Bổ sung trường `tsd: list[Any]` vào `Kre8Project`. Khi xuất ra context, dữ liệu sẽ được truyền dưới dạng biến `tsd_scenarios`.
4. Jinja2 Template: Tạo một template hoàn toàn mới: `pltf/templates/testc.txt`.
    * Template này không sinh ra ứng dụng, mà sinh ra hàm `main()` chuyên biệt cho kiểm thử.
    * *Luồng chạy của C Code:* Gọi `uedp_core_init()` $\rightarrow$ Lặp qua các `steps` $\rightarrow$ Đánh giá `expect` bằng các lệnh `if/else` đơn giản $\rightarrow$ Trả về `0` (Pass) hoặc khác `0` (Fail). Việc không phụ thuộc vào Framework bên thứ 3 (như Unity/CMock) giúp giữ vững triết lý "siêu nhẹ" của OS.
5. Post-generation (Build Script): Tạo module `testgen.py` (tương đương `modalcvert.py`) được kích hoạt bởi script `jainerator_test.sh`.

### Tận dụng Nền tảng có sẵn (Reusability)

* Linux Backend: Việc bơm tín hiệu trong `post_isr` có thể tái sử dụng trực tiếp hàm `pal_linux_simulate_interrupt()` đã có sẵn trong `linux.c`. Điều này giúp việc chạy TSD trên môi trường mô phỏng (Linux) gần như "miễn phí".
* Escape Hatch (Lối thoát hiểm): Nếu khối `expect` chưa hỗ trợ đủ các điều kiện phức tạp trong phiên bản v0, hãy cho phép dùng `c_stmt` hoặc `c_call` bên trong `expect`. Lập trình viên có thể viết lệnh assert C thô thay vì bị tắc nghẽn chờ DSL hoàn thiện.

---

## TLC (Test Level Coverager) - Phân lớp Kiểm thử

Thay vì tạo ra một ngôn ngữ hay cú pháp riêng biệt cho TLC, chúng ta thiết kế TLC như một Lớp Lọc (Filter Dimension) gắn liền với TSD.

### Cơ chế hoạt động

1. Gắn nhãn (Tagging): Mỗi kịch bản TSD mang một trường `level: ut|ct|st|it`.
2. Phân mảnh sinh mã (Selective Generation): Công cụ `testgen.py` sẽ nhận cờ (flag) `--level`. Ví dụ, nếu cờ là `--level ut`, nó chỉ chọn các kịch bản TSD có `level: ut` để sinh ra file `test_ut.c`. Điều này tránh việc sinh ra một file nhị phân khổng lồ.
3. Tối ưu Biên dịch CMake (The OBJECT advantage):
    * Nhờ việc tái cấu trúc Core thành CMake `OBJECT` library, ta chỉ cần biên dịch Core một lần: `add_library(uedp OBJECT ...)`.
    * Sau đó, cho mỗi mức TLC, tạo một trình thực thi riêng cực rẻ: `add_executable(uedp_test_ut $<TARGET_OBJECTS:uedp> test_ut.c)`.
4. Tích hợp CTest tự nhiên: Map các cấp độ TLC vào thuộc tính `LABELS` của CMake `add_test()`. Lập trình viên chỉ cần gõ `ctest -L ut` để chạy riêng các bài Unit Test mà không cần công cụ runner bên ngoài.

---

## 3. Khuyến nghị Triển khai Phiên bản v0 (Minimal Slice)

Tránh việc thiết kế quá đà (over-engineering) ngay từ đầu. Hãy bắt đầu với một "lát cắt" tối thiểu cho v0:

1. Tính năng TSD v0: Chỉ hỗ trợ `post_isr` và `advance_tick` (cho Steps); chỉ hỗ trợ kiểm tra `tsm_state` và `gda_value` (cho Expect).
2. Tính năng TLC v0: Chỉ hỗ trợ một mức duy nhất là `ut`. Sinh ra một file test duy nhất và gắn cứng vào CMake.
3. Quy ước Đặt tên: Vì `docs/test/README.md` đang quy định tiền tố `vir-/phy-/logic-` cho các file Test Object (Task/Logic), TSD cần một quy ước riêng để tránh nhầm lẫn. Đề xuất: Dùng tiền tố `tsd-<name>-v#.yaml` cho các tệp chứa kịch bản kiểm thử.

Khi "lát cắt mỏng" này có thể thực hiện vòng đời khép kín (Round-trip) và chạy thành công trên một file mẫu như `vir-phy-logic-testobj-v0.yaml`, chúng ta mới tiến hành mở rộng các luật `expect` và tích hợp `CTest` đầy đủ.
