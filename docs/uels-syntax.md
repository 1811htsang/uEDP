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

```yaml
action_snippet_folded: >
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

Việc sử dụng `<<` cho phép merge các trường từ anchor vào dictionary hiện tại, giúp giảm thiểu lỗi và tăng tính nhất quán trong cấu hình.

<!-- TODO
- Bổ sung thêm phần tag include của YAML nâng cao để hỗ trợ việc include các file cấu hình con, ví dụ `!include "signals.yaml"`.
- Bổ sung thêm việc tìm hiểu các giải pháp để hỗ trợ lấy alias từ file include mà không cần phải khai báo lại trong file chính.
-->

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

Lưu ý rằng cú pháp này dùng để làm ví dụ mẫu, không phải là cú pháp chính thức của μE-LS. Người dùng cần tham khảo tài liệu chính thức bên dưới để biết các quy tắc và cú pháp đầy đủ.

### Lưu ý khi thiết kế PLD Parser (Python)

- **Dấu ngoặc kép:** Không bắt buộc đối với chuỗi đơn giản, nhưng nên dùng nếu chuỗi chứa ký tự đặc biệt (như `:`, `{`, `}`, `[`, `]`, `,`, `&`, `*`, `#`, `?`, `|`, `-`, `<`, `>`, `=`, `!`, `%`, `@`).
- **Giá trị Hex:** PyYAML nhận diện `0x` là số nguyên. Có thể sử dụng `!!str 0x12` nếu muốn ép kiểu về chuỗi.
- **Kiểm tra thụt lề:** 90% lỗi YAML đến từ việc thụt lề không đều (ví dụ dòng dùng 2 spaces, dòng dùng 3 spaces). Nên tích hợp một bộ **YAML Linter** vào công cụ PLTF để báo lỗi cho người dùng ngay lập tức.

## Cú pháp μE-LS

Cú pháp μE-LS được thiết kế để mô tả các cấu trúc logic trong hệ thống μE(DP)/-OS, bao gồm các khối như Task, State Machine (TSM), Signal, Policy, và các hành động (Action Snippets). Các cấu hình như Pool, Queue và Timer được cấu hình tự động bởi Kconfig + pre-PLTF + Jinja2, do đó không cần khai báo trong μE-LS. Tuy nhiên, người dùng có thể tùy chỉnh các thông số này thông qua Kconfig.

<!-- TODO
Cần rewrite lại phần này tương ứng với các khối phát triển đã có bên nhánh feat.
-->

### Hướng dẫn đọc nhanh

1. Đọc phần quy ước YAML trước để tránh lỗi thụt lề và kiểu dữ liệu.
2. Đọc phần Task để hiểu `tlist`, `task`, `tsm`, `fsm`, `exec` và `steps`.
3. Đọc SII, PPLP, APE, OCE để nắm các block mở rộng bám trực tiếp vào core API.
4. Xem ví dụ tổng hợp ở cuối tài liệu nếu muốn viết cấu hình đầu tiên thật nhanh.

### Bản đồ syntax -> core

| Khối | Ý nghĩa | Syntax chính | Syntax phụ / tùy chỉnh | Core mapping |
| --- | --- | --- | --- | --- |
| Task Norm | Task có trạng thái hoặc xử lý message | `tlist -> task -> tsm/fsm/exec/escal` | `tsm`, `fsm`, `exec`, `escal`, `on_ntry`, `on_actv`, `on_exit`, `on_recv`, `steps`, `cact` | `uedp_task_norm_create()`, `uedp_task_norm_post_msg()` |
| Task Poll | Task vòng lặp nhẹ, không theo message | `tlist -> task -> poll/steps` | `poll`, `steps`, `actv`, `to`, `sig`, `data`, `ability` | `uedp_task_poll_create()`, `uedp_task_poll_set_ability()` |
| SII | Đưa signal từ ISR vào hệ thống | `isr -> to/sig` | `to`, `sig` | `uedp_task_norm_post_isr()`, `uedp_msg_drain_isr_pool()` |
| PPLP | Cấu hình logging pipeline | `pplp -> itnlog -> level/tag/output` | `level`, `tag`, `output.backend`, `output.sink`, `log.timestamp`, `log.msg` | `uedp_itnlog_set_filter()`, `uedp_itnlog_set_output()` |
| APE | Gọi urgent message / priority escalation | `escal -> trigger -> post_urgent` | `mode: slnf`, `mode: non-slnf`, `scope: self`, `keep_queue_order`, `extra_rounds`, `post_urgent` | `uedp_task_norm_post_urgent()`, `uedp_task_norm_set_urgent()` |
| OCE | Service chạy ngoài luồng logic chính | `outexec -> name/handler/context/state` | `name`, `handler`, `context`, `state` | `ocesvc_register()`, `ocesvc_scheduler()` |

