# Đánh giá đề xuất mexecjn (chain) - cho phép thay đổi thứ tự thực thi

> **Trạng thái: đang đánh giá, chưa triển khai** - nội dung dưới đây là bản review + đề xuất.

Thiết kế OCE hiện tại là **FCFS thuần theo thứ tự đăng ký** (`llist_append()` luôn thêm vào cuối, `ocesvc_scheduler()` luôn thực thi node `READY` đầu tiên gặp được) - không có khái niệm ưu tiên hay khả năng đổi thứ tự sau khi đã đăng ký. Yêu cầu "mexecjn" đặt ra: cho phép thay đổi thứ tự các service cần xử lý để đảm bảo sự kiện quan trọng được ưu tiên đúng logic.

**Hai phát hiện khi rà lại `uedp_ocesvc.c`/`.h`, làm rõ đúng nghi vấn đã nêu trong `to-do.md`:**

- `ocesvc_t.id` **không được dùng cho bất kỳ logic quản lý nào** trong core: `ocesvc_unregister()` gọi `llist_remove(&ocesvc_list, svc)` - xoá bằng con trỏ `svc`, không phải bằng `id`; `ocesvc_scheduler()` cũng duyệt bằng con trỏ `llist_node_t`, không tra `id` ở đâu cả. `id` hiện chỉ tồn tại như nhãn debug - khớp đúng với mô tả trong `docs/uels-syntax.md` (mục OCE): *"core hiện tại `ocesvc_register()` tự gán `uint8_t id`, vì vậy `name` ở μE-LS nên được hiểu là nhãn logic..."*, tức bản thân tài liệu μE-LS cũng chưa từng dựa vào `id` cho bất kỳ ngữ nghĩa thứ tự/định danh chức năng nào.
- `ocesvc_t.next` là **field chết**: struct khai báo riêng `struct ocesvc_t* next`, nhưng toàn bộ traversal thực tế lại đi qua `llist_node_t.next` bên trong `llist_t` - field này không bao giờ được gán hay đọc ở đâu, nhiều khả năng là tàn dư từ một thiết kế linked-list thủ công trước khi `llist` được đưa vào dùng chung.

`ocesvc_register()` hiện phải chạy `ocesvc_find_free_id()` - quét tuyến tính tối đa 255 giá trị để tìm ID trống - chỉ để gán cho một con số không phục vụ logic nào cả. Đây là chi phí runtime thật, không chỉ là vấn đề thẩm mỹ code.

**Ba hướng thiết kế cho mexecjn:**

| Hướng | Cách làm | Ưu | Nhược |
| --- | --- | --- | --- |
| A. Priority field + insert có thứ tự | Thêm field `priority` mới, mở rộng `llist.h` với `llist_insert_sorted()` | Thứ tự luôn đúng ngay lúc đăng ký | Phải sửa `llist.h` - thư viện dùng chung cho nhiều module khác, ảnh hưởng rộng hơn phạm vi OCE |
| B. API reorder tường minh (`ocesvc_reorder()`) | Dùng `llist_remove()` + `llist_append()` sẵn có, tự cài lại logic chèn giữa | Không cần sửa `llist.h`, tái dùng nguyên API hiện có | Tốn O(n) mỗi lần reorder, phải viết thêm logic chèn-giữa vì `llist` chỉ có `append` (cuối danh sách) |
| C. Bitmask ready kiểu `TASK_NORM`/`[APE]` | Scheduler chọn theo mức ưu tiên dạng bitmap | Mạnh nhất, nhất quán với core | Over-engineer cho OCE - đi ngược tinh thần "chỉ chạy khi hệ thống rảnh, đơn giản" vốn là lý do OCE tồn tại (xem mục Lợi ích/Tác hại ở trên) |

**Đề xuất: gộp 2 vấn đề (mexecjn + bỏ ID) thành 1 giải pháp duy nhất**, thay vì xử lý riêng lẻ:

