# Tài liệu ghi chú tổng hợp libunity vào PLTF

Tài liệu này tổng hợp các ghi chú và hướng dẫn liên quan đến việc tích hợp `libunity` vào hệ thống PLTF.

## Nhắc lại về dự trù kiến trúc hiện tại

Hiện tại PLTF đang được thiết kế với các `testobj` đóng vai trò là object khởi tạo toàn bộ scenario và tích hợp kiểm tra sử dụng `c_stmt` để thực hiện các lệnh kiểm tra.

Tuy nhiên, với nhu cầu cần thiết của việc phải có một framewok unit test riêng biệt. Unity (định danh `libunity`) được lựa chọn để thực hiện các unit test. Unity là một framework unit test phổ biến trong cộng đồng lập trình C, cung cấp các tính năng cơ bản để viết và chạy các unit test.

Với nhu cầu tích hợp này kèm các specification mới về testing tại [test-spec](/docs/test/test-spec.md), một vấn đề mới phát sinh khi `tezcal` và `testobj` xung đột với nhau về kiến trúc và cần phải được giải quyết để đảm bảo tính nhất quán và hiệu quả trong quá trình phát triển và kiểm thử.

## Vấn đề xung đột kiến trúc

`testobj` ra đời ở giai đoạn trước khi có `tezcal`. Lúc đó chúng ta chưa có bộ test runner chuyên biệt, nên bạn buộc phải dùng `c_stmt` để nhét cả code logic lẫn code tự kiểm tra như `printf("[A] DATA MATCHED")` vào chung một file. Bản chất `testobj` lúc đó là Self-Testing Hybrid (Mô hình tự kiểm tra).

Còn `tezcal` ra đời sau với tư cách là một Test Driver (Người lái kịch bản) chuyên nghiệp theo chuẩn phân cấp (`tezuit` > `tezk` > `teztep`).

Nếu để cả hai cùng làm nhiệm vụ kiểm tra, pipeline sẽ bị loạn do không biết ai là người quyết định kết quả test (Pass/Fail) và mã nguồn C sinh ra ở `sources/test/tezprjec/tez.c` sẽ bị chồng chéo.

## Phân định vai trò

Trong kỹ nghệ kiểm thử hệ thống nhúng, hai thực thể này bắt buộc phải tách rời:

- `testobj` là môi trường test, nó chỉ đại diện cho ưng dụng và logic nghiệp vụ thuần túy (Task, máy trạng thái TSM/FSM, biến GDA, routing PSE). Nó không được chứa các câu lệnh kiểm tra, không assert, không tự phán xét mình đúng hay sai.
- `tezcal` là kịch bản kiểm thử (Test Scenario / Driver), đóng vai trò kích thích (Stimulus) và giám sát (Oracle). Nó bơm tín hiệu vào `testobj`, ra lệnh nhịp thời gian chạy (`advance_tick`), và kiểm tra trạng thái của `testobj` qua các lệnh Assert của `libunity`.

## Chuyển giao quyền lực

Để giải quyết triệt để sự trùng lặp, bạn nên thực hiện 3 bước tái cấu trúc sau:

### Rút toàn bộ code kiểm tra ra khỏi `testobj`

File `testobj` (ví dụ `testobj-v0.yaml`) chỉ giữ lại logic vận hành của máy trạng thái. Xóa bỏ các đoạn `c_stmt` mang tính chất "tự kiểm tra dữ liệu".

Ví dụ trước đây (Trong `testobj`):

```yaml
# CŨ: testobj tự kiểm tra chính nó
- actv:
    kind: c_stmt
    code: |
      if (data && strcmp(data, GDA_SYSTEM_STATUS) == 0) {
        printf("[A] DATA MATCHED\n");
      } else {
        printf("[A] MISMATCHED\n");
      }
```

Thì bây giờ (Trong `testobj`):, Chỉ làm đúng phận sự của một Task bình thường trong hệ điều hành: nhận tin nhắn, xử lý dữ liệu (nếu có), cập nhật trạng thái/GDA, gửi tiếp tin nhắn.

```yaml
# MỚI: testobj thuần logic
- actv:
    kind: c_stmt
    code: |
      // Chỉ nhận dữ liệu và copy vào buffer nội bộ
      char* data = (char*)(msg->data);
      strncpy(local_buf, data, sizeof(local_buf));
```

### Chuyển toàn bộ vai trò kích thích & kiểm tra vào `tezcal`

