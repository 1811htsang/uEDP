# PLTF Implementation Plan — Post-1.2.1 Deferred Features (TSD/TLC)

<!-- STATUS - PROPOSAL: kế hoạch triển khai chi tiết, chưa có dòng code nào được viết -->

- Ngày viết: 2026-09-23
- Tách ra từ: [`pltf-impl.md`](./pltf-impl.md) §3.3(b) và §3.4 — hai hạng mục được đánh giá là *không* thuộc scope tối thiểu của v1.2.1 (baseline TSD/TLC tĩnh).
- Mục đích tài liệu: triển khai chi tiết cho (A) TLC coverage runtime/động và (B) chiều `level` (ut/ct/st/it) của TLC, kèm đề xuất nên đưa mỗi hạng mục vào v1.2.2 hay v1.2.3.

Tài liệu `pltf-impl.md` giả định TLC chỉ cần coverage tĩnh (đối chiếu tập target khai báo trong TSD scenario, không cần chạy binary) và không có chiều `level`. Hai phần dưới đây mở rộng đầy đủ nếu quyết định làm thêm, tách riêng vì cả hai đều không bắt buộc để TSD/TLC baseline chạy được.

## A. TLC Coverage Runtime (động)

### A.1. Mục tiêu

Thay vì chỉ kiểm tra "target có được nhắc tới trong ít nhất 1 TSD scenario hay không" (static, §3.3(a) của `pltf-impl.md`), đo coverage thật bằng cách đếm số lần mỗi state/transition/on_recv handler thực sự được thực thi khi binary test chạy.

### A.2. Điểm hook: core dispatch, không phải per-generated-function

`uedp_tsm_dispatch()` (`sources/core/src/uedp_tsm.c`) và `uedp_fsm_dispatch()` (`sources/core/src/uedp_fsm.c`) là 2 hàm core, viết tay, không sinh code, mà MỌI transition/on_recv của MỌI task đều đi qua. Đây là điểm hook đúng — instrument một lần ở core thay vì phải sinh thêm code đếm vào từng hàm `{state}_onst`/`{state}_trans` do `modalcvert.py` tạo ra (sẽ phải sửa `appc.txt` ở nhiều chỗ, rủi ro cao hơn nhiều).

Nguyên tắc bắt buộc: toàn bộ instrumentation phải nằm sau `#ifdef UEDP_COVERAGE_ENABLED`, biên dịch ra rỗng hoàn toàn khi macro không được định nghĩa — để không ảnh hưởng build production (`PLAT=STM32F103/STM32H723/ESP32S3` hay `PLAT=LINUX` thường) dù chỉ 1 byte.

### A.3. Module coverage registry mới (core, optional)

File mới:

- `sources/core/inc/uedp_covrt.h`
- `sources/core/src/uedp_covrt.c`

API đề xuất:

```c
void uedp_covrt_init(void);
void uedp_covrt_hit_state(task_id_t tid, tsm_state_id_t state_id);
void uedp_covrt_hit_trans(task_id_t tid, tsm_state_id_t from_state, ui8 sig);
void uedp_covrt_hit_onrecv(task_id_t tid, ui8 sig);
void uedp_covrt_dump(const char* path);
```

Lưu trữ: mảng tĩnh cố định kích thước (ví dụ 128 entry) kiểu `{ task_id_t tid; ui16 key; ui32 count; }` với linear probing theo `(tid, key)` — tránh phải sinh thêm 1 pregen step khai báo số lượng state/task (giữ đơn giản, không phụ thuộc codegen mới). Nếu bảng đầy, tăng một counter tràn riêng (`overflow_count`) và log cảnh báo qua `uedp_itnlog` thay vì crash.

### A.4. Sửa 2 hàm dispatch core (có điều kiện)

- `uedp_tsm_dispatch()`: sau khi xác định `cur_state` xử lý tín hiệu và (nếu có) chuyển trạng thái, gọi:
  - `uedp_covrt_hit_state(tid, cur_state)` — mỗi lần state được active.
  - `uedp_covrt_hit_trans(tid, prev_state, sig)` — CHỈ khi `cur_state` thực sự đổi giá trị sau dispatch (đếm transition = state change thật, không phải mọi tín hiệu được nhận trong state đó — tránh coverage ảo).
- `uedp_fsm_dispatch()`: gọi `uedp_covrt_hit_onrecv(tid, msg->sig)` ngay trước khi gọi `fsm->state(msg)`.

