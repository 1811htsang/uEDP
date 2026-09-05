# Tài liệu hướng dẫn sử dụng μEDP

Tác giả: Shang Huang - Huỳnh Thanh Sang

## I. Giới thiệu chung

μEDP - micro-EDP là một module lõi được thiết kế để hỗ trợ mô hình lập trình hướng sự kiện (event-driven programming) trên các nền tảng nhúng. Mục tiêu của μEDP là cung cấp một giải pháp linh hoạt, dễ sử dụng và có khả năng mở rộng cho việc phát triển ứng dụng nhúng mà không phụ thuộc vào phần cứng cụ thể.

## II. Cấu trúc thư mục

<!-- CRITICAL
Xem lại commit số c3f4c9866ec1ea548f2e108f059328fe0dd68183 để revert lại các thay đổi liên quan đến cấu trúc thư mục của project

Lưu ý này áp dụng đối với cả bản VN và EN (hiện tại chưa có)
-->

```text
μEDP/
├── core/                        # Định nghĩa và triển khai logic chính của μEDP
│   ├── inc/                     # uedp_msg.h, uedp_task.h, uedp_timer.h, uedp_fsm.h, uedp_tsm.h
│   │   └── uedp_core.h          # Định nghĩa các tín hiệu, hằng số và cấu trúc dữ liệu cốt lõi của μEDP
│   └── src/                     # Triển khai logic scheduler, timer engine, message manager
├── pal/                         # BACKEND (Lớp trừu tượng)
│   ├── pal_core.h               # Khai báo thống nhất chung cho toàn bộ PAL và các dịch vụ hệ thống
│   ├── services/                # Hardware Services (Mapping phần cứng)
│   │   ├── logdp/               # pal_logdp.h chứa các khai báo API log để triển khai bộ dispatch log ra nhiều backend
│   │   ├── memrp/               # pal_memrp.h chứa các khai báo API memory profiling để triển khai trên từng nền tảng
│   │   └── rprintf/             # pal_rprintf.h chứa các khai báo API rprintf để triển khai trên từng nền tảng
│   └── arch/                    # Implementation (Mã nguồn chi tiết từng chip)
│       └── .../                 # Mỗi nền tảng sẽ có một thư mục riêng để triển khai
├── app/                         # Định nghĩa logic ứng dụng, bao gồm các tác vụ và FSM do người dùng tạo ra
│   ├── config/                  # Chứa cấu hình ứng dụng, Core và PAL
│   ├── declaration/             # Khai báo các thiết kế cho logic nghiệp vụ
│   ├── interface/               # Chứa các triển khai cho truyền tín hiệu từ ngoài vào Core
│   ├── kconfig/                 # Chứa các cấu hình cho ứng dụng bằng Kconfig
│   └── app.c                    # Implementation chính của logic hoạt động của ứng dụng người dùng
├── common/                      # Các tiện ích và cấu trúc dữ liệu chung được sử dụng trong toàn bộ dự án
│   ├── container/               # Các cấu trúc dữ liệu như FIFO, Ring Buffer, Linked List được triển khai thuần C
│   ├── kconfiglib/              # Chứa cấu hình thực thi Kconfig terminal
│   ├── kconfigspec/                  # Cấu hình python để sinh code từ Kconfig terminal
│   └── xprintf/                 # Thư viện xprintf sử dụng cho việc format chuỗi log và xuất ra nhiều backend khác nhau
└── test/                        # Các test case mẫu để kiểm tra các tính năng của μEDP
    ├── test01/                  # Test cơ bản với các tác vụ ISR và TSM 
    ├── test02/                  # Test với các tính năng như message pooling và memrp
    ├── test03/                  # Test với các tính năng như message pooling và memrp
    └── test04/                  # Test với tính năng itnlog
```

## III. Hướng dẫn sử dụng

### Kconfig

Kconfig là một công cụ cấu hình được sử dụng để quản lý các tùy chọn và thiết lập trong dự án μEDP. Nó cho phép người dùng dễ dàng bật hoặc tắt các tính năng, điều chỉnh các thông số và tạo ra các cấu hình khác nhau cho ứng dụng.

Câu lệnh sử dụng Kconfig:

```bash
python uedp.py menuconfig
```

#### Điều kiện tiên quyết sử dụng Kconfig

- Sử dụng môi trường Linux (Có thể sử dụng docker để mapping thư mục dự án vào môi trường Linux và cài đặt các công cụ cần thiết).
- Cài đặt Python 3.x trên hệ thống.

#### Các cấu hình được hỗ trợ bởi μEDP

- Số lượng tác vụ norm sử dụng trong ứng dụng.
- Số lượng tác vụ poll sử dụng trong ứng dụng.
- Số lượng tín hiệu được định nghĩa trong ứng dụng.
- Kích thước pool BLANK, ALLOC, EXTAL và ISR.
- Kích thước hàng đợi tin nhắn cho từng tác vụ norm.
- Số lượng timer tối đa được sử dụng trong ứng dụng.
- Số lượng entry log tối đa được lưu trữ trong bộ đệm nội bộ của itnlog.
- Ngưỡng cảnh báo khi bộ đệm log gần đầy.
- Số lượng backend đăng ký trên logdp.
- Cấu hình tên tác vụ norm, tên hàng đợi tin nhắn, tên handler cho từng tác vụ norm.
- Cấu hình tên tác vụ poll, tên handler cho từng tác vụ poll.
- Cấu hình tên tín hiệu.
- Cấu hình object quản lý, bảng quản lý trạng thái (state descriptor), bảng quản lý chuyển trạng thái (transition descriptor) cho từng TSM. Kể từ 1.1.6, mỗi tác vụ norm được hỏi **riêng biệt** có dùng TSM hay không và dùng bao nhiêu state, không còn dùng chung 1 lựa chọn cho toàn bộ tác vụ.
- Cấu hình object quản lý, tên trạng thái cho từng FSM. Tương tự TSM, mỗi tác vụ norm cũng được hỏi riêng có dùng FSM hay không và số lượng state riêng, kể từ 1.1.6.

#### Cấu hình hỗ trợ tự động sinh code từ Kconfig

