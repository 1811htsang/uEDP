# Đánh giá FCR và đề xuất DEM, OPC/APC/IOC (v1.1.9 DOCS-001, DOCS-002)

> **Trạng thái: đang đánh giá, chưa triển khai.** Tài liệu phục vụ ticket `v1.1.9/DOCS-001` và `DOCS-002` (`docs/to-do.md`, epic `Rv1.1.8-i0/core.fcr-update`). Không thay đổi code.

## 1. Hiện trạng FCR

Theo `sources/core/inc/uedp_fcr.h` và `sources/core/src/uedp_fcr.c`:

- Mã FCR là `ui16`, byte cao là MOD, byte thấp là SUB (`UEDP_FCR_CODE(mod, sub)`).
- `g_fcr_table[]` là bảng `static const`. Mỗi entry gồm `severity` (WARN/ERROR/FATAL) và `action` (LOG_ONLY / RESET_TASK / SYS_RESET / SYS_PANIC).
- `uedp_fcr_raise()` ghi log qua itnlog trước, rồi thực thi action.
- FCR không có trạng thái: không debounce, không điều kiện bật/tắt, không lưu lại sau reset.

Trong ngữ cảnh DEM, FCR hiện đóng vai trò "DTC tức thời": có mã định danh và xử lý ngay, không có tầng đánh giá trung gian.

## 2. Phát hiện khi rà soát

Các điểm dưới đây nên được xử lý trước khi mở rộng sang DEM. Đây là các điểm để ghi nhận. Chưa sửa code trong tài liệu này.

| # | Vấn đề | Vị trí | Hệ quả |
| --- | --- | --- | --- |
| 1 | `UEDP_FCR_MSG_POOL_REF_RCM` được raise nhưng không có entry trong `g_fcr_table[]` | `uedp_msg.c:394`, `uedp_fcr.c` (bảng) | `uedp_fcr_lookup()` trả về entry `UEDP_FCR_UNKNOWN` (FATAL / SYS_PANIC), nên hệ thống panic. Ý định của mã này là chỉ báo "cần dùng GDP" và không panic. **Đây là điểm ưu tiên cao nhất.** |
| 2 | `UEDP_FCR_MSG_NO_REG_PSE` đã define nhưng không có entry và không được raise ở đâu | `uedp_fcr.h:61` | Mã chết hiện tại. Nếu sau này được raise mà chưa thêm entry thì sẽ panic như mục 1. |
| 3 | `TASK_QUEUE_FULL` trong code là `ERROR` + `SYS_PANIC` | `uedp_fcr.c:33` | `docs/test/pse-gdp-fcr-testset.md` (mục 3) ghi `ERROR` + `RESET_TASK`. Tài liệu và code không khớp, cần chốt một bên. |
| 4 | Log không chứa mã FCR gốc, chỉ có `desc` hoặc `extra_msg` | `uedp_fcr.c:100-105` | Khi debug, không thể biết chính xác mã nào được raise. Với DEM, cần mã để làm khoá định danh event. |
| 5 | `UEDP_FCR_ACT_RESET_TASK` chưa có handler | `uedp_fcr.c:108-122` | Đã được ghi nhận trong `arch-design-en.md` (Limitations). Nhắc lại vì đây là tiền đề của DOCS-001. |
| 6 | Comment `// DEPRECATED - Old TASK` đứng sau bảng | `uedp_fcr.c:69` | Comment đã lỗi thời, cần cập nhật hoặc xoá. |
| 7 | `UEDP_FCR_LIB_COLLAPSE` nằm trong khối `[PAL]` của bảng | `uedp_fcr.c:59` | Chỉ là sắp xếp, không ảnh hưởng hành vi. |

Gợi ý sửa cho mục 1 (chưa áp dụng): thêm entry `{ UEDP_FCR_MSG_POOL_REF_RCM, "MSG no suitable pool, use GDP ref", UEDP_FCR_SEV_ERROR, UEDP_FCR_ACT_LOG_ONLY }`. Đây là thay đổi hành vi, nên tách thành task riêng.

