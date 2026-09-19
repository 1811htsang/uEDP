# Tài liệu thiết kế: Di chuyển tính năng từ [KwDI] sang [PLTF]

## 1. Bối cảnh

`docs/to-do.md`, mục Phiên bản 1.2.0: The Infrastructure Preparation for μE-OS, ghi:

- `[ ] Bổ sung tài liệu thiết kế PLTF (Portable Local Test Framework) nhằm cung cấp khả năng kiểm thử tự động đa quy mô.`
- `[ ] Thực hiện triển khai thiết kế PLTF.`

Trong khi đó, `arch-design.md` mục [KwDI] Kconfig with Docker Integration (phiên bản 1.1.2) mới chỉ mô tả việc tích hợp Kconfig với Docker để cấu hình lõi μEDP qua giao diện dòng lệnh, chưa đề cập tới việc sinh code tự động có kiểm thử đi kèm. Tài liệu này lấp khoảng trống đó: mô tả kiến trúc gốc của KwDI, các giới hạn của nó, và thiết kế PLTF đã thay thế/mở rộng KwDI trong code hiện tại.

## 2. Kiến trúc thiết kế gốc từ KwDI

### 2.1 Thành phần

<!-- DEPRECATED - Old CRITICAL
Xem lại commit số c3f4c9866ec1ea548f2e108f059328fe0dd68183 để revert lại các thay đổi liên quan đến kiến trúc thiết gốc của KwDI

Lưu ý này áp dụng đối với cả bản VN và EN.
-->

KwDI gồm 3 phần chính, tất cả nằm gọn trong root repo và `sources/common/`:

- `Kconfig` (root) + `sources/app/kconfig/{core,pal,decl}.kconfig`: định nghĩa cây cấu hình.
- `sources/common/kconfiglib/`: thư viện `kconfiglib` + `menuconfig` (bên thứ 3) để đọc cây Kconfig và hiển thị giao diện `menuconfig` tương tác trên terminal.
- `sources/common/pyspec/`: các hàm sinh `decl.kconfig` (task norm, task poll, signal, hardware API) dựa theo số lượng người dùng nhập vào (`usrinp_pspec.py`, `tsknrmdcl.py`, `tskpoldcl.py`, `sigdcl.py`, `hwapidcl.py`).
- `uedp.py` (root): script duy nhất điều phối toàn bộ luồng — vừa thu thập input, vừa gọi `menuconfig`, vừa tự sinh code (`corecfg_gen`, `palcfg_gen`, `app_cfg_gen`, `app_decl_gen`, `pal_arch_gen`) bằng cách chèn trực tiếp chuỗi `#define` vào giữa 2 marker (`// KCONFIG_CORECFG_START` / `// KCONFIG_CORECFG_END`) trong các file header có sẵn ở `sources/app/config/`.
- `Dockerfile` (bản gốc): image `python:3.13-slim`, chỉ cài `kconfiglib`, `CMD ["python", "uedp.py", "menuconfig"]`.

### 2.2 Luồng hoạt động gốc

1. Người dùng chạy `docker build` rồi `docker run` (không có `docker-compose.yaml`, không có `entrypoint.sh`).
2. Container khởi động, chạy thẳng `python uedp.py menuconfig`.
3. `uedp.py` hỏi input (số task, số signal, có dùng FSM/TSM không...) → ghi vào `sources/app/kconfig/decl.kconfig`.
4. `kconfiglib.Kconfig("Kconfig")` load toàn bộ cây, mở `menuconfig` để người dùng chỉnh giá trị → ghi ra `.config`.
5. Ngay trong `main()` của `uedp.py`, gọi tuần tự `corecfg_gen()`, `palcfg_gen()`, `app_cfg_gen()`, `app_decl_gen()`, `pal_arch_gen()` — mỗi hàm tự duyệt `kconf.unique_defined_syms`, tự format chuỗi `#define ...`, rồi patch trực tiếp vào file `.h` đã tồn tại sẵn thông qua cặp marker.

### 2.3 Giới hạn của KwDI (lý do cần PLTF)