- Giá trị tín hiệu cho từng tác vụ poll. (Xuất phát từ 0xE6u)
- Mức ưu tiên cho từng tác vụ norm. (Xuất phát từ mức 0)
- Giá trị tín hiệu cho từng tín hiệu. (Xuất phát từ 0x01u)
- Tên handler cho từng TSM và FSM. (Dựa trên tên trạng thái và tên object quản lý)

#### Lưu ý khi sử dụng Kconfig

Kconfig chỉ hỗ trợ sinh code cho các giá trị định nghĩa, tên handler và tên trạng thái. Các logic xử lý trong handler, logic chuyển trạng thái trong TSM và FSM vẫn cần được người dùng tự triển khai trong phần implementation của ứng dụng ở `app.c`.

Kể từ phiên bản 1.1.6, khi chạy `menuconfig`, với mỗi tác vụ norm đã khai báo (`Số lượng tác vụ norm sử dụng trong ứng dụng`), công cụ sẽ hỏi lần lượt: "Task #i có dùng TSM không?", nếu có thì hỏi tiếp số lượng state; rồi "Task #i có dùng FSM không?", nếu có thì hỏi số lượng state tương ứng. Một tác vụ hoàn toàn có thể dùng cả TSM lẫn FSM cùng lúc, chỉ một trong hai, hoặc không dùng cái nào, và số lượng state của mỗi tác vụ không bị ràng buộc phải giống nhau. Đây là điểm khác biệt so với các phiên bản trước 1.1.6, khi công cụ chỉ hỏi 1 lần cho toàn bộ ứng dụng và áp dụng cùng 1 số lượng state cho mọi tác vụ norm dùng TSM/FSM.

### Message Pool

Message Pool là thành phần quản lý bộ nhớ cho các message được sử dụng trong μEDP. Nó cung cấp cơ chế cấp phát và thu hồi message một cách hiệu quả, giúp tối ưu hóa việc sử dụng bộ nhớ và đảm bảo rằng các message được xử lý đúng cách trong hệ thống.

#### Phân loại message pool

- `UEDP_MSG_TYPE_BLANK`: message không có payload.
- `UEDP_MSG_TYPE_ALLOC`: message có payload nhỏ hoặc vừa, vùng data riêng.
- `UEDP_MSG_TYPE_EXTAL`: message từ interface ngoài Core.
- `UEDP_MSG_TYPE_ISR`: tín hiệu từ ngữ cảnh ngắt.

Khi khởi tạo bằng `uedp_msg_pool_init()`, Core dựng sẵn các pool tĩnh cho `BLANK`, `ALLOC`, `EXTAL` và một FIFO riêng cho `ISR`. Với `ALLOC` và `EXTAL`, vùng data được bố trí theo chiều 2D `[queue_size][data_size]` để mỗi message có ô dữ liệu riêng.

#### Cách dùng

1. Gọi `uedp_msg_pool_init()` sau `uedp_core_init()`.
2. Gọi `uedp_msg_alloc(des_task_id, sig, size)` để lấy message từ pool phù hợp.
3. Dùng `uedp_msg_set_data_val()` nếu muốn copy giá trị vào payload.
4. Dùng `uedp_msg_set_data_ref()` nếu muốn truyền tham chiếu đến dữ liệu có vòng đời đủ dài.

#### Điểm cần lưu ý khi sử dụng message pool

- `uedp_msg_alloc()` tự chọn pool theo kích thước payload, nên giá trị `size` phải phản ánh đúng nhu cầu dữ liệu.
- Nếu dùng truyền tham chiếu thì buffer tham chiếu phải còn sống sau thời điểm message được xử lý.
- `uedp_msg_drain_isr_pool()` là đường đi riêng cho tín hiệu ISR; không nên tự đẩy dữ liệu ISR vào queue task thường.
- Với `ALLOC` và `EXTAL`, `data` là con trỏ tới vùng nhớ riêng của từng message, không phải payload inline nằm ngay trong header.
- Không cần tự gọi `uedp_msg_free()` cho message từ pool `ISR`, vì Core sẽ tự giải phóng sau khi handler chạy xong ở mỗi vòng lập lịch.

#### Định danh tác vụ nguồn/đích cho message

Kể từ phiên bản 1.1.5, `uedp_msg_t` hỗ trợ gán riêng ID của tác vụ nguồn (task gửi) và tác vụ đích (task nhận) cho từng message, thay vì chỉ có duy nhất tham số `des_task_id` truyền vào lúc `uedp_msg_alloc()`:

- `uedp_msg_set_src_task_id(msg, src_task_id)`: gán ID của tác vụ nguồn gửi message.
- `uedp_msg_set_des_task_id(msg, des_task_id)`: gán/đổi lại ID của tác vụ đích nhận message.

Ví dụ: task A muốn gửi cho task B một message nhưng vẫn cần task B biết message này đến từ task A (ví dụ để phản hồi lại đúng nơi gửi):

```c
uedp_msg_t* msg = uedp_msg_alloc(UEDP_TASK_NORM_B_ID, SIG_REQUEST, 0);
uedp_msg_set_src_task_id(msg, UEDP_TASK_NORM_A_ID);
uedp_task_norm_send_msg(msg);
```

Trong handler của task B, có thể đọc lại `msg->src_task_id` để biết nơi gửi và dùng `uedp_msg_set_des_task_id()` trên một message phản hồi mới để gửi ngược lại đúng task A. Đây là cơ chế đơn giản, người dùng tự chọn ID phù hợp - Core không tự ràng buộc hay xác thực cặp nguồn/đích này.

> **Lưu ý**: kể từ 1.1.5, API `internal_uedp_msg_pool_panic` đã được loại bỏ khỏi Core. Các lỗi nghiêm trọng liên quan đến message pool (hết chỗ, con trỏ không hợp lệ, ISR FIFO đầy...) giờ được báo cáo qua cơ chế FCR (xem mục "FCR (Fatal Code Return)" bên dưới) thay vì gọi thẳng panic như trước.

### FCR (Fatal Code Return) - Xử lý lỗi nghiêm trọng

Kể từ phiên bản 1.1.5, μEDP bổ sung FCR để định danh và xử lý các lỗi nghiêm trọng bên trong Core (pool hết chỗ, con trỏ không hợp lệ, ID tác vụ sai, transition không tồn tại...) một cách nhất quán, thay vì mỗi module tự xử lý im lặng theo cách riêng như trước.

