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

### Hướng đọc nhanh

1. Đọc phần quy ước YAML trước để tránh lỗi thụt lề và kiểu dữ liệu.
2. Đọc phần Task để hiểu `applg`, `task`, `tsm`, `fsm`, `exec` và `steps`.
3. Đọc SII, PPLP, APE, OCE để nắm các block mở rộng bám trực tiếp vào core API.
4. Xem ví dụ tổng hợp ở cuối tài liệu nếu muốn viết cấu hình đầu tiên thật nhanh.

### Bản đồ syntax -> core

| Khối | Ý nghĩa | Syntax chính | Syntax phụ / tùy chỉnh | Core mapping |
| --- | --- | --- | --- | --- |
| Task Norm | Task có trạng thái hoặc xử lý message | `applg -> task -> tsm/fsm/exec/ape` | `tsm`, `fsm`, `exec`, `ape`, `on_ntry`, `on_actv`, `on_exit`, `on_recv`, `steps`, `act` | `uedp_task_norm_create()`, `uedp_task_norm_post_msg()` |
| Task Poll | Task vòng lặp nhẹ, không theo message | `applg -> task -> poll/steps` | `poll`, `steps`, `actv`, `to`, `sig`, `data`, `ability` | `uedp_task_poll_create()`, `uedp_task_poll_set_ability()` |
| SII | Đưa signal từ ISR vào hệ thống | `isr -> to/sig` | `to`, `sig`, `NULL` payload, `uedp_task_norm_post_isr()` | `uedp_task_norm_post_isr()`, `uedp_msg_drain_isr_pool()` |
| PPLP | Cấu hình logging pipeline | `pplp -> itnlog -> level/tag/output` | `level`, `tag`, `output.backend`, `output.sink`, `log.timestamp`, `log.msg` | `uedp_itnlog_set_filter()`, `uedp_itnlog_set_output()` |
| APE | Gọi urgent message / priority escalation | `ape -> trigger -> post_urgent` | `mode: slnf`, `mode: non-slnf`, `scope: self`, `keep_queue_order`, `extra_rounds`, `post_urgent` | `uedp_task_norm_post_urgent()`, `uedp_task_norm_set_urgent()` |
| OCE | Service chạy ngoài luồng logic chính | `oce -> services -> handler/context` | `scheduler: fcfs`, `services[]`, `name`, `handler`, `context`, `state` | `ocesvc_register()`, `ocesvc_scheduler()` |

### Đánh giá so với source code hiện tại

Kết luận đối chiếu với core source và testspec hiện tại là: syntax đang dùng trong tài liệu phải giữ nguyên theo trục `on_ntry`, `on_actv`, `actv`, `act`, `steps`, `on_recv`, vì đây mới là shape mà generator và ví dụ test hiện tại đang bám vào. Các đề xuất như `on_entry`, `on_active`, `action`, `guard`, hay `data_kind: VALUE/REF` là hợp lý về mặt UX, nhưng hiện mới ở mức đề xuất mở rộng, chưa nên ghi như syntax chính thức của pre-1.2.0.

| Đề xuất | Đánh giá theo source | Hành động trên tài liệu |
| --- | --- | --- |
| `on_entry` / `on_active` | Chưa có trong testspec và generator hiện tại vẫn dùng `on_ntry` / `on_actv` | Giữ keyword hiện tại là chính thức, có thể ghi thêm alias đề xuất ở ghi chú |
| `action` thay cho `actv` | Core ví dụ và parser hiện tại vẫn dùng `actv` | Không đổi syntax chính, chỉ có thể nhắc đây là tên gợi nhớ cho UX tương lai |
| `guard` trong `trans` | Chưa thấy support ở core TSM/FSM hiện tại | Ghi là hướng mở rộng, không đưa vào grammar chính thức |
| `data_kind: VALUE/REF` cho `post_msg` | Core đã có D2MP và API `uedp_msg_set_data_val/ref`, nhưng chưa có trường PLD tương ứng | Mô tả ở phần mở rộng/D2MP, không coi là field bắt buộc của μE-LS hiện tại |
| APE local theo từng tnorm | Khớp với source: mỗi tnorm có `urgent_pending`, `base_pri`, `cur_pri` và API `set_urgent/post_urgent` | Giữ nguyên và nhấn mạnh là khai báo cục bộ theo task |
| `exec` và `on_recv` | Không trùng nghĩa: `exec` là hành vi phẳng, `on_recv` là dispatch của FSM | Giữ tách biệt để bảo toàn mô hình hiện tại |

Từ đánh giá này, hành động cần làm trên tài liệu là:

1. Giữ syntax hiện tại làm chuẩn chính thức.
2. Thêm ghi chú rõ ràng cho các alias / trường mở rộng chỉ ở mức định hướng
3. Không nâng các field UX mới thành grammar bắt buộc nếu chưa có support trong parser và generator.