- Không tách giai đoạn: thu thập input, cấu hình tương tác (menuconfig), và sinh code nằm chung trong một hàm `main()` của `uedp.py`. Muốn sinh lại code từ một `.config` có sẵn (ví dụ trong CI) vẫn phải chạy lại toàn bộ `menuconfig` tương tác.
- Sinh code kiểu "vá chuỗi" (marker-based patch): `corecfg_gen`/`palcfg_gen` yêu cầu file `.h` đích phải đã tồn tại sẵn với đúng cặp marker mới patch được — không tạo file mới từ đầu được, dễ vỡ nếu ai đó lỡ xoá marker.
- `sources/common/testspec/` (tiền thân dùng Jinja2, sau này đổi tên thành `pycdscriptor`) đã tồn tại nhưng chỉ là bản nháp chưa nối vào luồng thật: `appcfgpgen.py` gốc chỉ `print(output)` ra màn hình với `current_date` gán cứng `'16 May 2025'`, không đọc `.config` thật, không ghi file.
- Docker image tối giản, chỉ có `kconfiglib`: không có `gcc/cmake/gdb`, không có ESP-IDF, không thể build hay chạy test ngay trong container — người dùng vẫn phải thoát container để build bằng tay.
- Không có `entrypoint.sh`/`docker-compose.yaml`: container chạy `CMD` trực tiếp bằng root, không xử lý UID/GID → file được tạo ra (do mount volume) thuộc quyền sở hữu `root` trên máy host, gây phiền khi chỉnh sửa lại ở ngoài container.
- Không phân tách workspace: không có khái niệm thư mục riêng cho "mã nguồn lõi" và "không gian làm việc kiểm thử" — mọi thứ trộn chung trong repo.

## 3. Kiến trúc thiết kế của PLTF

Nguyên tắc cốt lõi của PLTF là tách rõ 2 giai đoạn vốn bị gộp chung trong KwDI:

- Giai đoạn 1 — Declaration & Interactive Config (vẫn do `uedp.py` đảm nhiệm, nhưng đã được rút gọn).
- Giai đoạn 2 — Test/Config Generation (chuyển toàn bộ sang `pltf/pycdscriptor/`, dùng Jinja2 template thay vì vá chuỗi).

Cấu trúc thư mục mới:

```text
pltf/
├── kconfigspec/                                                              # Sinh decl.kconfig (thay sources/common/pyspec cũ)
│   ├── usrinp.py
│   ├── tnorm.py
│   ├── tpoll.py
│   ├── sig.py
│   └── hwapi.py
├── templates/                                                                # Jinja2 template — sinh FILE MỚI, không vá chuỗi nữa
│   ├── appcfgh.txt
│   ├── appdeclh.txt
│   ├── corecfgh.txt
│   ├── palcfgh.txt
│   ├── archh.txt
│   ├── archc.txt
│   └── appc.txt                                                              # Template cho app.c, dùng bởi jnerator/postgen (μE-LS)
└── pycdscriptor/
    ├── attribarse/                                                           # Đọc .config thành context có cấu trúc
    │   ├── dotcfg.py
    │   └── glbda.py                                                          # Cầu nối context["task_tsm"/"task_fsm"] với glbda của μE-LS
    ├── lstaxer/                                                              # Parse + validate YAML μE-LS (xem mục 3.5)
    │   ├── symresolv.py, nullremov.py, strucjec.py
    │   ├── lukupmodel.py, vlid.py, kre8.py
    │   └── pydantic_model/                                                   # logic.py, resrc.py, misc.py
    ├── ustab/                                                                # Unified Symbol Table (xem mục 3.5)
    │   ├── gnnerate.py, cvert.py, xportstax.py
    │   └── custab.py                                                         # Orchestrator
    └── jnerator/
        ├── pregen/                                                           # Sinh khai báo Kconfig-based (mỗi file 1 artifact)
        │   ├── cfpcall.py                                                    # Parse .config 1 lần, trả context dùng chung
        │   ├── appcfgpgen.py, corecfgpgen.py, palcfgpgen.py, appdeclpgen.py
        │   ├── archdirpgen.py, archhpgen.py, archcpgen.py
        │   └── fpregen.py                                                    # Orchestrator, gọi tuần tự 7 generator ở trên
        └── postgen/                                                          # Sinh logic implementation từ YAML μE-LS đã validate
            ├── cgen.py                                                       # Entry point: yaml -> app.c
            └── modalcvert.py                                                 # Render app.c bằng template appc.txt
```

