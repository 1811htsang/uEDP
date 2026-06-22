# [1] Add lib
import os
import sys

# [2] Config specifier
current_dir = os.path.dirname(os.path.abspath(__file__))
kconfig_dir = os.path.join(current_dir, "sources", "common", "kconfiglib")
pyspec_dir = os.path.join(current_dir, "sources", "common", "pyspec")

# [3] Python inserter
sys.path.insert(0, kconfig_dir)
sys.path.insert(0, pyspec_dir)

# [4] Import kconfiglib và menuconfig after add to sys.path
from sources.common.kconfiglib import kconfiglib
from sources.common.kconfiglib import menuconfig
import argparse

# [5] Define marker for app_cfg.h
KCONFIG_DECL_TASK_NORM_START = "// KCONFIG_DECL_TASK_NORM_START"
KCONFIG_DECL_TASK_NORM_END = "// KCONFIG_DECL_TASK_NORM_END"

KCONFIG_DECL_TASK_POLL_START = "// KCONFIG_DECL_TASK_POLL_START"
KCONFIG_DECL_TASK_POLL_END = "// KCONFIG_DECL_TASK_POLL_END"

KCONFIG_DECL_SIGNAL_START = "// KCONFIG_DECL_SIGNAL_START"
KCONFIG_DECL_SIGNAL_END = "// KCONFIG_DECL_SIGNAL_END"

KCONFIG_DECL_MSG_QUEUE_START = "// KCONFIG_DECL_MSG_QUEUE_START"
KCONFIG_DECL_MSG_QUEUE_END = "// KCONFIG_DECL_MSG_QUEUE_END"

# Function to generate app_cfg.h declaration based on Kconfig configuration
def app_cfg_gen(kconf, target):
  # 1. Find the target file
  if not os.path.exists(target):
    print(f"[ERROR] File not found at sources/app/config: {target}")
    return False

  # 2. Generate appcfg content based on Kconfig configuration
  appcfg_kconfig_lines = []
  
  # 3. Define indentation (here using 2 spaces, can be changed to "\t" if preferred)
  indent = "\t" 

  # 4. Define prefix map
  prefix_map = {
    "APPCFG_" : "app_cfg.h"
  }

  for sym in kconf.unique_defined_syms:
    # Ignore blank or undefined symbols
    if sym.config_string == "" or sym.name.startswith("_"):
      continue

    line_content = ""

    if sym.name == "UEDP_MSG_ALLOC_N_VALUE":
      n_val = sym.str_value  # Lấy ra chuỗi số (ví dụ: "8")
      kconfig_lines.append(f"{indent}#define UEDP_MSG_ALLOC_DATA_MAX (sizeof(void*) * {n_val}u)")
      continue # Bỏ qua logic xử lý mặc định bên dưới để không bị trùng lặp
    elif sym.name == "UEDP_MSG_EXTAL_N_VALUE":
      n_val = sym.str_value  # Lấy ra chuỗi số (ví dụ: "8")
      kconfig_lines.append(f"{indent}#define UEDP_MSG_EXTAL_DATA_MAX (sizeof(void*) * {n_val}u)")
      continue # Bỏ qua logic xử lý mặc định bên dưới để không bị trùng lặp
        
    # Biến đổi dòng cấu hình từ dạng 'CONFIG_ABC=y' sang '#define CONFIG_ABC 1' hoặc chuỗi/số tương ứng
    if sym.type == kconfiglib.BOOL or sym.type == kconfiglib.TRISTATE:
      if sym.str_value == "y":
        kconfig_lines.append(f"{indent}#define {sym.name} 1")
      # Nếu bằng 'n' thì thường trong C sẽ omit (không định nghĩa) hoặc #define 0 tùy bạn chọn
    elif sym.type == kconfiglib.INT or sym.type == kconfiglib.HEX:
      kconfig_lines.append(f"{indent}#define {sym.name} ({sym.str_value}u)")
    elif sym.type == kconfiglib.STRING:
      kconfig_lines.append(f"{indent}#define {sym.name} \"{sym.str_value}\"")

  kconfig_content = "\n".join(kconfig_lines)

  # 2. Đọc file file header hiện tại
  with open(target, "r", encoding="utf-8") as f:
    file_content = f.read()

  # 3. Tìm vị trí cặp thẻ Anchor để thay thế nội dung ở giữa
  start_marker = "// KCONFIG_CORECFG_START"
  end_marker = "// KCONFIG_CORECFG_END"

  if start_marker not in file_content or end_marker not in file_content:
    print(f"[ERROR] Không tìm thấy cặp thẻ đánh dấu {start_marker} trong file {target}!")
    return False

  # Tách file làm 3 phần: Trước marker, nội dung kconfig mới, Sau marker
  parts = file_content.split(start_marker)
  before_part = parts[0] + start_marker
  after_part = parts[1].split(end_marker)[1]
  
  # Ghép lại thành nội dung file mới hoàn chỉnh
  new_file_content = f"{before_part}\n{kconfig_content}\n{indent}{end_marker}{after_part}"

  # 4. Ghi đè lại vào file
  with open(target, "w", encoding="utf-8") as f:
    f.write(new_file_content)
      
  return True