# Tài liệu thiết kế μE-LS (Logical Syntax-izer) cho μE(DP)/-OS

Đây là tài liệu thiết kế cho μE-LS, một cấu trúc khai báo theo chuẩn YAML nằm trong tính năng PLD (Parse-able Logical Descriptor) của μE(DP)/-OS. Tài liệu này mô tả các khái niệm, cú pháp và cách sử dụng của μE-LS để định nghĩa các cấu trúc logic và tính năng trong hệ thống μE(DP)/-OS.

## Mục tiêu thiết kế

- Cung cấp một cách tiếp cận nhất quán và dễ hiểu để định nghĩa các cấu trúc logic trong hệ thống μE(DP)/-OS.
- Hỗ trợ khả năng mở rộng và tùy chỉnh các cấu trúc logic theo nhu cầu của người dùng nhằm giảm thiểu sự phức tạp trong việc phát triển và bảo trì hệ thống.
- Đảm bảo tính tương thích với các công cụ và thư viện hiện có trong hệ sinh thái μE(DP)/-OS, đặc biệt chú trọng đến việc đưa PLD/μE-LS làm cơ sở hạ tầng phát triển cho PLTF (Portable Local Test Framework) và TLC (Test Level Coverager) ở phiên bản 1.2.0.

## Định dạng YAML

Cú pháp của μE-LS được thiết kế để dễ đọc và dễ viết, dựa trên chuẩn YAML. Đây là định dạng dữ liệu phổ biến bên cạnh JSON, XML nhưng có ưu điểm là dễ đọc hơn và hỗ trợ các tính năng nâng cao như anchors, aliases, và multi-line strings.

### Quy tắc chung

- **Thụt lề (Indentation):** Sử dụng khoảng trắng (spaces), không bao giờ sử dụng phím Tab. Số lượng khoảng trắng phải nhất quán trong cùng một cấp (thường là 2 hoặc 4).
- **Phân biệt chữ hoa/thường:** YAML có phân biệt chữ hoa và chữ thường (`Task` khác với `task`).
- **Phần mở rộng tệp:** Thường sử dụng `.yaml` hoặc `.yml`.

### Cấu trúc dữ liệu cơ bản

#### Cặp Key-Value

Giá trị được phân cách với khóa bằng dấu hai chấm và một khoảng trắng.

```yaml
task_name: "BLINKER_TASK"  # Chuỗi (Strings)
priority: 7                # Số nguyên (Integers)
is_enabled: true           # Boolean (true/false)
stack_usage: null          # Giá trị rỗng (Null)
```

#### Danh sách / Mảng

Sử dụng dấu gạch ngang `-` kèm theo một khoảng trắng cho mỗi phần tử.

```yaml
signals:
  - SIG_START
  - SIG_STOP
  - SIG_TIMER
```

#### Dictionaries / Nested Objects

Sử dụng thụt đầu dòng để thể hiện cấu trúc cha-con.

```yaml
task_config:
  id: KID_TASK_SENSOR
  priority: LEVEL_5
  queue:
    size: 16
    type: STATIC
```

### Các tính năng nâng cao

#### Chuỗi đa dòng (Multi-line Strings)

Rất hữu ích để viết các đoạn mã C (Action Snippets) trong PLD.

- Dấu `|` (Literal): Giữ nguyên các ký tự xuống dòng.
- Dấu `>` (Folded): Thay thế các ký tự xuống dòng bằng khoảng trắng.

```yaml
action_snippet: |
  if (data > 100) {
    status = ERROR;
    log_err("Value out of range");
  }
```

#### Chú thích

Sử dụng dấu `#` cho các ghi chú trên một dòng.

```yaml
# Đây là chú thích cấu hình Task
task_id: 0xE5 # Tác vụ USR mặc định
```

#### Neo và Tham chiếu (Anchors & Aliases)

