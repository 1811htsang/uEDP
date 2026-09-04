# Tài liệu thiết kế: Di chuyển tính năng từ [KwDI] sang [PLTF]

## 1. Bối cảnh

`docs/to-do.md`, mục **Phiên bản 1.2.0: The Infrastructure Preparation for μE-OS**, ghi:

- `[ ] Bổ sung tài liệu thiết kế PLTF (Portable Local Test Framework) nhằm cung cấp khả năng kiểm thử tự động đa quy mô.`
- `[ ] Thực hiện triển khai thiết kế PLTF.`

Trong khi đó, `arch-design.md` mục **[KwDI] Kconfig with Docker Integration** (phiên bản 1.1.2) mới chỉ mô tả việc tích hợp Kconfig với Docker để cấu hình lõi μEDP qua giao diện dòng lệnh, chưa đề cập tới việc sinh code tự động có kiểm thử đi kèm. Tài liệu này lấp khoảng trống đó: mô tả kiến trúc gốc của KwDI, các giới hạn của nó, và thiết kế PLTF đã thay thế/mở rộng KwDI trong code hiện tại.

## 2. Kiến trúc thiết kế gốc từ KwDI

### 2.1 Thành phần

KwDI gồm 3 phần chính, tất cả nằm gọn trong root repo và `sources/common/`:

- `Kconfig` (root) + `sources/app/kconfig/{core,pal,decl}.kconfig`: định nghĩa cây cấu hình.
- `sources/common/kconfiglib/`: thư viện `kconfiglib` + `menuconfig` (bên thứ 3) để đọc cây Kconfig và hiển thị giao diện `menuconfig` tương tác trên terminal.
- `sources/common/pyspec/`: các hàm sinh `decl.kconfig` (task norm, task poll, signal, hardware API) dựa theo số lượng người dùng nhập vào (`usrinp.py`, `tsknrmdcl.py`, `tskpoldcl.py`, `sigdcl.py`, `hwapidcl.py`).
- `uedp.py` (root): script duy nhất điều phối toàn bộ luồng — vừa thu thập input, vừa gọi `menuconfig`, vừa **tự sinh code** (`corecfg_gen`, `palcfg_gen`, `app_cfg_gen`, `app_decl_gen`, `pal_arch_gen`) bằng cách chèn trực tiếp chuỗi `#define` vào giữa 2 marker (`// KCONFIG_CORECFG_START` / `// KCONFIG_CORECFG_END`) trong các file header có sẵn ở `sources/app/config/`.
- `Dockerfile` (bản gốc): image `python:3.13-slim`, chỉ cài `kconfiglib`, `CMD ["python", "uedp.py", "menuconfig"]`.

### 2.2 Luồng hoạt động gốc

1. Người dùng chạy `docker build` rồi `docker run` (không có `docker-compose.yaml`, không có `entrypoint.sh`).
2. Container khởi động, chạy thẳng `python uedp.py menuconfig`.
3. `uedp.py` hỏi input (số task, số signal, có dùng FSM/TSM không...) → ghi vào `sources/app/kconfig/decl.kconfig`.
4. `kconfiglib.Kconfig("Kconfig")` load toàn bộ cây, mở `menuconfig` để người dùng chỉnh giá trị → ghi ra `.config`.
5. Ngay trong `main()` của `uedp.py`, gọi tuần tự `corecfg_gen()`, `palcfg_gen()`, `app_cfg_gen()`, `app_decl_gen()`, `pal_arch_gen()` — mỗi hàm tự duyệt `kconf.unique_defined_syms`, tự format chuỗi `#define ...`, rồi **patch trực tiếp** vào file `.h` đã tồn tại sẵn thông qua cặp marker.

### 2.3 Giới hạn của KwDI (lý do cần PLTF)

