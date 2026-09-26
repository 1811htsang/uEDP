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

Một lưu ý quan trọng để viết file cấu hình chính là đảm bảo nguyên tắc "Hữu trưởng thứ, Trống lui, Tồn giữ". Nghĩa là nếu xét giữa một tag A (tag cha) và tag B (tag con), tag cha NULL thì tag con nên thụt lùi, tag cha khác NULL thì tag con không thụt lùi. Nếu tag cha NULL mà tag con không thụt lùi thì parser sẽ báo lỗi. Nếu tag cha khác NULL mà tag con thụt lùi thì parser sẽ báo lỗi.

Ví dụ:

```yaml
- task: KID_TASK_A # Giữa task và tsm, task khác NULL nên tsm không thụt lùi
  tsm: # Giữa tsm và id, tsm NULL nên id thụt lùi
    - id: STATE_A_IDLE
```

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

<!-- DEPRECATED - Old TASK
- Bổ sung thêm phần tag include của YAML nâng cao để hỗ trợ việc include các file cấu hình con, ví dụ `!include "signals.yaml"`.
- Bổ sung thêm việc tìm hiểu các giải pháp để hỗ trợ lấy alias từ file include mà không cần phải khai báo lại trong file chính.
#STATUS - DONE
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

<!-- DEPRECATED - Old TASK
Cần rewrite lại phần này tương ứng với các khối phát triển đã có bên nhánh feat.
#STATUS - DONE, đã đủ generic introduction trước khi đi vào chi tiết từng khối syntax.
-->

### Hướng dẫn đọc nhanh

1. Đọc phần quy ước YAML trước để tránh lỗi thụt lề và kiểu dữ liệu.
2. Đọc phần Task để hiểu `tlist`, `task`, `tsm`, `fsm`, `exec` và `steps`.
3. Đọc SII, PPLP, APE, OCE để nắm các block mở rộng bám trực tiếp vào core API.
4. Xem ví dụ tổng hợp ở cuối tài liệu nếu muốn viết cấu hình đầu tiên thật nhanh.

### Bản đồ syntax -> core

| Khối | Ý nghĩa | Syntax chính | Syntax phụ / tùy chỉnh | Core mapping |
| --- | --- | --- | --- | --- |
| Task Norm | Task có trạng thái hoặc xử lý message | `tlist -> tnorm -> tsm/fsm/exec/escal` | `tsm`, `fsm`, `exec`, `escal`, `on_ntry`, `on_actv`, `on_exit`, `on_recv`, `steps`, [`actv`, `kind`, `code`, `func`, `args`] | `uedp_task_norm_create()`, `uedp_task_norm_post_msg()` |
| Task Poll | Task vòng lặp nhẹ, không theo message | `tlist -> tpoll -> exec` | `exec`, `steps`, [`actv`, `kind`, `code`, `func`, `args`], `ability` | `uedp_task_poll_create()`, `uedp_task_poll_set_ability()` |
| SII | Đưa signal từ ISR vào hệ thống | `isr -> to/sig` | `to`, `sig` | `uedp_task_norm_post_isr()`, `uedp_msg_drain_isr_pool()` |
| PPLP | Cấu hình logging pipeline | `pplp -> itnlog -> level/tag/output` | `level`, `tag`, `output.backend`, `output.sink`, `log.timestamp`, `log.msg` | `uedp_itnlog_set_filter()`, `uedp_itnlog_set_output()` |
| APE | Gọi urgent message / priority escalation | `escal -> trigger -> post_urgent` | `mode: slnf`, `mode: non-slnf`, `scope: self`, `keep_queue_order`, `extra_rounds`, `post_urgent` | `uedp_task_norm_post_urgent()`, `uedp_task_norm_set_urgent()` |
| OCE | Service chạy ngoài luồng logic chính | `outexec -> name/handler/context/state` | `name`, `handler`, `context`, `state` | `ocesvc_register()`, `ocesvc_scheduler()` |

