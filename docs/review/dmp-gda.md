# [GDA] Global Data Area - Cân nhắc quản lý biến toàn cục phục vụ truyền tham chiếu trong PLD/μE-LS

> Trạng thái: **đang đánh giá, chưa triển khai**. Trả lời cho REVIEW/NOTE/TODO tại `docs/uels-syntax.md:~700-745` (mục "Khu vực dữ liệu toàn cục - Global Data Area").

μE-LS cho phép khai báo biến toàn cục qua khối `glbda:` (tên, kiểu dữ liệu, giá trị khởi tạo), dùng làm nguồn dữ liệu cho `actv`/`act` thông qua `data: *gdaN` kèm `ptype: REF` (truyền tham chiếu) hoặc `ptype: VAL` (truyền tham trị) - phục vụ đúng cơ chế `[D2MP]` đã có. Câu hỏi cần trả lời: có cần bổ sung 1 `dpool` mới trong core (`uedp_msg.c`) riêng cho GDA hay không, và ai chịu trách nhiệm quản lý vòng đời/kích thước khi `ptype: REF` được dùng.

## Không cần dpool mới ở tầng core - `ALLOC` hiện tại đã đủ

Rà lại `[D2MP]` thì thấy lo ngại ban đầu về truyền tham chiếu (*"tránh việc truyền trực tiếp địa chỉ của biến cục bộ vào payload... dẫn đến lỗi truy cập bộ nhớ khi message được xử lý sau khi biến cục bộ đã hết phạm vi"*) là **về biến cục bộ (local)**, có thời gian sống giới hạn trong 1 lần gọi hàm. Biến khai báo qua `glbda:` thì ngược lại - đây là **biến toàn cục thật (static storage duration)**, tồn tại suốt vòng đời chương trình, không có nguy cơ dangling-pointer như biến cục bộ. Do đó, việc `uedp_msg_set_data_ref(msg, &GLOBAL_VAR)` cho 1 biến GDA là an toàn ngay với cơ chế D2MP hiện có, **không cần FIFO tham chiếu toàn cục riêng** như đề xuất ban đầu cho trường hợp biến cục bộ.

Về kích thước: `ptype: REF` chỉ cần payload đủ chỗ chứa 1 con trỏ (`sizeof(void*)`) - pool `ALLOC` hiện đã được cấp `sizeof(void*) * 2u` trở lên theo đúng quy tắc `[DMP]` (`sizeof(void*) * 2^n`), thừa đủ chỗ cho 1 con trỏ tham chiếu mà không cần pool chuyên biệt mới.

## Vấn đề thật sự cần giải quyết: không nằm ở core runtime, mà ở tầng PLTF codegen

Câu hỏi gốc *"ai sẽ thực thi quyền quản lý... để copy dữ liệu từ biến toàn cục sang message"* thực chất đang gộp chung 2 vấn đề khác nhau:

1. **Sinh vùng nhớ tĩnh thật cho `glbda:`** - đây là việc của PLTF, không phải của `uedp_msg.c`. Hiện `pltf/pycdscriptor/generators/` đã có `corecfgpgen.py`, `appcfgpgen.py`, `appdeclpgen.py`... nhưng **chưa có generator nào sinh khai báo biến toàn cục thật** từ khối `glbda:`. Đề xuất bổ sung 1 generator mới (ví dụ `gda_tsgen.py`) sinh ra 1 cặp file `.h`/`.c` khai báo đúng các biến `name`/`type`/`initial_value` đã mô tả trong YAML - đây mới là "nơi quản lý" thật sự của dữ liệu GDA, không phải 1 dpool kiểu message-pool.
2. **Truy cập đồng thời (concurrency)** - đây là vấn đề thật, nhưng khác bản chất với lo ngại "dangling pointer" ban đầu. GDA đưa **shared mutable global state** (bypass hàng đợi message, truy cập trực tiếp qua con trỏ) trở lại vào một hệ thống vốn được thiết kế xoay quanh message-passing chính vì muốn tránh race condition giữa các task. Khi 1 task đang `ptype: VAL` (copy dữ liệu vào biến toàn cục) trong lúc task khác đang đọc qua `ptype: REF`, cần bảo vệ bằng critical section - tái dùng đúng cặp `pal_enter_critical()`/`pal_exit_critical()` đã dùng nhất quán khắp core, không cần cơ chế đồng bộ mới.

## Đề xuất kết luận