Cả hai đặt trong `#ifdef UEDP_COVERAGE_ENABLED ... #endif`, không đổi signature hàm, không đổi behavior khi tắt macro.

### A.5. Xuất dữ liệu sau khi chạy test

`tsd_runner.c` (sinh từ `testc.txt`, xem `pltf-impl.md` Bước 7) nhận thêm 1 flag CLI đơn giản (tự parse `argv`, không cần thư viện ngoài): `--covrt-dump=<path>`. Cuối `main()`, sau khi chạy hết các `run_scenario_*`, nếu flag được truyền thì gọi `uedp_covrt_dump(path)` — ghi ra file text đơn giản dạng `tid,key,type,count` mỗi dòng.

Giới hạn cần ghi rõ: dump chỉ chạy nếu `main()` thoát bình thường (không qua `pal_sys_fatal()`/`abort()`). Đây là giới hạn chấp nhận được cho v1 — không cố exception-safe dump qua signal handler.

### A.6. Pipeline: cần thêm bước build+run, không chỉ codegen

Coverage runtime không thể tính xong trong `jainerator.sh` (thuần codegen) — cần binary đã build và đã chạy xong. Đề xuất script mới, tách khỏi `jainerator.sh`:

```bash
# runtest.sh (mới, chạy SAU jainerator.sh, không phải một phần của nó)
cmake --build build --target uedp_tsd_test
./build/uedp_tsd_test --covrt-dump=sources/test/generated/tlc_runtime_counts.txt
python -m pltf.pycdscriptor.jnerator.postgen.covrpt \
  --yaml sources/app/lstaxizer.yaml \
  --runtime-counts sources/test/generated/tlc_runtime_counts.txt \
  --output sources/test/generated/tlc_report.md
```

`covrpt.py` (đã có từ `pltf-impl.md` Bước 9) cần thêm arg `--runtime-counts` (optional): nếu có, tính % coverage thật từ số đếm; nếu không, fallback về static mode (§3.3(a)) như hiện tại — không phá tính năng đã có.

### A.7. CMake

Thêm option `UEDP_COVERAGE_ENABLED` (default OFF), chỉ có tác dụng khi `UEDP_BUILD_TSD_TEST=ON`:

```cmake
option(UEDP_COVERAGE_ENABLED "Instrument TSM/FSM dispatch for runtime TLC coverage" OFF)
if (UEDP_COVERAGE_ENABLED)
  if (NOT UEDP_BUILD_TSD_TEST)
    message(FATAL_ERROR "UEDP_COVERAGE_ENABLED requires UEDP_BUILD_TSD_TEST=ON")
  endif()
  target_sources(uedp_tsd_test PRIVATE sources/core/src/uedp_covrt.c)
  target_compile_definitions(uedp_tsd_test PRIVATE UEDP_COVERAGE_ENABLED)
endif()
```

`uedp_covrt.c` không được thêm vào `SOURCES`/target `uedp` chính — chỉ vào `uedp_tsd_test`.

### A.8. Rủi ro cần review kỹ trước khi code

1. Đây là hạng mục duy nhất trong toàn bộ TSD/TLC đụng vào core dispatch runtime đã ổn định (`uedp_tsm.c`/`uedp_fsm.c`) — dù có `#ifdef`, vẫn cần review diff cẩn thận để đảm bảo build production tuyệt đối không đổi (nên diff `.o`/objdump giữa trước/sau khi macro tắt để xác nhận zero-cost).
2. Định nghĩa "transition được cover" (state thực sự đổi) cần thống nhất với người viết TSD trước, vì khác với cách hiểu "signal được gửi tới đúng state" — 2 định nghĩa cho ra % khác nhau.
3. Cần script `runtest.sh` mới — một quy trình 2 pha (build+run rồi mới report) khác hẳn các bước còn lại vốn chỉ là codegen thuần, nên phải viết doc vận hành riêng (không chỉ thêm 2 dòng vào `jainerator.sh` như suy nghĩ ban đầu).

## B. TLC `level` (ut/ct/st/it)

### B.1. Mục tiêu

Cho phép mỗi mục `tlc[]` khai báo nó thuộc mức kiểm tra nào — theo đúng mục đã ghi trong `docs/to-do.md` (v1.2.1): "cho phép chỉ định mức kiểm tra từ ut (unit), ct (component), st (system) và it (integration)".