So với `sources/common/kconfiglib/` (vẫn giữ nguyên, không di chuyển vì đây là thư viện bên thứ 3, không phải phần tự viết), toàn bộ phần tự viết của KwDI (`kconfigspec`, `pycdscriptor`) được gom về một chỗ duy nhất là `pltf/`, tách khỏi `sources/common/` — phản ánh đúng ý nghĩa "Portable": `pltf/` không phụ thuộc vào cấu trúc `sources/`, có thể tái sử dụng cho một dự án μEDP khác chỉ cần trỏ đúng đường dẫn output.

### 3.1 `uedp.py` sau khi refactor

`uedp.py` giờ chỉ còn đúng một trách nhiệm: sinh `decl.kconfig` và chạy `menuconfig` tương tác.

```python
from pltf.kconfigspec import user_input, task_norm_declaration, task_poll_declaration, signal_declaration, hardware_api_declaration

def main():
  os.environ["KCONFIG_CONFIG"] = ".config"
  os.environ["MENUCONFIG_STYLE"] = "aquatic"
  # fsm_flags/tsm_flags/n_tsm_st_list/n_fsm_st_list giờ là list, mỗi phần tử
  # tương ứng với 1 task norm (mỗi task khai báo FSM/TSM riêng kèm số lượng
  # state riêng, thay vì dùng chung 1 cờ + 1 số lượng như trước 1.1.6).
  (n_norm, n_poll, n_sig, fsm_flags, tsm_flags, n_tsm_st_list, n_fsm_st_list, n_hw_api) = user_input(DEFAULT_VALS)
  open("sources/app/kconfig/decl.kconfig", "w").close()
  task_norm_declaration(n_norm, n_tsm_st_list, n_fsm_st_list, tsm_flags, fsm_flags)
  task_poll_declaration(n_poll)
  signal_declaration(n_sig)
  hardware_api_declaration(n_hw_api)
  kconf = kconfiglib.Kconfig("Kconfig")
  if os.path.exists(".config"):
    kconf.load_config(".config")
  menuconfig.menuconfig(kconf)
  kconf.write_config(".config")
```

Toàn bộ 5 hàm `corecfg_gen`, `palcfg_gen`, `app_cfg_gen`, `app_decl_gen`, `pal_arch_gen` đã bị loại khỏi `uedp.py` — không còn logic sinh code nào ở đây nữa. `uedp.py` dừng lại đúng ở bước ghi ra `.config`, việc sinh code được nhường hoàn toàn cho `pltf/pycdscriptor/`. Đây là thay đổi quan trọng nhất khi so với KwDI: tách bạch "thu thập cấu hình" khỏi "sinh code", cho phép chạy lại bước sinh code nhiều lần từ cùng một `.config` mà không cần lặp lại `menuconfig`.

Điểm khác so với bản trước 1.1.6: `user_input()` trả về 4 list thay vì 4 giá trị đơn — `fsm_flags`, `tsm_flags`, `n_tsm_st_list`, `n_fsm_st_list`, mỗi phần tử ứng với 1 task norm theo đúng thứ tự đã khai báo. `task_norm_declaration()` dùng đúng 4 list này để sinh `APPCFG_TSM_TASK_{i}`/`APPCFG_FSM_TASK_{i}` với số state riêng cho từng task #i thay vì 1 cờ + 1 số lượng dùng chung cho toàn bộ task như bản KwDI/PLTF ban đầu.

### 3.2 `pltf/kconfigspec/` — sinh khai báo Kconfig

Logic gần như giữ nguyên so với `sources/common/pyspec/` cũ của KwDI (khi đó module còn tên `pyspec`, file còn mang hậu tố `_pspec`, ví dụ `tsknrmdcl.py`).

Sau khi chuyển vào `pltf/`, module trải qua 2 lần đổi tên riêng biệt: trước tiên bỏ hậu tố `_pspec` cho gọn (`usrinp_pspec.py` → `usrinp.py`, `tnorm_pspec.py` → `tnorm.py`, tương tự cho `tpoll`, `sig`, `hwapi`), sau đó đổi tên cả package từ `pyspec` sang `kconfigspec` để phản ánh đúng vai trò "sinh khai báo Kconfig" thay vì giữ tên gọi mang tính lịch sử từ KwDI. Vẫn sinh `sources/app/kconfig/decl.kconfig` theo cú pháp Kconfig thô (`menu`, `config ... string`, `default`, `depends on`). Điểm khác biệt so với KwDI là các module này giờ nằm trong `pltf/kconfigspec/`, và được `uedp.py` import gọn qua package `pltf.kconfigspec` (xem mục 3.1) thay vì `sources.common.pyspec.*` như bản KwDI gốc.