Số lượng state trong `tsm`/`fsm` của mỗi `tnorm` (hàng Task Norm ở trên) khớp 1-1 với khai báo `APPCFG_TSM_TASK_{i}_STATE_{j}`/`APPCFG_FSM_TASK_{i}_STATE_{j}` do `kconfigspec.tnorm` và `kconfigspec.usrinp` sinh riêng cho từng task #i — xem mục "Đồng bộ với `kconfigspec.usrinp` / `kconfigspec.tnorm`" bên dưới.

### Task - Tác vụ

Identifier trong tài liệu là `process-syntax` để thuận tiện cho việc đề cập nội dung.

Trong μE-LS, mỗi task được khai báo trong danh sách `tlist`. Một task có thể đi theo một trong ba nhánh chính: `tsm` nếu cần state machine dạng bảng, `fsm` nếu cần dispatch theo handler, hoặc `exec` nếu chỉ cần hành vi tuyến tính hoặc lặp lại.

`task` là định danh logic do PLTF sinh ra từ Kconfig; `tnorm` và `tpoll` là hai kiểu hành vi, không phải hai hệ syntax tách biệt. Các cấu hình Pool, Queue, Timer và Stack vẫn thuộc Kconfig, nên μE-LS chỉ mô tả hành vi và quan hệ giữa task, signal, action.

Tuy nhiên, sự nhập nhằng giữa non-HSMC tnorm (sử dụng `exec`) và tpoll sẽ xảy ra với cú pháp chỉ sử dụng `task`. Do đó, ở tag tổng đại diện cho entry của từng task trong `tlist` sẽ bổ sung thêm 2 loại tag là `tnorm` và `tpoll` để phân biệt rõ ràng.

Về tổng quát, một `tnorm` nên được viết theo cấu trúc sau:

```yaml
project: "uEDP"
tlist:
  - tnorm: TASK_USR
    tsm:
      - id: STATE_USR_IDLE
        trans:
          - sig: KID_SIG_USR_START
            goto: STATE_USR_WAITING
        on_ntry: NULL # Có thể NULL
        on_actv: NULL
        on_exit: NULL
      - id: STATE_USR_WAITING
        trans:
          - sig: KID_SIG_USR_STOP
            goto: STATE_USR_IDLE
        on_ntry:
          steps:
            - actv:
                kind: c_call
                function: printf
                args: ['"[System Task USR] Received START signal, entering WAITING state."']
        on_actv:
          steps:
            - actv: 
                kind: c_stmt
                code: |
                  // Logic xử lý trong trạng thái WAITING
                  printf("[System Task USR] Processing in WAITING state.\n");
                  for (int i = 0; i < 5; i++) {
                    printf("[System Task USR] Loop iteration %d\n", i);
                  }
        on_exit: NULL
```

`tsm` nên được dùng khi `tnorm` cần quản lý vòng đời trạng thái rõ ràng và có thể sinh ra `on_entry`, `on_exit` và `on_active` tự động bởi `pycdscriptor`.

`fsm` nên được dùng khi `tnorm` chỉ cần dispatch theo tín hiệu với state handler trực tiếp hoặc có thể lồng vào TSM để xử lý các hành vi phức tạp hơn.

Ví dụ FSM nên viết theo kiểu sau:

```yaml
project: "uEDP"
tlist:
  - tnorm: KID_TASK_B
    fsm:
      - id: STATE_B_IDLE
        on_recv:
          - sig: KID_SIG_0x12
            goto: STATE_B_BUSY
            steps:
              - actv: 
                  kind: c_call
                  function: printf
                  args: ['"[Task B] Received SIG_0x12, transitioning to BUSY state."']
              - actv: 
                  kind: c_stmt
                  code: |
                    // Logic xử lý trong trạng thái BUSY
                    printf("[Task B] Processing in BUSY state.\n");
                    for (int i = 0; i < 3; i++) {
                      printf("[Task B] BUSY loop iteration %d\n", i);
                    }
      - id: STATE_B_BUSY
        on_recv:
          - sig: KID_SIG_0xAA
            goto: STATE_B_IDLE
            steps:
              - actv: 
                  kind: c_call
                  function: printf
                  args: ['"[Task B] Received SIG_0xAA, transitioning back to IDLE state."']
```

