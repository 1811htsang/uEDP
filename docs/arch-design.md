# Tài liệu phân tích thiết kế kiến trúc lõi μEDP

## Kiến trúc thiết kế gốc từ AKEDP

Phiên bản AKEDP ban đầu là tiền đề cho μEDP; thiết kế gốc này đóng góp nhiều ý tưởng cốt lõi (quản lý message pool tĩnh, scheduler dựa trên bitmask, TSM/FSM nhẹ, và cơ chế xử lý ISR) nhưng cũng có một số giới hạn thực tế khi áp dụng cho các nền tảng và yêu cầu thời gian thực khắt khe hơn.

Các đặc trưng chính của thiết kế gốc AKEDP:

- Sử dụng nhiều pool tin nhắn tĩnh: tin nhắn "pure" (header-only), "common" (header + dữ liệu cố định), và "dynamic" (header + heap-allocated payload).
- Quản lý tham chiếu (ref-count) để cho phép chia sẻ message giữa nhiều consumer.
- Scheduler dựa trên ready-mask bitmask (O(1)) để tìm task ưu tiên cao nhất.
- TSM (table-driven) cho transitions có cấu trúc rõ ràng; FSM (function-pointer) cho trạng thái nhẹ, trao đổi con trỏ hàm để đổi hành vi.
- Cơ chế ISR: khuyến nghị ISR chỉ tạo/điền message interface rồi forward/post tới task; có wrapper critical-section cho thao tác an toàn trong ngắt.
- Hệ sinh thái container hỗ trợ (FIFO, ring buffer, log queue) để quản lý các hàng đợi và dữ liệu trong ngữ cảnh ngắt.

Hạn chế của thiết kế gốc AKEDP (tại sao cần cải tiến trong μEDP):

- Heap allocation cho "dynamic" messages gây phân mảnh và không đảm bảo tính định thời (non-deterministic) — vấn đề cho hệ thời gian thực.
- Ref-counting làm code và debugging phức tạp; dễ dẫn tới leak hoặc double-free nếu quản lý sai ở các đường đi ISR/forward.
- Kích thước pool và cấu hình hầu hết cố định ở thời điểm biên dịch, thiếu khả năng điều chỉnh linh hoạt theo nhu cầu của ứng dụng.
- Critical-section có thể còn thô (coarse-grained), làm tăng độ trễ ngắt và giảm khả năng đáp ứng trong hệ có nhiều ISR.
- Một số cổng (ports) giả định các thuộc tính kiến trúc (ví dụ alignment/sizeof(void*) và ordering) dẫn tới vấn đề portability khi chuyển giữa 32-bit/64-bit hoặc giữa MCU và Linux.
- Thiếu cơ chế FIFO ISR-to-core đủ mạnh để cô lập hoàn toàn core khỏi ISR trong mọi tải — nguy cơ overflow nếu ISR quá nhiều tín hiệu trong thời gian ngắn mà không có biện pháp để Core tuần tự hóa và xử lý chúng một cách an toàn.
- Thiếu công cụ chẩn đoán và reporting lỗi trung tâm (ví dụ thống kê pool exhaustion, ref-count mismatch) khiến việc vận hành và debug khó khăn trên hệ thật.

Những bài học rút ra từ hạn chế này đã trực tiếp dẫn tới các quyết định thiết kế trong μEDP (ví dụ: tách rõ hơn PAL/Core, FIFO ISR-safe bên trong core, pool sizing theo words-aligned, và giảm dependency vào heap khi có thể).

## Kiến trúc thiết kế của μEDP

μEDP được chia thành 3 tầng chức năng rõ rệt nhằm đạt mục tiêu "Zero-Touch Porting":

### Application Layer (Tầng Ứng dụng)

Chứa các tác vụ nghiệp vụ do người dùng tự định nghĩa và FSM của ứng dụng. Tầng này chỉ tương tác với Core thông qua bộ API chuẩn như `uedp_post_msg()` hoặc `uedp_timer_set()`. Tầng này không chứa bất kỳ code nào liên quan đến phần cứng hay thanh ghi vi điều khiển, đảm bảo tính độc lập và dễ dàng di chuyển giữa các nền tảng khác nhau.

Lưu ý: Trong thiết kế testing ở thư mục `test/`, các khai báo về task handler, FSM handler, task table, ... đều nằm gọn trong implementation của từng test case để thực hiện testing một cách độc lập và dễ quản lý. Tuy nhiên, khi sử dụng thực tế trên ứng dụng người dùng, các khai báo này nên được đặt trong thư mục `app/` để tách biệt rõ ràng.

### μEDP Core (Tầng Lõi - Bất biến)

Chứa logic thuần túy của mô hình lập trình hướng sự kiện, bao gồm:

- Scheduler: Bộ lập lịch đa nhiệm ưu tiên dựa trên Bitmask (O(1)).
- Message Manager: Quản lý Pool bộ nhớ tĩnh, chống phân mảnh.
- Timer Service: Quản lý danh sách liên kết các bộ định thời phần mềm.
- FSM/TSM Engines: Bộ máy thực thi máy trạng thái.
- Itnlog: Hệ thống logging cho phép thu thập toàn bộ dữ liệu tại thời điểm gọi, sau đó có thể đẩy sang logdp và rprintf để xuất ra nhiều đích khác nhau.

Trong tầng này, core được thiết kế độc lập hoàn toàn với phần cứng nhằm đảm bảo tính di động và dễ dàng tích hợp vào bất kỳ nền tảng nhúng nào. Core sẽ chỉ tương tác với phần cứng thông qua các hàm trừu tượng được cung cấp bởi tầng PAL.

#### Message Manager