### B.2. Đề xuất cú pháp (cần quay lại review ở `uels-syntax.md` trước khi impl thật)

```yaml
tlc:
  - id: tlc-001
    target: TASK_X
    level: ut          # ut | ct | st | it
    cover: [states, trans]
    threshold: 80
    report: summary
```

### B.3. Các điểm chạm trong pipeline (đều là mở rộng, không phá cấu trúc hiện có)

1. Pydantic model (`pydantic_model/tsd.py`, xem `pltf-impl.md` Bước 1): thêm field `level: Literal["ut", "ct", "st", "it"]` vào `C_tlc_obj` — bắt buộc (không default), buộc người viết YAML khai báo rõ.
2. Structural validation (`strucjec_target_tlc`, Bước 2): kiểm `level` nằm trong 4 giá trị hợp lệ, lỗi thì `exit(1)` theo đúng convention `strucjec.py` hiện có.
3. lukupmodel (`lukupmodel_tlc_logic`, Bước 4): truyền thẳng `level` vào model đã parse — không cần resolve symbol vì đây là literal, không phải reference.
4. Context-Type Match (`vlid.py`): không cần sửa — `level` không phải symbol trỏ tới section khác, nên không nằm trong phạm vi `symresolv_load()`.
5. IR builder (`tsdcvert.py::build_tlc_context`, Bước 6): group kết quả theo `level` ngoài group theo `id`, ví dụ trả về `{"ut": [...], "ct": [...], "st": [...], "it": [...]}`.
6. Report generator (`covrpt.py`, Bước 9): xuất báo cáo có heading riêng theo từng level — cho phép ví dụ CI chỉ gate theo threshold của level `it` (integration) trong khi để `ut` mang tính thông tin.

### B.4. Câu hỏi kiến trúc lớn hơn — cố tình KHÔNG giải quyết trong bản đầu

`level` có nên quyết định cách chạy TSD scenario không (ví dụ: `level: ut` thì gọi thẳng hàm state/action mà không qua `uedp_task_scheduler()`/PAL, còn `level: it` mới chạy full scheduler như hiện tại)? Đây là một thiết kế lớn hơn nhiều (cần 1 chế độ "gọi trực tiếp" tách biệt khỏi runner hiện tại).

Đề xuất bản đầu: `level` chỉ là metadata để phân loại báo cáo, mọi TSD scenario vẫn chạy qua cùng một `tsd_runner.c`/TSM+FSM dispatch machinery như nhau bất kể `level` khai báo là gì. Việc tách chế độ chạy theo level (unit test "trần", không qua scheduler) nên là một đề xuất riêng, review syntax + impl riêng, không gộp vào đây — xem review chi tiết ở [`tlc-post-1.2.3.md`](./tlc-post-1.2.3.md) (thực hiện sau khi v1.2.3 hoàn tất).

## C. Đề xuất phân bổ version

| Hạng mục | Đề xuất version | Lý do |
| --- | --- | --- |
| B. TLC `level` | v1.2.2 | Thuần additive, không đụng core runtime, rủi ro thấp — hợp lý làm ngay sau khi baseline TSD/TLC tĩnh (v1.2.1) ổn định. |
| A. Runtime coverage | v1.2.3 | Duy nhất đụng vào core dispatch (`uedp_tsm.c`/`uedp_fsm.c`), cần thêm quy trình build+run 2 pha ngoài codegen thuần, cần review riêng về zero-overhead khi tắt macro — nên tách hẳn 1 version, không gộp chung với B. |

Nếu muốn gộp cả hai vào cùng 1 version, khuyến nghị vẫn làm B trước A trong cùng version đó (thứ tự phụ thuộc: `covrpt.py` đã có group-by-level từ B thì report runtime coverage ở A tận dụng luôn cấu trúc đó, đỡ phải sửa lại `covrpt.py` hai lần).

## D. Kết luận

Cả hai hạng mục đều không chặn việc ship baseline TSD/TLC tĩnh theo `pltf-impl.md`. Đề xuất chốt: B → v1.2.2, A → v1.2.3, theo bảng ở mục C. Nếu người review quyết định khác (ví dụ gộp cả 2 vào v1.2.2, hoặc đẩy cả 2 xa hơn), cấu trúc từng bước ở mục A/B vẫn giữ nguyên — chỉ đổi nhãn version.