Nếu `tnorm` không cần TSM/FSM thì dùng `exec` để mô tả các hành vi tuyến tính. Đây là lựa chọn phù hợp cho các tác vụ đơn giản.

```yaml
project: "uEDP"
tlist:
  - tnorm: KID_TASK_SIMPLE
    exec:
      - on_sig: SIG_A
        steps:
          - actv: 
              kind: c_call
              function: printf
              args: ['"[Task Simple] Received SIG_A, executing action."']
```

`tpoll` nên đi theo nhịp polling riêng và chỉ khai báo các bước xử lý tuần tự, không gắn với state machine:

```yaml
project: "uEDP"
tlist:
  - tpoll: KID_TASK_POLL
    exec:
      - actv: 
          kind: c_call
          function: printf
          args: ['"[Task Poll] Executing periodic action."']
```

Trong current core, `tpoll` chỉ nên dùng cho logic nhẹ, còn các tác vụ dọn dẹp hệ thống, flush log hoặc đồng bộ nền nên được đẩy sang OCE.

<!-- STATUS
Bộ sinh code Python `pycdscriptor` đã hỗ trợ đầy đủ cho process-syntax thông qua kiểm tra SIL vir-testobj.
Ngoài ra actv-obj-post đã được loại bỏ hoàn toàn khỏi pydscriptor và tài liệu.
-->

Trong thiết kế ban đầu, các action object - `actv-obj` được thiết kế tương thích với tính năng của post_message trong core API, trong một số tài liệu sẽ có naming convention tương ứng là `actv-obj-post`. Tuy nhiên, thông qua kiểm tra SIL vir-testobj Linux, các `actv-obj-post` đã bộc lộ nhược điểm cứng nhắc và không tương thích với các API đa dụng khác, do đó đã được loại bỏ hoàn toàn khỏi `pycdscriptor` và tài liệu. Thay vào đó, các actv-obj được thiết kế để tương thích với cú pháp C-type, cho phép người dùng viết trực tiếp các đoạn mã C trong YAML hoặc gọi chỉ định hàm với parameter tương ứng. Điều này giúp tăng tính linh hoạt và khả năng mở rộng của μE-LS.

#### Các lưu ý khi thiết kế logic với HSMC

Nên vẽ sơ đồ trạng thái trước khi viết YAML để tránh nhầm lẫn giữa các tầng điều phối và logic chuyển trạng thái của tác vụ.

Với TSM trên từng tác vụ, hãy xác định rõ theo thứ tự:

- Tín hiệu đầu vào cần được cover ở `ot_ntry` và `il_ntry` để tránh bỏ sót tín hiệu.
- Tín hiệu đầu ra cần được cover ở `actv` để đảm bảo hành vi logic được thực thi đúng theo thiết kế.
- Các thao tác khác cần thực thi sẽ được cover ở `on_exit` để đảm bảo trạng thái được dọn dẹp đúng cách trước khi chuyển sang trạng thái tiếp theo.
- Các trạng thái cần thao tác khóa chuyển trạng thái sẽ phải sử dụng các tín hiệu được mặc định define trong `uedp_core.h` để tránh các vấn đề lặp lại hoặc bỏ sót tín hiệu trong quá trình chuyển trạng thái.

Với FSM, hãy xác định rõ theo thứ tự:

- Các trạng thái cần có để xây dựng số lượng hàm tương ứng.
- Các thao tác chuyển trạng thái ứng với từng hàm.

Khi đó, TSM sẽ trở thành lớp quản lý chỉ báo trạng thái toàn cục của tác vụ trong khi FSM sẽ trở thành lớp quản lý hành vi logic của từng trạng thái. Việc tách biệt này giúp giảm thiểu sự phức tạp trong việc phát triển và bảo trì hệ thống.