Message Manager của μEDP hiện dùng mô hình pool tĩnh kết hợp free-list để tránh phân mảnh và tránh phụ thuộc vào heap trong đường đi xử lý chính. Source hiện có 3 pool chính:

- `BLANK`: message không có payload, dùng cho tín hiệu đơn giản.
- `ALLOC`: message có payload nhỏ hoặc vừa, data buffer tách riêng khỏi header.
- `EXTAL`: message từ interface ngoài core, cũng dùng vùng dữ liệu riêng.

`UEDP_MSG_TYPE_NORM` vẫn được định nghĩa trong enum để dự phòng cho các biến thể pool sau này, nhưng trong implementation hiện tại chưa được dựng thành một pool riêng trong `uedp_msg_pool_init()`.

Ngoài ra còn có một hàng đợi riêng cho tín hiệu ISR. Hàng đợi này không đi qua message pool thông thường mà được khởi tạo bằng FIFO riêng để giảm rủi ro khi ghi nhận tín hiệu từ ngữ cảnh ngắt.

Một message trong core mang các trường chính: `src_task_id`, `des_task_id`, `sig`, `type`, `ref_count`, `data`, metadata `interface`, và `timestamp` khi debug flag bật. Khi cấp phát, `uedp_msg_alloc()` sẽ chọn pool phù hợp theo kích thước payload; khi giải phóng, `uedp_msg_free()` trả message về đúng pool tương ứng.

#### Task Manager

Task Manager là lớp điều phối task message-driven và poll-driven.

- Task message-driven được khai báo bằng `task_norm_t`, gồm `id`, `pri`, handler, FIFO nội bộ và buffer FIFO.
- Task poll-driven được khai báo bằng `task_poll_t`, gồm `id`, `ability`, handler.
- Khi `uedp_task_norm_create()` chạy, core đếm danh sách task đến phần tử `UEDP_TASK_NORM_EOT_ID` rồi tự khởi tạo FIFO nội bộ cho từng task bằng `UEDP_TASK_MSG_QUEUE_SIZE` slot con trỏ.
- Scheduler hiện tại là ưu tiên bitmask: lấy priority cao nhất đang ready, xử lý đúng một message, rồi quay lại vòng lặp lần sau.

Đường đi xử lý của task hiện tại được lưu tạm trong các biến nội bộ để phục vụ API lấy ngữ cảnh hiện tại và các module khác như itnlog hoặc timer.

#### Timer Service

Timer Service dùng pool cố định `UEDP_TIMER_MAX_NODES` node và danh sách liên kết cho free-list/active-list. Mỗi node lưu `des_task_id`, `sig`, `type`, `period`, `counter`, `is_active`.

- `uedp_timer_init()` chỉ cần dựng lại free-list.
- `uedp_timer_set()` tạo hoặc cập nhật timer theo cặp task ID + signal.
- `uedp_timer_remove()` xóa timer khỏi active-list và trả node về free-list.
- `uedp_timer_tick()` được gọi trong ngữ cảnh ngắt tick để giảm counter; khi hết hạn sẽ phát sinh message ISR sang task đích.

Timer hiện hỗ trợ 2 loại: one-shot và periodic. Giá trị `period` được quy đổi từ milliseconds sang tick theo `UEDP_TIMER_TICK`.

#### ISR Bridge

μEDP tách riêng đường đi từ ISR vào core bằng một FIFO nội bộ cho tín hiệu ISR. Thay vì tạo message ngay trong ngắt, ISR chỉ đăng ký cặp task ID và signal vào FIFO, còn `uedp_task_scheduler()` sẽ gọi `uedp_msg_drain_isr_pool()` ở đầu chu kỳ để rút các tín hiệu này vào luồng xử lý bình thường.

Cách làm này giữ đường đi trong ISR ngắn hơn, giảm nguy cơ tranh chấp và giúp core vẫn giữ được tính độc lập với phần cứng cụ thể.

### PAL - Platform Abstraction Layer (Tầng Trừu tượng)

Tầng này đóng vai trò là cầu nối giữa Core và phần cứng cụ thể. PAL cung cấp các dịch vụ hệ thống như quản lý ngắt, thao tác bit, và các hàm tiện ích khác mà Core yêu cầu để hoạt động. Mỗi nền tảng sẽ có một triển khai riêng của PAL, nhưng tất cả đều tuân thủ cùng một giao diện chung để đảm bảo tính nhất quán.

Trong đó `pal_core.h` chứa các khai báo chung cho toàn bộ PAL, bao gồm các hàm dịch vụ hệ thống như `pal_enter_critical()`, `pal_exit_critical()`, và `pal_get_highest_priority()`. Các dịch vụ này sẽ được triển khai khác nhau tùy theo nền tảng (ví dụ: trên STM32 sẽ sử dụng ngắt để quản lý critical section, trong khi trên Linux sẽ sử dụng mutex). Điều này giúp Core hoàn toàn không phải quan tâm đến chi tiết phần cứng, từ đó đạt được mục tiêu "Zero-Touch Porting".

#### Chuỗi xuất log: xprintf, rprintf và logdp

μEDP tách phần xuất log thành ba lớp để tránh Core phụ thuộc trực tiếp vào `printf` và đồng thời cho phép một log entry được phân phối tới nhiều backend:

- `xprintf`: lớp formatter ở mức ký tự, cung cấp các hàm như `xprintf()`, `xfprintf()` và `xsprintf()` để ghép chuỗi theo format ổn định trên nhiều nền tảng.
- `rprintf`: lớp redirect print ở tầng PAL, nhận một `uedp_itnlog_entry_t`, định dạng entry thành chuỗi hiển thị và đẩy chuỗi đó ra backend qua `write` hoặc `putc`.
- `logdp`: lớp dispatch của PAL, giữ một bảng callback có kiểu `void (*)(uedp_itnlog_entry_t*)` để fan-out cùng một log entry ra nhiều đích như UART, console, file hoặc trace buffer.