Mỗi lỗi nghiêm trọng được gán một mã cố định (`uedp_fcr_code_t`), tra ra mức độ nghiêm trọng (`severity`: `WARN`/`ERROR`/`FATAL`) và một hành động xử lý tương ứng, rồi luôn được ghi log qua `itnlog` (tag `ITNLOG_TAG_FCR`) trước khi thực thi hành động đó:

- `UEDP_FCR_ACT_LOG_ONLY`: chỉ ghi log, không can thiệp luồng chạy.
- `UEDP_FCR_ACT_RESET_TASK`: đánh dấu để tầng trên (task giám sát hoặc OCE) tự khôi phục tác vụ liên quan.
- `UEDP_FCR_ACT_SYS_RESET`: gọi `pal_sys_reset()` khởi động lại toàn hệ thống.
- `UEDP_FCR_ACT_SYS_PANIC`: gọi `pal_sys_fatal()` dừng hệ thống ngay lập tức.

#### Tự khai báo mã lỗi ở tầng ứng dụng

Ứng dụng có thể tự khai báo mã lỗi riêng bằng module `UEDP_FCR_MOD_APP` và tự raise khi phát hiện điều kiện bất thường trong logic của mình:

```c
#define APP_FCR_SENSOR_TIMEOUT UEDP_FCR_CODE(UEDP_FCR_MOD_APP, 0x01)

// Khi phát hiện cảm biến không phản hồi trong thời gian cho phép:
UEDP_FCR_RAISE_MSG(APP_FCR_SENSOR_TIMEOUT, "sensor #2 timeout sau 500ms");
```

`UEDP_FCR_RAISE(code)` và `UEDP_FCR_RAISE_MSG(code, extra)` tự động điền `__FILE__`/`__LINE__` vào entry log; `RAISE_MSG` cho phép truyền thêm mô tả ngữ cảnh cụ thể (ví dụ tên hàm, giá trị tham số sai) để dễ debug hơn so với mô tả mặc định trong bảng mã lỗi.

#### Điểm cần lưu ý

- Vì tra bảng mã lỗi không thấy sẽ mặc định rơi vào `UEDP_FCR_UNKNOWN` (`SEV_FATAL` + `ACT_SYS_PANIC`), người dùng nên luôn đăng ký đúng mã lỗi mình định dùng thay vì truyền mã tuỳ ý.
- `UEDP_FCR_ACT_SYS_RESET`/`UEDP_FCR_ACT_SYS_PANIC` sẽ dừng hoặc khởi động lại toàn hệ thống ngay khi raise - cần cân nhắc kỹ trước khi raise các mã lỗi thuộc nhóm này từ logic ứng dụng.
- Hiện tại `g_fcr_table[]` là bảng `static const`, tầng ứng dụng chưa thể tự đăng ký thêm entry mới lúc runtime - muốn thêm mã lỗi và hành động xử lý tương ứng phải khai báo trực tiếp trong mã nguồn Core.
- `UEDP_FCR_ACT_RESET_TASK` hiện chưa tự động khôi phục tác vụ liên quan, mới dừng ở mức ghi log `ERROR` - việc khôi phục vẫn cần tầng trên tự xử lý.

### Dpool GDA (Global Data Pool) - Quản lý dữ liệu toàn cục

Kể từ phiên bản 1.1.6, μEDP bổ sung GDP (Global Data Pool) để quản lý biến toàn cục dùng chung giữa nhiều tác vụ một cách tường minh, phục vụ cho khối `glbda:` của PLD/μE-LS (xem `docs/uels-syntax.md` và `docs/review/dmp-gda.md`). Khác với `Message Pool`, GDP không cấp phát vùng nhớ và không có khái niệm "free" một slot đã đăng ký - nó chỉ đăng ký tên định danh trỏ tới một vùng nhớ `static`/`global` đã tồn tại sẵn do người dùng tự khai báo, vì biến toàn cục được xem là sống suốt vòng đời chương trình.

#### Cách dùng Dpool GDA

1. Gọi `uedp_gdp_init()` một lần sau `uedp_core_init()`, trước khi dùng bất kỳ API GDP nào khác.
2. Khai báo biến toàn cục (`static`/`global`) trong ứng dụng, sau đó đăng ký vào GDP bằng `uedp_gdp_register(name, data_ptr, size)`.
3. Dùng `uedp_gdp_get_ref(name)` khi cần con trỏ tham chiếu trực tiếp tới dữ liệu (tương ứng `ptype: REF` trong μE-LS).
4. Dùng `uedp_gdp_get_val(name, out_buf, buf_size)` / `uedp_gdp_set_val(name, in_buf, buf_size)` khi cần đọc/ghi giá trị qua sao chép thay vì giữ tham chiếu trực tiếp (tương ứng `ptype: VAL` trong μE-LS).
5. Gọi `uedp_gdp_unregister(name)` nếu không còn cần tra cứu biến đó qua GDP nữa (vùng nhớ thật vẫn không bị giải phóng).

Ví dụ khai báo và sử dụng một biến trạng thái toàn cục dùng chung giữa 2 task:

```c
static ui32 g_system_status = 0;

void app_init(void) {
  uedp_gdp_init();
  uedp_gdp_register("GDA_SYSTEM_STATUS", &g_system_status, sizeof(g_system_status));
}

// Task A cập nhật trạng thái bằng cách ghi giá trị mới (ptype: VAL)
void task_a_update_status(ui32 new_status) {
  uedp_gdp_set_val("GDA_SYSTEM_STATUS", &new_status, sizeof(new_status));
}

// Task B đọc trực tiếp qua con trỏ tham chiếu (ptype: REF)
void task_b_check_status(void) {
  ui32* status_ref = (ui32*)uedp_gdp_get_ref("GDA_SYSTEM_STATUS");
  if (status_ref != NULL && *status_ref != 0) {
    // xử lý khi hệ thống có trạng thái khác 0
  }
}
```

#### Điểm cần lưu ý khi sử dụng Dpool GDA