### 3.3 `pltf/pycdscriptor/attribarse/dotcfg.py` — trái tim của pipeline sinh code

Đây là phần thay thế trực tiếp cho logic duyệt `kconf.unique_defined_syms` từng nằm rải rác trong `corecfg_gen`/`palcfg_gen` của `uedp.py` cũ. `dotcfg.cfp_parse_dotcfg(config_path)` đọc thẳng file `.config` (dạng text `CONFIG_KEY=value`) — không cần `kconfiglib` nữa ở bước này — và trả về một `context` dict có cấu trúc:

- `core_configs`, `pal_configs`: danh sách chuỗi `#define` cho `CORE_*` / `PAL_*`.
- `tasknorm_defs`, `taskpoll_defs`, `sig_defs`: tự động gán ID hex tăng dần, bắt đầu từ `0xE6` (task norm), `0xD4` (task poll), `0x01` (signal) — khớp với dải `[HES] Heximal Encoding Signals` đã mô tả trong `arch-design.md` (`TASK_NORM` ở `0xEx`, `TASK_POLL` ở `0xDx`).
- `msgq_defs`, `normhler_lists`, `pollhler_lists`: danh sách tên hàng đợi / handler để sinh bảng task.
- `appcfg_tsm_*`, `appcfg_fsm_*`, `tsmio_lists`, `fsmio_lists`: dữ liệu riêng cho TSM/FSM (đối tượng, bảng chuyển trạng thái, danh sách state).
- `arch_name`, `arch_apis`: tên kiến trúc PAL và danh sách Hardware API cần sinh.
- `task_tsm`, `task_fsm`: map task → danh sách state, ban đầu chỉ dự trù cho μE-LS, nay đã được `pltf/pycdscriptor/attribarse/glbda.py` và pipeline `lstaxer` tiêu thụ thực sự (xem mục 3.5).

Kể từ khi có `pltf/pycdscriptor/jnerator/pregen/cfpcall.py`, việc mỗi generator tự gọi `dotcfg.cfp_parse_dotcfg()` độc lập đã được loại bỏ: `cfpcall.main()` parse `.config` đúng 1 lần, trả về `context` dùng chung, rồi orchestrator `fpregen.py` truyền `context` này vào cho cả 7 generator (xem mục 3.4). Đây là điểm đã được khắc phục so với thiết kế PLTF ban đầu.

### 3.4 `pltf/templates/` + `pltf/pycdscriptor/jnerator/pregen/` — sinh file bằng Jinja2

Khác với cơ chế "vá chuỗi vào giữa 2 marker" của KwDI, mỗi generator trong PLTF render toàn bộ nội dung file từ template Jinja2 rồi ghi đè hoàn toàn file đích. Từ khi có `cfpcall.py` (mục 3.3), mỗi generator nhận sẵn `context` qua tham số thay vì tự parse `.config`:

```python
# pltf/pycdscriptor/jnerator/pregen/corecfgpgen.py
def main(context):
  env = Environment(loader=FileSystemLoader('./pltf/templates'))
  template = env.get_template('corecfgh.txt')
  output = template.render(current_date=context["current_date"], core_configs=context['core_configs'])
  with open("sources/app/config/core_cfg.h", "w", encoding="utf-8") as f:
    f.write(output)
```

7 generator tương ứng 7 artifact đầu ra (tất cả nằm trong `pltf/pycdscriptor/jnerator/pregen/`):

