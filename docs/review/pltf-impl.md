# PLTF Implementation Plan — TSD & TLC

<!-- STATUS - PROPOSAL: kế hoạch triển khai chi tiết, chưa có dòng code nào được viết -->

- Ngày viết: 2026-09-23
- Phạm vi: `pltf/pycdscriptor` (lớp `lstaxer` + `jnerator`), `pltf/templates`, `CMakeLists.txt`, pipeline `entrypoint.sh`/`jainerator.sh`
- Liên quan: [`docs/uels-syntax.md`](../uels-syntax.md) (cú pháp đề xuất TSD/TLC), [`docs/to-do.md`](../to-do.md) (mục "Bổ sung tài liệu review thiết kế TSD/TLC" và "Hoàn thiện thiết kế TLC" trong v1.2.1 — Safe Input Filter)
- Mục đích tài liệu: đề xuất các bước triển khai chi tiết (implementation roadmap) cho TSD (Test Scenario Descriptor) và TLC (Test Level Coverager), dựa trên kiến trúc codegen thực tế hiện có — không phải bản ghi tranh luận nhiều vòng như các review khác trong thư mục này.

## 1. Mục tiêu

TSD/TLC hiện chỉ tồn tại dưới dạng đề xuất cú pháp YAML trong `docs/uels-syntax.md` — chưa có parser, validator hay codegen nào trong `pycdscriptor`. Tài liệu này trả lời câu hỏi: để một khối `tsd:`/`tlc:` trong `lstaxizer.yaml` thực sự sinh ra được một test runner chạy được trên `PLAT=LINUX`, cần thêm/sửa những gì, theo thứ tự nào, và những điểm thiết kế nào phải chốt trước khi viết code.

## 2. Kiến trúc pipeline hiện tại (tóm tắt để tham chiếu)

```graph
entrypoint.sh:
  1. python -m pltf.pycdscriptor.jnerator.pregen.fpregen   # app_cfg.h, app_decl.h, core/pal/arch config+decl
  2. python -m pltf.pycdscriptor.ustab.custab               # .config -> ustab
  3. python -m pltf.pycdscriptor.ustab.ankorpin              # ghi anchor tnorm{N}-ank/tpoll{N}-ank/sig{N}-ank/gda{N}-ank vào lstaxizer.yaml

jainerator.sh:
  4. python -m pltf.pycdscriptor.lstaxer.vlid                                    # structure + context-type-match validation
  5. python -m pltf.pycdscriptor.jnerator.postgen.cgen --yaml ... --output app.c # codegen cuối: build IR -> Jinja2 render appc.txt
```

Các lớp liên quan trong `lstaxer`:

- `pydantic_model/{resrc,logic,misc}.py` — model Pydantic cho từng loại object trong YAML, export qua `pydantic_model/__init__.py`.
- `strucjec.py` — duyệt YAML event-stream để validate cấu trúc (không dùng Pydantic ở bước này), orchestrator là `strucjec_calib()`.
- `lukupmodel.py` — parse YAML đã qua anchor thành các Pydantic model cụ thể (`lukupmodel_tnorm_logic`, `lukupmodel_glbda_logic`, …).
- `vlid.py` — hardcode đọc `sources/app/lstaxizer.yaml`, gọi `strucjec_calib()` rồi kiểm Context-Type Match qua `symresolv_load()`.

Codegen cuối (`jnerator/postgen`):

- `modalcvert.py::build_appc_context()` — build dict context cho Jinja2 từ YAML text.
- `pltf/templates/appc.txt` — template Jinja2 duy nhất sinh toàn bộ `sources/app/app.c` (task table, TSM/FSM state functions, `logic_init()`, `app_main()` với `while(1) uedp_task_scheduler();`).

Build: `CMakeLists.txt` có `PLAT` = `TEST|LINUX|STM32F103|STM32H723|ESP32S3`. `PLAT=LINUX` link `pthread`/`m`, thêm `sources/pal/arch/linux/linux.c`, define `LINUX_PLATFORM`. `APP_SRCS` chỉ gồm `sources/app/app.c`.

Tiền lệ gần nhất cho một test runner Linux: `sources/test/deprecated/test04/test.c` (deprecated nhưng đúng cấu trúc: `TEST_CHECK` macro, `uedp_itnlog_set_output()` để capture log, pthread tick thread 1ms gọi `uedp_timer_tick()`, vòng lặp scheduler giới hạn 100000 lần + `usleep(100)`, thoát bằng cờ `g_test_done`).

