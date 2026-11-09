# Review: Thiết kế lớn hơn cho Test Level (ut/ct/st/it) — sau khi v1.2.3 hoàn tất

<!-- STATUS - PROPOSAL: review thiết kế, chưa có dòng code nào được viết -->

- Ngày viết: 2026-09-23
- Tách ra từ: [`pltf-impl-post-1.2.1.md`](./pltf-impl-post-1.2.1.md) §B.4 — câu hỏi kiến trúc cố tình chưa giải quyết ở đó: "`level` có nên quyết định cách chạy TSD scenario không?"
- Điều kiện tiên quyết: review này giả định [`pltf-impl.md`](./pltf-impl.md) (TSD/TLC baseline tĩnh, v1.2.1), §B (TLC `level` = metadata thuần, v1.2.2), và §A (runtime coverage, v1.2.3) của `pltf-impl-post-1.2.1.md` đã triển khai xong và ổn định. Review này không thay thế các tài liệu đó — chỉ mở rộng câu hỏi đã cố tình bỏ ngỏ ở §B.4.
- Mục đích: đánh giá chi tiết liệu `level` (ut/ct/st/it) có nên đổi cơ chế thực thi của TSD scenario (thay vì chỉ là nhãn phân loại báo cáo), và nếu có thì thiết kế đó nên như thế nào.

## 1. Bối cảnh: vì sao câu hỏi này quan trọng

Ở baseline (v1.2.1–v1.2.3), mọi TSD scenario — bất kể ý định là kiểm thử ở mức nào — đều chạy qua đúng một cơ chế: `tsd_runner.c` khởi tạo toàn bộ `app_task_table`, chạy `uedp_task_scheduler()` trong vòng lặp có tick thread, và bơm message/ISR/urgent qua đúng API production (`uedp_task_norm_post_msg`, v.v.). Đây thực chất là một bài test system-level (nhiều task, qua scheduler thật, qua PAL Linux thật) cho mọi scenario, dù người viết TSD có gắn nhãn `tlc.level: ut` cho mục coverage liên quan hay không.

Nói cách khác: nhãn `level` ở §B không đổi hành vi chạy — nó chỉ đổi cách nhóm báo cáo. Nếu người dùng thực sự muốn một bài kiểm thử "unit" đúng nghĩa (gọi thẳng 1 hàm, cô lập khỏi scheduler/queue/PAL), baseline hiện tại không cung cấp việc đó. Đây là khoảng cách giữa "gắn nhãn ut" và "chạy như ut" — review này phân tích khoảng cách đó.

## 2. Phân tích từng mức theo đúng nghĩa kiểm thử (không chỉ theo nhãn)

### 2.1. `ut` (unit) — cô lập hoàn toàn khỏi runtime

Đúng nghĩa unit test: gọi trực tiếp một hàm sinh ra bởi codegen (ví dụ một `{state}_onst` cụ thể, hoặc một `{state}_entry`/`{state}_exit`), truyền vào một `uedp_msg_t` dựng tay, và assert side-effect (ví dụ GDA bị ghi, hay giá trị trả về/biến cục bộ nếu có). Không qua `uedp_task_scheduler()`, không qua `uedp_task_norm_post_msg()` (không có hàng đợi), không cần tick thread.

Hệ quả thiết kế: cần một chế độ runner hoàn toàn khác — không dựng `app_task_table`/scheduler, chỉ include đúng các hàm state/action cần test rồi gọi trực tiếp.

### 2.2. `ct` (component) — một task, cô lập khỏi các task khác

Đúng nghĩa: chạy TSM+FSM đầy đủ của một task qua scheduler thật (để test transition chain nhiều bước), nhưng không phụ thuộc hành vi của các task khác trong hệ thống — nghĩa là nếu task này gửi message tới task khác, cần một cách "chặn lại" và ghi nhận (mock) thay vì để task đích thật xử lý (vì task đích có thể chưa sẵn sàng/không phải đối tượng đang test).

Hệ quả thiết kế: cần cơ chế stub hoá cross-task post, khó hơn `ut` nhiều vì phải sửa hành vi của API production (`uedp_task_norm_post_msg`) có điều kiện — rủi ro làm sai lệch chính hành vi định test.

### 2.3. `st` (system) — đúng như baseline hiện tại

Nhiều task, qua scheduler thật, qua PAL Linux thật. Đây chính là những gì `tsd_runner.c` baseline (v1.2.1) đã làm — không cần thêm gì.

### 2.4. `it` (integration) — vượt khỏi phạm vi codegen

Đúng nghĩa thường gặp trong ngữ cảnh embedded: tích hợp với phần cứng thật hoặc nhiều subsystem thật (không phải PAL giả lập Linux) — ví dụ chạy trên STM32F103 thật, đo tín hiệu GPIO thật, hoặc tích hợp với driver bên thứ ba. `pycdscriptor` hiện chỉ sinh code C và có duy nhất `PLAT=LINUX` làm target giả lập được — không có khả năng tự động hoá build+run trên hardware thật.

Nhận định: `it` theo đúng nghĩa integration-với-phần-cứng không thể là một "chế độ chạy" mà `tsd_runner.c` tự động hỗ trợ được — nó cần một quy trình hoàn toàn khác (flash hardware, harness đo ngoài, CI runner riêng có board thật) nằm ngoài phạm vi pipeline `pycdscriptor`.

## 3. Vấn đề thiết kế cốt lõi: `level` phải nằm ở đâu?