Dùng để tái sử dụng cấu hình, tránh lặp lại (Don't Repeat Yourself).

- Dấu `&`: Định nghĩa một mốc (Anchor).
- Dấu `*`: Tham chiếu đến mốc đó (Alias).

```yaml
# Định nghĩa cấu hình mẫu
default_config: &base_settings
  priority: LEVEL_1
  stack_size: 256

# Sử dụng lại cho các Task khác
task_a:
  <<: *base_settings
  id: TASK_A

task_b:
  <<: *base_settings
  id: TASK_B
  priority: LEVEL_9 # Ghi đè (Override) giá trị mặc định
```

### Ví dụ tổng hợp

Mô tả một Task hoàn chỉnh kết hợp các quy tắc trên:

```yaml
!include "signals.yaml" # Giả định tính năng include mở rộng

task_definition:
  id: KID_TASK_MOTOR
  tsm:
    initial: STATE_STOPPED
    states:
      - id: STATE_STOPPED
        transitions:
          - when: SIG_START
            go_to: STATE_RUNNING
            steps:
              - action: post_msg
                to: KID_TASK_UI
                sig: SIG_LCD_UPDATE
                data: |
                  {
                    "status": "MOTOR_START",
                    "code": 200
                  }
      - id: STATE_RUNNING
        on_entry:
          - action: timer_set
            ms: 1000
            type: PERIODIC
```

Lưu ý rằng cú pháp này dùng để làm ví dụ mẫu, không phải là cú pháp chính thức của μE-LS. Người dùng cần tham khảo tài liệu chính thức để biết các quy tắc và cú pháp đầy đủ.

### Lưu ý khi thiết kế PLD Parser (Python)

- **Dấu ngoặc kép:** Không bắt buộc đối với chuỗi đơn giản, nhưng nên dùng nếu chuỗi chứa ký tự đặc biệt (như `:`, `{`, `}`, `[`, `]`, `,`, `&`, `*`, `#`, `?`, `|`, `-`, `<`, `>`, `=`, `!`, `%`, `@`).
- **Giá trị Hex:** PyYAML nhận diện `0x` là số nguyên. Có thể sử dụng `!!str 0x12` nếu muốn ép kiểu về chuỗi.
- **Kiểm tra thụt lề:** 90% lỗi YAML đến từ việc thụt lề không đều (ví dụ dòng dùng 2 spaces, dòng dùng 3 spaces). Nên tích hợp một bộ **YAML Linter** vào công cụ PLTF để báo lỗi cho người dùng ngay lập tức.

## Cú pháp μE-LS

Cú pháp μE-LS được thiết kế để mô tả các cấu trúc logic trong hệ thống μE(DP)/-OS, bao gồm các khối như Task, State Machine (TSM), Signal, Policy, và các hành động (Action Snippets). Các cấu hình như Pool, Queue và Timer được cấu hình tự động bởi Kconfig + pre-PLTF + Jinja2, do đó không cần khai báo trong μE-LS. Tuy nhiên, người dùng có thể tùy chỉnh các thông số này thông qua Kconfig.

### Tác vụ với tính năng HSMC

Tác vụ (Task) cùng với tin nhắn (Message) và tín hiệu (Signal) là các khối cơ bản trong μE(DP)/-OS. Mỗi Task có thể được định nghĩa với các thuộc tính như ID, Priority, Stack Size, và các hành vi thông qua TSM/FSM. Ngoài ra ở các phiên bản mới, Task cũng sẽ được bổ sung các cơ chế đặc biệt như SSI, SIF, ... để hỗ trợ các tính năng OS nâng cao.

Trong thiết kế, tác vụ được chia thành 2 loại là task Norm (tnorm) và task Poll (tpoll) nhằm phục vụ các mục đích khác nhau. Task Norm thường được sử dụng cho các tác vụ có trạng thái và hành vi phức tạp, trong khi Task Poll thường được sử dụng cho các tác vụ đơn giản, chủ yếu thực hiện kiểm tra định kỳ hoặc xử lý dữ liệu từ các nguồn bên ngoài.

Về tổng quát, Task Norm gồm các khai báo như sau:

- `tnorm`: ID của Task NORM (ví dụ: `KID_TASK_SENSOR`).
- `tsm`: Định nghĩa trạng thái thông minh (State Machine) của Task.
  - `id`: ID của trạng thái (tức tên trạng thái).
  - `trans`: Các chuyển đổi từ trạng thái này sang trạng thái khác dựa trên tín hiệu nhận được.
    - `sig`: Tín hiệu kích hoạt chuyển đổi trạng thái kế tiếp.
    - `goto`: Trạng thái đích sau khi chuyển đổi.
  - `on_ntry`: Các hành động thực hiện khi Task vào trạng thái này. Dùng cho việc khởi tạo, thiết lập bộ hẹn giờ, gửi tin nhắn, ...
    - `steps`: Danh sách các hành động cần thực hiện khi rời khỏi trạng thái.
      - `actv`: Hành động cụ thể (ví dụ: `post_msg`, `timer_set`, ...).
        - `to`: Đích đến của hành động (ví dụ: Task nhận tin nhắn).
        - `sig`: Tín hiệu gửi đi (nếu hành động là gửi tin nhắn).
        - `data`: Dữ liệu gửi kèm (nếu hành động là gửi tin nhắn).
  - `on_exit`: Các hành động thực hiện khi Task rời khỏi trạng thái này. Dùng cho việc dọn dẹp, hủy bỏ các bộ hẹn giờ, giải phóng tài nguyên, ...
    - Tương tự như `on_entry`, nhưng được thực hiện khi Task rời khỏi trạng thái này.
  - `on_actv`: Các hành động thực hiện khi Task đang ở trạng thái này. Dùng cho việc kiểm tra điều kiện, xử lý dữ liệu, gửi tin nhắn định kỳ, ...
    - Tương tự như `on_entry` và `on_exit`, nhưng được thực hiện liên tục khi Task đang ở trạng thái này.
- `fsm`: Định nghĩa trạng thái hữu hạn (Finite State Machine) của Task. Tương tự như `tsm`, nhưng thường được sử dụng cho các Task có hành vi đơn giản hơn và cần quản lý luồng trạng thái cục bộ hơn.
  - `id`: ID của trạng thái (tức tên trạng thái).
  - `on_recv`: Chuyển đổi trạng thái dựa trên tín hiệu nhận được.
    - `sig`: Tín hiệu kích hoạt chuyển đổi trạng thái kế tiếp.
      - `goto`: Trạng thái đích sau khi chuyển đổi.
      - `actv`: Hành động cụ thể (ví dụ: `post_msg`, `timer_set`, ...).
        - Nếu hành động là single-step thì dùng `act`, nếu hành động là multi-step thì dùng `steps`.

Ví dụ về một Task Norm với TSM:

```yaml
- tnorm: KID_TASK_USR
  tsm:
  - id: STATE_USR_IDLE
    trans:
    - sig: KID_SIG_USR_START
      goto: STATE_USR_WAITING
    on_ntry: NULL
    on_actv: NULL
    on_exit: NULL
  - id: STATE_USR_WAITING
    trans:
    - sig: KID_SIG_USR_STOP
      goto: STATE_USR_IDLE
    on_ntry: # Khi vừa vào trạng thái chờ, kích hoạt Task A
      steps:
      - actv: post_msg
        to: KID_TASK_A
        sig: KID_SIG_USR_START
        data: NULL
    on_actv: # Khi active, gửi log thông báo đang chờ tín hiệu STOP
      steps:
      - actv: log
        to: KID_TASK_USR
        sig: KID_SIG_LOG
        data: "System Task USR: Waiting for STOP signal..."
    on_exit: # Khi rời khỏi trạng thái chờ, gửi log thông báo kết thúc
      steps:
      - actv: log
        to: KID_TASK_USR
        sig: KID_SIG_LOG
        data: "System Task USR: Sequence Finished."
```

Ví dụ về một Task Norm với FSM:

```yaml
- tnorm: KID_TASK_B
  fsm:
  - id: STATE_B_IDLE
    on_recv:
    - sig: KID_SIG_0x12
      goto: STATE_B_BUSY
      steps: # Multi-step: Gửi đồng thời 0x34 và 0xFF cho A
      - actv: post_msg
        to: KID_TASK_A
        sig: KID_SIG_0x34
        data: NULL
      - actv: post_msg
        to: KID_TASK_A
        sig: KID_SIG_0xFF
        data: NULL
  - id: STATE_B_BUSY
    on_recv:
    - sig: KID_SIG_0xAA
      goto: STATE_B_IDLE
      act: # Single-step: Gửi tín hiệu STOP kết thúc hệ thống
        actv: post_msg
        to: KID_TASK_USR
        sig: KID_SIG_USR_STOP
```

Đối với task Poll, cấu trúc khai báo sẽ đơn giản hơn, chủ yếu tập trung vào việc định nghĩa các hành vi kiểm tra định kỳ.

Có thể định nghĩa một task Poll như sau:

```yaml
- tpoll: KID_TASK_POLL
  steps:
  - actv: poll_led
    ???

```

<!-- 
  Comment:
    Bổ sung thêm tag cho tpoll
 -->