### Các lưu ý chung

Nếu tính năng không sử dụng thì set giá trị đi kèm là `NULL` hoặc bỏ qua. Điều này áp dụng đối với các tính năng như:

- PPLP.
- APE.
- ISR.
- OCE.
- TSM (on_ntry, on_actv, on_exit).

<!-- TODO
Cần kiểm tra các trường hợp đặc biệt trong cú pháp để xử lý thành các bug-fix release.
-->

<!-- NOTE
Đưa cho Minh kiểm tra phần này với source code hiện tại để đảm bảo rằng cú pháp μE-LS khớp với core API và các ví dụ test hiện tại. Nếu có sự khác biệt, cần ghi chú rõ ràng trong tài liệu để thực hiện bổ sung bug-fix.
-->

### Đánh giá so với source code hiện tại

Kết luận đối chiếu với core source và testspec hiện tại là: syntax đang dùng trong tài liệu phải giữ nguyên theo trục `on_ntry`, `on_actv`, `actv`, `cact`, `steps`, `on_recv`, vì đây mới là shape mà generator và ví dụ test hiện tại đang bám vào. Các đề xuất như `on_entry`, `on_active`, `action`, `guard`, hay `data_kind: VALUE/REF` là hợp lý về mặt UX, nhưng hiện mới ở mức đề xuất mở rộng, chưa nên ghi như syntax chính thức của pre-1.2.0.

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

Trong μE-LS, mỗi task được khai báo trong danh sách `tlist`. Một task có thể đi theo một trong ba nhánh chính: `tsm` nếu cần state machine dạng bảng, `fsm` nếu cần dispatch theo handler, hoặc `exec`/`poll` nếu chỉ cần hành vi tuyến tính.

`task` là định danh logic do PLTF sinh ra từ Kconfig; `tnorm` và `tpoll` là hai kiểu hành vi, không phải hai hệ syntax tách biệt. Các cấu hình Pool, Queue, Timer và Stack vẫn thuộc Kconfig, nên μE-LS chỉ mô tả hành vi và quan hệ giữa task, signal, action.

Về tổng quát, một task Norm nên được viết theo cấu trúc sau:

```yaml
project: "uEDP"
tlist:
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
tlist:
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
      cact:
        actv: post_msg
        to: KID_TASK_USR
        sig: KID_SIG_USR_STOP
        data: NULL
```

Nếu một task không cần TSM/FSM thì dùng `exec` để mô tả các hành vi tuyến tính. Đây là lựa chọn phù hợp cho các task đơn giản hoặc các script test nhanh.

```yaml
project: "uEDP"
tlist:
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
tlist:
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

Với thiết kế PPLP, hệ thống được chia ra làm 3 phần là `itnlog` (filter), `logdp` (sink) và `rprintf` (backend). Cấu hình PPLP nên được viết theo kiểu sau:

```yaml
pplp:
  itnlog:
    level: ITNLOG_LEVEL_INFO
    tag: ITNLOG_TAG_TSK
    filter: enable
      level: ITNLOG_LEVEL_FATAL
      tag: ITNLOG_TAG_TSM
    output: output_func
  logdp:
    register:
    - func: sink_func_1
    - func: sink_func_2
  rprintf:
  - contract: name // add name
    init: init_func
    putc: putc_func
    write: write_func
    is_ready: true
  - contract: ...
