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
