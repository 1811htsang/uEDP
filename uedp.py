# [1] Add lib
import os
import sys

# [2] Config specifier
current_dir = os.path.dirname(os.path.abspath(__file__))
kconfig_dir = os.path.join(current_dir, "sources", "common", "kconfiglib")
pyspec_dir = os.path.join(current_dir, "sources", "common", "pyspec")

# [3] Python inserter
sys.path.insert(0, kconfig_dir)
sys.path.insert(1, pyspec_dir)

# [4] Import kconfiglib và menuconfig after add to sys.path
import kconfiglib
import menuconfig
import argparse

# [5] Import user input function from pyspec
from sources.common.pyspec.usrinp import user_input
from sources.common.pyspec.tsknrmdcl import task_norm_declaration
from sources.common.pyspec.tskpoldcl import task_poll_declaration
from sources.common.pyspec.sigdcl import signal_declaration
from sources.common.pyspec.appcfgpen import app_cfg_gen
from sources.common.pyspec.appdeclgen import msg_queue_gen, sig_gen, tsk_norm_handler_gen, tsk_norm_gen, tsk_poll_gen, tsk_poll_handler_gen

# [6] Global variables to hold user input values (if needed)
DEFAULT_VALS = {
  "num_tasks_norm": 8,
  "num_tasks_poll": 8,
  "num_signals": 10,
  "is_use_fsm": False,
  "is_use_tsm": False,
  "num_tsm_states": 0,
  "num_fsm_states": 0
}

def update_core_cfg_header(kconf, header_path):
  """Hàm đọc cấu hình từ Kconfig và chèn vào vị trí chỉ định trong file .h"""
  if not os.path.exists(header_path):
    print(f"[ERROR] Không tìm thấy file gốc tại: {header_path}")
    return False

  # 1. Tạo chuỗi nội dung cấu hình từ Kconfig với format của C
  # kconfiglib hỗ trợ duyệt qua các symbol đang hoạt động
  kconfig_lines = []
  
  # Định nghĩa ký tự thụt lề (ở đây là 2 khoảng trắng - hoặc thay bằng "\t" nếu bạn muốn)
  indent = "  " 

  for sym in kconf.unique_defined_syms:
    # Bỏ qua các symbol không được chọn hoặc là kiểu không xác định
    if sym.config_string == "":
      continue

    # Bỏ qua các symbol có tên bắt đầu bằng "_" (thường là các symbol nội bộ hoặc không cần thiết)
    if sym.name.startswith("_"):
      continue

    if sym.name == "CORE_UEDP_MSG_ALLOC_N_VALUE":
      n_val = sym.str_value  # Lấy ra chuỗi số (ví dụ: "8")
      kconfig_lines.append(f"{indent}#define UEDP_MSG_ALLOC_DATA_MAX (sizeof(void*) * {n_val}u)")
      continue # Bỏ qua logic xử lý mặc định bên dưới để không bị trùng lặp
    elif sym.name == "CORE_UEDP_MSG_EXTAL_N_VALUE":
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
  with open(header_path, "r", encoding="utf-8") as f:
    file_content = f.read()

  # 3. Tìm vị trí cặp thẻ Anchor để thay thế nội dung ở giữa
  start_marker = "// KCONFIG_CORECFG_START"
  end_marker = "// KCONFIG_CORECFG_END"

  if start_marker not in file_content or end_marker not in file_content:
    print(f"[ERROR] Không tìm thấy cặp thẻ đánh dấu {start_marker} trong file {header_path}!")
    return False

  # Tách file làm 3 phần: Trước marker, nội dung kconfig mới, Sau marker
  parts = file_content.split(start_marker)
  before_part = parts[0] + start_marker
  after_part = parts[1].split(end_marker)[1]
  
  # Ghép lại thành nội dung file mới hoàn chỉnh
  new_file_content = f"{before_part}\n{kconfig_content}\n{indent}{end_marker}{after_part}"

  # 4. Ghi đè lại vào file
  with open(header_path, "w", encoding="utf-8") as f:
    f.write(new_file_content)
      
  return True

def main():
  os.environ["KCONFIG_CONFIG"] = ".config"
  
  # LẤY GIÁ TRỊ TỪ HÀM NHẬP
  (n_norm, n_poll, n_sig, use_fsm, use_tsm, n_tsm_st, n_fsm_st) = user_input(DEFAULT_VALS)

  # Tạo file mới (ghi đè "w" lần đầu để xóa nội dung cũ)
  # Sau đó các hàm tiếp theo dùng "a" để ghi tiếp vào
  # Ở đây tôi sửa lại logic ghi file để an toàn hơn:
  
  # 1. Reset file
  open("sources/app/kconfig/decl.kconfig", "w").close()

  # 2. Gọi các hàm tạo với giá trị mới
  task_norm_declaration(n_norm, n_tsm_st, n_fsm_st)
  task_poll_declaration(n_poll)
  signal_declaration(n_sig)

  # Khởi tạo kconfiglib
  kconf = kconfiglib.Kconfig("Kconfig")
  if os.path.exists(".config"):
    kconf.load_config(".config")

  menuconfig.menuconfig(kconf)
  kconf.write_config(".config")
  
  corecfg_target = os.path.join("sources", "app", "config", "core_cfg.h")
  if update_core_cfg_header(kconf, corecfg_target):
    print(f"\n[SUCCESS] Cấu hình đã được chèn thành công vào {corecfg_target}!")

  appcfg_target = os.path.join("sources", "app", "config", "app_cfg.h")
  if app_cfg_gen(kconf, appcfg_target):
    print(f"\n[SUCCESS] Cấu hình FSM/TSM đã được chèn thành công vào {appcfg_target}!")

  appdecl_tsk_norm_target = os.path.join("sources", "app", "declaration", "app_decl.h")
  if tsk_norm_gen(kconf, appdecl_tsk_norm_target):
    print(f"\n[SUCCESS] Định nghĩa task norm đã được chèn thành công vào {appdecl_tsk_norm_target}!")

  appdecl_tsk_poll_target = os.path.join("sources", "app", "declaration", "app_decl.h")
  if tsk_poll_gen(kconf, appdecl_tsk_poll_target):
    print(f"\n[SUCCESS] Định nghĩa task poll đã được chèn thành công vào {appdecl_tsk_poll_target}!")

  appdecl_sig_target = os.path.join("sources", "app", "declaration", "app_decl.h")
  if sig_gen(kconf, appdecl_sig_target):
    print(f"\n[SUCCESS] Định nghĩa signal đã được chèn thành công vào {appdecl_sig_target}!")

  appdecl_msg_queue_target = os.path.join("sources", "app", "declaration", "app_decl.h")
  if msg_queue_gen(kconf, appdecl_msg_queue_target):
    print(f"\n[SUCCESS] Định nghĩa message queue đã được chèn thành công vào {appdecl_msg_queue_target}!")

  appdecl_tsk_norm_handler_target = os.path.join("sources", "app", "declaration", "app_decl.h")
  if tsk_norm_handler_gen(kconf, appdecl_tsk_norm_handler_target):
    print(f"\n[SUCCESS] Định nghĩa task norm handler đã được chèn thành công vào {appdecl_tsk_norm_handler_target}!")

  appdecl_tsk_poll_handler_target = os.path.join("sources", "app", "declaration", "app_decl.h")
  if tsk_poll_handler_gen(kconf, appdecl_tsk_poll_handler_target):
    print(f"\n[SUCCESS] Định nghĩa task poll handler đã được chèn thành công vào {appdecl_tsk_poll_handler_target}!")

if __name__ == "__main__":
  main()