| Generator | File sinh ra | Ghi chú |
| --- | --- | --- |
| `corecfgpgen.py` | `sources/app/config/core_cfg.h` | Thay `corecfg_gen()` cũ |
| `palcfgpgen.py` | `sources/app/config/pal_cfg.h` | Thay `palcfg_gen()` cũ |
| `appcfgpgen.py` | `sources/app/config/app_cfg.h` | Thay `app_cfg_gen()` cũ |
| `appdeclpgen.py` | `sources/app/declaration/app_decl.h` | Thay `app_decl_gen()` cũ |
| `archdirpgen.py` | thư mục `sources/pal/arch/<arch_name>/` | Tạo thư mục trước khi 2 generator dưới ghi file vào |
| `archhpgen.py` | `sources/pal/arch/<arch_name>/<arch_name>_arch.h` | Thay `pal_arch_gen()` cũ (phần `.h`) |
| `archcpgen.py` | `sources/pal/arch/<arch_name>/<arch_name>_arch.c` | Thay `pal_arch_gen()` cũ (phần `.c`) |

`cfpcall.py` (parse `.config` 1 lần, trả `context`) và `fpregen.py` (orchestrator, tên gọi trong task-list trước đây là "tsgen") cùng nằm trong `pltf/pycdscriptor/jnerator/pregen/`, chạy tuần tự cả 7 generator với `context` dùng chung:

```python
# pltf/pycdscriptor/jnerator/pregen/fpregen.py
from . import appcfgpgen, corecfgpgen, palcfgpgen, appdeclpgen
from . import archdirpgen, archhpgen, archcpgen, cfpcall

if __name__ == "__main__":
  context = cfpcall.main()
  appcfgpgen.main(context); corecfgpgen.main(context); palcfgpgen.main(context)
  appdeclpgen.main(context); archdirpgen.main(context); archhpgen.main(context); archcpgen.main(context)
```

Vì dùng template render-toàn-file thay vì patch, PLTF không còn phụ thuộc vào việc file đích đã tồn tại từ trước với marker cố định — đây là điểm cải thiện trực tiếp lên giới hạn "sinh code kiểu vá chuỗi" đã nêu ở mục 2.3.

### 3.5 μE-LS / PLD: pipeline `lstaxer` + `ustab` (đã triển khai)

Khác với giai đoạn đầu của PLTF (khi `attribarse/glbda.py` và `test.yaml` còn là bản nháp/PoC độc lập, chưa được orchestrator nào gọi tới), tính đến 1.1.6 pipeline μE-LS (Logical Syntax-izer, thuộc tính năng PLD - Parse-able Logical Descriptor) đã được triển khai đầy đủ và tích hợp vào `entrypoint.sh` (xem mục 4.2). Cú pháp YAML đầy đủ được mô tả chi tiết trong `docs/uels-syntax.md`; ở đây chỉ tóm tắt các module liên quan trực tiếp tới pipeline sinh code:

- `pltf/pycdscriptor/lstaxer/`: bộ parse + validate file YAML μE-LS (ví dụ `sources/app/lstaxizer.yaml`) — gồm `symresolv.py` (dựng Symbol Resolution Map để giải quyết anchor/alias), `nullremov.py` (dọn các khai báo `NULL` thừa người dùng không cần điền), `strucjec.py` (chuẩn hoá cấu trúc `actvobj`/`fsm`/`tsm` của `tlist`), `lukupmodel.py` (đưa dữ liệu post-validate vào `pydantic_model`), `vlid.py` (5 chiến lược validate), và `kre8.py` (tổng hợp toàn bộ thành `context` cho bước sinh code, hàm `build_generator_context()`).
- `pltf/pycdscriptor/attribarse/glbda.py`: điểm nối giữa `dotcfg.py` (field `task_tsm`/`task_fsm`, mục 3.3) và khối `glbda:` trong cú pháp μE-LS.
- `pltf/pycdscriptor/jnerator/postgen/`: giai đoạn sinh code sau khi logic đã được validate (`cgen.py` gọi `lstaxer.kre8.build_generator_context()` rồi `modalcvert.generate_appc()` để render `sources/app/app.c` trực tiếp từ YAML — đây là điểm khác biệt lớn nhất so với `jnerator/pregen/`, vốn chỉ sinh khai báo/định nghĩa chứ không sinh logic implementation).
- `pltf/pycdscriptor/ustab/`: Unified Symbol Table — `gnnerate.py` dựng bảng ký hiệu từ `.config`/Kconfig, `xportstax.py` export toàn bộ config sang YAML để đối chiếu ngược với μE-LS, `custab.py` là orchestrator gọi cả hai.