## 3. Các quyết định thiết kế cần chốt trước khi viết code

### 3.1. FSM state là function pointer, không phải id — vấn đề lớn nhất

`uedp_fsm_t.state` (`sources/core/inc/uedp_fsm.h`) có kiểu `state_handler` (`void (*)(uedp_msg_t*)`), không phải một enum/id như `uedp_tsm_t.cur_state` (`tsm_state_id_t`, so sánh được trực tiếp với hằng số `{STATE}_ID`).

Hệ quả: `expect.fsm_state: SOME_STATE_ID` trong TSD (xem `docs/uels-syntax.md`) không thể so sánh bằng `==` với một hằng số int như `expect.state` làm được. Codegen phải sinh thêm một bảng tra cứu id → function pointer cùng lúc với các hàm `{state}_onst` (đã được `modalcvert.py::build_appc_context()` đặt tên qua `tnorm_codegen[...].active_name`).

Đề xuất: trong template test runner mới (không đụng vào `appc.txt`/`app.c` production), sinh thêm một mảng tĩnh per-task, ví dụ:

```c
static const struct { const char* id; state_handler fn; } fcm_fsm_lookup_TASK_X[] = {
  { "STATE_A", STATE_A_onst },
  { "STATE_B", STATE_B_onst },
};
```

rồi hàm helper `fsm_state_matches(task_id, "STATE_A")` duyệt mảng và so `fsm->state == fn`. Bảng này lấy dữ liệu trực tiếp từ `tnorm_codegen` (đã có `active_name` cho từng state) trong `modalcvert.py`, nên không cần thêm cấu trúc dữ liệu mới ở tầng IR — chỉ cần thêm một hàm build context mới tái sử dụng `tnorm_codegen`.

### 3.2. Test runner là artifact riêng biệt, không chèn vào `app.c`

`app.c` là code sản xuất (`app_main()` chạy `while(1)`). TSD cần một `main()` khác: khởi tạo, bơm scenario, chờ, assert, thoát bằng exit code — không thể sống chung vòng lặp vô hạn của `app_main()`.

Đề xuất: sinh một file C hoàn toàn mới (ví dụ `sources/test/generated/tsd_runner.c`) từ một template Jinja2 mới (`pltf/templates/testc.txt`), dùng lại các hàm/table đã có sẵn trong `app.c` (task table, TSM/FSM state functions, `logic_init()`) bằng cách `#include "app.c"` có điều kiện, hoặc — sạch hơn — thêm guard `#ifndef UEDP_TSD_RUNNER_BUILD` quanh `app_main()` trong `appc.txt` để `tsd_runner.c` có thể `#include "../../app/app.c"` và tự viết `main()` của riêng nó mà không đụng độ symbol `main`. Cách include-file thay vì link riêng object tránh phải sinh lại toàn bộ task/TSM/FSM tables lần hai.

### 3.3. TLC coverage: tính tĩnh (static, tại thời điểm codegen) chứ chưa làm runtime instrumentation

`tlc.cover: [states, trans, on_recv]` có thể hiểu theo 2 cách:

- (a) coverage tĩnh: đối chiếu tập `target` các state/transition/on_recv có xuất hiện trong ít nhất một TSD scenario nào không, tính % và so với `threshold` — làm được hoàn toàn ở Python, ngay trong pipeline generate, không cần chạy binary.
- (b) coverage runtime động: instrument code sinh ra để đếm mỗi state/transition thực sự được thực thi bao nhiêu lần khi chạy test — cần thêm bộ đếm global + báo cáo sau khi chạy xong.

Đề xuất cho v1.2.1: triển khai (a) trước (static, đơn giản, không đụng runtime core), vì nó đã đủ để trả lời câu hỏi "TSD có che phủ đủ state/transition quan trọng theo yêu cầu review hay chưa" mà roadmap `to-do.md` đặt ra. (b) để ngỏ cho một vòng review sau nếu cần coverage khi-chạy-thật — chi tiết triển khai (b) đã tách sang [`pltf-impl-post-1.2.1.md`](./pltf-impl-post-1.2.1.md) mục A, đề xuất v1.2.3.