- **Không tách giai đoạn**: thu thập input, cấu hình tương tác (menuconfig), và sinh code nằm chung trong một hàm `main()` của `uedp.py`. Muốn sinh lại code từ một `.config` có sẵn (ví dụ trong CI) vẫn phải chạy lại toàn bộ `menuconfig` tương tác.
- **Sinh code kiểu "vá chuỗi" (marker-based patch)**: `corecfg_gen`/`palcfg_gen` yêu cầu file `.h` đích phải **đã tồn tại sẵn** với đúng cặp marker mới patch được — không tạo file mới từ đầu được, dễ vỡ nếu ai đó lỡ xoá marker.
- **`sources/common/testspec/`** (tiền thân dùng Jinja2) đã tồn tại nhưng chỉ là **bản nháp chưa nối vào luồng thật**: `appcfgpgen.py` gốc chỉ `print(output)` ra màn hình với `current_date` gán cứng `'16 May 2025'`, không đọc `.config` thật, không ghi file.
- **Docker image tối giản**, chỉ có `kconfiglib`: không có `gcc/cmake/gdb`, không có ESP-IDF, không thể build hay chạy test ngay trong container — người dùng vẫn phải thoát container để build bằng tay.
- **Không có `entrypoint.sh`/`docker-compose.yaml`**: container chạy `CMD` trực tiếp bằng root, không xử lý UID/GID → file được tạo ra (do mount volume) thuộc quyền sở hữu `root` trên máy host, gây phiền khi chỉnh sửa lại ở ngoài container.
- **Không phân tách workspace**: không có khái niệm thư mục riêng cho "mã nguồn lõi" và "không gian làm việc kiểm thử" — mọi thứ trộn chung trong repo.

## 3. Kiến trúc thiết kế của PLTF

Nguyên tắc cốt lõi của PLTF là **tách rõ 2 giai đoạn** vốn bị gộp chung trong KwDI:

- **Giai đoạn 1 — Declaration & Interactive Config** (vẫn do `uedp.py` đảm nhiệm, nhưng đã được rút gọn).
- **Giai đoạn 2 — Test/Config Generation** (chuyển toàn bộ sang `pltf/testspec/`, dùng Jinja2 template thay vì vá chuỗi).

Cấu trúc thư mục mới:

```text
pltf/
├── pyspec/                 # Sinh decl.kconfig (thay sources/common/pyspec cũ)
│   ├── usrinp.py
│   ├── tnorm.py
│   ├── tpoll.py
│   ├── sig.py
│   └── hwapi.py
├── templates/               # Jinja2 template — sinh FILE MỚI, không vá chuỗi nữa
│   ├── appcfgh.txt
│   ├── appdeclh.txt
│   ├── corecfgh.txt
│   ├── palcfgh.txt
│   ├── archh.txt
│   └── archc.txt
└── testspec/
    ├── attribarse/            # Đọc .config (và tương lai là YAML) thành context có cấu trúc
    │   ├── dotcfg.py
    │   ├── glbda.py       # Bản nháp cho hướng μE-LS (xem mục 3.5)
    │   └── test.yaml
    └── generators/           # Mỗi file phụ trách 1 artifact đầu ra
        ├── appcfgpgen.py
        ├── corecfgpgen.py
        ├── palcfgpgen.py
        ├── appdeclpgen.py
        ├── archdirpgen.py
        ├── archhpgen.py
        ├── archcpgen.py
        └── fpregen.py          # Orchestrator, gọi tuần tự 7 generator ở trên
```

So với `sources/common/kconfiglib/` (vẫn giữ nguyên, không di chuyển vì đây là thư viện bên thứ 3, không phải phần tự viết), toàn bộ phần **tự viết** của KwDI (`pyspec`, `testspec`) được gom về một chỗ duy nhất là `pltf/`, tách khỏi `sources/common/` — phản ánh đúng ý nghĩa "Portable": `pltf/` không phụ thuộc vào cấu trúc `sources/`, có thể tái sử dụng cho một dự án μEDP khác chỉ cần trỏ đúng đường dẫn output.

### 3.1 `uedp.py` sau khi refactor

`uedp.py` giờ chỉ còn đúng một trách nhiệm: sinh `decl.kconfig` và chạy `menuconfig` tương tác.

```python
from pltf.pyspec.usrinp import user_input
from pltf.pyspec.tnorm import task_norm_declaration
from pltf.pyspec.tpoll import task_poll_declaration
from pltf.pyspec.sig import signal_declaration
from pltf.pyspec.hwapi import hardware_api_declaration

def main():
  os.environ["KCONFIG_CONFIG"] = ".config"
  os.environ["MENUCONFIG_STYLE"] = "aquatic"
  (n_norm, n_poll, n_sig, use_fsm, use_tsm, n_tsm_st, n_fsm_st, n_hw_api) = user_input(DEFAULT_VALS)
  open("sources/app/kconfig/decl.kconfig", "w").close()
  task_norm_declaration(n_norm, n_tsm_st, n_fsm_st, use_tsm, use_fsm)
  task_poll_declaration(n_poll)
  signal_declaration(n_sig)
  hardware_api_declaration(n_hw_api)
  kconf = kconfiglib.Kconfig("Kconfig")
  if os.path.exists(".config"):
    kconf.load_config(".config")
  menuconfig.menuconfig(kconf)
  kconf.write_config(".config")
```