Nói cách khác, pipeline hiện tại có 3 giai đoạn nối tiếp: pre-logicdef (`jnerator/pregen`, sinh khai báo Kconfig-based) → post-logicdef (`jnerator/postgen`, sinh logic implementation từ YAML đã validate) → ustab (đối chiếu, xuất bảng ký hiệu). `docs/uels-syntax.md` mô tả chi tiết ngữ nghĩa YAML; tài liệu này chỉ tập trung vào việc pipeline PLTF gọi các module đó ra sao.

## 4. Docker & orchestration mới

### 4.1 `Dockerfile`

So với bản KwDI gốc (chỉ `python:3.13-slim` + `kconfiglib`), Dockerfile của PLTF mở rộng đáng kể để phục vụ đúng vai trò "Local Test Framework":

- Cài thêm toolchain build: `git wget cmake binutils gcc make g++ gdb`.
- Cài ESP-IDF v5.1 vào `/opt/esp-idf` (biến `IDF_PATH`) — chuẩn bị sẵn để build/test trên kiến trúc ESP32 ngay trong container, không cần thoát ra ngoài như KwDI.
- Cài thêm `jinja2`, `pytest`, `pyserial` bên cạnh `kconfiglib` — phục vụ pipeline render template và (trong tương lai) chạy test tự động.
- Tạo sẵn 2 thư mục làm việc tách biệt: `/uedp-libs` (mã nguồn lõi μEDP, mount từ repo) và `/uedp-test` (workspace riêng cho PLTF) — hiện thực hoá đúng mục "Không phân tách workspace" đã nêu là giới hạn của KwDI.
- Cài `gosu` để hạ quyền từ `root` xuống user thường trước khi vào shell tương tác.
- `ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]` thay vì `CMD` gọi thẳng `python uedp.py menuconfig` như bản gốc.

### 4.2 `entrypoint.sh`

```bash
#!/bin/bash
set -e
USER_ID=${MY_UID:-1000}
GROUP_ID=${MY_GID:-1000}
# Tạo user "uedp_user" khớp UID/GID của người dùng trên host
if ! id -u uedp_user >/dev/null 2>&1; then
  groupadd -g $GROUP_ID uedp_group 2>/dev/null || true
  useradd --shell /bin/bash -u $USER_ID -g $GROUP_ID -o -c "" -m uedp_user
fi
chown $USER_ID:$GROUP_ID /uedp-libs
chown $USER_ID:$GROUP_ID /uedp-test
export HOME=/home/uedp_user
echo "source $IDF_PATH/export.sh > /dev/null 2>&1" >> /home/uedp_user/.bashrc
# [ENTRY 1] Thu thập input + menuconfig (giữ nguyên như KwDI)
python uedp.py menuconfig
# [ENTRY 2] Sinh khai báo pre-logicdef (Kconfig-based) từ .config
python -m pltf.pycdscriptor.jnerator.pregen.fpregen
# [ENTRY 3] Sinh logic implementation (app.c) từ post-logicdef YAML (μE-LS)
python -m pltf.pycdscriptor.jnerator.postgen.cgen \
  --yaml sources/app/lstaxizer.yaml \
  --output sources/app/app.c
# [ENTRY 4] Sinh/đối chiếu Unified Symbol Table
python -m pltf.pycdscriptor.ustab.custab
chown -R $USER_ID:$GROUP_ID /uedp-libs/*
exec gosu uedp_user bash
```

Bốn điểm thiết kế đáng chú ý:

- Xử lý UID/GID qua biến môi trường `MY_UID`/`MY_GID` (mặc định `1000`): giải quyết trực tiếp vấn đề "file sinh ra thuộc quyền `root` trên host" của KwDI, vì volume `.:/uedp-libs` được mount 2 chiều.
- 4 giai đoạn nối tiếp trong cùng một lần chạy container: `menuconfig` (KwDI, giữ nguyên) → `fpregen` (pre-logicdef, sinh khai báo Kconfig-based, mục 3.4) → `cgen` (post-logicdef, sinh `app.c` từ YAML μE-LS đã validate, mục 3.5) → `ustab.custab` (đối chiếu, xuất bảng ký hiệu). Với người dùng, trải nghiệm vẫn là "một lệnh, một lần chạy", nhưng bên trong là 4 pipeline tách biệt, có thể gọi lại độc lập bằng `python -m pltf.pycdscriptor....`.
- So với bản PLTF ban đầu (chỉ có 2 ENTRY: menuconfig + 1 script sinh code duy nhất), việc tách thêm ENTRY 3/4 phản ánh đúng việc pipeline μE-LS (mục 3.5) đã được tích hợp thật vào orchestration, không còn là script debug độc lập.
- `exec gosu uedp_user bash` ở cuối: sau khi sinh code xong, container không thoát ngay mà rơi vào shell với quyền user thường, cho phép làm việc tiếp (`cd /uedp-test` để phát triển PLTF, hoặc `exit` để chỉ lấy code vừa sinh).

