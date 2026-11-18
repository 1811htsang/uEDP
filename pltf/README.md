# PLTF

PLTF (Portable Local Test Framework) là framework kiểm thử nhiều cấp độ (UT/CT/IT/ST) cho μE-(DP)/OS. Project chuyển đổi cấu hình và kịch bản YAML thành mã C được sinh bằng Jinja2, đồng thời hỗ trợ tạo test runner cho libUnity.

## Chức năng chính

- Validate YAML logic bằng `pycdscriptor.lstaxer.vlid`.
- Đọc cấu hình Kconfig và sinh các declaration/configuration header, source C và thư mục kiến trúc.
- Sinh `app.c` từ YAML bằng post-generation pipeline.
- Sinh test runner libUnity từ phần `tezuit` trong YAML, hỗ trợ level `ut` và `ct`.
- Xuất dữ liệu UST và xử lý alias trước khi validate/generate.

## Yêu cầu

- Python 3.10 trở lên.
- `pip`.
- Docker/ESP-IDF chỉ cần khi chạy workflow tích hợp tương ứng; chúng không được cấu hình trong repository này.

## Cài đặt

Từ thư mục gốc của repository:

```bash
python3 -m venv .venv
source .venv/bin/activate
python -m pip install -r requirements.txt
```

Repository hiện không có `pyproject.toml` hoặc package metadata, vì vậy các lệnh bên dưới được chạy trực tiếp từ thư mục gốc.

## Cấu trúc mã nguồn

```asciidoc
pltf/                                                                                 
 │                                                                                    
 ├── uedp.py                         # Tạo cấu hình bằng Kconfig/menuconfig           
 ├── jainerator.py                   # Validate và sinh app.c                         
 ├── ntrypont.py                     # Entry point cho các pipeline IT/test/docker    
 ├── kconfigspec/                    # Sinh declaration Kconfig từ lựa chọn người dùng
 ├── pycdscriptor/lstaxer/           # Load, resolve và validate YAML                 
 ├── pycdscriptor/jnerator/pregen/   # Sinh mã từ .config                             
 ├── pycdscriptor/jnerator/postgen/  # Sinh app.c và test runner                      
 ├── pycdscriptor/ustab/             # Xử lý và export UST                            
 └── templates/                      # Template C/header dùng bởi generator           
```

## Workflow chính

### 1. Tạo hoặc cập nhật cấu hình Kconfig

`uedp.py` mở menuconfig, ghi cấu hình vào `.config` và tạo declaration động dưới `sources/app/kconfig/`:

```bash
python uedp.py
```

Lưu ý: lệnh này yêu cầu 1 số file `Kconfig` và cấu trúc thư mục `sources/` của project đầu vào.

### 2. Chạy pipeline

`ntrypont.py` cung cấp các chế độ sau:

```bash
python ntrypont.py --it      # Có mở menuconfig
python ntrypont.py --n-it    # Không mở menuconfig, dùng .config hiện có
python ntrypont.py --test    # Pipeline + validate + sinh app.c và test runner
python ntrypont.py --docker  # Thiết lập user/quyền và mở shell trong môi trường Docker
```

Pipeline sinh mã từ `.config`, sau đó chạy các bước export/anchor UST. Chế độ `--test` dùng các đường dẫn mặc định:

- YAML: `sources/app/tezprjec.yaml`
- App output: `sources/app/app.c`
- Test output: `sources/test/tezprjec/tez.c`

### 3. Validate và sinh `app.c`

Để chạy riêng post-generation pipeline:

```bash
python jainerator.py \
  --yaml sources/app/lstaxizer.yaml \
  --output sources/app/app.c
```

Lệnh trên chạy validator trước, sau đó gọi `pycdscriptor.jnerator.postgen.cgen`.

Có thể gọi từng module trực tiếp:

```bash
python -m pycdscriptor.lstaxer.vlid
python -m pycdscriptor.jnerator.postgen.cgen \
  --yaml sources/app/lstaxizer.yaml \
  --output sources/app/app.c
```

Validator hiện đọc cố định `sources/app/lstaxizer.yaml`, vì vậy file này phải tồn tại trước khi chạy `vlid` hoặc `jainerator.py`.

### 4. Sinh test runner libUnity

```bash
python -m pycdscriptor.jnerator.postgen.testgen \
  --yaml sources/app/tezprjec.yaml \
  --output sources/test/tezprjec/tez.c
```

Tùy chọn `--level` giới hạn test theo level:

```bash
python -m pycdscriptor.jnerator.postgen.testgen \
  --yaml sources/app/tezprjec.yaml \
  --output sources/test/tezprjec/ut.c \
  --level ut
```

Input cho test runner cần có key `tezuit`. Mỗi scenario cần key `tezk`; `level` chỉ nhận `ut` hoặc `ct`, mặc định là `ct`. Các bước được đọc từ `teztep`, còn assertion được đọc từ `xpect`.

## Kiểm tra nhanh

Repository hiện chưa chứa test tự động. Có thể kiểm tra syntax Python bằng:

```bash
python -m compileall kconfigspec pycdscriptor jainerator.py ntrypont.py uedp.py
```

Để kiểm tra đầy đủ pipeline, cần đặt repository trong một project uEDP có đủ `Kconfig`, `.config`, YAML và các thư mục `sources/` mà các generator yêu cầu.

## Lưu ý về đường dẫn

Các generator sử dụng đường dẫn tương đối so với thư mục chạy lệnh. Hãy chạy command từ thư mục gốc của project. Những file được sinh như `.config`, `sources/app/app.c`, các header C và `sources/test/tezprjec/tez.c` không nên chỉnh sửa thủ công nếu chúng được tạo lại bởi pipeline.