Toàn bộ 5 hàm `corecfg_gen`, `palcfg_gen`, `app_cfg_gen`, `app_decl_gen`, `pal_arch_gen` **đã bị loại khỏi `uedp.py`** — không còn logic sinh code nào ở đây nữa. `uedp.py` dừng lại đúng ở bước ghi ra `.config`, việc sinh code được nhường hoàn toàn cho `pltf/testspec/`. Đây là thay đổi quan trọng nhất khi so với KwDI: **tách bạch "thu thập cấu hình" khỏi "sinh code"**, cho phép chạy lại bước sinh code nhiều lần từ cùng một `.config` mà không cần lặp lại `menuconfig`.

### 3.2 `pltf/pyspec/` — sinh khai báo Kconfig

Logic gần như giữ nguyên so với `sources/common/pyspec/` cũ (đổi tên file theo hậu tố `_pspec` cho nhất quán, ví dụ `tsknrmdcl.py` → `tnorm.py`), vẫn sinh `sources/app/kconfig/decl.kconfig` theo cú pháp Kconfig thô (`menu`, `config ... string`, `default`, `depends on`). Điểm khác biệt là các module này giờ nằm trong `pltf/`, được `uedp.py` import qua `pltf.pyspec.*` thay vì `sources.common.pyspec.*`.

### 3.3 `pltf/testspec/attribarse/dotcfg.py` — trái tim của pipeline sinh code

Đây là phần thay thế trực tiếp cho logic duyệt `kconf.unique_defined_syms` từng nằm rải rác trong `corecfg_gen`/`palcfg_gen` của `uedp.py` cũ. `dotcfg.cfp_parse_dotcfg(config_path)` đọc thẳng file `.config` (dạng text `CONFIG_KEY=value`) — **không cần** `kconfiglib` nữa ở bước này — và trả về một `context` dict có cấu trúc:

- `core_configs`, `pal_configs`: danh sách chuỗi `#define` cho `CORE_*` / `PAL_*`.
- `tasknorm_defs`, `taskpoll_defs`, `sig_defs`: tự động gán ID hex tăng dần, bắt đầu từ `0xE6` (task norm), `0xD4` (task poll), `0x01` (signal) — khớp với dải `[HES] Heximal Encoding Signals` đã mô tả trong `arch-design.md` (`TASK_NORM` ở `0xEx`, `TASK_POLL` ở `0xDx`).
- `msgq_defs`, `normhler_lists`, `pollhler_lists`: danh sách tên hàng đợi / handler để sinh bảng task.
- `appcfg_tsm_*`, `appcfg_fsm_*`, `tsmio_lists`, `fsmio_lists`: dữ liệu riêng cho TSM/FSM (đối tượng, bảng chuyển trạng thái, danh sách state).
- `arch_name`, `arch_apis`: tên kiến trúc PAL và danh sách Hardware API cần sinh.
- `task_tsm`, `task_fsm`: map task → danh sách state, dự trù cho tích hợp μE-LS sau này (xem mục 3.5).

Mỗi generator (`*_tsgen.py`) tự gọi `dotcfg.cfp_parse_dotcfg()` độc lập — đây là điểm còn trùng lặp cần lưu ý, xem mục 5.

### 3.4 `pltf/templates/` + `pltf/testspec/generators/` — sinh file bằng Jinja2

Khác với cơ chế "vá chuỗi vào giữa 2 marker" của KwDI, mỗi generator trong PLTF **render toàn bộ nội dung file từ template Jinja2 rồi ghi đè hoàn toàn** file đích:

```python
# pltf/testspec/generators/corecfgpgen.py
context = dotcfg.cfp_parse_dotcfg(config_dir)
env = Environment(loader=FileSystemLoader('./pltf/templates'))
template = env.get_template('corecfgh.txt')
output = template.render(current_date=context["current_date"], core_configs=context['core_configs'])
with open("sources/app/config/core_cfg.h", "w", encoding="utf-8") as f:
  f.write(output)
```

7 generator tương ứng 7 artifact đầu ra:

| Generator | File sinh ra | Ghi chú |
| --- | --- | --- |
| `corecfgpgen.py` | `sources/app/config/core_cfg.h` | Thay `corecfg_gen()` cũ |
| `palcfgpgen.py` | `sources/app/config/pal_cfg.h` | Thay `palcfg_gen()` cũ |
| `appcfgpgen.py` | `sources/app/config/app_cfg.h` | Thay `app_cfg_gen()` cũ |
| `appdeclpgen.py` | `sources/app/declaration/app_decl.h` | Thay `app_decl_gen()` cũ |
| `archdirpgen.py` | thư mục `sources/pal/arch/<arch_name>/` | Tạo thư mục trước khi 2 generator dưới ghi file vào |
| `archhpgen.py` | `sources/pal/arch/<arch_name>/<arch_name>_arch.h` | Thay `pal_arch_gen()` cũ (phần `.h`) |
| `archcpgen.py` | `sources/pal/arch/<arch_name>/<arch_name>_arch.c` | Thay `pal_arch_gen()` cũ (phần `.c`) |

`tsgen.py` là orchestrator, chạy tuần tự cả 7 generator và in log tiến trình:

```python
import appcfgpgen, corecfgpgen, palcfgpgen, appdeclpgen
import archdirpgen, archhpgen, archcpgen

if __name__ == "__main__":
  appcfgpgen.main(); corecfgpgen.main(); palcfgpgen.main()
  appdeclpgen.main(); archdirpgen.main(); archhpgen.main(); archcpgen.main()
```

Vì dùng template render-toàn-file thay vì patch, PLTF **không còn phụ thuộc vào việc file đích đã tồn tại từ trước với marker cố định** — đây là điểm cải thiện trực tiếp lên giới hạn "sinh code kiểu vá chuỗi" đã nêu ở mục 2.3.

### 3.5 Hướng mở rộng: μE-LS / YAML test spec (`attribarse/glbda.py`, `test.yaml`)

`pltf/testspec/attribarse/glbda.py` và `test.yaml` là bản **nháp/PoC**, hiện **chưa được `tsgen.py` gọi tới** — chỉ chạy độc lập để debug. Đây là bước chuẩn bị hạ tầng cho μE-LS (Logical Syntax-izer), mô tả chi tiết trong `docs/uels-syntax.md`: một cú pháp khai báo dạng YAML cho Task/TSM/FSM/Signal/Action, thuộc tính năng PLD (Parse-able Logical Descriptor), dự kiến làm nền tảng cho PLTF và TLC (Test Level Coverager) ở chính phiên bản 1.2.0. `test.yaml` hiện đã minh hoạ một kịch bản 3 task (`KID_TASK_USR` dùng TSM, `KID_TASK_A` dùng TSM, `KID_TASK_B` dùng FSM) với các action `post_msg`/`log` — đúng mô hình dữ liệu mà `dotcfg.py` đã chuẩn bị sẵn field `task_tsm`/`task_fsm` để tiếp nhận.

## 4. Docker & orchestration mới

### 4.1 `Dockerfile`

So với bản KwDI gốc (chỉ `python:3.13-slim` + `kconfiglib`), Dockerfile của PLTF mở rộng đáng kể để phục vụ đúng vai trò "Local Test Framework":