```

Với `itnlog`:

- `level` và `tag` là placeholder để tự động set giá trị cho các log entry.
- `filter` là danh sách các rule để lọc log theo level và tag.
- `output` là callback function để xử lý log entry đã lọc. Nếu là PPLP hoàn chỉnh thì `output` sẽ gọi `logdp` để đẩy log ra sink đã đăng ký.

Với `logdp`:

- `register` là danh sách các callback function để xử lý log entry. Mỗi function sẽ nhận log entry và thực hiện hành vi xuất log ra console, UART hoặc file.

Với `rprintf`:

- `contract` là tên của backend xuất log, ví dụ `UART`, `FILE`, `CONSOLE`.
- `init`, `putc`, `write` là các callback function để khởi tạo, xuất ký tự và xuất chuỗi log.
- `is_ready` là cờ để kiểm tra backend đã sẵn sàng nhận log hay chưa.

<!-- comment
- Kiểm tra lại cú pháp logging pipeline, đảm bảo các trường `level`, `tag`, `output` được ánh xạ đúng với core API.

```yaml
pplp:
  itnlog:
    level: ITNLOG_LEVEL_INFO
    tag: ITNLOG_TAG_TSK
    filter: enable
      level: ITNLOG_LEVEL_FATAL
      tag: ITNLOG_TAG_TSM
    output: output_func
  logdp:
    register:
    - func: sink_func_1
    - func: sink_func_2
  rprintf:
  - contract: name // add name
    init: init_func
    putc: putc_func
    write: write_func
    is_ready: true
  - contract: ...
```

 -->

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

Trong core hiện tại, ISR chỉ cần `to` và `sig`; payload `data` chưa được dùng ở đường `uedp_task_norm_post_isr()` và cũng bị cấm sử dụng do ISR không được phép thao tác trực tiếp với vùng dữ liệu của Core.

Syntax này đảm bảo sự ràng buộc ISR chỉ có một hành động duy nhất, giúp giảm thiểu thời gian xử lý ngắt và tránh các vấn đề về đồng bộ hóa với các Task khác.

### APE - Lời gọi vượt quyền tạm thời

APE hay S-LnF APE là cơ chế được triển khai ở phiên bản 1.1.0 và 1.1.1 để hỗ trợ tnorm có thể gọi các hàm vượt quyền tạm thời (Privilege Escalation) trong môi trường μE(DP)/-OS. Trong μE-LS, APE là khai báo cục bộ theo từng tnorm: mỗi task có thể tự định nghĩa trigger APE cho chính nó, và Core chỉ cung cấp cơ chế thực thi tương ứng qua `uedp_task_norm_post_urgent()` và `uedp_task_norm_set_urgent()`.

Cú pháp khai báo APE trong μE-LS được hỗ trợ chỉ dành cho tnorm nên sẽ không có phần khai báo riêng cho tpoll. Một tnorm có thể đặt APE ngang hàng với `tsm`, `fsm` hoặc `exec`, hoặc đưa vào `actv` như một action để tự kích hoạt APE cho chính nó.

```yaml
escal:
  enabled: true # if false thì tnorm không có APE
  mode: slnf
  trigger:
  - on_sig: SIG_CALL_URGENT # Kích hoạt APE khi nhận signal này
    post_urgent: # Tự gọi urgent message cho chính tnorm để thực thi hành vi ưu tiên
      to: KID_TASK_USR
      sig: SIG_EXEC_URGENT
      data: NULL