- GDP không sở hữu vùng nhớ `data` - nếu đăng ký một biến cục bộ (local variable) thay vì `static`/`global`, con trỏ sẽ trỏ tới vùng nhớ không còn hợp lệ sau khi hàm khai báo kết thúc.
- Số lượng slot tối đa GDP quản lý cùng lúc mặc định là `UEDP_GDP_MAX_SLOTS` (16), có thể override bằng macro trước khi include header nếu cần nhiều hơn.
- `uedp_gdp_get_val()`/`uedp_gdp_set_val()` sẽ raise FCR nếu `buf_size` không khớp/không đủ so với kích thước đã đăng ký, thay vì âm thầm đọc/ghi sai vùng nhớ.
- `uedp_gdp_get_ref()` trả về con trỏ trực tiếp vào vùng nhớ thật, không bọc critical section - phù hợp với scheduler single-core, non-preemptive hiện tại của μEDP; cần xem xét lại nếu chạy trên môi trường đa nhân.
- Tên đăng ký (`name`) trong GDP nên khớp với `name` khai báo trong khối `glbda:` của μE-LS để giữ nhất quán giữa tài liệu thiết kế PLD/μE-LS và code thực thi.

### Task

Task trong μEDP có hai kiểu: message-driven và poll-driven.

#### Task message-driven

Task message-driven được khai báo bằng `task_norm_t` với 5 thành phần:

- `id`: ID task.
- `base_pri`: mức ưu tiên gốc.
- `cur_pri`: mức ưu tiên hiện tại (có thể thay đổi khi chạy).
- `urgent_pending`: cờ báo task có tín hiệu urgent đang chờ xử lý.
- `task_norm`: handler chính.
- `msg_queue`: FIFO nội bộ.
- `msg_queue_buffer`: buffer con trỏ cho FIFO.

Khi `uedp_task_norm_create()` chạy, Core sẽ tự khởi tạo FIFO cho mỗi task đến phần tử `UEDP_TASK_NORM_EOT_ID`. Kích thước hàng đợi mặc định lấy từ `UEDP_TASK_MSG_QUEUE_SIZE`.

`uedp_task_scheduler()` hiện chọn task có priority cao nhất đang ready, lấy đúng một message từ queue của task đó, dispatch handler, rồi giải phóng message sau khi handler chạy xong.

#### Task poll-driven

Task poll-driven được khai báo bằng `task_poll_t` với `id`, `ability`, và `task_poll`.

- `uedp_task_poll_create()` chỉ đếm danh sách đến `UEDP_TASK_POLL_EOT_ID`.
- `uedp_task_poll_set_ability()` bật/tắt task poll theo ID.
- Khi không có task message-driven nào ready, scheduler sẽ chạy các poll task đang bật.

#### API ngữ cảnh task

Trong lúc task đang chạy, có thể lấy ngữ cảnh hiện tại bằng:

- `uedp_task_norm_get_current_id()`
- `uedp_task_norm_get_current_msg()`

Các API này đặc biệt hữu ích cho itnlog vì logger lấy `task_id` và `msg_sig` từ ngữ cảnh hiện tại.

#### Tăng ưu tiên tạm thời

Sau khi xử lý xong 1 message hoặc được ISR gọi tăng mức ưu tiên, task có thể tăng ưu tiên tạm thời bằng `uedp_task_norm_set_urgent(task_id_t tid)` và `uedp_task_norm_post_urgent(task_id_t tid, uedp_msg_t* msg)`. Scheduler sẽ dùng mức ưu tiên này cho vòng lập lịch tiếp theo. Khi task không còn message nào trong queue, mức ưu tiên sẽ tự reset về `base_pri`.

### ISR

ISR trong μEDP không nên xử lý logic phức tạp trực tiếp. Đường đi chuẩn là:

1. ISR gọi API đăng ký tín hiệu cho Core.
2. Core đưa cặp task ID + signal vào FIFO ISR nội bộ.
3. Ở đầu vòng scheduler, Core rút FIFO này và chuyển thành luồng xử lý bình thường.

Đường đi này được dùng chung cho tín hiệu từ timer tick và các ngắt khác.

Trong code hiện tại, `uedp_task_norm_post_isr()` là API dành riêng cho ISR, còn `uedp_timer_tick()` cũng dùng API này khi timer hết hạn để đưa tín hiệu về task đích.

### Timer

Timer Service của μEDP dùng pool cố định `UEDP_TIMER_MAX_NODES` node, không cấp phát heap.

#### Cách dùng khai báo

1. Gọi `uedp_timer_init()` sau khi khởi tạo Core.
2. Gọi `uedp_timer_set(task_id, sig, ms, type)` để tạo timer mới hoặc cập nhật timer đã tồn tại.
3. Gọi `uedp_timer_remove(task_id, sig)` để xóa timer.
4. Gọi `uedp_timer_tick()` trong ngữ cảnh tick định kỳ của nền tảng.

#### Hành vi thực tế

- `type` hiện hỗ trợ `UEDP_TIMER_ONE_SHOT` và `UEDP_TIMER_PERIODIC`.
- `ms` được quy đổi sang số tick bằng `UEDP_TIMER_TICK`.
- Khi timer hết hạn, Core phát sinh signal về task đích bằng đường đi ISR-safe.
- Với timer periodic, counter được nạp lại sau mỗi lần hết hạn; với one-shot, node được trả về free-list.

#### Tài nguyên

- Số timer tối đa đồng thời là `UEDP_TIMER_MAX_NODES`.
- `uedp_timer_get_stats()` cho phép kiểm tra số timer đang hoạt động và capacity tối đa.

#### Mục tiêu sử dụng

Timer có thể được sử dụng như 1 công cụ để tạo ra delay hoặc timeout trong các task. Ví dụ, trong 1 task nào đó cần sử dụng blocking API như UART, I2C hay các giao thức truyền thông, ta có thể dùng timer để tạo ra timeout cho các API này, tránh việc task bị treo vô thời hạn nếu có sự cố xảy ra. Ngoài ra, timer cũng có thể được dùng để tạo ra các sự kiện định kỳ, ví dụ như đọc cảm biến mỗi 1 khoảng thời gian nhất định hoặc gửi heartbeat để báo rằng hệ thống vẫn đang hoạt động.