### 3.4. Chiều `level` (ut/ct/st/it) cho TLC — đề xuất hoãn, không đưa vào lần này

`docs/to-do.md` (v1.2.1) có mục riêng: "Hoàn thiện thiết kế TLC để cho phép chỉ định mức kiểm tra từ ut (unit), ct (component), st (system) và it (integration)". Cú pháp hiện tại trong `docs/uels-syntax.md` (`target`/`cover`/`threshold`/`report`) chưa có field này.

Đề xuất: không mở rộng cú pháp cho `level` trong lần triển khai này — giữ scope là TSD/TLC như đã viết trong `uels-syntax.md`. Việc thêm `level:` nên là một review syntax riêng (quay lại `uels-syntax.md` trước, rồi mới impl), vì nó ảnh hưởng đến cách phân nhóm report chứ không phải cơ chế thực thi cốt lõi. Chi tiết triển khai đã tách sang [`pltf-impl-post-1.2.1.md`](./pltf-impl-post-1.2.1.md) mục B, đề xuất v1.2.2. Ghi rõ đây là câu hỏi mở ở mục 6.

## 4. Kế hoạch triển khai từng bước

### Bước 1 — Pydantic model cho TSD/TLC

File mới: `pltf/pycdscriptor/lstaxer/pydantic_model/tsd.py`, theo đúng khuôn mẫu kích cỡ/docstring `# STUB` như `misc.py`/`resrc.py`:

- `C_tsd_step_obj` (`type: post_msg|post_isr|post_urgent|wait`, các field tuỳ loại: `dest`/`sig`/`msg`/`ms`)
- `C_tsd_expect_obj` (`type: state|signal_sent|fsm_state|gda`, `target`, `value`)
- `C_tsd_obj` (`id`, `target`, `steps: list[C_tsd_step_obj]`, `expect: list[C_tsd_expect_obj]`)
- `C_tsd_list_obj` (root list)
- `C_tlc_obj` (`id`, `target`, `cover: list[str]`, `threshold`, `report`)
- `C_tlc_list_obj`

Wire vào `pydantic_model/__init__.py` theo đúng pattern `from . import logic, misc, resrc` hiện có (thêm `tsd` vào import và re-export danh sách tên).

### Bước 2 — Structural validation (`strucjec.py`)

Thêm `strucjec_target_tsd(...)` và `strucjec_target_tlc(...)`, mô phỏng `strucjec_target_glbda`/`strucjec_target_glbda_item` (duyệt event-stream, kiểm field bắt buộc theo từng `type` của step/expect). Gọi cả hai từ `strucjec_calib()` — giữ nguyên nguyên tắc `exit(1)` khi lỗi cấu trúc.

### Bước 3 — Context-Type Match (`vlid.py`)

`vlid.py` hiện có 3 trường hợp exception hardcode (`data`↔`glbda`, `<<`↔`tnorms`/`tpolls`, `on_sig`↔`sigs`). Thêm các cặp mới:

- `tsd[].target` ↔ `tnorms`/`tpolls` (id task được test)
- `tsd[].steps[].dest` ↔ `tnorms`/`tpolls`
- `tsd[].expect[].target` ↔ tuỳ `type` (state→tnorm state id, fsm_state→tnorm fsm state id, gda→`glbda`, signal_sent→`sigs`)
- `tlc[].target` ↔ `tnorms`/`tpolls`

Đây là nơi dễ sai nhất vì `expect.target` đổi ý nghĩa theo `expect.type` — nên viết một hàm resolve riêng thay vì nhét thêm if/else vào `symresolv_load()`.

### Bước 4 — Parse YAML → model (`lukupmodel.py`)

Thêm `lukupmodel_tsd_logic(...)` và `lukupmodel_tlc_logic(...)`, theo pattern `lukupmodel_glbda_logic`/`lukupmodel_outexec_logic` (đơn giản hơn `lukupmodel_tnorm_logic` vì TSD/TLC không có action tree lồng nhau kiểu `_parse_actv_obj`).

### Bước 5 — Anchors (`ankorpin.py`)

`ankorpin.py` hiện chỉ chèn anchor cho `tnorms`/`tpolls`/`sigs`/`glbda` — các section có id được tham chiếu chéo dạng `<<`. `tsd`/`tlc` không cần anchor riêng vì chúng chỉ *tham chiếu tới* các id đã có anchor (task/state/signal/gda), không phải nguồn bị tham chiếu. → Không cần sửa `ankorpin.py`.