## 3. DOCS-001: Cơ chế DEM kết hợp FCR (đề xuất)

### 3.1 Khái niệm

- **DEM (Diagnostic Event Management)**: tầng nhận báo cáo lỗi từ nhiều nguồn (FCR raise, monitor ở tầng ứng dụng), đánh giá qua các giai đoạn, rồi quyết định trạng thái sự kiện và lưu lại.
- **Event**: một FCR code cộng với ngữ cảnh. FCR code đóng vai trò khoá định danh của event, tương đương DTC.

Luồng đề xuất:

```graph
Nguồn ──report(code, qualifier)──> ECS ──> DBS ──> Event status ──> pzsto
(FCR raise /                     (enable?)  (qualified?)   (FCR action, cờ OPC/APC/IOC)
 monitor ứng dụng)
```

| Giai đoạn | Vai trò | Ví dụ |
| --- | --- | --- |
| **ECS** (Enable Condition Stage) | Quyết định event có được phép xét hay không. Nếu không enable, báo cáo bị bỏ và chỉ tăng bộ đếm "suppressed". | Monitor ADC chỉ bật khi nguồn đã ổn định. |
| **DBS** (Debouncing Stage) | Lọc nhiễu. Hai dạng: theo bộ đếm (N lần lỗi liên tiếp, hoặc M trong cửa sổ) và theo thời gian (lỗi kéo dài ≥ T ms). | Một lỗi tức thời không được xác nhận; lỗi kéo dài 50 ms thì được xác nhận. |
| **Event status** | Trạng thái của event sau DBS: đang debounce, đã xác nhận (confirmed), đã hồi phục (healed). | |
| **pzsto** (persistence store) | Lưu event đã confirmed vào bộ nhớ không mất khi reset, để đọc lại sau khởi động. | Cần NVM/EEPROM qua PAL. |

### 3.2 Quan hệ với FCR action

FCR action vẫn là điểm xử lý cuối cùng. Đề xuất phân luồng theo severity:

| Severity | Đi qua ECS / DBS | Ghi pzsto | Action |
| --- | --- | --- | --- |
| WARN | Có | Khi confirmed | LOG_ONLY |
| ERROR | Có | Khi confirmed | LOG_ONLY hoặc RESET_TASK |
| FATAL | **Không** (bypass) | Best-effort, xem câu hỏi mở | SYS_RESET hoặc SYS_PANIC |

Lý do bypass với FATAL: panic hoặc reset không thể chờ debounce. Nếu để enable condition chặn FATAL, lỗi nghiêm trọng có thể bị nuốt.

### 3.3 Câu hỏi mở

- Ghi pzsto trong đường panic có an toàn không? Nếu ghi flash trong lúc `pal_sys_fatal()` có thể làm hỏng dữ liệu hoặc treo hệ thống. Cần quyết định ghi trước hay sau `pal_sys_fatal()`, hoặc không ghi.
- Số lượng event tối đa trong pzsto và chính sách khi đầy: ghi đè vòng (ring) hay giữ lần xuất hiện đầu tiên.
- API đọc và xoá (clear) pzsto thuộc PAL nào. Repo hiện không có API NVM chung. Kết quả tìm kiếm chỉ thấy tham chiếu trong `sources/pal/arch/stm32f103/` và `stm32h723/`, cần xác nhận thêm.
- Cấu hình ECS/DBS là tĩnh (bảng biên dịch) hay runtime?
- DEM chỉ tiêu thụ FCR raise, hay cũng được phép tự phát event?

### 3.4 Hướng ứng dụng trong framework

- `uedp_fcr_raise()` nên giữ nguyên làm điểm vào. DEM được gắn phía sau, để không phải sửa khoảng 76 call site FCR hiện có.
- Tham số ECS/DBS có thể được khai báo trong μE-LS (PLD). Syntax cụ thể cần đề xuất riêng, không nằm trong phạm vi DOCS-001.

## 4. DOCS-002: OPC, APC, IOC (định nghĩa làm việc)