Luồng dữ liệu chuẩn trong kiến trúc hiện tại là:

1. Core hoặc application tạo ra `uedp_itnlog_entry_t`.
2. `pal_logdp_dispatch()` phát entry đó tới toàn bộ callback đã đăng ký bằng `pal_logdp_register()`.
3. Mỗi callback thường giữ một `pal_rprintf_service_t` riêng, copy entry vào `service->entry` rồi gọi `pal_rprintf_flush_entry()`.
4. `pal_rprintf_flush_entry()` gọi `xfprintf()` để format chuỗi log và xuất ra backend đã được khởi tạo trước.

Thiết kế này có hai lợi ích chính:

- Tách biệt trách nhiệm: `xprintf` lo format, `rprintf` lo chuyển đổi entry thành chuỗi xuất, còn `logdp` lo phân phối.
- Mở rộng backend: cùng một log entry có thể đồng thời xuất ra UART, màn hình debug và file log mà không cần sửa logic của Core.

Lưu ý khi tích hợp:

- `pal_logdp_dispatch()` chỉ truyền con trỏ entry, nên callback không nên giữ nguyên con trỏ đó nếu entry chỉ có vòng đời ngắn; tốt nhất là copy nội dung vào service nội bộ trước khi flush.
- `pal_rprintf_service_t` cho phép `init` là `NULL` nếu backend đã được khởi tạo sẵn từ BSP hoặc application.
- `pal_rprintf_flush_entry()` chỉ thực sự xuất dữ liệu khi `is_ready()` trả về `true`.
- Định dạng mặc định của `rprintf` là một dòng có timestamp, task ID, signal ID và message, được tạo bằng `xfprintf()` thay vì ghép chuỗi thủ công.

## IV. Logic thiết kế chi tiết

### Quản lý bộ nhớ tin nhắn

Hệ thống quản lý bộ nhớ sử dụng bộ nhớ tĩnh để chống phân mảnh. Core sẽ tự động điều phối việc cấp phát bộ nhớ dựa theo kiến trúc sử dụng được trả về trong `sizeof(void*)` của từng message.

Ví dụ:

- Trên kiến trúc Linux 64-bit, `sizeof(void*)` trả về 8.
- Trên kiến trúc STM32 32-bit, `sizeof(void*)` trả về 4.

Dựa theo kiến trúc này, khi người dùng muốn khai báo tùy chỉnh kích thước message pool thì cần đảm bảo kích thước tuân thủ theo quy tắc `sizeof(void*) * 2^n` để đảm bảo hiệu quả trong việc quản lý bộ nhớ và tránh lãng phí không gian lưu trữ. Core sẽ tự động điều phối thông qua cấu hình PAL, giúp tối ưu hóa hiệu suất và sử dụng bộ nhớ một cách hiệu quả.

Trong đó, Core được thiết kế với 4 loại pool sau:

- `BLANK`: Mặc định là 8 đơn vị, mỗi đơn vị có kích thước phụ thuộc vào `sizeof(uedp_msg_t)`, dùng để cấp phát các message không có payload, phù hợp cho các tín hiệu đơn giản.
- `ALLOC`: Kích thước mặc định là 16 đơn vị, mỗi đơn vị có kích thước phụ thuộc vào `sizeof(void*) * 2u`, dùng để cấp phát các message có payload, cho phép truyền tham trị và truyền tham chiếu một cách linh hoạt.
- `EXTAL`: Kích thước mặc định là 16 đơn vị, mỗi đơn vị có kích thước phụ thuộc vào `sizeof(void*) * 4u`, dùng để cấp phát các message từ bên ngoài core, cho phép cô lập tài nguyên để Core xử lý trước khi truyền vào hệ thống và các tác vụ được đăng ký để nhận các message này.
- `ISR`: Kích thước mặc định là 16 đơn vị, mỗi đơn vị có kích thước phụ thuộc vào `sizeof(uedp_msg_isr_t)`, dùng để ISR truyền tín hiệu vào hệ thống trên FIFO, giúp cô lập tín hiệu từ ISR và đảm bảo an toàn khi truyền vào hệ thống.

### ISR-Safe Injection

Để đảm bảo an toàn khi truyền tín hiệu từ ISR vào hệ thống, μEDP bổ sung một FIFO nội bộ bên trong Core để lưu trữ các tín hiệu từ ISR. Khi có ngắt (ví dụ: UART, Timer), PAL sẽ đẩy tín hiệu vào FIFO này. Core sẽ "drain" (rút dữ liệu) từ FIFO này vào các Task Queue ở đầu mỗi chu kỳ Scheduler. Cơ chế này giúp loại bỏ hoàn toàn việc Core phải biết về ISR, đồng thời đảm bảo an toàn và hiệu quả khi truyền tín hiệu từ ISR vào hệ thống.

Nhằm đảm bảo tính an toàn và tránh tranh chấp tài nguyên, việc drain FIFO này được thực hiện trong critical section, đảm bảo rằng quá trình này không bị gián đoạn bởi các tác vụ khác hoặc ISR khác. Điều này giúp duy trì tính nhất quán của dữ liệu và đảm bảo rằng các tín hiệu từ ISR được xử lý một cách an toàn và hiệu quả trong hệ thống μEDP.

Khi một timer hết hạn, `uedp_timer_tick()` cũng đi qua cơ chế này bằng cách gọi `uedp_task_norm_post_isr()` để đưa tín hiệu của timer về task đích theo cùng một luồng xử lý.

### Data-to-Message passing