### Task - Tác vụ

Trong μE-LS, mỗi task được khai báo trong danh sách `applg`. Một task có thể đi theo một trong ba nhánh chính: `tsm` nếu cần state machine dạng bảng, `fsm` nếu cần dispatch theo handler, hoặc `exec`/`poll` nếu chỉ cần hành vi tuyến tính.

`task` là định danh logic do PLTF sinh ra từ Kconfig; `tnorm` và `tpoll` là hai kiểu hành vi, không phải hai hệ syntax tách biệt. Các cấu hình Pool, Queue, Timer và Stack vẫn thuộc Kconfig, nên μE-LS chỉ mô tả hành vi và quan hệ giữa task, signal, action.

Về tổng quát, một task Norm nên được viết theo cấu trúc sau:

```yaml
project: "uEDP"
applg:
  - task: KID_TASK_USR
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
        on_ntry:
          steps:
            - actv: post_msg
              to: KID_TASK_A
              sig: KID_SIG_USR_START
              data: NULL
        on_actv:
          steps:
            - actv: log
              to: KID_TASK_USR
              sig: KID_SIG_LOG
              data: "System Task USR: Waiting for STOP signal..."
        on_exit:
          steps:
            - actv: log
              to: KID_TASK_USR
              sig: KID_SIG_LOG
              data: "System Task USR: Sequence Finished."
```

`tsm` nên được dùng khi task cần quản lý vòng đời trạng thái rõ ràng và có thể sinh ra `on_entry`, `on_exit` và `on_active` ở tầng codegen. `fsm` nên được dùng khi task chỉ cần dispatch theo tín hiệu với state handler trực tiếp.

Ví dụ FSM nên viết theo kiểu sau:

```yaml
project: "uEDP"
applg:
  - task: KID_TASK_B
    fsm:
      - id: STATE_B_IDLE
        on_recv:
          - sig: KID_SIG_0x12
            goto: STATE_B_BUSY
            steps:
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
            act:
              actv: post_msg
              to: KID_TASK_USR
              sig: KID_SIG_USR_STOP
              data: NULL
```

Nếu một task không cần TSM/FSM thì dùng `exec` để mô tả các hành vi tuyến tính. Đây là lựa chọn phù hợp cho các task đơn giản hoặc các script test nhanh.

```yaml
project: "uEDP"
applg:
  - task: KID_TASK_SIMPLE
    exec:
      - on_sig: SIG_A
        steps:
          - actv: post_msg
            to: KID_TASK_B
            sig: SIG_B
            data: NULL
          - actv: log
            to: KID_TASK_SIMPLE
            sig: SIG_LOG
            data: "Task Simple received SIG_A and sent SIG_B to Task B."
```

Task Poll nên đi theo nhịp polling riêng và chỉ khai báo các bước xử lý tuần tự, không gắn với state machine:

```yaml
project: "uEDP"
applg:
  - task: KID_TASK_POLL
    poll:
      - actv: poll_led
        to: NULL
        sig: NULL
        data: NULL
```

Trong current core, task poll chỉ nên dùng cho logic nhẹ, còn các tác vụ dọn dẹp hệ thống, flush log hoặc đồng bộ nền nên được đẩy sang OCE.

### PPLP - Cấu hình logging pipeline

PPLP khai báo chính sách logging cho Core và backend xuất log. Trong runtime, `itnlog` chỉ giữ filter và callback output; việc flush ra console, UART hoặc file nên đi qua OCE hoặc callback đã đăng ký.

```yaml
pplp:
  itnlog:
    level: DEBUG
    tag: TSK
    output:
      backend: logdp
      sink: printf
```

- `level` ánh xạ vào `uedp_itnlog_set_level()`.
- `tag` ánh xạ vào `uedp_itnlog_set_tag()`.
- `output` ánh xạ vào `uedp_itnlog_set_output()`.

Nếu muốn mô tả một log event trong tài liệu, có thể dùng dạng:

```yaml
log:
  timestamp: 1234
  level: INFO
  tag: TSK
  msg: "Task entered ACTIVE"
```

### ISR - Dịch vụ ngắt

Dịch vụ ngắt là một khối logic quan trọng trong hệ thống μE(DP)/-OS, cho phép xử lý các sự kiện ngắt từ phần cứng hoặc phần mềm. Trong thiết kế môi trường phần cứng đơn nhân, ISR và Task là 2 khối logic có tính tranh chấp cao, do đó cần được thiết kế cẩn thận để tránh các vấn đề về đồng bộ hóa và hiệu suất.

Ở API thủ công, ISR được thiết kế API riêng biệt nhằm đảm bảo xử lý ngắn gọn nhưng vẫn đáp ứng logic của Task.