- Không thêm field `priority` mới (tránh đổi kích thước struct) mà **tái sử dụng chính field `id` hiện có**, đổi ngữ nghĩa từ "identity tự gán" sang "priority do người dùng khai báo trước khi đăng ký".
- Bỏ hẳn `ocesvc_find_free_id()` - vòng quét O(n) không còn lý do tồn tại vì `id` không còn cần đảm bảo duy nhất theo kiểu auto-increment.
- `ocesvc_scheduler()` đổi từ "node đầu tiên `READY`" sang "node `READY` có `id` (priority) nhỏ nhất" - tái dùng đúng field đang có.
- Xoá luôn field chết `ocesvc_t.next` - không liên quan trực tiếp mexecjn nhưng tiện dọn cùng đợt, giảm kích thước struct thật sự.
- Không cần API `reorder` riêng: muốn đổi thứ tự một service, chỉ cần `unregister()` rồi `register()` lại với `id` (priority) mới - tái dùng đúng 2 API sẵn có, không thêm API mới, không đụng đến `llist.h`.
- Nếu được chấp thuận, `docs/uels-syntax.md` (mục `outexec`) cần bổ sung thêm 1 trường (ví dụ `priority:`) để μE-LS phản ánh đúng ngữ nghĩa mới của `id`.

**Rủi ro cần xác nhận trước khi triển khai**: đây là **breaking change** đối với API `ocesvc_register()` - bất kỳ chỗ nào (hiện tại hoặc PLTF sinh code sau này) đang ngầm định `id` tự tăng dần 0,1,2... theo thứ tự đăng ký (ví dụ để log/debug đếm thứ tự) sẽ bị đổi ý nghĩa hoàn toàn sang "priority do người dùng chọn". Cần rà lại toàn bộ chỗ dùng `.id` của OCE (kể cả trong test và tooling) trước khi đổi.

## Review 12/08/2026 235121

Đối với thiết kế hiện tại OCE được xem xét không triển khai mức ưu tiên mà chỉ thực hiện đăng ký theo thứ tự FCFS, việc bổ sung mức ưu tiên chỉ xuất hiện AOCE (μE-OS) với "xử lý ưu tiên theo thời gian, kèm theo cơ chế expected execution time, quantum và error callback". Do đó ở đề xuất mới nhất là ocesvc.mexecjn và ID-remove, cần thật sự xem xét lại:

1. Việc loại bỏ ID và thay đổi ý nghĩa của ID sang priority có thể gây ra breaking change và không nằm trong dự trù ban đầu của OCE. Tuy nhiên các thiết kế hiện tại của OCE không có cross-module dependency, chỉ có syntax-dependency theo lộ trình phát triển μE-LS, do đó việc thay đổi ý nghĩa của ID có thể được chấp nhận nếu được kiểm tra kỹ lưỡng. Nhưng điều này cũng dẫn việc phải suy xét bổ sung cả SCB-full theo dự trù của AOCE lẫn thay đổi thiết kế syntax của μE-LS, escalate các tính năng dự kiến của AOCE vào OCE, vì nếu không có SCB-full + μE-LS + AOCE-escalate thì việc thay đổi ý nghĩa của ID sẽ gây ra sự không đồng nhất trong cách xử lý các service.
2. Việc bổ sung hỗ trợ thay đổi chuỗi thực thi (mexecjn) có đảm bảo không gây ra sự phức tạp trong việc quản lý các service, đặc biệt là khi có nhiều service đăng ký với cùng một priority không? Ngoài ra, bản thân tính năng này có thật sự cần thiết trong lộ trình đưa OCE lên AOCE hay không, hay chỉ là một tính năng phụ trợ cho OCE? Bởi vì nếu không có AOCE, việc thay đổi thứ tự thực thi sẽ không có ý nghĩa nhiều, vì OCE chỉ chạy khi hệ thống rảnh, và các service được thiết kế để chạy trong thời gian ngắn. Còn ở AOCE, việc thay đổi thứ tự thực thi sẽ được quản lý bởi scheduler của AOCE, do đó việc bổ sung mexecjn trong OCE có thể là thừa.