### 4.3 `docker-compose.yaml`

```yaml
services:
  uedp_udc:
    image: uedp-p:latest
    container_name: uedp_udc
    stdin_open: true
    tty: true
    volumes:
      - .:/uedp-libs
    working_dir: /uedp-libs
    hostname: container_env
```

Thay cho việc gõ `docker run` thủ công (không có ở KwDI), `docker-compose.yaml` cố định lại toàn bộ tham số cần thiết cho phiên làm việc tương tác (`stdin_open`/`tty` để `menuconfig` hoạt động được), giúp lệnh khởi động rút gọn còn `docker compose run uedp_udc` (hoặc `up`), nhất quán giữa các máy phát triển khác nhau — đúng tinh thần "Portable" của PLTF.

### 4.4 `.dockerignore`

```text
.git
__pycache__
*.pyc
sources/test/
.vscode/
*.o
*.a
docs/
```

Giữ nguyên từ KwDI, không đổi — vẫn loại `docs/` (chứa PDF tham khảo + video hướng dẫn dung lượng lớn ở `docs/references/`, `docs/videos/`) ra khỏi build context để giảm thời gian `docker build`.

## 5. So sánh KwDI vs PLTF

| Khía cạnh | KwDI (gốc) | PLTF (hiện tại) |
| --- | --- | --- |
| Vị trí code tự viết | `sources/common/{kconfiglib,pyspec}` | `pltf/{kconfigspec,templates,pycdscriptor}` (`kconfiglib` — thư viện 3rd-party — vẫn ở lại `sources/common/kconfiglib`) |
| Số giai đoạn | 1 (gộp chung trong `uedp.py`) | 4 (declaration+menuconfig trong `uedp.py` → `jnerator/pregen` sinh khai báo → `jnerator/postgen` sinh logic từ μE-LS → `ustab` đối chiếu ký hiệu) |
| Cơ chế sinh code | Vá chuỗi giữa 2 marker vào file `.h` có sẵn | Render toàn bộ file mới bằng Jinja2 template (`pregen`) hoặc từ YAML đã validate (`postgen`) |
| Input cho bước sinh code | Trực tiếp object `kconf` (`kconfiglib`) | File `.config` đã ghi ra đĩa, parse 1 lần qua `cfpcall.py` (`pregen`) và file YAML μE-LS đã qua `lstaxer.vlid` (`postgen`) |
| Docker image | `python:3.13-slim` + `kconfiglib` | + `gcc/cmake/gdb`, + ESP-IDF v5.1, + `jinja2/pytest/pyserial`, + `gosu` |
| Khởi động container | `CMD` gọi thẳng `uedp.py menuconfig` | `entrypoint.sh` (tạo user, xử lý UID/GID, chạy tuần tự 4 ENTRY, rồi vào shell) |
| Orchestration | Không có (`docker run` thủ công) | `docker-compose.yaml` (service `uedp_udc`) |
| Workspace | Trộn chung 1 thư mục | Tách `/uedp-libs` (core lib) và `/uedp-test` (PLTF workspace) |
| Test spec nâng cao (μE-LS/PLD) | Không có | Đã triển khai đầy đủ: `lstaxer/` (parse + validate) + `attribarse/glbda.py` (cầu nối `dotcfg`) + `jnerator/postgen` (sinh `app.c`) + `ustab/` (đối chiếu ký hiệu) — xem `docs/uels-syntax.md` và mục 3.5 |

## 6. Việc còn thiếu / rủi ro cần lưu ý khi tiếp tục triển khai