### Itnlog

Itnlog là cơ chế logging nội bộ của μEDP, được dùng để thay thế cho kiểu debug bằng `printf` rải rác trong luồng xử lý. Cách này giúp Core không phụ thuộc trực tiếp vào stdio, đồng thời cho phép đổi đích xuất log theo từng nền tảng mà không phải sửa logic xử lý. Khi cần xuất cùng một entry ra nhiều backend, nên ghép thêm `logdp` và `rprintf` thay vì chỉ dùng một callback chuỗi đơn.

#### Cách dùng cơ bản

1. Gọi `uedp_itnlog_init()` sau khi đã khởi tạo Core và trước khi bắt đầu chạy scheduler.
2. Đăng ký một wrapper xuất log bằng `uedp_itnlog_set_output()`.
3. Gọi `uedp_itnlog_log()` ở nơi cần ghi nhận sự kiện.
4. Gọi `uedp_itnlog_dump()` khi muốn in toàn bộ log đang có trong bộ đệm.

Ví dụ cấu hình trên Linux:

```c
static void itnlog_stdout_output(const char* text) {
  printf("%s", text);
  fflush(stdout);
}

int main(void) {
  uedp_core_init();
  uedp_msg_pool_init();
  uedp_timer_init();

  uedp_itnlog_init();
  uedp_itnlog_set_output(itnlog_stdout_output);
  uedp_itnlog_set_filter(ITNLOG_LEVEL_DEBUG, ITNLOG_TAG_TSK);

  while (1) {
    uedp_task_scheduler();
    usleep(100);
    uedp_itnlog_dump();
  }
}
```

Khi viết code ứng dụng, thay vì chèn `printf` trực tiếp trong handler, nên gọi `uedp_itnlog_log()` với một tag phù hợp như `TSK`, `MSG`, `FSM`, `TSM` hoặc `TIM`. Sau đó dùng `uedp_itnlog_dump()` ở thời điểm muốn xuất toàn bộ buffer log ra đích đã cấu hình.

Lưu ý quan trọng: `uedp_itnlog_set_output()` nhận một hàm có chữ ký `void (*)(const char*)`. Vì vậy không nên truyền trực tiếp `printf` vào API này, mà nên bọc `printf` hoặc `fputs` trong một wrapper như ví dụ trên.

#### Định dạng dòng log

Khi dump, mỗi entry được ghép thành một dòng theo mẫu hoặc tùy theo thiết kế của callback output, nhưng mặc định sẽ có định dạng như sau:

```text
[ITNLOG] tmstmp task_id msg_id msg
```

Trong đó `task_id` và `msg_id` được xuất ở dạng hex để dễ map với ID nội bộ của Core. `msg_id` chính là `msg_sig` của message hiện tại khi log được ghi. Ví dụ:

```text
[ITNLOG] 0 0xE4 0x01 System is alive and running...
```

#### Lọc theo tag

Nếu cần lọc log theo module, dùng `uedp_itnlog_set_tag("TSK")`, `uedp_itnlog_set_tag("MSG")`, `uedp_itnlog_set_tag("FSM")`, `uedp_itnlog_set_tag("TSM")`, hoặc `uedp_itnlog_set_tag("TIM")`.

Lưu ý:

- `NULL` có nghĩa là không lọc theo tag.
- Khi xuất ra terminal, nên dùng một wrapper riêng thay vì truyền trực tiếp `printf` làm callback để tránh lỗi do khác kiểu chữ ký hàm và để chủ động flush `stdout`.
- `uedp_itnlog_log()` lấy `task_id` từ task hiện tại và lấy `msg_sig` từ message hiện tại, nên chỉ nên gọi khi đang ở trong ngữ cảnh scheduler xử lý message hợp lệ.

#### Lưu ý khi debug

- Nếu log chưa hiện ngay trên terminal, kiểm tra callback output có flush `stdout` hay không.
- Nếu muốn in log theo thời điểm nhất định, có thể gọi `uedp_itnlog_dump()` trong polling task hoặc ngay trước khi kết thúc testcase.
- `uedp_itnlog_log()` chỉ ghi vào bộ đệm nội bộ, còn việc hiển thị phụ thuộc vào callback output và thời điểm dump.
- Nếu buffer log đầy, `uedp_itnlog_log()` sẽ làm logger tự dump trước khi ghi tiếp theo thiết kế hiện tại trong source.

### Logdp, Rprintf và Xprintf

`xprintf` là tầng formatter ở mức ký tự. Nó cung cấp các API như `xprintf()`, `xfprintf()` và `xsprintf()` để format chuỗi theo cùng một bộ quy tắc trên nhiều nền tảng. Trong luồng rprintf hiện tại, `pal_rprintf_flush_entry()` dùng `xfprintf()` để dựng chuỗi log ra buffer trung gian trước khi đẩy tới backend.

`logdp` là tầng dispatch của PAL. Nó giữ một bảng callback có kiểu `void (*)(uedp_itnlog_entry_t*)` và cho phép cùng một log entry được phát tới nhiều đích. Đây là lớp phù hợp khi người dùng muốn một entry vừa ra UART, vừa ra console, vừa ra file log hoặc trace buffer.

`rprintf` là tầng redirect print. Nó nhận một `pal_rprintf_service_t` chứa entry cần xuất và các callback backend như `init`, `putc`, `write`, `is_ready`. Khi `pal_rprintf_flush_entry()` được gọi, nó sẽ kiểm tra backend, format entry bằng `xfprintf()`, rồi xuất chuỗi kết quả qua `write` nếu có, hoặc rải từng ký tự qua `putc` nếu không có `write`.

#### Cách ghép các tầng

1. Khởi tạo backend cụ thể như UART, console hoặc file.
2. Tạo một `pal_rprintf_service_t` cho backend đó.
3. Viết một adapter callback có chữ ký `void (*)(uedp_itnlog_entry_t*)`.
4. Trong adapter, copy `*entry` vào `service.entry` rồi gọi `pal_rprintf_flush_entry(&service)`.
5. Đăng ký adapter bằng `pal_logdp_register()`.
6. Khi có log entry cần phát, gọi `pal_logdp_dispatch(&entry)`.

Ví dụ tối giản trên Linux:

```c
static void linux_putc(unsigned char c) {
  putchar(c);
}

static void linux_write(const uint8_t* data, uint16_t len) {
  fwrite(data, 1, len, stdout);
  fflush(stdout);
}

static bool linux_is_ready(void) {
  return true;
}

static pal_rprintf_service_t linux_rprintf = {
  .entry = {0},
  .init = NULL,
  .putc = linux_putc,
  .write = linux_write,
  .is_ready = linux_is_ready,
};

static void linux_log_output(uedp_itnlog_entry_t* entry) {
  linux_rprintf.entry = *entry;
  pal_rprintf_flush_entry(&linux_rprintf);
}

int main(void) {
  uedp_core_init();
  uedp_msg_pool_init();
  uedp_timer_init();
  uedp_itnlog_init();
  uedp_itnlog_set_output(pal_logdp_dispatch);
  pal_logdp_register(linux_log_output);

  while (1) {
    uedp_task_scheduler();
    uedp_itnlog_dump();
  }
}
```

Điểm cần nhớ:

- Nếu chỉ cần một đích xuất log, có thể gọi trực tiếp `pal_rprintf_flush_entry()` mà không cần qua `logdp`.
- Nếu cần nhiều đích xuất log, hãy đăng ký nhiều callback khác nhau vào `logdp`; mỗi callback nên sở hữu một `pal_rprintf_service_t` riêng.
- `xprintf` thường không cần gọi trực tiếp trong ứng dụng khi đã dùng `rprintf`, vì `rprintf` đã dùng `xfprintf()` để format chuỗi đầu ra.
- `pal_rprintf_service_t` cho phép `init = NULL` nếu backend đã được BSP hoặc application khởi tạo sẵn.

#### Đặt tên cho backend rprintf

Kể từ phiên bản 1.1.5, `pal_rprintf_service_t` có thêm trường `name` (chuỗi, ví dụ `"UART"`, `"FILE"`, `"CONSOLE"`) để đặt nhãn logic cho backend. Trường này không ảnh hưởng tới logic dispatch của Core - nó chỉ phục vụ mục đích debug trace và ánh xạ 1-1 với trường `contract` trong khối `pplp.rprintf[]` của cú pháp μE-LS, giúp tài liệu thiết kế PLD/μE-LS và code thực thi khớp tên với nhau:

```c
static pal_rprintf_service_t linux_rprintf = {
  .name = "CONSOLE",
  .entry = {0},
  .init = NULL,
  .putc = linux_putc,
  .write = linux_write,
  .is_ready = linux_is_ready,
};
```

Nếu ứng dụng có nhiều backend rprintf, nên đặt `name` khác nhau và khớp với `contract` tương ứng đã khai báo trong μE-LS để dễ tra cứu khi debug.

### Khai báo các giá trị TASK_NORM, TASK_POLL, SIG và STATE

Dựa theo dải tín hiệu, chúng ta thực hiện tham khảo trong testcase như sau:

- TASK_NORM thì khai báo từ `0xE6` đến `0xEE` (tránh dùng `0xEF` vì đã được định nghĩa là EOT).
- TASK_POLL thì khai báo từ `0xD4` đến `0xDE` (tránh dùng `0xDF` vì đã được định nghĩa là EOT).
- SIG thì khai báo từ `0x01` đến `0xFF` (tránh dùng các giá trị đã được định nghĩa sẵn trong các dải tín hiệu đặc biệt như FSM_SIG, TSM_SIG, TSM_STATE).

### Khai báo các message queue, buffer toàn cục, FSM và TSM

Người dùng nên khai báo các message queue và buffer toàn cục cho từng tác vụ trong implementation của từng test case để đảm bảo tính độc lập và dễ quản lý.

Ví dụ:

```c
static uedp_msg_t* usr_q_mem[8];
static uedp_msg_t* a_q_mem[8];
static uedp_msg_t* b_q_mem[8];

static const char* data_a_to_b = "Hello from Task A!";
static const char* data_b_to_a = "Hello from Task B!";

static uedp_tsm_t blinker_tsm;

static uedp_fsm_t fsm_usr;
static uedp_fsm_t fsm_a;
static uedp_fsm_t fsm_b;
```

Lưu ý rằng các buffer toàn cục này dùng cho việc chứa các message có kích thước quá lớn so với kích thước đã khai báo của pool, khi đó người dùng sẽ sử dụng cơ chế truyền tham chiếu để truyền địa chỉ của dữ liệu vào payload của message, do đó cần đảm bảo rằng các buffer này có phạm vi toàn cục để tránh lỗi truy cập bộ nhớ khi message được xử lý sau khi biến cục bộ đã hết phạm vi.

Ngoài ra, nên tuân thủ theo thứ tự khai báo là message queue, buffer toàn cục, TSM và FSM để đảm bảo tính nhất quán và dễ quản lý trong quá trình phát triển ứng dụng.

### Khai báo các handler cho Task, TSM và FSM

Nên khai báo - declaration các handler cho Task, TSM và FSM trong implementation của từng test case để đảm bảo tính độc lập và dễ quản lý.

Ví dụ:

```c
static void fn_on_active_exit(uedp_msg_t* msg);
static void fn_on_active_entry(uedp_msg_t* msg);

static void fn_on_idle_entry(uedp_msg_t* msg);

static void fn_active_logic(uedp_msg_t* msg);

static void usr_state_idle(uedp_msg_t* msg);
static void usr_state_active(uedp_msg_t* msg);

static void task_a_state_idle(uedp_msg_t* msg);
static void task_a_state_active(uedp_msg_t* msg);

static void task_b_state_idle(uedp_msg_t* msg);
static void task_b_state_active(uedp_msg_t* msg);

static void task_usr_handler(uedp_msg_t* msg);
static void task_a_handler(uedp_msg_t* msg);
static void task_b_handler(uedp_msg_t* msg);
```

Lưu ý, nên tuân thủ theo thứ tự khai báo là handler cho TSM, handler cho FSM và cuối cùng là handler cho Task để đảm bảo tính nhất quán và dễ quản lý trong quá trình phát triển ứng dụng.

### Khởi tạo TSM

#### Khởi tạo TSM table