<!-- TODO
Minh đọc phần review này để bổ sung tiếp tục các đánh giá chi tiết hơn. Dự trù ở tính năng mexecjn và decision ID-remove sẽ chỉ có 1 vòng review này để thống nhất release v1.1.5, đồng thời làm căn cứ để xác định mexecjn có được đưa vào lộ trình thiết kế v1.1.6/7/8 hay không.
-->

## Phản hồi review (Minh)

Đồng ý với cả 2 điểm đã nêu. Đề xuất ban đầu ("tái dùng `id` làm `priority`" + `mexecjn`) đã đánh giá thiếu đúng phần quan trọng nhất: **cả 2 đều ngầm giả định OCE cần một khái niệm ưu tiên**, trong khi thiết kế gốc đã minh định priority chỉ thuộc về AOCE, đi kèm SCB-full. Đổi ý nghĩa 1 field mà không kéo theo toàn bộ hạ tầng đi cùng nó (SCB-full + cập nhật μE-LS + escalate tính năng AOCE tương ứng) đúng là tạo ra trạng thái nửa vời - service bị xử lý không đồng nhất, còn tệ hơn không làm gì. Bổ sung cho điểm 2: đề xuất `mexecjn` trước đó cũng **chưa xử lý trường hợp nhiều service cùng priority** (tie-break) - một lỗ hổng thiết kế thật, càng củng cố việc tính năng này chưa đủ chín để đưa vào OCE ở mức hiện tại.

**Quyết định chốt:**

1. **`mexecjn`: KHÔNG đưa vào OCE, dời hẳn sang phạm vi AOCE/μE-OS.** Đúng như phân tích trên: OCE chỉ chạy lúc hệ thống rảnh với service ngắn hạn, thứ tự thực thi gần như không có giá trị thực tế ở tầng này; khi lên AOCE, việc "đổi thứ tự" sẽ được bao trọn tự nhiên trong logic scheduler thật (ưu tiên theo thời gian, quantum, expected execution time) - không cần thiết kế `mexecjn` như một cơ chế riêng lẻ, tách biệt khỏi scheduler AOCE.
2. **Đổi ý nghĩa `id` → `priority`: KHÔNG làm.** Giữ nguyên lý do đã nêu ở điểm 1 - chi phí kéo theo (SCB-full + μE-LS + AOCE-escalate) vượt xa phạm vi của 1 quyết định đổi field.
3. **Tách riêng phát hiện "`id` không được dùng cho logic quản lý"** (nêu ở phần đầu tài liệu) khỏi quyết định priority - đây là 2 vấn đề độc lập. Đề xuất phạm vi hẹp hơn nhiều cho v1.1.6+ (không nằm trong v1.1.5): bỏ vòng quét O(n) `ocesvc_find_free_id()`, coi `id` như nhãn debug người dùng tự đặt (giống `name` của `rprintf`, đúng như chính `uels-syntax.md` đã mô tả) - **không gán bất kỳ ngữ nghĩa priority/ordering nào**, nên không kéo theo SCB/μE-LS/AOCE-escalate như lo ngại ở điểm 1. Quyết định này để ngỏ, chưa chốt trong vòng review này, chỉ ghi nhận làm điểm cân nhắc riêng khi lên kế hoạch v1.1.6/7/8.
4. **Kết quả cho `docs/to-do.md` mục 1.2.0**: bỏ hẳn `mexecjn` khỏi mọi lộ trình 1.1.6/7/8 đã đề xuất trước đó - tính năng này chỉ xuất hiện lại khi team chính thức bắt tay thiết kế AOCE.