- Đường dẫn `sys.path` còn sót lại từ trước khi chuyển `pltf/`: `uedp.py` vẫn chèn `sources/common/kconfigspec` vào `sys.path` dù thư mục này không còn tồn tại (module thật đã chuyển hẳn sang `pltf/kconfigspec/` và được import qua package `pltf.kconfigspec`) — dòng chèn này hiện là dead code, vô hại nhưng gây nhầm lẫn khi đọc code. Việc dọn dẹp này đã được liệt kê riêng trong `docs/to-do.md` ("chore filename để thống nhất các module riêng biệt của PLTF"), chưa thực hiện tại thời điểm viết tài liệu này.
- Vị trí `ustab.custab` trong `entrypoint.sh` cần xem lại: hiện `ustab.custab` chạy ở ENTRY cuối cùng, sau khi `cgen` đã sinh `app.c` — cần xác nhận lại thứ tự này có đúng ý đồ thiết kế hay không (`docs/to-do.md` đang để mục riêng "Sửa đổi vị trí ustab.custab trong pipeline trên entrypoint.sh" ở trạng thái chưa xử lý).
- `entrypoint.sh` luôn chạy `uedp.py menuconfig` ở mỗi lần container khởi động: phù hợp cho phiên làm việc tương tác trên máy dev, nhưng chưa có nhánh non-interactive (ví dụ đọc thẳng `.config` có sẵn, bỏ qua menuconfig) để dùng trong CI/CD.
- Chưa có test tự động cho chính `pltf/`: bản thân testing framework (parser, generator, template, cả pipeline `lstaxer`/`ustab` mới) hiện chưa có test riêng để đảm bảo không hồi quy khi sửa template hay parser.
- `lstaxer.nullremov` đang bị đánh dấu có thể dư thừa: `docs/to-do.md` ghi nhận cân nhắc loại bỏ module này khỏi pipeline chung vì có thể làm phức tạp thêm việc parse mà không mang lại lợi ích tương xứng - cần theo dõi quyết định cuối cùng để cập nhật lại mục 3.5 nếu module bị loại bỏ.
- Chưa có BST (Basic Software Test) trên phần cứng thật cho pipeline PLD/μE-LS: mới dừng ở mức sinh code + review thiết kế, chưa có vòng kiểm thử thực tế xác nhận `app.c` sinh ra từ μE-LS chạy đúng trên hardware.

<!-- STATUS
1. Add task to remove dead code in `uedp.py` related to `sys.path` manipulation.
2. The position of `ustab.custab` in `entrypoint.sh` is aleady fit for the design, therefore no change is needed.
3. Approved, can be split as a parameterized option to skip `menuconfig` for CI/CD usage.
4. Not approved, `/pltf` test suite has already meant to be CI/CD until now, therfore, the task is not needed and leaving non-unit test is intentional for bug fixing as a training session for newcomers.
5. Approved, `lstaxer.nullremov` has completely been removed from the pipeline, therefore the task is not needed.
6. Approved, BST on real hardware is not in the scope of this document, therefore the task has already appeaered in `docs/to-do.md` and is not needed to be repeated here.
-->

## 7. Kết luận

PLTF không thay thế Kconfig hay Docker của KwDI, mà tách lớp phần sinh code ra khỏi phần thu thập cấu hình, đồng thời container hoá đầy đủ hơn (toolchain build + ESP-IDF + user permission handling) để container không chỉ dùng để chạy `menuconfig` một lần mà có thể dùng làm môi trường phát triển và kiểm thử μEDP xuyên suốt. Khác với giai đoạn đầu (khi hướng mở rộng μE-LS/PLD còn là bản nháp `glbda.py`/`test.yaml` độc lập), tính đến 1.1.6 pipeline này đã được triển khai đầy đủ và tích hợp thật vào `entrypoint.sh` qua 3 giai đoạn `pregen` → `postgen` → `ustab` (mục 3.4, 3.5, 4.2). Phần việc lớn còn lại không còn là "tích hợp μE-LS" nữa, mà chuyển sang dọn dẹp kỹ thuật (đường dẫn `sys.path` còn sót, vị trí `ustab.custab` trong pipeline), bổ sung test tự động cho `pltf/`, và chạy BST trên phần cứng thật — như liệt kê ở mục 6.