- **Không bổ sung dpool GDA mới trong `uedp_msg.c`** - tái dùng nguyên `ALLOC` pool hiện có cho việc truyền tham chiếu tới biến GDA.
- **Bổ sung generator PLTF mới** (`gda_tsgen.py` hoặc tên tương đương) để sinh vùng nhớ tĩnh thật từ khối `glbda:` - đây là hạng mục thuộc phạm vi PLD/μE-LS (v1.1.7 theo lộ trình đã đề xuất), không phải core (`uedp_msg.c`).
- **Truy cập GDA đồng thời giữa các task phải bọc `pal_enter_critical()`/`pal_exit_critical()`** - cần ghi rõ yêu cầu này vào tài liệu cú pháp `glbda:`/`ptype:` để người dùng μE-LS biết đây không phải truy cập "miễn phí", vẫn cần core (hoặc code sinh ra) chèn bảo vệ tương ứng khi copy dữ liệu.
- Việc này nên được xác nhận thêm trước khi triển khai generator, vì đụng tới cả `arch-design.md` (core) lẫn `uels-syntax.md` (cú pháp) lẫn `pltf/` (codegen) - phạm vi rộng hơn 1 thay đổi đơn lẻ.

## Review 19/08/2026 090000

### Bổ sung generator PLTF mới

Chấp thuận vì hiện tại chưa có take into account cho `glbda:` trong codegen, cần sinh ra biến tĩnh thật để tránh dangling pointer khi dùng `ptype: REF`.

### Truy cập GDA đồng thời giữa các task phải bọc bảo vệ

Ở thời điểm hiện tại μEDP chưa phát triển đến việc sử dụng cho môi trường đa nhân như AMP/SMP, do đó việc truy cập biến cục bộ chỉ xảy ra tuần tự ở 1 task duy nhất trong từng vòng lập lịch.

ISR cũng không thể xảy ra việc tranh chấp vì ISR không được phép gọi `actv`/`act`. Do đó, việc bọc critical section là **không cần thiết** ở thời điểm hiện tại, nhưng vẫn nên ghi chú trong tài liệu cú pháp để người dùng biết rằng nếu triển khai μEDP cho môi trường đa nhân trong tương lai thì cần bổ sung critical section.

Ít nhất cần tới thời điểm HELF/AMP version thì mới cần bổ sung critical section, do đó có thể ghi chú trong tài liệu cú pháp rằng việc bọc critical section là tùy chọn cho môi trường đa nhân, nhưng không bắt buộc cho môi trường đơn nhân hiện tại.

### Không bổ sung dpool GDA mới

Do theo thiết kế ban đầu, ALLOC được sử dụng để dành cho mục đích điều động và cấp phát bộ nhớ cho các message với các API hiện hữu.

Hiện tại chưa có API triển khai việc sử dụng ALLOC cho các mục đích ngoài nên cần suy xét lại việc sử dụng thêm 1 dpool GDA (`GLBAL`) mới với kích thước `sizeof(void)`.

Nếu sử dụng ALLOC, giả sử khai báo 1 biến toàn cục như `int x = 1;` và `const char* str = "Hello";`, thì làm sao để sử dụng với bản chất thông tin là biến toàn cục hỗ trợ truyền tham trị và tham chiếu. Ngoài ra, sau khi sử dụng xong thì message sẽ có thể được giải phóng, nhưng biến toàn cục vẫn tồn tại trong bộ nhớ, do đó việc sử dụng ALLOC cần phải có cơ chế vòng đời trên dpool. Trong khi đó việc sử dụng dpool GDA cho phép các biến toàn cục được assign vị trí an toàn và không cần quản lý vòng đời của chúng, do đó việc sử dụng dpool GDA là hợp lý hơn.

## Phản hồi vòng 2 (Minh)

### Critical section

Đúng là đã bỏ qua đúng đặc điểm cốt lõi của scheduler hiện tại: single-core, không preemptive, mỗi vòng lập lịch chỉ dispatch đúng 1 task tại 1 thời điểm; ISR không được gọi `actv`/`act` nên không có đường tranh chấp nào từ ngắt cả. Rút lại đề xuất bọc `pal_enter_critical()`/`pal_exit_critical()` bắt buộc - **không cần thiết ở bản hiện tại**. Sẽ ghi chú trong tài liệu cú pháp `glbda:` rằng việc bọc critical section chỉ cần cân nhắc khi μEDP phát triển tới môi trường đa nhân (AMP/SMP/HELF), không bắt buộc cho môi trường đơn nhân hiện tại.