Mục này sẽ liên hệ với tài liệu thiết kế `docs/arch-design.md` để trình bày một cách liền lạc từ kiến trúc thiết kế API C-type đến cú pháp μE-LS, từ đó giúp người dùng dễ dàng hiểu và áp dụng trong việc phát triển hệ thống.

### PPLP - Cấu hình logging pipeline

<!-- STATUS
Hiện tại `pycdscriptor` chưa hỗ trợ sinh cấu hình PPLP, nhưng core đã hỗ trợ đầy đủ các API liên quan đến logging pipeline.
Trong thiết kế thì PPLP có cấu hình riêng biệt với process-syntax nên có thể tích hợp module riêng để sinh code PPLP từ YAML.
-->

<!-- DEPRECATED - Old TASK
Cân nhắc đưa lộ trình hỗ trợ PPLP vào roadmap của μE-LS ở phiên bản 1.1.7/1.1.8 cùng với APE.

# STATUS - Đã chính thức đưa PPLP vào roadmap của μE-LS ở phiên bản 1.1.7/1.1.8, còn đối với APE thì có thể loại bỏ do c_call và c_stmt đã có thể gọi trực tiếp các hàm vượt quyền tạm thời trong core API.
-->

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

<!-- TASK
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

# STATUS - Task này sẽ đưa vào lộ trình phát triển μE-LS ở phiên bản 1.1.7/1.1.8, nên không cần DEPRECATED task này.
```

-->

<!-- DEPRECATED - Old TASK
Loại bỏ toàn bộ cú pháp ISR vì bản thân `process-syntax` đã có thể xử lý syntax C-type với `actv: c_stmt` hoặc `actv: c_call`.
#STATUS - DONE
-->

### APE - Lời gọi vượt quyền tạm thời

<!-- DEPRECATED - Old TASK
Loại bỏ toàn bộ cú pháp ISR vì bản thân `process-syntax` đã có thể xử lý syntax C-type với `actv: c_stmt` hoặc `actv: c_call`.
-->

<!-- REVIEW
Trong thiết kế lõi, chưa tính đến trường hợp `tnorm` gọi APE trong TSM hoặc out-context của `tsm_dispatch()` và cả `pycdscriptor` chưa hỗ trợ sinh code cho APE. 
Do đó, cần review lại tính cần thiết của syntax này với use-case sử dụng trên API C-type gốc trước khi quyết định giữ lại hay loại bỏ hoàn toàn.
-->

<!-- STATUS - IN-PROGRESS
Đề xuất thiết kế mới cho APE như sau:

1. Giữ lại cú pháp APE trong μE-LS nhưng đưa level của syntax lên mức toàn cục cho tác vụ, nghĩa là trước khi bất kỳ logic nào được thực thi, bao gồm cả `tsm`, `fsm` và `exec`, APE sẽ được đăng ký trigger và tác động trước như một interferencer bảo vệ logic lẫn quyền ưu tiên.
2. Cho phép APE trong actv-obj của logic `tsm`, `fsm` và `exec`, nghĩa là người dùng có thể sử dụng với `c_call` và `c_stmt` để gọi APE cho chính mình hoặc cho các tnorm khác, miễn là không bị báo lỗi.
-->

<!-- DEPRECATED - Old TASK
Cân nhắc đưa lộ trình hỗ trợ APE vào roadmap của μE-LS ở phiên bản 1.1.7/1.1.8.

# STATUS - Đã loại bỏ APE khỏi roadmap của μE-LS ở phiên bản 1.1.7/1.1.8, do c_call và c_stmt đã có thể gọi trực tiếp các hàm vượt quyền tạm thời trong core API.
-->

APE hay S-LnF APE là cơ chế được triển khai ở phiên bản 1.1.0 và 1.1.1 để hỗ trợ tnorm có thể gọi các hàm vượt quyền tạm thời (Privilege Escalation) trong môi trường μE(DP)/-OS. Trong μE-LS, APE là khai báo cục bộ theo từng tnorm: mỗi task có thể tự định nghĩa trigger APE cho chính nó, và Core chỉ cung cấp cơ chế thực thi tương ứng qua `uedp_task_norm_post_urgent()` và `uedp_task_norm_set_urgent()`.

Cú pháp khai báo APE trong μE-LS được hỗ trợ chỉ dành cho tnorm nên sẽ không có phần khai báo riêng cho tpoll. Một tnorm có thể đặt APE ngang hàng với `tsm`, `fsm` hoặc `exec`, hoặc đưa vào `actv` như một action để tự kích hoạt APE cho chính nó.

```yaml
escal:
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
  mode: non-slnf
  trigger:
  - on_sig: SIG_CALL_IO_BOOST # Kích hoạt APE khi nhận signal này
    post_urgent: NULL # Không cần tự gửi urgent message mới
  - on_sig: SIG_URGENT_NEXT # Kích hoạt APE khi nhận signal này
    post_urgent: NULL # Không cần tự gửi urgent message mới