```

`post_urgent` tương ứng trực tiếp với `uedp_task_norm_post_urgent()`: message được đẩy vào đầu queue, còn priority escalation được core xử lý bằng `uedp_task_norm_set_urgent()`. Vì APE là local cho từng tnorm, chính tnorm đó phải chịu trách nhiệm xử lý trigger và quyết định khi nào tự tăng ưu tiên cho chính mình.

Ngoài ra, do API hiện tại chưa triển khai restriction policy nên 1 tnorm có thể gọi APE cho chính nó hoặc cho các tnorm khác, miễn là các tnorm đó đã được khai báo APE trong μE-LS. Tuy nhiên, việc gọi APE cho tnorm khác nên được hạn chế để tránh các vấn đề về đồng bộ hóa và ưu tiên xử lý.

Với `mode: non-slnf`, core không cần self-post một message khẩn cấp. Thay vào đó, tnorm được cho phép chạy thêm đúng một vòng nữa theo thứ tự message queue sẵn có của chính nó, rồi mới quay lại trạng thái bình thường. Cách này phù hợp khi người dùng cần ưu tiên xử lý ngữ cảnh hiện tại mà không muốn thay đổi thứ tự queue bằng một urgent message mới.

```yaml
escal:
  enabled: true
  mode: non-slnf
  trigger:
  - on_sig: SIG_CALL_IO_BOOST # Kích hoạt APE khi nhận signal này
    post_urgent: NULL # Không cần tự gửi urgent message mới
  - on_sig: SIG_URGENT_NEXT # Kích hoạt APE khi nhận signal này
    post_urgent: NULL # Không cần tự gửi urgent message mới
```

Ví dụ:

```yaml
project: "uEDP"
tlist:
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
  escal:
    enabled: true
    mode: slnf
    trigger:
    - on_sig: SIG_CALL_URGENT # Kích hoạt APE khi nhận signal này
      post_urgent:
        to: KID_TASK_USR
        sig: SIG_EXEC_URGENT
        data: NULL
    - on_sig: SIG_URGENT_NEXT # Kích hoạt APE khi nhận signal này
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
  escal:
    enabled: true
    mode: non-slnf
    trigger:
    - on_sig: SIG_CALL_IO_BOOST # Kích hoạt APE khi nhận signal này
      post_urgent: NULL # Không cần tự gửi urgent message mới
    - on_sig: SIG_URGENT_NEXT # Kích hoạt APE khi nhận signal này
      post_urgent: NULL # Không cần tự gửi urgent message mới
```

Khi dùng `mode: non-slnf`, tnorm không bắt buộc phải tự gửi một urgent message mới. Mục đích là cho phép chính task đó giữ nhịp xử lý thêm một vòng với queue hiện có, nên trigger thường chỉ cần là một signal nội bộ hoặc một action local do cùng task phát ra. Nếu có urgent message mới, nó sẽ được xử lý theo thứ tự FIFO bình thường, không phải ưu tiên.

> Thống nhất cú pháp
> `trigger` ở cả 2 mode được xem xét làm một danh sách các trigger, mỗi trigger có thể là một signal hoặc một action. Khi trigger được kích hoạt, nếu `post_urgent` không NULL thì sẽ gửi urgent message mới; nếu NULL thì task sẽ tiếp tục xử lý queue hiện tại thêm một vòng nữa.

### OCE - Dịch vụ ngoài ngữ cảnh logic

OCE (Out-Context Execution) là cơ chế được triển khai ở phiên bản 1.1.3 để hỗ trợ tnorm có thể thực hiện các dịch vụ ngoài ngữ cảnh logic (Out-Context Services) trong môi trường μE(DP)/-OS.

```yaml
outexec:
- name: OCE_ITNLOG_DUMP
  handler: itnlog_dump
  context: pplp_ctx
  state: READY