Trong TSM, mỗi một state sẽ có 1 bảng mô tả chuyển trạng thái là `tsm_trans_t` để định nghĩa

- Tín hiệu chuyển trạng thái
- Trạng thái chuyển tín hiệu tiếp theo
- Hàm logic cần thực thi khi chuyển trạng thái

Ví dụ:

```c
const tsm_trans_t blink_idle_trans[] = {
  { SIG_USR_START, STATE_BLINK_ACTIVE, NULL },
  { SIG_USR_STOP,  UEDP_TSM_STATE_STAY, NULL } 
};

const tsm_trans_t blink_active_trans[] = {
  { SIG_INTERNAL_TICK, UEDP_TSM_STATE_STAY, fn_active_logic },
  { SIG_USR_STOP,      STATE_BLINK_IDLE,      NULL },
  { SIG_USR_START,     UEDP_TSM_STATE_STAY, NULL }
};
```

Sau khi khai báo đầy đủ các bảng chuyển trạng thái thì sẽ tiến hành khai báo bảng mô tả trạng thái `tsm_state_desc_t` để định nghĩa những trạng thái mà TSM có thể có, trong đó sẽ liên kết mỗi trạng thái với hàm on_entry, on_exit và bảng chuyển trạng thái tương ứng.

Ví dụ:

```c
const tsm_state_desc_t blinker_tsm_table[] = {
  { STATE_BLINK_IDLE,   fn_on_idle_entry,   NULL,              blink_idle_trans,   1 },
  { STATE_BLINK_ACTIVE, fn_on_active_entry, fn_on_active_exit, blink_active_trans, 2 }
};
```

Lưu ý rằng mỗi một state không nhất thiết phải có hàm on_entry và on_exit, nếu không cần thiết thì có thể để là NULL. Tuy nhiên, bảng chuyển trạng thái và số lượng lượt chuyển trạng thái thì bắt buộc phải có để định nghĩa được logic chuyển trạng thái của TSM.

#### Khởi tạo Task table

Mỗi một tác vụ sẽ được định nghĩa trong bảng tác vụ `task_norm_t` với các thông tin như sau:

- ID của tác vụ
- Mức độ ưu tiên của tác vụ
- Handler của tác vụ
- Bộ nhớ dùng cho message queue của tác vụ

Ví dụ:

```c
task_norm_t app_task_table[] = {
  { UEDP_TASK_NORM_USR_ID,  UEDP_TASK_PRI_LEVEL_8, task_norm_usr_handler, {0}, usr_q_mem  },
  { TASK_NORM_A_ID,           UEDP_TASK_PRI_LEVEL_7, task_norm_a_handler,   {0}, a_q_mem    },
  { TASK_NORM_B_ID,           UEDP_TASK_PRI_LEVEL_6, task_norm_b_handler,   {0}, b_q_mem    },
  { UEDP_TASK_NORM_EOT_ID,  UEDP_TASK_PRI_LEVEL_0, NULL,                  {0}, NULL       }
};
```

Trong đó, tham số thứ 4 là FIFO nội bộ của task mà Core sẽ tự động khởi tạo dựa vào tham số thứ 5. Do đó ở đây tham số thứ 4 sẽ để là {0} để Core tự động khởi tạo FIFO dựa vào bộ nhớ đã khai báo ở tham số thứ 5.

Lưu ý rằng mỗi một tác vụ nên có mức độ ưu tiên khác nhau để đảm bảo rằng Core có thể xử lý tín hiệu một cách chính xác, nếu tất cả các tác vụ đều có cùng mức độ ưu tiên thì Core sẽ gặp lỗi xử lý tín hiệu, do đó cần lưu ý việc phân bổ mức độ ưu tiên cho các tác vụ trong hệ thống.

Ngoài ra `UEDP_TASK_NORM_USR_ID` chính là tác vụ mặc định mà người dùng sử dụng để truyền tín hiệu bắt đầu cho Core. Do đó nếu người dùng muốn sử dụng một tác vụ khác để truyền tín hiệu bắt đầu cho Core thì cần phải thay đổi lại ID của tác vụ này thành `UEDP_TASK_NORM_USR_ID` để đảm bảo rằng Core có thể nhận được tín hiệu bắt đầu và có mức ưu tiên cao nhất để được xử lý trước các tác vụ khác trong hệ thống.

### Khởi tạo FSM

FSM được khởi tạo tương tự như TSM, trong đó mỗi một trạng thái sẽ là 1 hàm handler để xử lý logic của trạng thái đó. Khi FSM nhận được tín hiệu và được task handler dispatch FSM, Core sẽ gọi hàm handler tương ứng với trạng thái hiện tại của FSM để xử lý logic và quyết định trạng thái tiếp theo dựa trên tín hiệu nhận được.

### Khởi tạo Tick handler

Khởi tạo này phụ thuộc vào nền tảng và cách triển khai.

Ví dụ:

- Ở Linux thì sử dụng một thread riêng để thực hiện việc tick với độ trễ cố định, trong đó thread này sẽ gọi API `uedp_timer_tick()` của Core để cập nhật thời gian và xử lý các bộ định thời phần mềm.
- Ở STM32 thì gọi trực tiếp vào `SysTick_Handler()` để thực hiện việc tick, trong đó hàm này sẽ gọi API `uedp_timer_tick()` của Core để cập nhật thời gian và xử lý các bộ định thời phần mềm.
- Ở các nền tảng khác thì có thể sử dụng một bộ định thời phần cứng để tạo ra ngắt định kỳ, trong đó trong hàm xử lý ngắt này sẽ gọi API `uedp_timer_tick()` của Core để cập nhật thời gian và xử lý các bộ định thời phần mềm.

### Khởi tạo ứng dụng

Sau khi đã hoàn thành việc khai báo các handler, khởi tạo TSM table và Task table thì sẽ tiến hành khởi tạo ứng dụng theo các trình tự sau:

- Khởi tạo môi trường với `uedp_core_init()`, trong đó sẽ thực hiện cấu hình môi trường tùy thuộc theo nền tảng.
- Khởi tạo message pool với `uedp_msg_pool_init()`, trong đó sẽ thực hiện khởi tạo các pool bộ nhớ tĩnh dựa trên cấu hình đã khai báo trong PAL.
- Khởi tạo timer với `uedp_timer_init()`, trong đó sẽ thực hiện khởi tạo các bộ định thời phần mềm và thiết lập tick handler tùy thuộc theo nền tảng.
- Khởi tạo bảng tác vụ với `uedp_task_norm_create()`, trong đó sẽ thực hiện khởi tạo các tác vụ dựa trên bảng tác vụ đã khai báo, đồng thời thiết lập FIFO nội bộ cho từng tác vụ dựa trên bộ nhớ đã khai báo.
- Khởi tạo TSM và FSM với `uedp_tsm_init()` và `uedp_fsm_init()`, trong đó sẽ thực hiện khởi tạo các TSM và FSM dựa trên bảng mô tả trạng thái đã khai báo, đồng thời thiết lập trạng thái ban đầu cho từng TSM và FSM.
- Truyền tín hiệu khởi đầu vào `UEDP_TASK_NORM_USR_ID` với `uedp_post_msg()`, trong đó sẽ thực hiện truyền tín hiệu bắt đầu vào tác vụ mặc định của người dùng để kích hoạt hệ thống và bắt đầu xử lý các tín hiệu tiếp theo.
- Vòng lặp chính sẽ thực thi `uedp_task_scheduler()` để bắt đầu vòng lặp xử lý tín hiệu của hệ thống, trong đó Core sẽ liên tục kiểm tra và xử lý các tín hiệu từ các tác vụ dựa trên mức độ ưu tiên đã thiết lập, đồng thời quản lý các bộ định thời phần mềm và thực thi logic của TSM và FSM khi có tín hiệu tương ứng.
- Sau vòng lặp chính, ocesvc sẽ thực hiện việc dọn dẹp và giải phóng tài nguyên, trong đó sẽ gọi các API tương ứng để giải phóng bộ nhớ, hủy các tác vụ và TSM/FSM, đồng thời đảm bảo rằng tất cả các tín hiệu đã được xử lý trước khi kết thúc chương trình. Thiết kế này được xử lý ở các phiên bản sau của Core, trong đó sẽ cung cấp các API để dọn dẹp và giải phóng tài nguyên một cách an toàn và hiệu quả.

### Trình tự khởi tạo khuyến nghị

Để phù hợp với source hiện tại, thứ tự khởi tạo nên là:

1. `uedp_core_init()`
2. `uedp_msg_pool_init()`
3. `uedp_gdp_init()` nếu có dùng biến toàn cục qua GDP (Dpool GDA)
4. `uedp_timer_init()`
5. `uedp_tsm_init()` và `uedp_fsm_init()` nếu có TSM và FSM
6. `uedp_itnlog_init()` nếu dùng logger.
7. Khởi tạo backend xuất log nếu dùng `rprintf`.
8. Đăng ký callback vào `pal_logdp_register()` nếu muốn fan-out log ra nhiều đích.
9. `uedp_itnlog_set_output()` và các API cấu hình log khác nếu dùng đường xuất log dạng chuỗi.
10. `uedp_task_norm_create()`
11. `uedp_task_poll_create()` nếu có poll task
12. Gửi message khởi đầu vào `UEDP_TASK_NORM_USR_ID`
13. Vòng lặp `uedp_task_scheduler()`

## IV. Các lưu ý quan trọng

- Việc phân bổ mức độ ưu tiên cho các tác vụ là rất quan trọng để đảm bảo rằng Core có thể xử lý tín hiệu một cách chính xác. Nếu tất cả các tác vụ đều có cùng mức độ ưu tiên thì Core sẽ gặp lỗi xử lý tín hiệu, do đó cần lưu ý việc phân bổ mức độ ưu tiên cho các tác vụ trong hệ thống.
- Khi sử dụng cơ chế truyền tham chiếu để truyền địa chỉ của dữ liệu vào payload của message, cần đảm bảo rằng các buffer chứa dữ liệu này có phạm vi toàn cục để tránh lỗi truy cập bộ nhớ khi message được xử lý sau khi biến cục bộ đã hết phạm vi.
- Trong thiết kế TSM, việc sử dụng cơ chế "Stay" và "Back" giúp tối ưu hóa hiệu suất và tránh lặp lại các hàm on_entry và on_exit không cần thiết, tuy nhiên cần lưu ý rằng việc sử dụng cơ chế này cần phải được thực hiện một cách cẩn thận để đảm bảo rằng logic chuyển trạng thái vẫn được duy trì một cách chính xác và không gây ra lỗi logic trong hệ thống.
- Khi thiết kế FSM, việc sử dụng mô hình Pointer-Swapping giúp đạt được sự linh hoạt tối đa, tuy nhiên cần lưu ý rằng việc thay đổi logic xử lý ngay lập tức chỉ bằng một phép gán con trỏ có thể dẫn đến lỗi nếu không được quản lý cẩn thận, do đó cần đảm bảo rằng các trạng thái và logic xử lý được thiết kế một cách rõ ràng và dễ hiểu để tránh nhầm lẫn và lỗi logic trong hệ thống.
- Kể từ 1.1.5, các lỗi nghiêm trọng trong Core được báo cáo qua FCR; cần lưu ý rằng một số mã lỗi có hành động xử lý là `UEDP_FCR_ACT_SYS_RESET`/`UEDP_FCR_ACT_SYS_PANIC`, sẽ khởi động lại hoặc dừng hệ thống ngay khi raise, kể cả khi raise từ mã lỗi tự khai báo ở tầng ứng dụng.
- Kể từ 1.1.6, khi dùng Dpool GDA, chỉ nên đăng ký các biến có phạm vi `static`/`global` vào GDP; đăng ký một biến cục bộ sẽ khiến con trỏ lấy qua `uedp_gdp_get_ref()` trỏ tới vùng nhớ không còn hợp lệ sau khi hàm khai báo kết thúc.
- Trong quá trình phát triển ứng dụng, nên tuân thủ theo các hướng dẫn và cấu trúc đã đề ra để đảm bảo tính nhất quán và dễ quản lý trong hệ thống, đồng thời nên thường xuyên kiểm tra và debug để đảm bảo rằng hệ thống hoạt động một cách ổn định và hiệu quả.