```

<!-- REVIEW
Trong thiết kế API C-type hiện tại, `uedp_task_norm_post_urgent` chỉ hỗ trợ gửi non-data urgent message,
do đó, `post_urgent` trong μE-LS cũng chỉ hỗ trợ gửi urgent message không kèm dữ liệu và giữ nguyên style actv-obj-post của actv-obj cũ trên process-syntax.
-->

Ví dụ:

```yaml
project: "uEDP"
tlist:
- tnorm: KID_TASK_USR
  exec:
    - on_sig: SIG_CALL_URGENT
      steps:
        - actv:
            kind: c_stmt
            code: |
              // Logic xử lý khi nhận SIG_CALL_URGENT
              printf("[Task USR] Received SIG_CALL_URGENT, preparing to escalate.\n");
    - on_sig: SIG_EXEC_URGENT
      steps:
        - actv: 
            kind: c_call
            function: printf
            args: ['"[Task USR] Executing urgent action after escalation."']
  escal:
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
- tnorm: KID_TASK_IO
  exec:
  - on_sig: SIG_CALL_IO_BOOST
    steps:
      - actv: 
          kind: c_stmt
          code: |
            // Logic xử lý khi nhận SIG_CALL_IO_BOOST
            printf("[Task IO] Received SIG_CALL_IO_BOOST, preparing to boost IO.\n");
  - on_sig: SIG_EXEC_IO_BOOST
    steps:
      - actv: 
          kind: c_call
          function: printf
          args: ['"[Task IO] Executing IO boost action after escalation."']
  escal:
    mode: non-slnf
    trigger:
      - on_sig: SIG_CALL_IO_BOOST # Kích hoạt APE khi nhận signal này
        post_urgent: NULL # Không cần tự gửi urgent message mới
      - on_sig: SIG_URGENT_NEXT # Kích hoạt APE khi nhận signal này
        post_urgent: NULL # Không cần tự gửi urgent message mới