```

Trong core hiện tại, `ocesvc_register()` tự gán `uint8_t id`, vì vậy `name` ở μE-LS nên được hiểu là nhãn logic để PLTF sinh code và debug trace. `handler` phải khớp kiểu `void (*)(ocesvc_t*)`, còn `context` là vùng dữ liệu mà service sẽ dùng khi được scheduler gọi.

OCE nên được dùng cho các việc như flush log, đồng bộ nền, hoặc dọn tài nguyên sau vòng scheduler chính. Nó không nên bị lẫn với task poll vì poll vẫn nằm trong path ứng dụng, còn OCE là service hậu trường của hệ thống.

<!-- comment
  Trong μE-LS, cú pháp hiện tại không hỗ trợ việc cho phép chỉ định service tiếp theo được gọi sau khi service hiện tại hoàn tất. Nếu muốn mở rộng, có thể thêm trường `next_service` hoặc `callback` để chỉ định service tiếp theo, nhưng hiện tại chưa có support trong core. Do đó, tính năng này sẽ được xem xét trong các phiên bản tương lai của μE-LS.
 -->

### Template tham chiếu tổng hợp

Khi cần một khung khai báo đầy đủ để tham chiếu nhanh, có thể dùng template sau. Các giá trị `NULL` hoặc placeholder chỉ mang tính gợi ý, người dùng thay thế theo bài toán thực tế.

```yaml
project: "uEDP"
tlist:
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
      cact:
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
    level: ITNLOG_LEVEL_INFO
    tag: ITNLOG_TAG_TSK
    filter: enable
      level: ITNLOG_LEVEL_FATAL
      tag: ITNLOG_TAG_TSM
    output: output_func
  logdp:
    register:
    - func: sink_func_1
    - func: sink_func_2
  rprintf:
  - contract: name // add name
    init: init_func
    putc: putc_func
    write: write_func
    is_ready: true
  - contract: ...

escal:
  enabled: true
  mode: slnf
  trigger:
    on_sig: SIG_CALL_URGENT
    post_urgent:
      to: KID_TASK_USR
      sig: SIG_EXEC_URGENT
      data: NULL

outexec:
- name: OCE_ITNLOG_DUMP
  handler: itnlog_dump
  context: pplp_ctx
  state: READY
```

### Phân biệt `act`, `actv`, `cact` và `steps`

- `act` là một hành vi đơn lẻ, có thể là `post_msg`, `log`, `timer_set`, v.v. Nó được dùng trong `steps` hoặc `cact` của các khai báo non-HSMC, tức là sử dụng `exec`.
- `actv` là một alias cho `act`, dùng để nhấn mạnh đây là hành vi đang được thực thi trong ngữ cảnh hiện tại của FSM/TSM. Nó có thể chứa các trường bổ sung như `to`, `sig`, `data` để xác định hành vi cụ thể. được sử dụng trong khai báo HSMC, tức là trong `on_ntry`, `on_actv`, `on_exit`, hoặc `on_recv`.
- `cact` là một alias cho `act`, viết tắt của `call act`, chuỗi hành vi được thực hiện khi một transition được kích hoạt, thường dùng trong `on_recv` của FSM. Nó có thể chứa một hoặc nhiều `actv` trong danh sách `steps`.
- `steps` là một danh sách các hành vi (`actv`) được thực hiện tuần tự trong một ngữ cảnh cụ thể, như `on_ntry`, `on_actv`, `on_exit`, hoặc `on_recv`. Mỗi bước trong `steps` có thể là một hành vi đơn lẻ hoặc một hành vi phức tạp, tùy thuộc vào logic của task.

> Kết luận đơn giản: `act` là hành vi cơ bản, `actv` là hành vi được thực thi trong ngữ cảnh cụ thể, `cact` là hành vi được thực hiện khi transition được kích hoạt, và `steps` là danh sách các hành vi được thực hiện theo thứ tự.

<!-- TODO
100826 - Cân nhắc thay đổi 2 keyword `act` và `actv` để tránh nhầm lẫn.
110826 - Cân nhắc remove `cact` và chỉ dùng `steps` trong `on_recv` để thống nhất cú pháp. ~ Bổ sung task list để thực thi việc sửa đổi này.
-->

### Khu vực dữ liệu toàn cục - Global Data Area

Bổ sung thêm phần mô tả về khu vực dữ liệu toàn cục (Global Data Area) trong μE-LS. Khu vực này được sử dụng để lưu trữ các biến và cấu trúc dữ liệu phục vụ tính năng D2MP (Data-to-Message Passing).

```yaml
glbda:
- '1': &gda1
  name: GLOBAL_VAR_1
  type: int
  initial_value: 0