Cấu trúc khai báo ISR trong μE-LS bao gồm các thành phần sau:

- `isr`: ID của ISR (ví dụ: `KID_ISR_TIMER`).
- `to`: Task nhận signal từ ISR.
- `sig`: Signal được đẩy vào FIFO ISR và chuyển thành message ở đầu vòng scheduler.

```yaml
isr:
  - id: KID_ISR_TIMER
    to: KID_TASK_TIM
    sig: KID_SIG_TIM_TICK
```

Trong core hiện tại, ISR chỉ cần `to` và `sig`; payload `data` chưa được dùng ở đường `uedp_task_norm_post_isr()`. Nếu cần dữ liệu lớn hơn, nên tạo message bình thường ở task hoặc dùng cơ chế mở rộng của PAL sau này.

Syntax này đảm bảo sự ràng buộc ISR chỉ có một hành động duy nhất, giúp giảm thiểu thời gian xử lý ngắt và tránh các vấn đề về đồng bộ hóa với các Task khác.

### APE - Lời gọi vượt quyền tạm thời

APE hay S-LnF APE là cơ chế được triển khai ở phiên bản 1.1.0 và 1.1.1 để hỗ trợ tnorm có thể gọi các hàm vượt quyền tạm thời (Privilege Escalation) trong môi trường μE(DP)/-OS. Trong μE-LS, APE là khai báo cục bộ theo từng tnorm: mỗi task có thể tự định nghĩa trigger APE cho chính nó, và Core chỉ cung cấp cơ chế thực thi tương ứng qua `uedp_task_norm_post_urgent()` và `uedp_task_norm_set_urgent()`.

Cú pháp khai báo APE trong μE-LS được hỗ trợ chỉ dành cho tnorm nên sẽ không có phần khai báo riêng cho tpoll. Một tnorm có thể đặt APE ngang hàng với `tsm`, `fsm` hoặc `exec`, hoặc đưa vào `actv` như một action để tự kích hoạt APE cho chính nó.

```yaml
ape:
  enabled: true
  mode: slnf
  trigger:
    on_sig: SIG_CALL_URGENT # Kích hoạt APE khi nhận signal này
    post_urgent: # Tự gọi urgent message cho chính tnorm để thực thi hành vi ưu tiên
      to: KID_TASK_USR
      sig: SIG_EXEC_URGENT
      data: NULL
```

`post_urgent` tương ứng trực tiếp với `uedp_task_norm_post_urgent()`: message được đẩy vào đầu queue, còn priority escalation được core xử lý bằng `uedp_task_norm_set_urgent()`. Vì APE là local cho từng tnorm, chính tnorm đó phải chịu trách nhiệm xử lý trigger và quyết định khi nào tự tăng ưu tiên cho chính mình.

Ngoài ra, do API hiện tại chưa triển khai restriction policy nên 1 tnorm có thể gọi APE cho chính nó hoặc cho các tnorm khác, miễn là các tnorm đó đã được khai báo APE trong μE-LS. Tuy nhiên, việc gọi APE cho tnorm khác nên được hạn chế để tránh các vấn đề về đồng bộ hóa và ưu tiên xử lý.

Với `mode: non-slnf`, core không cần self-post một message khẩn cấp. Thay vào đó, tnorm được cho phép chạy thêm đúng một vòng nữa theo thứ tự message queue sẵn có của chính nó, rồi mới quay lại trạng thái bình thường. Cách này phù hợp khi người dùng cần ưu tiên xử lý ngữ cảnh hiện tại mà không muốn thay đổi thứ tự queue bằng một urgent message mới.

Ví dụ:

```yaml
project: "uEDP"
applg:
  - task: KID_TASK_USR
    exec:
      - on_sig: SIG_CALL_URGENT
        steps:
          - actv: post_msg
            to: KID_TASK_USR
            sig: SIG_EXEC_URGENT
            data: NULL
      - on_sig: SIG_EXEC_URGENT
        steps:
          - actv: log
            to: KID_TASK_USR
            sig: SIG_LOG
            data: "Executing urgent action..."
    ape:
      enabled: true
      mode: slnf
      trigger:
        on_sig: SIG_CALL_URGENT # Kích hoạt APE khi nhận signal này
        post_urgent:
          to: KID_TASK_USR
          sig: SIG_EXEC_URGENT
          data: NULL
  - task: KID_TASK_IO
    exec:
      - on_sig: SIG_CALL_IO_BOOST
        steps:
          - actv: post_msg
            to: KID_TASK_IO
            sig: SIG_EXEC_IO_BOOST
            data: NULL
      - on_sig: SIG_EXEC_IO_BOOST
        steps:
          - actv: log
            to: KID_TASK_IO
            sig: SIG_LOG
            data: "Executing IO boost action..."
    ape:
      enabled: true
      mode: non-slnf
      trigger:
        on_sig: SIG_CALL_IO_BOOST # Kích hoạt APE khi nhận signal này
        post_urgent: NULL # Không cần tự gửi urgent message mới
```