### Bước 6 — IR context builder cho TSD/TLC

File mới: `pltf/pycdscriptor/jnerator/postgen/tsdcvert.py` (đặt tên song song với `modalcvert.py`, không sửa trực tiếp file đó để tránh phình logic app.c).

- `build_tsd_context(yaml_text)`: parse ra danh sách scenario, với mỗi step/expect resolve symbol thật (task id C macro, signal C macro) — tái sử dụng các helper `_c_symbol()`/`_resolve_task_symbol()` đã có trong `modalcvert.py` (import chéo, không copy-paste).
- Với `expect.fsm_state`: build thêm bảng lookup theo mục 3.1, lấy dữ liệu từ `build_appc_context()` đã chạy trước đó (gọi `modalcvert.build_appc_context()` rồi trích `tnorm_codegen[...]`).
- `build_tlc_context(yaml_text, tsd_context)`: đối chiếu tập target được TSD cover với tập target khai báo trong `tlc[].cover`, tính % theo mục 3.3(a).

### Bước 7 — Template test runner mới

File mới: `pltf/templates/testc.txt`. Cấu trúc dựa theo tiền lệ `test04/test.c`:

- `#include` app logic (theo cách include ở mục 3.2) + `test.h`-tương-đương tự sinh nếu cần khai báo thêm.
- `TEST_CHECK(desc, cond)` macro (copy nguyên mẫu từ `test04/test.c`).
- Capture log qua `uedp_itnlog_set_output()`/`uedp_itnlog_set_filter()` — dùng cho `expect.signal_sent` nếu cần xác nhận qua log thay vì state trực tiếp.
- `{% for scenario in tsd_scenarios %}` sinh 1 hàm `run_scenario_{{ scenario.id }}(void)` thực hiện tuần tự các `steps` (map `post_msg`→`uedp_task_norm_post_msg`, `post_isr`→`pal_linux_simulate_interrupt` [chỉ hợp lệ khi build LINUX — cần warn/skip khi target khác LINUX], `post_urgent`→`uedp_task_norm_post_urgent`, `wait`→`usleep`), rồi các `TEST_CHECK` cho từng `expect`.
- pthread tick thread 1ms gọi `uedp_timer_tick()` (giống `test04`).
- `main()`: `uedp_core_init()` → `uedp_msg_pool_init()` → `uedp_timer_init()` → `uedp_itnlog_init()` → `uedp_task_norm_create(app_task_table)` → áp dụng workaround `cur_pri` đã ghi nhận trong `test04/test.c` (gán `cur_pri = base_pri` thủ công cho từng task, vì `uedp_task_norm_create()` chưa tự làm việc này — đây là gap có sẵn ở core, không phải của TSD, nhưng runner sinh ra phải biết để không bị "task không bao giờ chạy") → tạo tick thread → gọi từng `run_scenario_*` → tổng kết PASS/FAIL → trả exit code khác 0 nếu có FAIL.

### Bước 8 — Postgen CLI module mới

File mới: `pltf/pycdscriptor/jnerator/postgen/testgen.py`, mirror `cgen.py` (26 dòng): args `--yaml` (default `sources/app/lstaxizer.yaml`) và `--output` (ví dụ mặc định `sources/test/generated/tsd_runner.c`), gọi `tsdcvert.generate_testc()` (render `testc.txt`). Nếu YAML không có key `tsd`, script thoát sớm với thông báo (không lỗi) để không phá pipeline cho các project chưa dùng TSD.

### Bước 9 — Coverage report generator

File mới: `pltf/pycdscriptor/jnerator/postgen/covrpt.py`. Input: `build_tlc_context()` từ bước 6. Output: file JSON/Markdown (ví dụ `sources/test/generated/tlc_report.md`) liệt kê từng `tlc[].id`, % coverage tính được, so với `threshold`, PASS/FAIL. Đây là bước Python thuần, không sinh C — chạy ngay trong lúc generate, không cần chạy binary test.

### Bước 10 — Build system wiring (`CMakeLists.txt`)

Thêm option mới, ví dụ `UEDP_BUILD_TSD_TEST` (mặc định OFF), độc lập với `PLAT`:

```cmake
option(UEDP_BUILD_TSD_TEST "Build generated TSD test runner (requires PLAT=LINUX)" OFF)
if (UEDP_BUILD_TSD_TEST)
  if (NOT PLAT STREQUAL "LINUX")
    message(FATAL_ERROR "UEDP_BUILD_TSD_TEST requires PLAT=LINUX")
  endif()
  add_executable(uedp_tsd_test sources/test/generated/tsd_runner.c ${PAL_PLATFORM_SRCS} ${CORE_SRCS} ${COMMON_XPRINTF_SRCS} ${PAL_SERVICE_SRCS})
  target_compile_definitions(uedp_tsd_test PRIVATE UEDP_TSD_RUNNER_BUILD)
  target_link_libraries(uedp_tsd_test pthread m)
  target_include_directories(uedp_tsd_test PRIVATE ...) # giống target chính
endif()
```

Lưu ý: `tsd_runner.c` không nằm trong `APP_SRCS` của target `uedp` chính — nó là một executable riêng, tránh việc build production `uedp` bị kéo theo code test.

### Bước 11 — Pipeline wiring (`jainerator.sh`)

Thêm 2 dòng sau `cgen`, có điều kiện (kiểm tra key `tsd`/`tlc` tồn tại trong YAML trước khi chạy, hoặc để chính `testgen.py`/`covrpt.py` tự no-op khi thiếu key — cách sau ưu tiên hơn vì giữ script bash đơn giản):

```bash
python -m pltf.pycdscriptor.jnerator.postgen.testgen --yaml sources/app/lstaxizer.yaml --output sources/test/generated/tsd_runner.c
python -m pltf.pycdscriptor.jnerator.postgen.covrpt   --yaml sources/app/lstaxizer.yaml --output sources/test/generated/tlc_report.md
```

### Bước 12 — Thứ tự rollout đề xuất (để giảm rủi ro)

1. Bước 1–4 (parse + validate only, chưa sinh code) — có thể review độc lập, rủi ro thấp.
2. Bước 6–8 với scope tối thiểu: chỉ hỗ trợ `post_msg`/`wait` + `expect.state` (không đụng FSM/GDA) — chứng minh pipeline end-to-end chạy được trên 1 ví dụ thật.
3. Mở rộng `post_isr`/`post_urgent`/`expect.signal_sent`/`expect.gda`.
4. `expect.fsm_state` (phần khó nhất, theo mục 3.1) làm riêng, có thể lùi lại một PR sau.
5. TLC (bước 9) làm cuối cùng vì phụ thuộc dữ liệu từ TSD context đã ổn định.

## 5. Câu hỏi mở / rủi ro cần chốt với người review

1. `expect.fsm_state`: đồng ý cách tiếp cận bảng lookup id→function pointer sinh riêng cho test runner (mục 3.1), hay muốn thêm hẳn một `state_id_t` enum song song vào core FSM (đổi cấu trúc `uedp_fsm_t`, ảnh hưởng rộng hơn nhiều)?
2. `post_isr` trên non-Linux target: `pal_linux_simulate_interrupt()` chỉ tồn tại trong `linux.c`. Nếu `tsd.target` trỏ tới task build cho STM32/ESP32, bước này cần báo lỗi rõ ràng ở generate-time (validate) thay vì lỗi compile khó hiểu — nên thêm rule ở Bước 2/3.
3. TLC `level` (ut/ct/st/it): xác nhận hoãn sang review syntax riêng (mục 3.4) thay vì làm luôn trong đợt này.
4. Coverage runtime (mục 3.3(b)): có cần ngay trong v1.2.1, hay static coverage là đủ cho mốc này?
5. Vị trí output file sinh ra: đề xuất `sources/test/generated/` (thư mục mới, tách khỏi `sources/test/deprecated/`) — cần xác nhận quy ước đặt tên/thư mục này có khớp với hướng tổ chức project không.

## 6. Đề xuất kết luận

Triển khai theo đúng 12 bước ở mục 4, đi theo lộ trình rollout ở mục 12 để có thể review từng phần nhỏ thay vì một PR khổng lồ. Trước khi bắt đầu Bước 1, cần chốt trả lời cho 5 câu hỏi mở ở mục 5 — đặc biệt câu 1 (FSM state) vì nó quyết định hình dạng của cả template `testc.txt` và context builder `tsdcvert.py`.