Dựa theo thiết kế bộ nhớ quản lý tin nhắn, nhằm đảm bảo việc có thể truyền toàn bộ nội dung của một message vào payload của message khác một cách an toàn và hiệu quả, μEDP sử dụng cơ chế Data-to-Message passing. Cơ chế này cho phép người dùng truyền dữ liệu trực tiếp vào payload của message mà không cần phải lo lắng về việc quản lý bộ nhớ hoặc phân mảnh.

Trong đó, nếu kích thước của dữ liệu nhỏ hơn kích thước đã khai báo của pool, Core cung cấp API là `uedp_msg_set_data_val` để truyền dữ liệu trực tiếp vào payload của message. Nếu kích thước của dữ liệu lớn hơn kích thước đã khai báo của pool, người dùng có thể sử dụng API `uedp_msg_set_data_ref` để truyền địa chỉ của dữ liệu vào payload của message.

Do đó cần lưu ý rằng đối với việc truyền tham chiếu thì nên bổ sung 1 FIFO toàn cục để lưu trữ các tham chiếu này nhằm tránh việc truyền trực tiếp địa chỉ của biến cục bộ vào payload của message, điều này có thể dẫn đến lỗi truy cập bộ nhớ khi message được xử lý sau khi biến cục bộ đã hết phạm vi.

Khi thực hiện lấy dữ liệu từ truyền tham chiếu thì người dùng có thể tham khảo cách khai báo trong `test02` như sau:

```c
static const char* data_a_to_b = "Hello from Task A!";
uintptr_t received_addr = (uintptr_t)(*(char**)(msg->data));
char* final_str = *(char**)received_addr;
```

Trong đó `uintptr_t` cho phép lấy địa chỉ không cần xét đến kiểu dữ liệu, giúp đảm bảo tính linh hoạt và an toàn khi truyền tham chiếu trong message mà không phụ thuộc vào kiến trúc hoặc kiểu dữ liệu cụ thể.

Ở đây, tài liệu lấy ví dụ về việc truyền tham chiếu một chuỗi ký tự từ Task A sang Task B thông qua message. Do bản thân `data_a_to_b` là con trỏ cấp 2 nên nếu chỉ sử dụng `char* received_str = *(char**)(msg->data)` thì chỉ lấy được thông tin địa chỉ con trỏ `data_a_to_b` mà không lấy được nội dung của chuỗi ký tự.

Do đó, cần phải sử dụng thêm một bước để lấy được nội dung thực sự của chuỗi ký tự thông qua việc giải tham chiếu hai lần như trong ví dụ trên. Trong thực tế sử dụng thì người dùng sẽ tùy thuộc vào kiểu dữ liệu cụ thể mà có cách giải tham chiếu phù hợp để lấy được nội dung thực sự từ payload của message khi sử dụng cơ chế truyền tham chiếu này.

### TSM / FSM Control

Trong μEDP, mỗi Task (Active Object) không chỉ là một hàm xử lý mà là một thực thể có "trí nhớ". Để quản lý trí nhớ này, hệ thống cung cấp hai cấp độ máy trạng thái:

- TSM (Macro-level): Quản lý các chế độ vận hành lớn (Operational Modes).
- FSM (Micro-level): Quản lý logic nghiệp vụ chi tiết (Functional Logic).

#### TSM - Task State Machine

TSM được thiết kế theo mô hình Table-Driven (Dựa trên bảng) để đảm bảo tính minh bạch và đoán định được (Deterministic).

##### Logic thiết kế

TSM tách biệt hoàn toàn giữa Dữ liệu cấu hình (nằm trong Flash) và Dữ liệu vận hành (nằm trong RAM):

- `tsm_state_desc_t` (Bộ mô tả trạng thái): Chứa ID, hàm on_entry, hàm on_exit và một mảng các transitions.
- `tsm_trans_t` (Bảng chuyển trạng thái): Định nghĩa: "Nếu đang ở trạng thái X, nhận tín hiệu Y -> Thực hiện hàm Z -> Nhảy sang trạng thái K".
- `uedp_tsm_t` (Đối tượng quản lý): Lưu trữ trạng thái hiện tại (cur_state) và trạng thái trước đó (prev_state).

##### Cơ chế hoạt động

- Tự động hóa Entry/Exit: Khi thực hiện `tsm_trans`, Core tự động gọi hàm thoát của trạng thái cũ và hàm vào của trạng thái mới. Điều này đảm bảo tài nguyên (như Timer) luôn được dọn dẹp sạch sẽ.
- Cơ chế "Stay" & "Back":
  - STAY: Thực thi logic nhưng không đổi trạng thái (tránh lặp lại Entry/Exit vô ích).
  - BACK: Tự động quay lại trạng thái trước đó nhờ biến prev_state, giải quyết bài toán "State Explosion".
- Tra cứu O(1): Sử dụng 16-bit ID giúp tốc độ chuyển trạng thái đạt mức tối đa của phần cứng.

Có thể tham khảo thiết kế chương trình mẫu trong `test01` để thấy rõ cách sử dụng TSM trong μEDP, nơi TSM được sử dụng để quản lý các chế độ vận hành của Task một cách hiệu quả và linh hoạt.

#### FSM - Finite State Machine

FSM được thiết kế theo mô hình Pointer-Swapping (Tráo đổi con trỏ) để đạt được sự linh hoạt tối đa.

##### Cấu trúc dữ liệu

- state_handler: Một con trỏ hàm nhận tham số là uedp_msg_t*.
- uedp_fsm_t: Chỉ chứa một biến duy nhất là con trỏ đến hàm trạng thái hiện tại.

##### Đặc điểm vận hành