```

Khi dùng `mode: non-slnf`, tnorm không bắt buộc phải tự gửi một urgent message mới. Mục đích là cho phép chính task đó giữ nhịp xử lý thêm một vòng với queue hiện có trước khi hoàn trả vòng làm việc về cho các task khác, nên trigger thường chỉ cần là một signal nội bộ hoặc một action local do cùng task phát ra. Nếu có urgent message mới, nó sẽ được xử lý theo thứ tự FIFO bình thường, không phải ưu tiên.

> Thống nhất cú pháp
> `trigger` ở cả 2 mode được xem xét làm một danh sách các trigger, mỗi trigger có thể là một signal hoặc một action. Khi trigger được kích hoạt, nếu `post_urgent` không NULL thì sẽ gửi urgent message mới; nếu NULL thì task sẽ tiếp tục xử lý queue hiện tại thêm một vòng nữa.

<!-- DEPRECATED - Old TASK
Cân nhắc loại bỏ toàn bộ cú pháp ISR vì bản thân `process-syntax` đã có thể xử lý syntax C-type với `actv: c_stmt` hoặc `actv: c_call`.
-->

### OCE - Dịch vụ ngoài ngữ cảnh logic

<!-- REVIEW
Hiện tại đang cân nhắc 1 trong 2 hướng:

1. Loại bỏ hoàn toàn syntax cho OCE do bản thân `process-syntax` đã có thể xử lý syntax C-type với `actv: c_stmt` hoặc `actv: c_call`. OCE sẽ được triển khai trực tiếp trong core API, không cần khai báo trong μE-LS.
2. Giữ syntax OCE trong μE-LS để thuận tiện cho việc khai báo các dịch vụ ngoài ngữ cảnh logic, nghĩa là các ocesvc được khai báo sẵn ở out-context, các tnorm có thể register trong context tương ứng thông qua actv-obj.c_call hoặc actv-obj.c_stmt. Cách này sẽ giúp người dùng dễ dàng quản lý các dịch vụ ngoài ngữ cảnh logic mà không cần phải viết code trực tiếp trong core.
-->

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

<!-- DEPRECATED - Old NOTE
Trong μE-LS, cú pháp hiện tại không hỗ trợ việc cho phép chỉ định service tiếp theo được gọi sau khi service hiện tại hoàn tất. Nếu muốn mở rộng, có thể thêm trường `next_service` hoặc `callback` để chỉ định service tiếp theo, nhưng hiện tại chưa có support trong core. Do đó, tính năng này sẽ được xem xét trong các phiên bản tương lai của μE-LS.
-->

<!-- STATUS
Theo các tài liệu review về ocesvc.mexecjn trước đó, tính năng chỉ định service thực thi kế tiếp sẽ không được hỗ trợ hiện tại vì lộ trình thiết kế AOCE (Advanced Out-Context Execution) chưa được triển khai. Do đó, cú pháp μE-LS hiện tại chỉ hỗ trợ khai báo các dịch vụ ngoài ngữ cảnh logic dạng FCFS.
-->

### Template tham chiếu tổng hợp

<!-- DEPRECATED - Old TASK
Merge nhánh feat để đưa logic-testobj làm ví dụ tổng hợp cho template tham chiếu nhanh.

# STATUS - DONE
-->

<!-- TASK - Old STUB
Sẽ thêm nội dung để hướng dẫn người dùng truy cập các logic-testobj do lộ trình bổ sung 1 folder riêng biệt của testobj cho project.
-->

### Phân biệt `act`, `actv` và `steps`

<!-- DEPRECATED - Old TASK
Loại bỏ toàn bộ mục này do không còn dùng `act` và `actv` nữa, chỉ giữ lại `steps` và `actv` trong các khai báo HSMC.
#STATUS - DONE
-->

// DOC

Hiện tại section này sẽ không còn được dùng trong tài liệu chính thức, nhưng vẫn giữ lại để tham khảo cho các phiên bản trước đó để làm căn cứ nếu ở lộ trình phát triển tương lai cần quay lại.

### Quy tắc mapping action và payload

<!-- DEPRECATED - Old TASK
Loại bỏ toàn bộ liên quan đến D2MP vì bản thân `process-syntax` đã có thể xử lý syntax C-type với `actv: c_stmt` hoặc `actv: c_call`.
#STATUS - DONE
-->

// DOC

Hiện tại sub-section về D2MP sẽ không còn được dùng trong tài liệu chính thức, nhưng vẫn giữ lại để tham khảo cho các phiên bản trước đó để làm căn cứ nếu ở lộ trình phát triển tương lai cần quay lại.

```yaml
- actv:
    kind: c_stmt
    code: |
      pal_memrp_report(&memrp_info);
- actv:
    kind: c_call
    function: helper
    args: [arg_a, arg_b]