- '2': &gda2
  name: GLOBAL_VAR_2
  type: string
  initial_value: "default"
...
```

Trong đó:

- `name`: Tên của biến toàn cục.
- `type`: Kiểu dữ liệu của biến.
- `initial_value`: Giá trị ban đầu của biến.

Thiết kế này cho phép người dùng khai báo biến toàn cục với kiểu dữ liệu và giá trị khởi tạo phục vụ tính năng D2MP, tức truyền tham chiếu và truyền tham trị giữa các task thông qua message. Các biến này sẽ được quản lý bởi core và có thể được truy cập từ các task khác nhau trong hệ thống. Cho phép hỗ trợ alias và tham chiếu để tránh lặp lại khai báo biến toàn cục.

Ví dụ mẫu việc sử dụng biến toàn cục trong μE-LS:

```yaml
glbda:
- '1': &gda1
  name: GLOBAL_COUNTER
  type: const char*
  initial_value: "msg: hello"

tlist:
- task: TASK_A
  exec:
  - on_sig: SIG_USR
    act:
    - actv: increment_global
      to: TASK_B
      sig: SIG_HELLO
      data: *gda1  # Tham chiếu đến biến toàn cục GLOBAL_COUNTER
      ptype: REF  # Chỉ định truyền tham chiếu, nếu muốn truyền tham trị thì dùng VAL
- task: TASK_B
  exec:
  - on_sig: SIG_HELLO
    act:
    - actv: log # action này thực hiện tự post chính mình với việc truyền VAL để copy dữ liệu từ biến toàn cục
      to: TASK_B
      sig: SIG_LOG
      data: *gda1  # Tham chiếu đến biến toàn cục GLOBAL_COUNTER
      ptype: VAL  # Chỉ định truyền tham trị, nếu muốn truyền tham chiếu thì dùng REF
```

<!-- REVIEW
Cân nhắc thiết kế hoặc bổ sung thông tin trong tài liệu để làm rõ khi dùng `ptype: REF` thì ai sẽ thực thi quyền quản lý và kích thước dpool để copy dữ liệu từ biến toàn cục sang message. Cần đảm bảo rằng việc truyền tham chiếu và tham trị được thực hiện một cách an toàn và hiệu quả, tránh các vấn đề về đồng bộ hóa và quản lý bộ nhớ.
-->

<!-- NOTE
Suy xét việc bổ sung thiết kế mới trong mã nguồn thêm 1 dpool hỗ trợ tính năng GDA (Global Data Area) để quản lý các biến toàn cục, đặc biệt là khi sử dụng `ptype: REF` để truyền tham chiếu. Điều này sẽ giúp đảm bảo rằng các task có thể truy cập và sử dụng dữ liệu toàn cục một cách an toàn và hiệu quả, đồng thời tránh các vấn đề về đồng bộ hóa và quản lý bộ nhớ. 

Có thể cân nhắc đưa cho Minh trong việc thực thi.
-->

<!-- TODO
- Bổ sung vào tài liệu thiết kế 1 dpool riêng cho GDA để quản lý các biến toàn cục, đặc biệt là khi sử dụng `ptype: REF` để truyền tham chiếu. Điều này sẽ giúp đảm bảo rằng các task có thể truy cập và sử dụng dữ liệu toàn cục một cách an toàn và hiệu quả, đồng thời tránh các vấn đề về đồng bộ hóa và quản lý bộ nhớ.
- Bổ sung 1 đoạn thông tin trong tài liệu để chỉ rõ quyền quản lý các biến toàn cục và truyền tham chiếu sẽ được thực hiện quản lý bởi ai, tính năng sẽ nằm trong phiên bản nào, 
-->