- Tính cơ động: Cho phép thay đổi logic xử lý ngay lập tức chỉ bằng một phép gán con trỏ.
- Dispatch trực tiếp: Scheduler gọi fsm_dispatch, Core sẽ thực thi ngay hàm mà con trỏ đang trỏ tới.
- Phù hợp với Logic tạm thời: Dùng cho các chuỗi hành động ngắn hạn như giải mã giao thức (UART parsing) hoặc Menu giao diện.

Có thể tham khảo thiết kế chương trình mẫu trong `test03` để thấy rõ cách sử dụng FSM trong μEDP, nơi FSM được sử dụng để quản lý logic giải mã giao thức UART một cách linh hoạt và hiệu quả.

Trong `test03` FSM được thiết kế với mỗi hàm là state_handler là 1 trạng thái. Mỗi trạng thái đều có 3 tín hiệu là `UEDP_FSM_SIG_INIT`, `UEDP_FSM_SIG_ENTRY`, `UEDP_FSM_SIG_EXIT` để quản lý vòng đời của trạng thái, sau đó mới đến các tín hiệu nghiệp vụ khác. Khi có sự kiện chuyển trạng thái thì sẽ thực hiện theo thứ tự là `EXIT` -> `ENTRY` để đảm bảo rằng tài nguyên được dọn dẹp sạch sẽ trước khi vào trạng thái mới.

Lưu ý rằng trong thiết kế của `test03`, FSM của các tác vụ luôn được khởi tạo vào `state_idle`, chỉ có các `state_idle` mới chứa tín hiệu `UEDP_FSM_SIG_INIT` để thực hiện các thao tác khởi tạo FSM, sau đó phụ thuộc vào tín hiệu bắt đầu từ người dùng mà sẽ chuyển sang `state_active` để thực hiện các chức năng chính của bài test. Điều này giúp đảm bảo rằng FSM luôn được khởi tạo đúng cách và có thể hoạt động một cách hiệu quả ngay khi nhận được tín hiệu bắt đầu từ người dùng.

Ngoài ra thì đối với trường hợp looping của một trạng thái thì có thể xử lý thông qua việc calling isolation - bỏ mặc trạng thái không gọi tới. Ví dụ trong `test03`:

```c
void usr_state_active(uedp_msg_t* msg) {
  switch (msg->sig) {
    case UEDP_FSM_SIG_EXIT:
      printf("[USR] Exiting ACTIVE state...\n");
      break;
    case UEDP_FSM_SIG_ENTRY:
      printf("[USR] Entering ACTIVE state. System is now active.\n");
      // Thực hiện gửi SIG_USR_START tới task A để kích hoạt chuỗi hành động
      uedp_msg_t* msg_to_a = uedp_msg_alloc(TASK_NORM_A_ID, SIG_USR_START, 0);
      uedp_task_norm_post_msg(TASK_NORM_A_ID, msg_to_a);
      printf("[USR] Sent START signal to Task A. Waiting for further signals...\n");
      break;
    case SIG_USR_STOP:
      printf("[USR] Received STOP signal. Transitioning to IDLE state...\n");
      uedp_fsm_go_next(&fsm_usr, usr_state_idle); 
      /**
       * @brief Có thể dùng uedp_fsm_go_back(&fsm_usr) để quay lại trạng thái trước đó, 
       *        nhưng ở context này thì go_next sẽ trực quan hơn 
       *        để thể hiện rõ ràng việc chuyển đổi trạng thái từ ACTIVE về IDLE 
       *        khi nhận được tín hiệu STOP.
       */
      break;
    default:
      printf("[USR] Encountered unexpected signal in ACTIVE state: %x\n", msg->sig);
      break;
  }
}
```

Khi ở `state_active` và truyền tín hiệu qua tác vụ A thì FSM của TASK_USR trở thành loop vì không gọi tới. Điều này cho phép FSM của TASK_USR vẫn duy trì trạng thái `state_active` và có thể tiếp tục nhận và xử lý các tín hiệu khác mà không bị gián đoạn bởi việc chuyển trạng thái, đồng thời đảm bảo rằng tài nguyên được quản lý một cách hiệu quả trong suốt quá trình hoạt động của trạng thái này.

Một lưu ý khác cần để tâm trong `test03` khi tác vụ A nhận `SIG_TSK_B_TO_A` thì sẽ gọi `uedp_fsm_go_next(&fsm_a, task_a_state_idle)` để chuyển trạng thái của tác vụ A về `state_idle`. Ở đây người dùng hoàn toàn có thể sử dụng `uedp_fsm_go_back(&fsm_a)` để quay lại trạng thái trước đó. Tuy nhiên trong context này thì `go_next` sẽ trực quan hơn để thể hiện rõ ràng việc chuyển đổi trạng thái từ `state_active` về `state_idle` khi nhận được tín hiệu `SIG_TSK_B_TO_A`, điều này giúp cho code dễ đọc và dễ hiểu hơn, đồng thời vẫn đảm bảo rằng FSM của tác vụ A được quản lý một cách hiệu quả và có thể hoạt động một cách linh hoạt trong suốt quá trình xử lý tín hiệu.

#### Phối hợp giữa TSM và FSM

μEDP khuyến khích người dùng sử dụng mô hình lồng nhau để tối ưu hóa code, trong đó:

- Lớp bảo vệ TSM (Vỏ):
  - Xác định Task đang ở chế độ nào (ví dụ: MODE_NORMAL, MODE_CONFIG, MODE_ALARM).
  - Nếu một tin nhắn đến gây ra sự thay đổi chế độ, TSM sẽ thực hiện chuyển đổi và quản lý các dịch vụ hệ thống (như bật/tắt các Polling Task liên quan).
- Lớp thực thi FSM (Lõi):
  - Nằm bên trong các hàm xử lý của TSM.
  - Thực hiện các phép tính toán, xử lý dữ liệu từ tin nhắn và tương tác với phần cứng.