```

`c_stmt` và `c_call` được giữ nguyên để codegen nhưng không có semantic validation sâu như `post_msg`. Generator phải reject hoặc báo rõ khi built-in action thiếu tham số bắt buộc. Alias phải được resolve trước khi mapping; không dùng alias đã bị `safe_load()` chuyển thành `None` làm payload hợp lệ.

Lưu ý rằng ở thời điểm hiện `args` chưa hỗ trợ để resolve alias trong danh sách, nên cần tránh dùng alias trong `args` nếu không muốn gặp lỗi runtime.

Ngoài ra, trong cấu trúc sử dụng mapping action với `c_stmt` và `c_call` cần đảm bảo tag `kind`, `function`, và `args` được khai báo với indent lùi vào sau `actv` để tránh lỗi YAML. Các trường hợp này cần được kiểm tra kỹ lưỡng trong quá trình codegen để đảm bảo tính nhất quán và tránh lỗi runtime.

<!-- DEPRECATED - Old TASK
100826 - Cân nhắc thay đổi 2 keyword `act` và `actv` để tránh nhầm lẫn.
110826 - Cân nhắc remove `cact` và chỉ dùng `steps` trong `on_recv` để thống nhất cú pháp. ~ Bổ sung task list để thực thi việc sửa đổi này. 
#STATUS - DONE
-->

### Khu vực dữ liệu toàn cục - Global Data Area

Bổ sung thêm phần mô tả về khu vực dữ liệu toàn cục (Global Data Area) trong μE-LS. Khu vực này được sử dụng để lưu trữ các biến và cấu trúc dữ liệu phục vụ tính năng D2MP (Data-to-Message Passing).

```yaml
glbda:
- &gda1
  name: GLOBAL_VAR_1
  type: int
  initial_value: 0
- &gda2
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
- &gda1
  name: GLOBAL_COUNTER
  type: const char*
  initial_value: "msg: hello"

tlist:
- tnorm: TASK_A
  exec:
  - on_sig: SIG_USR
    act:
    - actv: increment_global
      to: TASK_B
      sig: SIG_HELLO
      data: *gda1  # Tham chiếu đến biến toàn cục GLOBAL_COUNTER
      ptype: REF  # Chỉ định truyền tham chiếu, nếu muốn truyền tham trị thì dùng VAL
- tnorm: TASK_B
  exec:
  - on_sig: SIG_HELLO
    act:
    - actv: log # action này thực hiện tự post chính mình với việc truyền VAL để copy dữ liệu từ biến toàn cục
      to: TASK_B
      sig: SIG_LOG
      data: *gda1  # Tham chiếu đến biến toàn cục GLOBAL_COUNTER
      ptype: VAL  # Chỉ định truyền tham trị, nếu muốn truyền tham chiếu thì dùng REF