Khi dùng `mode: non-slnf`, tnorm không bắt buộc phải tự gửi một urgent message mới. Mục đích là cho phép chính task đó giữ nhịp xử lý thêm một vòng với queue hiện có, nên trigger thường chỉ cần là một signal nội bộ hoặc một action local do cùng task phát ra. Nếu có urgent message mới, nó sẽ được xử lý theo thứ tự FIFO bình thường, không phải ưu tiên.

### OCE - Dịch vụ ngoài ngữ cảnh logic

OCE (Out-Context Execution) là cơ chế được triển khai ở phiên bản 1.1.3 để hỗ trợ tnorm có thể thực hiện các dịch vụ ngoài ngữ cảnh logic (Out-Context Services) trong môi trường μE(DP)/-OS.

```yaml
oce:
  scheduler: fcfs
  services:
    - name: OCE_ITNLOG_DUMP
      handler: itnlog_dump
      context: pplp_ctx
      state: READY
```

Trong core hiện tại, `ocesvc_register()` tự gán `uint8_t id`, vì vậy `name` ở μE-LS nên được hiểu là nhãn logic để PLTF sinh code và debug trace. `handler` phải khớp kiểu `void (*)(ocesvc_t*)`, còn `context` là vùng dữ liệu mà service sẽ dùng khi được scheduler gọi.

OCE nên được dùng cho các việc như flush log, đồng bộ nền, hoặc dọn tài nguyên sau vòng scheduler chính. Nó không nên bị lẫn với task poll vì poll vẫn nằm trong path ứng dụng, còn OCE là service hậu trường của hệ thống.

### Template tham chiếu tổng hợp

Khi cần một khung khai báo đầy đủ để tham chiếu nhanh, có thể dùng template sau. Các giá trị `NULL` hoặc placeholder chỉ mang tính gợi ý, người dùng thay thế theo bài toán thực tế.

```yaml
project: "uEDP"
applg:
  - task: KID_TASK_USR
    tsm:
      - id: STATE_USR_IDLE
        trans:
          - sig: SIG_START
            goto: STATE_USR_RUN
        on_ntry: NULL
        on_actv: NULL
        on_exit: NULL
      - id: STATE_USR_RUN
        trans:
          - sig: SIG_STOP
            goto: STATE_USR_IDLE
        on_ntry:
          steps:
            - actv: post_msg
              to: KID_TASK_A
              sig: SIG_A
              data: NULL
        on_actv:
          steps:
            - actv: log
              to: KID_TASK_USR
              sig: SIG_LOG
              data: "Task is running"
        on_exit:
          steps:
            - actv: log
              to: KID_TASK_USR
              sig: SIG_LOG
              data: "Task is stopping"

  - task: KID_TASK_A
    fsm:
      - id: STATE_A_IDLE
        on_recv:
          - sig: SIG_A
            goto: STATE_A_BUSY
            steps:
              - actv: post_msg
                to: KID_TASK_B
                sig: SIG_B
                data: NULL
      - id: STATE_A_BUSY
        on_recv:
          - sig: SIG_B
            goto: STATE_A_IDLE
            act:
              actv: post_msg
              to: KID_TASK_USR
              sig: SIG_DONE
              data: NULL

  - task: KID_TASK_SIMPLE
    exec:
      - on_sig: SIG_SIMPLE
        steps:
          - actv: post_msg
            to: KID_TASK_A
            sig: SIG_A
            data: NULL

  - task: KID_TASK_POLL
    poll:
      - actv: poll_led
        to: NULL
        sig: NULL
        data: NULL

isr:
  - id: KID_ISR_TIMER
    to: KID_TASK_TIM
    sig: KID_SIG_TIM_TICK

pplp:
  itnlog:
    level: DEBUG
    tag: TSK
    output:
      backend: logdp
      sink: printf

ape:
  enabled: true
  mode: slnf
  trigger:
    on_sig: SIG_CALL_URGENT
    scope: self
    post_urgent:
      to: KID_TASK_USR
      sig: SIG_EXEC_URGENT
      data: NULL

oce:
  scheduler: fcfs
  services:
    - name: OCE_ITNLOG_DUMP
      handler: itnlog_dump
      context: pplp_ctx
      state: READY
```

<!-- 
  Comment:
    - Hoàn thiện APE cho tnorm, bổ sung các hành vi gọi hàm vượt quyền tạm thời.
    - Bổ sung thiết kế nếu sử dụng dạng non-HSMC, cho phép chỉ định hành vi của tnorm với các tín hiệu tương ứng.
 -->