- Cài thêm toolchain build: `git wget cmake binutils gcc make g++ gdb`.
- Cài **ESP-IDF v5.1** vào `/opt/esp-idf` (biến `IDF_PATH`) — chuẩn bị sẵn để build/test trên kiến trúc ESP32 ngay trong container, không cần thoát ra ngoài như KwDI.
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
chown $USER_ID:$GROUP_ID /uedp-libs /uedp-test
export HOME=/home/uedp_user
echo "source $IDF_PATH/export.sh > /dev/null 2>&1" >> /home/uedp_user/.bashrc
# [ENTRY 1] Giai đoạn KwDI — thu thập input + menuconfig
python uedp.py menuconfig
# [ENTRY 2] Giai đoạn PLTF — sinh code từ .config
python pltf/testspec/generators/tsgen.py
exec gosu uedp_user bash
```

Ba điểm thiết kế đáng chú ý:

- **Xử lý UID/GID qua biến môi trường `MY_UID`/`MY_GID`** (mặc định `1000`): giải quyết trực tiếp vấn đề "file sinh ra thuộc quyền `root` trên host" của KwDI, vì volume `.:/uedp-libs` được mount 2 chiều.
- **Gộp cả 2 giai đoạn KwDI + PLTF trong cùng một lần chạy container**: `entrypoint.sh` gọi `uedp.py menuconfig` (giai đoạn KwDI, giữ nguyên) rồi gọi ngay `pltf/testspec/generators/tsgen.py` (giai đoạn PLTF, mới) — với người dùng, trải nghiệm vẫn là "một lệnh, một lần chạy", nhưng bên trong đã là 2 pipeline tách biệt, có thể gọi lại độc lập.
- **`exec gosu uedp_user bash`** ở cuối: sau khi sinh code xong, container không thoát ngay mà rơi vào shell với quyền user thường, cho phép làm việc tiếp (`cd /uedp-test` để phát triển PLTF, hoặc `exit` để chỉ lấy code vừa sinh).

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
| Vị trí code tự viết | `sources/common/{kconfiglib,pyspec}` | `pltf/{pyspec,templates,testspec}` (`kconfiglib` — thư viện 3rd-party — vẫn ở lại `sources/common/kconfiglib`) |
| Số giai đoạn | 1 (gộp chung trong `uedp.py`) | 2 (declaration+menuconfig trong `uedp.py`, sinh code trong `pltf/testspec`) |
| Cơ chế sinh code | Vá chuỗi giữa 2 marker vào file `.h` có sẵn | Render toàn bộ file mới bằng Jinja2 template |
| Input cho bước sinh code | Trực tiếp object `kconf` (`kconfiglib`) | File `.config` đã ghi ra đĩa (`dotcfg.py` tự parse lại) |
| Docker image | `python:3.13-slim` + `kconfiglib` | + `gcc/cmake/gdb`, + ESP-IDF v5.1, + `jinja2/pytest/pyserial`, + `gosu` |
| Khởi động container | `CMD` gọi thẳng `uedp.py menuconfig` | `entrypoint.sh` (tạo user, xử lý UID/GID, chạy tuần tự KwDI-stage → PLTF-stage, rồi vào shell) |
| Orchestration | Không có (`docker run` thủ công) | `docker-compose.yaml` (service `uedp_udc`) |
| Workspace | Trộn chung 1 thư mục | Tách `/uedp-libs` (core lib) và `/uedp-test` (PLTF workspace) |
| Test spec nâng cao | Không có | `attribarse/glbda.py` + `test.yaml` (bản nháp, hướng tới μE-LS/PLD — xem `docs/uels-syntax.md`) |

## 6. Việc còn thiếu / rủi ro cần lưu ý khi tiếp tục triển khai

- **`glbda.py` chưa nối vào `tsgen.py`**: hiện chỉ là script debug độc lập (`python pltf/testspec/attribarse/glbda.py`), chưa có generator nào tiêu thụ dữ liệu từ `test.yaml`. Đây là phần việc chính còn lại để hoàn thiện hướng μE-LS.
- **Lặp code parse `.config`**: cả 6 generator (`appcfg`, `corecfg`, `palcfg`, `appdecl`, `arch_h`, `arch_c`) đều tự gọi `dotcfg.cfp_parse_dotcfg(config_dir)` riêng lẻ thay vì parse một lần rồi truyền chung `context` — có thể gộp lại trong `tsgen.py` để tránh đọc file `.config` nhiều lần.
- **`entrypoint.sh` luôn chạy `uedp.py menuconfig` ở mỗi lần container khởi động**: phù hợp cho phiên làm việc tương tác trên máy dev, nhưng chưa có nhánh non-interactive (ví dụ đọc thẳng `.config` có sẵn, bỏ qua menuconfig) để dùng trong CI/CD.
- **Chưa có test tự động cho chính `pltf/`**: bản thân testing framework (parser, generator, template) hiện chưa có test riêng để đảm bảo không hồi quy khi sửa template hay parser.
- **`archdirpgen.py`** dùng cú pháp f-string lồng dấu ngoặc kép kiểu Python 3.12+ (`f"{context["arch_name"]}"`) — cần xác nhận tương thích khi image build với `python:3.13-slim` (khớp) nhưng cần lưu ý nếu sau này đổi base image xuống bản Python cũ hơn.

## 7. Kết luận

PLTF không thay thế Kconfig hay Docker của KwDI, mà **tách lớp** phần sinh code ra khỏi phần thu thập cấu hình, đồng thời container hoá đầy đủ hơn (toolchain build + ESP-IDF + user permission handling) để container không chỉ dùng để chạy `menuconfig` một lần mà có thể dùng làm môi trường phát triển và kiểm thử μEDP xuyên suốt. Phần còn thiếu lớn nhất trước khi PLTF có thể coi là hoàn thiện đúng như mô tả trong `docs/to-do.md` là tích hợp `glbda.py`/`test.yaml` (hướng μE-LS) vào pipeline `tsgen.py`, hiện vẫn đang ở dạng bản nháp độc lập.