Repo hiện không có định nghĩa nào cho ba thuật ngữ này. Các định nghĩa dưới đây là **đề xuất làm việc**, cần người phụ trách ticket xác nhận trước khi dùng trong tài liệu chính thức.

| Viết tắt | Tên đầy đủ | Phạm vi chặn (đề xuất) | Ví dụ |
| --- | --- | --- | --- |
| **OPC** | Operation Preventing Condition | Chặn việc thực hiện một operation (lệnh hoặc chức năng) đang được yêu cầu. | Không cho phép lệnh điều khiển actuator khi nguồn đang lỗi. |
| **APC** | Activation Preventing Condition | Chặn việc kích hoạt (start, vào trạng thái active) một chức năng hoặc service. | Không kích hoạt OCE service khi điều kiện chưa thoả. |
| **IOC** | Inhibition Preventing Condition | Ức chế một đường tín hiệu hoặc đầu ra cụ thể, trong khi chức năng vẫn chạy. | Không publish một signal ra ngoài khi module phát tín hiệu đang lỗi. |

Phân biệt đề xuất giữa APC và OPC: APC được kiểm tra tại thời điểm chuyển vào trạng thái hoạt động. OPC được kiểm tra mỗi lần operation được thực hiện. Cần xác nhận lại với người phụ trách ticket.

### 4.1 Liên kết DOCS-001 và DOCS-002

- DEM là nguồn: khi event đạt trạng thái confirmed, sinh ra cờ điều kiện tương ứng.
- OPC, APC, IOC là đầu ra tác động: các cờ này được consumer kiểm tra trước khi thực hiện hành động.
- Đây là đường nối giữa hai ticket: DOCS-001 mô tả cách sinh cờ, DOCS-002 mô tả cách cờ chặn hành động.

### 4.2 Điểm ứng dụng tiềm năng (chưa quyết định)

- `ocesvc_scheduler()` trong `uedp_ocesvc.c`: APC trước khi chạy service.
- `uedp_tsm` / `uedp_fsm`: APC khi chuyển vào state.
- `uedp_msg_publish()` trong `uedp_msg.c`: IOC theo signal.
- Tầng operation/actuator ở ứng dụng: OPC.

Lưu ý: OCE được thiết kế theo nguyên tắc chỉ chạy khi hệ thống rảnh và service ngắn (xem `docs/review/ocesvc-mexecjn.md`). Nếu thêm APC vào OCE, cần cân nhắc để không làm tăng độ phức tạp của scheduler.

## 5. Bước tiếp theo đề xuất

1. Xử lý các điểm mục 2, đặc biệt mục 1 (`POOL_REF_RCM` gây panic) và mục 3 (không khớp test set). Tách thành task riêng.
2. Xác nhận định nghĩa OPC, APC, IOC với người phụ trách ticket (mục 4).
3. Quyết định pzsto có phụ thuộc API NVM của PAL hay không, và cách xử lý ghi trong đường panic (mục 3.3).
4. Sau khi các điểm trên được chốt, mới cập nhật `docs/eng/arch-design-en.md`, `docs/vni/arch-design.md` và user manual.

Hai ticket DOCS-001 và DOCS-002 trong `docs/to-do.md` vẫn để trạng thái `[ ]`, cho đến khi người phụ trách xác nhận nội dung.

## Trạng thái cập nhật cho các findings

- Vấn đề 1,2 đã được giải quyết trong nhánh `sub-feat/Rv1.1.7-bf1` (commit `core.fcr-update`), nhưng chưa merge vào nhánh chính để cho ra release.
- Vấn đề 3 đã được giải quyết từ trước khi release phiên bản 1.1.7, do đó không cần bổ sung tài liệu.
- Vấn đề 4 sẽ được open ticket riêng để bổ sung log FCR code hay các concatenation đặc biệt.
- Vấn đề 5 đã deprecated do không còn sử dụng, nên không cần cập nhật tài liệu.
- Vấn đề 6,7 là các vấn đề về comment và sắp xếp, không ảnh hưởng hành vi, nên không cần cập nhật tài liệu.