Mọi hành động: "Bơm tín hiệu bắt đầu", "Bơm tín hiệu khẩn cấp", "Kiểm tra xem Task A có chuyển sang state WAITING không", "Kiểm tra biến GDA có bằng 1 không" sẽ do `tezcal` đảm nhận thông qua `teztep`.

```yaml
# Trong file tezcal-pse_test.yaml
- tezk: *tezk_1
  subidentif: "TC_NEG_DATA_PASSING"
  dscript: "Test case kiểm tra truyền dữ liệu"
  teztep:
    - '1':
      desc: "Bơm tín hiệu 0x12 từ ngoài vào"
      call:
        - act: |
            // Bơm tín hiệu 0x12 vào Task A
            uint8_t signal = 0x12;
            uedp_send_signal(TASK_NORM_A, &signal, sizeof(signal));
    - '2':
      desc: "Cho scheduler chạy 5 tick và kiểm tra kết quả"
      call:
        - act: |
            // Cho scheduler chạy 5 tick
            for (int i = 0; i < 5; i++) {
                uedp_scheduler_tick();
            }
  xpect:
    # tezcal kiểm tra chứ testobj không tự kiểm tra
    - act: "TEST_ASSERT_EQUAL_STRING(GDA_SYSTEM_STATUS, local_buf);"
    - act: "TEST_ASSERT_EQUAL_INT(tsm_task_a_state_waiting, uedp_tsm_get_state(TASK_NORM_A));"
```

### Quy hoạch lại Pipeline sinh mã `sources/test/tezprjec/tez.c`

Bộ Generator (`pycdscriptor`) sẽ phân tách việc sinh mã như sau:

- Từ `testobj` sẽ sinh ra file `app_logic.c`: Chứa các hàm handler của task, các bảng TSM/FSM. Đây là mã nguồn ứng dụng thông thường.
- Từ `tezcal` sẽ sinh ra file `tez.c`: File này chứa hàm `main()`, các hàm `setUp()`, `tearDown()`. Nó gọi `UNITY_BEGIN()`, khởi chạy các Test Case do `tezcal` định nghĩa, tương tác với các Task trong `app_logic.c`, và gọi `UNITY_END()`.
- CMake liên kết: `add_executable(tez_runner sources/test/tezprjec/tez.c sources/app/app_logic.c <uedp_core>)`

---

## Mối quan hệ trong `ftezsec.yaml` (Bức tranh hoàn chỉnh)

Để hệ thống hoàn toàn tự động, file `ftezsec.yaml` sẽ là nơi gắn kết (Link) giữa Đối tượng (`testobj`) và Kịch bản (`tezcal`):

```yaml
fidentif: "FTEZSEC_LOGIC_VALIDATION"

testobj: "logic-testobj-v0.yaml" # Sinh ra app.c để làm môi trường test

# Chỉ định các kịch bản sẽ dùng để "tra tấn" đối tượng trên
ftezsec:
  - file: "tezcal-normal_sequence.yaml" # Sinh ra các test case tuần tự
  - file: "tezcal-fault_injection.yaml" # Sinh ra các test case bơm lỗi FCR
```

Do mỗi file `tezcal` chính là một bộ `tezuit` hoàn chỉnh cho một kịch bản kiểm thử, nên có thể viết nhiều file `tezcal` khác nhau để kiểm tra cùng một `testobj`. Điều này giúp tăng khả năng tái sử dụng và mở rộng các kịch bản kiểm thử mà không cần thay đổi logic của `testobj`.

## Inject code và mô hình thăm dò

### API điều khiển runtime

Lõi cung cấp `uedp_ctrl.h` làm ranh giới chính thức cho test runner. Callback inject nhận một `point` và `context`, vì vậy test có thể thay đổi fixture hoặc dừng tiến trình tại các điểm đã định mà không phải chèn chuỗi mã vào scheduler.

```c
static void inject(uedp_ctrl_pnt_t point, void* context) {
  test_fixture_t* fixture = context;
  if (point == UEDP_CTRL_BEFORE_TASK_DISPATCH) {
    fixture->dispatch_count++;
  }
}

uedp_ctrl_set_injector(inject, &fixture);
uedp_ctrl_set_auto_tick(false);
uedp_ctrl_advance_ticks(5, true, &scheduler_steps);
```

Các điểm inject hiện có gồm trước/sau scheduler, timer tick, task dispatch, FSM transition và TSM transition. `uedp_ctrl_run_scheduler(max_steps, &executed_steps)` giới hạn số lần dispatch, còn `uedp_ctrl_advance_ticks(tick_count, run_scheduler, &executed_steps)` cho phép test tự điều khiển thời gian logic mà không cần `sleep`. Trên Linux, gọi `uedp_ctrl_set_auto_tick(false)` trước khi chạy scenario để vô hiệu hóa thread tick nền; mặc định vẫn là `true` để giữ hành vi runtime hiện tại.