§B của `pltf-impl-post-1.2.1.md` đặt `level` trên `tlc[]` (mục coverage), không phải trên `tsd[]` (mục scenario thực thi). Nếu `level` chỉ dùng để nhóm báo cáo thì đặt ở `tlc` là hợp lý — coverage report là thứ cần nhóm theo mức. Nhưng nếu `level` phải quyết định cách chạy thì nó bắt buộc phải gắn với chính scenario (`tsd[]`), vì runner cần biết trước khi thực thi scenario đó nó sẽ chạy ở chế độ nào — `tlc` chỉ tổng hợp *sau khi* scenario đã chạy xong, không phải nơi điều khiển cách chạy.

Kết luận của mục này: thiết kế "level đổi cách chạy" đòi hỏi thêm field `level` vào chính `C_tsd_obj` (không chỉ `C_tlc_obj`), và cần quy tắc đối chiếu rõ ràng khi một `tlc[].target` liên kết tới nhiều `tsd[]` có `level` khác nhau (ví dụ: coverage report theo `tlc.level` có tính gộp cả kết quả từ `tsd` chạy ở `level` khác không, hay chỉ tính đúng những `tsd` cùng `level`?). Đây là điểm cần quyết định trước khi viết bất kỳ dòng code nào, không phải chi tiết cài đặt có thể để sau.

## 4. Tương tác với runtime coverage (v1.2.3)

Nếu `ut`/`ct` bỏ qua `uedp_tsm_dispatch()`/`uedp_fsm_dispatch()` (gọi thẳng hàm), thì các hook coverage runtime đã cài ở §A của `pltf-impl-post-1.2.1.md` (`uedp_covrt_hit_state`/`hit_trans`/`hit_onrecv`, đặt bên trong 2 hàm dispatch đó) sẽ không được kích hoạt cho các scenario chạy ở mode `ut`/`ct` kiểu gọi thẳng — vì luồng gọi thẳng đi vòng qua dispatch. Điều này làm coverage runtime của `ut`/`ct` bị thiếu số liệu một cách âm thầm nếu không xử lý.

Hai hướng xử lý, cần chọn 1:

- (i) Với scenario `ut`/`ct` gọi thẳng, gọi thủ công `uedp_covrt_hit_state()`/`hit_onrecv()` ngay tại điểm gọi trong runner sinh ra — runner "tự khai báo" nó đã cover gì, không dựa vào core tự đếm.
- (ii) Không tính coverage runtime cho `ut`/`ct` ở chế độ gọi thẳng — coi coverage runtime chỉ có ý nghĩa đầy đủ ở `st` (nơi dispatch thật sự chạy), và ghi rõ giới hạn này trong báo cáo (`covrpt.py`) thay vì báo sai số liệu.

Đề xuất: chọn (ii) cho lần đầu — đơn giản hơn, tránh nguy cơ (i) đếm trùng hoặc đếm sai nếu runner tự khai báo không khớp thực tế dispatch. Ghi rõ trong `tlc_report.md` dòng nào chỉ có coverage tĩnh, dòng nào có coverage runtime thật.

## 5. Rủi ro tổng thể nếu triển khai đầy đủ 4 mức chạy khác nhau

1. Một `testc.txt` phải sinh N kiểu runner khác nhau (gọi thẳng cho `ut`, scheduler+stub cho `ct`, scheduler đầy đủ cho `st`) — độ phức tạp template tăng đáng kể so với baseline hiện tại (1 kiểu runner duy nhất).
2. `ct` cần mock cross-task post — đây là thay đổi hành vi API production có điều kiện biên dịch, rủi ro cao nhất trong toàn bộ đề xuất TSD/TLC tính đến nay (cao hơn cả runtime coverage ở §A, vì runtime coverage chỉ *thêm* quan sát, còn mock ở đây *thay đổi* hành vi gọi).
3. `it` không tự động hoá được trong phạm vi `pycdscriptor` — cần quyết định dứt khoát: hoặc loại `it` khỏi tập giá trị hợp lệ của trường thực thi (chỉ giữ `it` như nhãn báo cáo thuần, không có runner tương ứng), hoặc chấp nhận đây là hạng mục cần một dự án hạ tầng CI/hardware riêng, không thuộc `pycdscriptor`.
4. Semantics coverage runtime khác nhau giữa các mode (mục 4) phải được ghi rõ ràng để không gây hiểu nhầm khi đọc `tlc_report.md`.

## 6. Đề xuất kết luận

1. Không triển khai "level đổi cách chạy" cho cả 4 mức cùng lúc. Nếu quyết định làm, thứ tự rủi ro tăng dần: `st` (đã có, không cần làm gì) → `ut` (gọi thẳng, đơn giản nhất, rủi ro thấp) → `ct` (cần mock cross-task, rủi ro cao) → `it` (đề xuất không tự động hoá trong `pycdscriptor`, tách hẳn ra khỏi phạm vi công cụ này).
2. Nếu triển khai `ut` (mode gọi thẳng), bắt buộc thêm field `level` vào `C_tsd_obj` (không chỉ `C_tlc_obj` như §B đề xuất) — cần quay lại `docs/uels-syntax.md` để cập nhật cú pháp `tsd[]` trước khi impl, và định nghĩa rõ quy tắc đối chiếu `tsd.level` ↔ `tlc.level` khi liên kết coverage (mục 3).
3. Với coverage runtime, chọn phương án (ii) ở mục 4: không tính coverage runtime cho scenario chạy ở mode gọi thẳng, ghi rõ giới hạn trong báo cáo thay vì suy diễn số liệu.
4. `it` nên được xác nhận với người review là out of scope cho `pycdscriptor` — nếu vẫn cần, nên mở một tài liệu review hạ tầng riêng (CI + hardware-in-loop), không phải một mục trong loạt tài liệu TSD/TLC này.