Qua các vòng thảo luận và design - debug trên Linux, thiết kế phối hợp giữa TSM và FSM cũng bổ sung các chốt an toàn như sau:

- Lock Splitting: Trong hàm tsm_trans, các lệnh khóa ngắt (`pal_enter_critical`) chỉ bao bọc việc thay đổi con trỏ. Các hàm logic (`on_entry/exit`) được gọi ngoài vùng găng để tránh Deadlock khi người dùng gọi tiếp các API khác (như `timer_set`).
- Atomic Pointer Swap: Việc thay đổi trạng thái được đảm bảo tính nguyên tử, không bị ngắt quãng bởi các luồng khác.
- RTC (Run-to-Completion): Đảm bảo một sự kiện trạng thái được xử lý xong xuôi trước khi Task nhận sự kiện tiếp theo, loại bỏ Race Condition ở mức logic.

### Quản lý dãy tín hiệu

μEDP sử dụng một hệ thống quản lý tín hiệu (Signal Management) để đảm bảo rằng các tín hiệu được xử lý một cách an toàn và hiệu quả trong hệ thống. Hệ thống này bao gồm:

- `TASK_NORM` nằm trong dãy `0xEx` với 16 đơn vị, trong đó đã được định nghĩa sẵn 6 TASK_NORM nội bộ là `TIM`, `IF`, `SYS`, `DBG`, `USR`, `IDLE` để phục vụ cho các chức năng hệ thống và người dùng. Bổ sung thêm 1 task `EOT` (End Of Table) với ID `0xEF` để đánh dấu kết thúc dãy tín hiệu TASK_NORM, giúp Core dễ dàng xác định được phạm vi của các tín hiệu này.
- `TASK_POLL` nằm trong dãy `0xDx` với 8 đơn vị, trong đó đã được định nghĩa sẵn 4 TASK_POLL nội bộ là `WDG`, `SYSLF`, `MEMRP`, `IDLE` để phục vụ cho các chức năng hệ thống và người dùng. Bổ sung thêm 1 task `EOT` (End Of Poll) với ID `0xDF` để đánh dấu kết thúc dãy tín hiệu TASK_POLL, giúp Core dễ dàng xác định được phạm vi của các tín hiệu này.
- `TASK_PRI` nằm trong dãy `0xCx` với 16 đơn vị nhằm xác định mức độ ưu tiên của các tác vụ, trong đó đã được định nghĩa sẵn 16 mức độ ưu tiên từ `UEDP_TASK_PRI_LEVEL_0` (thấp nhất) đến `UEDP_TASK_PRI_LEVEL_15` (cao nhất) để phục vụ cho việc lập lịch của hệ thống. Lưu ý rằng trong mỗi TASK_NORM, Core khuyến khích việc sử dụng các mức độ ưu tiên khác nhau, nếu tất cả các TASK_NORM đều sử dụng cùng một mức độ ưu tiên thì Core sẽ gặp lỗi xử lý tín hiệu, việc này sẽ được cân nhắc bổ sung thêm cơ chế kiểm tra lỗi này trong tương lai để đảm bảo tính ổn định của hệ thống.
- `FSM_SIG` nằm trong dãy `0xBx` với 16 đơn vị nhằm xác định các tín hiệu đặc biệt dành riêng cho việc quản lý trạng thái trong FSM, trong đó đã được định nghĩa 4 tín hiệu là `ENTRY`, `EXIT`, `INIT`, `LOOP` để phục vụ cho việc quản lý vòng đời của các trạng thái trong FSM.
- `TSM_SIG` nằm trong dãy `0xAx` với 16 đơn vị nhằm xác định các tín hiệu đặc biệt dành riêng cho việc quản lý trạng thái trong TSM, trong đó đã được định nghĩa 4 tín hiệu là `ENTRY`, `EXIT`, `INIT` để phục vụ cho việc quản lý vòng đời của các trạng thái trong TSM.
- `TSM_STATE` nằm trong dãy `0xAFx` với 16 đơn vị nhằm xác định các trạng thái đặc biệt dành riêng cho việc quản lý trạng thái trong TSM, trong đó đã được định nghĩa 4 trạng thái là `BACK`, `STAY`, `RESET` để phục vụ cho việc quản lý vòng đời của các trạng thái trong TSM.

Lưu ý rằng với mỗi dãy tín hiệu đều đảm bảo có khai báo offset để khi lấy tín hiệu xử lý theo chỉ số của pool hay các vấn đề liên quan đến việc quản lý tín hiệu thì Core có thể dễ dàng xác định được loại tín hiệu và xử lý một cách chính xác.

Khi khai báo bổ sung các tín hiệu mới thì người dùng không cần phải tự cấu hình lại offset vì offset này chỉ ảnh hưởng lên việc quản lý tín hiệu trong nội bộ Core, còn đối với người dùng thì chỉ cần tuân thủ theo dải tín hiệu đã được định nghĩa sẵn để đảm bảo rằng các tín hiệu được quản lý một cách chính xác và hiệu quả trong hệ thống.

### Itnlog - Hệ thống logging

Itnlog là tầng logging nội bộ của μEDP, được thiết kế để thay thế cho cách debug bằng `printf` rải rác trong luồng xử lý. Mục tiêu của itnlog là cung cấp một đường ghi log nhất quán, có thể lọc theo mức độ và tag, đồng thời dễ thay đổi đích xuất log theo từng nền tảng mà không làm Core phụ thuộc trực tiếp vào stdio.

Về mặt thiết kế, itnlog đóng vai trò là một lớp ghi nhận sự kiện nhẹ và tách biệt khỏi luồng xử lý chính:

- Core chỉ ghi log vào bộ đệm nội bộ, không gọi `printf` trực tiếp.
- Đích xuất log được đưa ra ngoài qua callback `uedp_itnlog_set_output()`, nên cùng một Core có thể xuất log ra console, UART, file hoặc backend debug khác.
- Khi cần debug cục bộ trên Linux, callback có thể bọc `printf` hoặc `fputs` rồi `fflush(stdout)` để bảo đảm log xuất ngay.
- Khi dùng trên nền tảng nhúng, callback có thể chuyển sang UART, semihosting, file log hoặc cơ chế trace riêng mà không phải sửa Core.

Thiết kế này giúp tránh việc rải `printf` trong logic nghiệp vụ, giảm phụ thuộc vào thư viện chuẩn ở Core, và giữ cho đường đi thời gian thực ổn định hơn khi cần ghi log với tần suất cao.

Trong thiết kế, itnlog không hỗ trợ flush khi gọi `uedp_itnlog_clear()` vì mục đích của hàm này là xóa log khỏi bộ đệm nội bộ và reset trạng thái thống kê, chứ không phải để xuất log ra ngoài. Việc flush log nên được thực hiện trong `uedp_itnlog_dump()` khi rút log ra và gửi đến callback, đảm bảo rằng chỉ những log đã được xử lý và lọc mới được xuất ra ngoài, giúp tối ưu hiệu suất và tránh việc xuất log không cần thiết.

Do đó, `uedp_itnlog_dump()` sẽ nên được đặt ngoài scheduler hoặc trong một task polling riêng để đảm bảo rằng việc xuất log không ảnh hưởng đến đường đi thời gian thực của các task chính, đồng thời vẫn đảm bảo rằng log được xuất ra một cách hiệu quả và có thể kiểm soát được thông qua các bộ lọc đã thiết lập.

Thiết kế này được gọi là Out-Context Execution (OCE), giúp đảm bảo 3 nguyên tắc sau:

1. Bảo vệ nguyên tắc RTC (Run-to-Completion): Các task chính không bị gián đoạn bởi việc xuất log, tránh làm tăng độ trễ xử lý.
2. Tận dụng nhịp nghỉ của CPU (Idle Time Utilization): Log được xuất ra trong các khoảng thời gian CPU không bận rộn, giúp tối ưu hiệu suất.
3. An toàn cho ngắt chồng ngắt (ISR Safety): Việc xuất log không xảy ra trong ngữ cảnh ISR, tránh rủi ro tranh chấp tài nguyên hoặc deadlock.

#### Mô hình hoạt động

Itnlog vận hành theo chuỗi sau:

1. `uedp_itnlog_init()` khởi tạo ring buffer nội bộ để lưu log entry.
2. `uedp_itnlog_log()` ghi từng entry vào bộ đệm nội bộ theo ngữ cảnh đang chạy.
3. `uedp_itnlog_dump()` rút toàn bộ entry ra, áp dụng bộ lọc level/tag, rồi xuất ra callback do người dùng cung cấp.

Về mặt lưu trữ, entry hiện tại là một cấu trúc nhỏ chứa `level`, `tag`, `task_id`, `msg_sig`, `msg`, `tmstmp` và `hash`. Bộ đệm dùng ring buffer cố định có kích thước `UEDP_ITNLOG_MAX_LOG_ENTRIES`, vì vậy khi số entry được ghi vượt ngưỡng thì logger sẽ tự dump trước khi ghi tiếp.

Khi dump, dòng log được ghép theo mẫu:

`[ITNLOG] tmstmp task_id msg_id msg`

Trong đó `task_id` và `msg_id` được xuất ở dạng hex, còn `tmstmp` là giá trị số nguyên không kèm hậu tố đơn vị. `msg_id` chính là `msg_sig` của message hiện tại tại thời điểm log được ghi.

#### Bộ lọc và ngữ nghĩa tag

Itnlog hỗ trợ lọc theo mức độ log và theo tag:

- `itnlog_filter_level` quyết định entry nào đủ điều kiện xuất.
- `itnlog_filter_tag` cho phép lọc theo module, ví dụ `TSK`, `MSG`, `FSM`, `TSM`, `TIM`.
- Nếu tag filter là `NULL`, itnlog hiểu là không lọc theo tag và chấp nhận mọi entry hợp lệ.

Thiết kế này giúp việc bật hoặc tắt log theo nhóm chức năng trở nên rõ ràng hơn khi debug, đặc biệt trong các test tích hợp như test 04. Do callback output chỉ nhận một chuỗi đã format sẵn, phần format log được cố định trong `uedp_itnlog_dump()` thay vì để người dùng tự ghép chuỗi ở callback.

#### Đích xuất log

Itnlog không ràng buộc đích xuất log vào một backend cố định. Core chỉ gọi callback `uedp_itnlog_set_output()` để chuyển một dòng log đã format sẵn ra tầng ứng dụng hoặc tầng PAL. Cách này cho phép cùng một logic logging có thể xuất ra console, UART, file, hoặc giao diện debug tùy nền tảng, thay vì phụ thuộc vào các lời gọi `printf` trong từng handler.

Khi dùng trên Linux, callback nên là một wrapper nhận `const char*` rồi in chuỗi đó ra terminal và flush ngay sau khi xuất để tránh log bị giữ trong buffer của `stdout` cho đến khi chương trình kết thúc.

#### Hành vi nội bộ cần lưu ý

- `uedp_itnlog_log()` lấy `task_id` từ tác vụ hiện tại và lấy `msg_sig` từ message hiện tại, nên nó chỉ an toàn khi scheduler đang xử lý một message hợp lệ.
- `uedp_itnlog_dump()` không chỉ in log mà còn xóa log khỏi ring buffer và reset trạng thái thống kê nội bộ.
- `uedp_itnlog_set_tag(NULL)` tương đương với tắt lọc theo tag.