### Dpool GDA riêng - đồng ý, rút lại kết luận "không cần dpool mới"

Đồng ý với lập luận: `ALLOC` được thiết kế gắn chặt với vòng đời message (`uedp_msg_alloc()`/`uedp_msg_free()`), không có API nào cho phép dùng nó ngoài mục đích message. Ví dụ cụ thể (`int x = 1;`, `const char* str = "Hello";`) cho thấy đúng vấn đề: nếu tái dùng `ALLOC`, sẽ phải tự chế thêm cơ chế "never-free" đè lên trên vòng đời message sẵn có - phức tạp hơn hẳn so với việc có 1 dpool riêng mà biến toàn cục chỉ cần được gán vị trí an toàn, không cần quản lý vòng đời gì cả (không có khái niệm "free" một biến toàn cục).

**Kết luận cuối cùng**: bổ sung 1 dpool riêng cho dữ liệu toàn cục, tách hẳn khỏi `uedp_msg`'s `ALLOC`. Việc này khớp đúng với task tiếp theo đã được giao: triển khai **GDP (Global Data Pool)**, định danh nội bộ hệ thống `GAXES`. Thiết kế cụ thể (cấu trúc slot, API, tích hợp FCR) sau khi triển khai xong, thay thế hẳn đề xuất "tái dùng ALLOC" đã đưa ra ở vòng 1.

Generator PLTF mới cho `glbda:`sẽ sinh code gọi vào API đăng ký của GDP thay vì tự khai báo biến C rời rạc - giữ đúng tinh thần "PLTF sinh code, core quản lý vòng đời".

### Kết luận cuối cùng

Thống nhất

- **Bổ sung dpool GDA riêng** trong core (`uedp_msg.c`) để quản lý biến toàn cục phục vụ truyền tham chiếu trong PLD/μE-LS.
- **Bổ sung generator PLTF mới** (`gda_tsgen.py` hoặc tên tương đương) để sinh ra code khai báo biến toàn cục thật từ khối `glbda:` - đây là hạng mục thuộc phạm vi PLD/μE-LS (v1.1.7 theo lộ trình đã đề xuất), không phải core (`uedp_msg.c`).

## Cập nhật: API GDP đã triển khai (22/08/2026)

API `uedp_gdp_*` đã được chèn thẳng vào `uedp_msg.h`/`uedp_msg.c` (không tách file riêng, cùng lý
do đã bàn ở vòng review trước - GDP là phần mở rộng của DMP/D2MP, không phải module ngang hàng).

- Struct `uedp_gdp_slot_t` (`name`/`data`/`size`/`in_use`) + bảng tĩnh `UEDP_GDP_MAX_SLOTS` (mặc
  định 16, theo pattern `LOGDP_MAX_OUTPUT_FN`).
- 5 hàm: `uedp_gdp_init()`, `uedp_gdp_register()`, `uedp_gdp_unregister()`, `uedp_gdp_get_ref()`
  (cho `ptype: REF`), `uedp_gdp_get_val()`/`uedp_gdp_set_val()` (cho `ptype: VAL`).
- Đúng như kết luận đã chốt: **không** dùng lại `ALLOC`, **không** có khái niệm free/vòng đời -
  chỉ đăng ký tên ↔ con trỏ.
- `uedp_gdp_get_ref()` **không** bọc `pal_enter_critical()`/`pal_exit_critical()`, đúng theo kết
  luận "Phản hồi vòng 2": scheduler hiện tại single-core, non-preemptive, ISR không gọi
  `actv`/`act` nên không có tranh chấp thật. Đã ghi chú rõ trong docstring rằng cần xem xét lại
  nếu μEDP lên môi trường đa nhân (AMP/SMP/HELF).
- 4 mã FCR (`GDP_TABLE_FULL`/`GDP_NOT_FOUND`/`GDP_INVALID_PARAM`/`GDP_DUPLICATE_NAME`, module
  `0x97`) đã có sẵn trong `uedp_fcr.h` từ trước, dùng trực tiếp không cần thêm gì.
- Đã `gcc -fsyntax-only -Wall -Wextra` trên bản gộp - sạch, không phát sinh lỗi/warning mới.

**Còn lại (ngoài phạm vi core)**: generator `gda_tsgen.py` phía PLTF để sinh code gọi
`uedp_gdp_register()` từ khối `glbda:`.