```

<!-- DEPRECATED - Old TASK
Cân nhắc thiết kế hoặc bổ sung thông tin trong tài liệu để làm rõ khi dùng `ptype: REF` thì ai sẽ thực thi quyền quản lý và kích thước dpool để copy dữ liệu từ biến toàn cục sang message. Cần đảm bảo rằng việc truyền tham chiếu và tham trị được thực hiện một cách an toàn và hiệu quả, tránh các vấn đề về đồng bộ hóa và quản lý bộ nhớ.

#STATUS - DONE
-->

<!-- DEPRECATED - Old TASK
Suy xét việc bổ sung thiết kế mới trong mã nguồn thêm 1 dpool hỗ trợ tính năng GDA (Global Data Area) để quản lý các biến toàn cục, đặc biệt là khi sử dụng `ptype: REF` để truyền tham chiếu. Điều này sẽ giúp đảm bảo rằng các task có thể truy cập và sử dụng dữ liệu toàn cục một cách an toàn và hiệu quả, đồng thời tránh các vấn đề về đồng bộ hóa và quản lý bộ nhớ. 

Có thể cân nhắc đưa cho Minh trong việc thực thi.
#STATUS - DONE
-->

<!-- DEPRECATED - Old TASK
- Bổ sung vào tài liệu thiết kế 1 dpool riêng cho GDA để quản lý các biến toàn cục, đặc biệt là khi sử dụng `ptype: REF` để truyền tham chiếu. Điều này sẽ giúp đảm bảo rằng các task có thể truy cập và sử dụng dữ liệu toàn cục một cách an toàn và hiệu quả, đồng thời tránh các vấn đề về đồng bộ hóa và quản lý bộ nhớ.
- Bổ sung 1 đoạn thông tin trong tài liệu để chỉ rõ quyền quản lý các biến toàn cục và truyền tham chiếu sẽ được thực hiện quản lý bởi ai, tính năng sẽ nằm trong phiên bản nào.

Section này đã được review và cập nhật các task trong task list để thực hiện các thay đổi cần thiết trong mã nguồn và tài liệu từ phiên bản 1.1.6.

#STATUS - DONE
-->

### Cập nhật thông tin từ `kconfigspec` và `pycdscriptor`

<!-- DEPRECATED - Old TASK
Rename lại section này.
#STATUS - DONE
-->

Tầng khai báo Kconfig (`pltf/kconfigspec/usrinp.py` + `pltf/kconfigspec/tnorm.py`, sinh ra `sources/app/kconfig/decl.kconfig`) trước đây chỉ hỏi **một lần duy nhất** "Do you want to use FSM?" / "Do you want to use TSM?" kèm **một số lượng state dùng chung** cho toàn bộ `num_tasks_norm` task đã khai báo. Điều này không khớp với model μE-LS mô tả ở trên: mỗi `tnorm` trong `tlist` tự quyết định dùng `tsm` hay `fsm` (hoặc cả hai, hoặc không dùng cái nào), với số lượng state hoàn toàn độc lập theo độ dài mảng `tsm:`/`fsm:` khai báo riêng cho task đó.

`kconfigspec.usrinp.user_input()` và `kconfigspec.tnorm.task_norm_declaration()` đã được sửa đổi để hỏi và sinh cấu hình **theo từng task**: với mỗi task #i (`i` từ 1 đến `num_tasks_norm`), người dùng được hỏi riêng có dùng FSM không, có dùng TSM không, và nếu có thì bao nhiêu state — kết quả trả về là 4 list (`fsm_flags`, `tsm_flags`, `num_fsm_states_list`, `num_tsm_states_list`), trong đó phần tử thứ `i - 1` ứng với task #i. `task_norm_declaration()` dùng đúng 4 list này để sinh `APPCFG_TSM_TASK_{i}`/`APPCFG_FSM_TASK_{i}` kèm các state con `_STATE_{j}`, với số lượng `j` riêng biệt cho từng task, thay vì dùng chung 1 số `num_tsm_states`/`num_fsm_states` cho tất cả task như bản cũ.

Với PLD/μE-LS, thay đổi này có ý nghĩa: dữ liệu `task_tsm`/`task_fsm` mà `dotcfg_cfp.py` build từ `.config` (xem `pltf-design.md` mục 3.3) giờ có thể ánh xạ 1-1 với độ dài mảng `tsm:`/`fsm:` của từng `tnorm` trong `tlist`, không còn bị giới hạn "cả hệ thống chỉ có 1 số lượng state chung" như trước — một task hoàn toàn có thể vừa dùng TSM vừa dùng FSM cùng lúc (hoặc không dùng cái nào), với số state khác hẳn task còn lại, mà không ảnh hưởng tới phần khai báo của các task khác trong cùng `decl.kconfig`. Đây là điều kiện cần để pipeline sinh code từ μE-LS (xem mục 3.5 `pltf-design.md`, μE-LS Codegen) có thể đọc đúng số lượng state khai báo trong YAML mà không còn bị giới hạn bởi 1 con số cấu hình chung ở tầng Kconfig như trước.

Lưu ý: `kconfigspec` chỉ sinh khung khai báo tên (`APPCFG_TSM_TASK_{i}`, `APPCFG_TSM_TASK_{i}_STATE_{j}`, `APPCFG_FSM_TASK_{i}`, `APPCFG_FSM_TASK_{i}_STATE_{j}`, ...) ở tầng Kconfig — nội dung logic thật của từng state (`trans`, `on_ntry`, `on_actv`, `on_exit`, `on_recv`, `steps`) vẫn đến hoàn toàn từ khai báo `tsm:`/`fsm:` trong μE-LS, không phải từ Kconfig.