### Priority Escalation - Tăng ưu tiên tạm thời

Về mặt thiết kế ban đầu, μEDP chỉ có tổng số mức ưu tiên là 16, được chia đều cho tất cả các TASK_NORM. Mỗi TASK_NORM được ràng buộc đảm bảo mức ưu tiên duy nhất để tránh lỗi xử lý tín hiệu. Tuy nhiên, với trường hợp 1 TASK_NORM cần được tăng ưu tiên tạm thời để xử lý một tín hiệu quan trọng, thiết kế hiện tại chưa có cơ chế để thực hiện điều này một cách an toàn và hiệu quả.

Để giải quyết vấn đề này, có thể bổ sung một cơ chế Priority Escalation (Tăng ưu tiên tạm thời) trong Core. Cơ chế này sẽ cho phép một TASK_NORM được nâng lên mức ưu tiên cao hơn trong một khoảng thời gian ngắn để xử lý tín hiệu quan trọng, sau đó tự động hạ xuống mức ưu tiên ban đầu sau khi xử lý xong.

Về mặt nguyên tắc phân chia mức ưu tiên, tác giả đề xuất bổ sung thiết kế mới như sau:

- Mức ưu tiên được tăng lên tổng số lượng mức là 24, tức 16 mức ưu tiên gốc + 8 mức ưu tiên tăng tạm thời. Lưu ý rằng, giả sử số lượng TASK_NORM sử dụng dưới 16 thì có thể xem các mức ưu tiên dư thừa này như là các mức ưu tiên tăng tạm thời, điều này giúp đảm bảo rằng các TASK_NORM có thể được tăng ưu tiên tạm thời một cách linh hoạt mà không bị giới hạn bởi số lượng TASK_NORM đang sử dụng.
- Metadata của mỗi TASK_NORM sẽ được bổ sung thêm một trường `base_pri` để lưu trữ mức ưu tiên gốc của TASK_NORM, giúp Core có thể dễ dàng hạ xuống mức ưu tiên ban đầu sau khi xử lý xong tín hiệu quan trọng.
- API mới `uedp_task_norm_escalate_pri(task_id, new_pri)` sẽ được bổ sung để cho phép tăng ưu tiên tạm thời của một TASK_NORM, trong đó `new_pri` phải nằm trong khoảng từ `UEDP_TASK_PRI_LEVEL_16` đến `UEDP_TASK_PRI_LEVEL_23` để đảm bảo rằng mức ưu tiên tăng tạm thời không vượt quá giới hạn đã định.

Với thiết kế này, các vấn đề cần xử lý bao gồm:

- Phân phối mức ưu tiên nếu có nhiều TASK_NORM cùng được tăng ưu tiên tạm thời, tác giả đề xuất cơ chế tìm kiếm mức ưu tiên cao nhất tạm thời của bảng TASK_NORM, sau đó, với mỗi mức ưu tiên tăng dần, nếu có TASK_NORM nào đã được tăng lên mức đó thì sẽ tiếp tục tìm kiếm mức tiếp theo cho đến khi tìm được mức ưu tiên trống để gán cho TASK_NORM cần tăng ưu tiên tạm thời. Nếu tất cả các mức ưu tiên tăng tạm thời đều đã được sử dụng thì API sẽ giữ nguyên mức ưu tiên của TASK_NORM mà không thực hiện tăng ưu tiên tạm thời, đồng thời trả về lỗi để người dùng có thể xử lý tình huống này một cách phù hợp.

Thuật toán xử lý:

- Mức ưu tiên cần gán `target_pri` được tính theo công thức: `target_pri = current_max + step`.
- Core sẽ tra cứu bitmap `g_task_norm_ready` để tìm mức ưu tiên cao nhất đang ready, sau đó sẽ tăng dần `target_pri` từ `UEDP_TASK_PRI_LEVEL_16` đến `UEDP_TASK_PRI_LEVEL_23` để tìm mức ưu tiên trống.
- Nếu tìm được mức ưu tiên trống thì sẽ gán `target_pri` cho TASK_NORM cần tăng ưu tiên tạm thời, đồng thời lưu `base_pri` của TASK_NORM này để có thể hạ xuống sau khi xử lý xong tín hiệu quan trọng. Nếu 1 mức ưu tiên đã được gán cho một TASK_NORM khác thì sẽ tiếp tục tìm kiếm mức tiếp theo cho đến khi tìm được mức ưu tiên trống hoặc đã kiểm tra hết tất cả các mức ưu tiên tăng tạm thời. Mức tăng mỗi lần sẽ là 1 đơn vị thay vì 2 đơn vị.
- Sau khi TASK_NORM đã xử lý xong tín hiệu quan trọng, Core sẽ tự động hạ xuống mức ưu tiên ban đầu bằng cách gán `base_pri` trở lại cho TASK_NORM này, đồng thời cập nhật lại bitmap `g_task_norm_ready` để phản ánh sự thay đổi về mức ưu tiên.
- Nếu trường hợp tất cả các mức ưu tiên tạm thời đều được sử dụng thì API sẽ lưu vào `target_pri` một giá trị đặc biệt để đánh dấu rằng TASK_NORM đã pending, sau 1 vòng scheduling thì sẽ đảm bảo rằng TASK_NORM này được gán mức ưu tiên trống mới nhất để xử lý tín hiệu quan trọng. Trong lúc pending, TASK_NORM này sẽ vẫn giữ mức ưu tiên ban đầu nhưng sẽ được đánh dấu để scheduler ưu tiên gán mức ưu tiên tạm thời khi có slot trống, điều này giúp đảm bảo rằng tín hiệu quan trọng sẽ được xử lý một cách kịp thời ngay khi có khả năng tăng ưu tiên tạm thời.