Tracepoint dùng cùng lớp API nhưng không cho phép sửa trạng thái:

```c
static void trace(const uedp_ctrl_evt_t* event, void* context) {
  trace_buffer_append(context, event);
}

uedp_ctrl_set_tracer(trace, &trace_buffer);
```

Event trace hiện ghi nhận task dispatch, timer tick và TSM/FSM transition; `subject` trỏ đến đối tượng runtime tương ứng trong suốt thời gian callback.

Với việc tách biệt rõ ràng giữa `testobj` và `tezcal`, việc inject code vào `testobj` để kiểm tra các trạng thái nội bộ là cần thiết để thực hiện các kiểm tra chi tiết. `tezcal` sẽ sử dụng các hàm inject code để truy cập và kiểm tra trạng thái của `testobj` mà không làm thay đổi logic của nó.

Trong đó, `tezcal` sẽ thực hiện các bước sau:

- Bơm tín hiệu vào `testobj` thông qua các hàm API.
- Chạy scheduler để tiến hành các tick.
- Sử dụng các hàm inject code để kiểm tra trạng thái nội bộ của `testobj`, ví dụ như kiểm tra giá trị của các biến GDA, trạng thái của các Task, và các điều kiện khác mà `testobj` phải đáp ứng.
- Ghi nhận kết quả kiểm tra và báo cáo thông qua `libunity`.

Ngoài phương pháp inject code, `tezcal` cũng có thể sử dụng các cơ chế giám sát (monitoring) để theo dõi các sự kiện và trạng thái của `testobj` trong quá trình chạy test. Điều này có thể được thực hiện thông qua việc tạo ra một tác vụ giám sát riêng biệt để nhận thông báo từ `testobj`.

Việc can thiệp sâu sẽ là phương pháp cuối cùng khi các phương pháp kiểm tra thông thường không đủ để xác minh trạng thái của `testobj`. Tuy nhiên, cần lưu ý rằng việc inject code nên được thực hiện một cách cẩn thận để tránh làm thay đổi hành vi của `testobj` và đảm bảo tính toàn vẹn của các kiểm thử.

## Thiết kế tracepoint

Tracepoint là các điểm trong mã nguồn mà tại đó có thể ghi lại thông tin về trạng thái của hệ thống trong quá trình chạy. Trong ngữ cảnh của `tezcal`, tracepoint sẽ được sử dụng để ghi lại các sự kiện quan trọng trong quá trình kiểm thử, như việc chuyển đổi trạng thái của các Task, giá trị của các biến GDA, hoặc bất kỳ sự kiện nào khác mà cần được theo dõi.

Cụ thể ở scheduler, tracepoint có thể được đặt tại các điểm sau:

- Khi một Task chuyển sang trạng thái mới (ví dụ từ RUNNING sang WAITING).
- Khi một tín hiệu được gửi hoặc nhận bởi một Task.
- Khi một biến GDA được cập nhật hoặc truy xuất.
- Khi một lỗi hoặc ngoại lệ xảy ra trong quá trình thực thi.

## Can thiệp tốc độ thực thi

Trong một số trường hợp, để kiểm tra các điều kiện thời gian thực hoặc các trạng thái nhạy cảm với thời gian, `tezcal` có thể cần can thiệp vào tốc độ thực thi của `testobj` thông qua việc điều chỉnh tốc độ của scheduler để dễ dàng kiểm tra các trạng thái mong muốn. Điều này có thể được thực hiện bằng cách bổ sung watchpoint để cho phép điều hướng program counter vào đường thực thi đặc biệt, hoặc bằng cách sử dụng các cơ chế tạm dừng và tiếp tục thực thi của scheduler.

## Tổng hợp biên dịch

Với việc tách biệt rõ ràng giữa `testobj` và `tezcal`, chúng ta sẽ có 2 file nguồn chính và có khả năng biên dịch độc lập. Điều này gây ra vấn đề khi phải đặt `main` ở đâu.

Ở thời điểm hiện tại, `testobj` đang sử dụng riêng biệt 1 hàm `app_main` để khởi tạo và chạy các Task. Điều này cho phép `tezcal` có thể gọi `app_main` để khởi tạo môi trường test mà không cần phải định nghĩa lại hàm `main`. Trong khi đó, `tezcal` sẽ có hàm `main` riêng để thực hiện các kiểm thử và gọi các hàm từ `testobj`.
