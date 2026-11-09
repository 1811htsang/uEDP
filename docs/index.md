# μEDP Index — Docs ⇄ Code Mapping

## 0. Cách dùng

1. Đặt `index.md`, `arch-design.md`, `arch-design-en.md` trong cùng thư mục với repo `uEDP` đã clone (ví dụ `uEDP/docs/arch-design.md`, `uEDP/docs/arch-design-en.md`, `uEDP/docs/index.md`).
2. Tra tag `[XXX]` trong doc → tìm dòng tương ứng ở bảng bên dưới → mở code qua `[[...]]`.
3. Khi nghi ngờ câu tiếng Anh dịch sai một chi tiết kỹ thuật (số, tên hằng, tên API), mở thẳng code đã map để xác minh thay vì tin vào câu chữ của bản dịch.

---

## 1. Kiến trúc 3 tầng

| Tầng | Doc (VN) | Doc (EN) | Code |
| --- | --- | --- | --- |
| Application Layer | [[arch-design.md#Application Layer (Tầng Ứng dụng)]] | [[arch-design-en.md#Application Layer]] | [[sources/app/app.c]] · [[sources/app/declaration/app_decl.h]] · [[sources/app/config/app_cfg.h]] · [[sources/app/interface/task_if.h]] · [[sources/app/interface/task_if.cpp]] |
| μEDP Core (bất biến) | [[arch-design.md#μEDP Core (Tầng Lõi - Bất biến)]] | [[arch-design-en.md#μEDP Core (Core Layer — Immutable)]] | [[sources/core/inc/uedp_core.h]] |
| PAL | [[arch-design.md#PAL - Platform Abstraction Layer (Tầng Trừu tượng)]] | [[arch-design-en.md#PAL - Platform Abstraction Layer]] | [[sources/pal/pal_core.h]] |

## 2. Core modules (`sources/core/`)

| Module | Doc (VN) | Code header | Code impl |
| --- | --- | --- | --- |
| Message Manager | [[arch-design.md#Message Manager]] | [[sources/core/inc/uedp_msg.h]] | [[sources/core/src/uedp_msg.c]] |
| Task Manager | [[arch-design.md#Task Manager]] | [[sources/core/inc/uedp_task.h]] | [[sources/core/src/uedp_task.c]] |
| Timer Service | [[arch-design.md#Timer Service]] | [[sources/core/inc/uedp_timer.h]] | [[sources/core/src/uedp_timer.c]] |
| FSM Engine | [[arch-design.md#FSM - Finite State Machine]] | [[sources/core/inc/uedp_fsm.h]] | [[sources/core/src/uedp_fsm.c]] |
| TSM Engine | [[arch-design.md#TSM - Task State Machine]] | [[sources/core/inc/uedp_tsm.h]] | [[sources/core/src/uedp_tsm.c]] |
| Itnlog | [[arch-design.md#[PPLP] Plug-N-Play Logging Pipeline - Cơ chế logging linh hoạt và có thể mở rộng]] | [[sources/core/inc/uedp_itnlog.h]] | [[sources/core/src/uedp_itnlog.c]] |
| OCE (ocesvc) | [[arch-design.md#[OCE] Out-Context Execution - Thực thi ngoài ngữ cảnh]] | [[sources/core/inc/uedp_ocesvc.h]] | [[sources/core/src/uedp_ocesvc.c]] |
| ISR Bridge | [[arch-design.md#ISR Bridge]] | — | [[sources/core/src/uedp_msg.c]] (drain_isr_pool) · [[sources/core/src/uedp_task.c]] (scheduler + post_isr) |

## 3. Logic thiết kế chi tiết (các mục có tag `[XXX]`)

| Tag | Tên | Doc (VN) | Code chính |
| --- | --- | --- | --- |
| `[DMP]` | Deterministic Memory Pooling | [[arch-design.md#[DMP] Deterministic Memory Pooling - Quản lý bộ nhớ tin nhắn với cấp phát tĩnh độc lập vào kiến trúc]] | [[sources/core/inc/uedp_msg.h]] · [[sources/core/src/uedp_msg.c]] |
| `[SII]` | Safe ISR Injection | [[arch-design.md#[SII] Safe ISR Injection - Cơ chế an toàn để truyền tín hiệu từ ISR vào hệ thống]] | [[sources/core/src/uedp_msg.c]] (FIFO ISR) · [[sources/core/src/uedp_task.c]] (`uedp_task_norm_post_isr`) |
| `[D2MP]` | Data-to-Message Passing | [[arch-design.md#[D2MP] Data-to-Message Passing - Cơ chế xử lý truyền dữ liệu an toàn và hiệu quả qua message]] | [[sources/core/inc/uedp_msg.h]] (`uedp_msg_set_data_val/ref`) · ví dụ dùng: [[sources/test/test02/test.c]] |
| `[HSMC]` | Hybrid State Machine Control (TSM+FSM) | [[arch-design.md#[HSMC] Hybrid State Machine Control - Cơ chế quản lý máy trạng thái kết hợp giữa TSM và FSM]] | [[sources/core/inc/uedp_tsm.h]] / [[sources/core/src/uedp_tsm.c]] · [[sources/core/inc/uedp_fsm.h]] / [[sources/core/src/uedp_fsm.c]] · ví dụ TSM: [[sources/test/test01/test.c]] · ví dụ FSM: [[sources/test/test03/test.c]] |
| `[HES]` | Heximal Encoding Signals | [[arch-design.md#[HES] Heximal Encoding Signals - Mã hóa tín hiệu theo hệ thập lục phân]] | [[sources/core/inc/uedp_core.h]] (định nghĩa dải `TASK_NORM/TASK_POLL/TASK_PRI/FSM_SIG/TSM_SIG/TSM_STATE`) |
| `[PPLP]` | Plug-N-Play Logging Pipeline | [[arch-design.md#[PPLP] Plug-N-Play Logging Pipeline - Cơ chế logging linh hoạt và có thể mở rộng]] | [[sources/core/inc/uedp_itnlog.h]] / [[sources/core/src/uedp_itnlog.c]] · [[sources/pal/service/logdp/pal_logdp.h]] / [[sources/pal/service/logdp/pal_logdp.c]] · [[sources/pal/service/rprintf/pal_rprintf.h]] / [[sources/pal/service/rprintf/pal_rprintf.c]] · [[sources/common/xprintf/xprintf.h]] / [[sources/common/xprintf/xprintf.c]] |
| `[APE]` | Atomic Priority Escalation | [[arch-design.md#[APE] Atomic Priority Escalation - Tăng ưu tiên phân tử tạm thời]] | [[sources/core/inc/uedp_task.h]] / [[sources/core/src/uedp_task.c]] (`uedp_task_norm_set_urgent`, `internal_task_norm_reset_pri`) |
| `[SLNF]` | Safe LIFO-nested FIFO | [[arch-design.md#[SLNF] Safe LIFO-nested FIFO - Cơ chế xử lý tin nhắn khẩn cấp an toàn]] | [[sources/common/container/fifo/fifo.h]] / [[sources/common/container/fifo/fifo.c]] (`fifo_put_head`) |
| `[OCE]` | Out-Context Execution | [[arch-design.md#[OCE] Out-Context Execution - Thực thi ngoài ngữ cảnh]] | [[sources/core/inc/uedp_ocesvc.h]] / [[sources/core/src/uedp_ocesvc.c]] |
| `[SIF]` | Safe Input Filter (old SOCI) | [[arch-design.md#[SIF] Safe Input Filter - Bộ lọc đầu vào an toàn = old [SOCI]]] | ⚠️ **Chưa có code** — chỉ là tiêu đề đề xuất trong doc, chưa thấy module tương ứng trong `sources/`. Theo dõi ở [[docs/to-do.md]]. |
| `[KwDI]` | Kconfig with Docker Integration | [[arch-design.md#[KwDI] Kconfig with Docker Integration - Tích hợp Kconfig với Docker]] | [[Kconfig]] · [[Dockerfile]] · [[sources/app/kconfig/core.kconfig]] · [[sources/app/kconfig/decl.kconfig]] · [[sources/app/kconfig/pal.kconfig]] · [[sources/common/kconfiglib/kconfiglib.py]] |

## 4. PAL / kiến trúc phần cứng

| Mục | Code |
| --- | --- |
| PAL core interface | [[sources/pal/pal_core.h]] |
| logdp | [[sources/pal/service/logdp/pal_logdp.h]] · [[sources/pal/service/logdp/pal_logdp.c]] |
| rprintf | [[sources/pal/service/rprintf/pal_rprintf.h]] · [[sources/pal/service/rprintf/pal_rprintf.c]] |
| memrp (memory profiler, nhắc ở README, chưa có trong arch-design) | [[sources/pal/service/memrp/pal_memrp.h]] · [[sources/pal/service/memrp/pal_memrp.c]] |
| Port STM32F103 | [[sources/pal/arch/stm32_f103/stm32_f103_arch.h]] · [[sources/pal/arch/stm32_f103/stm32_f103_arch.c]] |
| Port STM32H723 | [[sources/pal/arch/stm32_h723/stm32_h723_arch.h]] · [[sources/pal/arch/stm32_h723/stm32_h723_arch.c]] |
| Port ESP32-WROOM-32 | [[sources/pal/arch/esp32_wr32/esp32_wr32_arch.h]] · [[sources/pal/arch/esp32_wr32/esp32_wr32_arch.c]] |
| Port ESP32-S3 | [[sources/pal/arch/esp32_s3/esp32_s3_arch.h]] · [[sources/pal/arch/esp32_s3/esp32_s3_arch.c]] |
| Port Linux (simulation) | [[sources/pal/arch/linux/linux_arch.h]] · [[sources/pal/arch/linux/linux_arch.c]] |

## 5. Container / tiện ích dùng chung (`sources/common/`)

| Loại | Code |
| --- | --- |
| FIFO (dùng cho ISR + S-LnF) | [[sources/common/container/fifo/fifo.h]] · [[sources/common/container/fifo/fifo.c]] |
| Ring buffer | [[sources/common/container/ring_buffer/ring_buffer.h]] · [[sources/common/container/ring_buffer/ring_buffer.c]] |
| Linked list (dùng cho Timer free-list, OCE list) | [[sources/common/container/llist/llist.h]] · [[sources/common/container/llist/llist.c]] |
| xprintf | [[sources/common/xprintf/xprintf.h]] · [[sources/common/xprintf/xprintf.c]] |

## 6. Test mẫu được nhắc trong doc

| Test | Dùng để minh họa | File |
| --- | --- | --- |
| `test01` | TSM (Table-Driven state machine) | [[sources/test/test01/test.c]] · [[sources/test/test01/test.h]] |
| `test02` | D2MP — truyền tham chiếu qua message | [[sources/test/test02/test.c]] · [[sources/test/test02/test.h]] |
| `test03` | FSM (Pointer-Swapping), UART parsing demo | [[sources/test/test03/test.c]] · [[sources/test/test03/test.h]] |
| `test04` | *(doc nhắc "test 04" cho demo filter itnlog theo tag)* | ⚠️ **Không tìm thấy** thư mục `test04` trong repo hiện tại (chỉ có `test01`–`test03`). Cần kiểm tra lại xem tài liệu nói tới test chưa được push, hay cần đổi số thứ tự. |

---

## Ghi chú kiểm tra bản dịch tiếng Anh (arch-design-en.md)

Đã đối chiếu song song `arch-design.md` (VN gốc) và `arch-design-en.md` (EN) theo từng đoạn, đặc biệt các chi tiết kỹ thuật dễ dịch sai (số liệu, dải hex, tên hằng, tên API, công thức):

- Các dải tín hiệu `0xEx / 0xDx / 0xCx / 0xBx / 0xAx / 0xAFx`, số lượng đơn vị (16/8/16/16/16/16), và tên hằng (`TIM, IF, SYS, DBG, USR, IDLE`, `WDG, SYSLF, MEMRP, IDLE`, `UEDP_TASK_PRI_LEVEL_0..15`...) — khớp chính xác giữa 2 bản.
- Công thức và con số ở phần `[APE]` (24 mức ưu tiên = 16 gốc + 8 tạm thời, dải `UEDP_TASK_PRI_LEVEL_16..23`, bước tăng 1 đơn vị, `target_pri = current_max + step`) — khớp chính xác.
- Tên API (`uedp_msg_alloc`, `uedp_msg_free`, `uedp_task_norm_set_urgent`, `internal_task_norm_reset_pri`, `fifo_put_head`, `pal_logdp_dispatch`, `pal_rprintf_flush_entry`, `xfprintf`, v.v.) — giữ nguyên, không bị dịch nhầm.
- Đoạn code mẫu trong `test02` (con trỏ cấp 2, `uintptr_t`) được giữ nguyên y hệt, không bị diễn giải sai.
- Kích thước pool mặc định (`BLANK` 8 đơn vị, `ALLOC/EXTAL/ISR` 16 đơn vị) và công thức `sizeof(void*) * 2^n` — khớp.

→ Nhìn chung bản dịch tiếng Anh **chính xác về mặt kỹ thuật**, không phát hiện sai lệch số liệu hay tên định danh nào trong quá trình đối chiếu song song. Phần này chỉ là rà soát nội dung — nếu muốn rà thêm về văn phong/thuật ngữ tiếng Anh (ví dụ cách dịch "tăng ưu tiên phân tử tạm thời" → "Temporary atomic priority increase") thì cần review riêng, không thuộc phạm vi đối chiếu kỹ thuật ở đây.
