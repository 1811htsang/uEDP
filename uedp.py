# Add lib
import os
import sys

# Find Kconfiglib path relative to this script and add it to sys.path
current_dir = os.path.dirname(os.path.abspath(__file__))
kconfig_dir = os.path.join(current_dir, "sources", "common", "kconfiglib")
sys.path.insert(0, kconfig_dir)

# Import kconfiglib và menuconfig after add to sys.path
import kconfiglib
import menuconfig
import argparse

# Global variables to hold user input values (if needed)
DEFAULT_VALS = {
  "num_tasks_norm": 8,
  "num_tasks_poll": 8,
  "num_signals": 10,
  "is_use_fsm": False,
  "is_use_tsm": False,
  "num_tsm_states": 0,
  "num_fsm_states": 0
}

# Add pipeline for user input to gen tasks, signal handling, etc.
def user_input():
  # Number of tasks to generate
  print(f'[INFO] Number of tasks norm to generate (default: {DEFAULT_VALS["num_tasks_norm"]}): ', end='')
  val = input().strip()
  num_tasks_norm = int(val) if val != '' else DEFAULT_VALS["num_tasks_norm"]

  # Number of tasks poll to generate
  print(f'[INFO] Number of tasks poll to generate (default: {DEFAULT_VALS["num_tasks_poll"]}): ', end='')
  val = input().strip()
  num_tasks_poll = int(val) if val != '' else DEFAULT_VALS["num_tasks_poll"]

  # Number of signals to generate
  print(f'[INFO] Number of signals to generate (default: {DEFAULT_VALS["num_signals"]}): ', end='')
  val = input().strip()
  num_signals = int(val) if val != '' else DEFAULT_VALS["num_signals"]

  # Ask user if they want to use FSM/TSM
  print('[INFO] Do you want to use FSM? (y/n, default: n): ', end='')
  is_use_fsm = input().strip().lower() == 'y'

  print('[INFO] Do you want to use TSM? (y/n, default: n): ', end='')
  is_use_tsm = input().strip().lower() == 'y'

  num_tsm_states = 0
  if is_use_tsm:
    print(f'[INFO] Number of TSM states to generate (default: {num_tasks_norm}): ', end='')
    val = input().strip()
    num_tsm_states = int(val) if val != '' else num_tasks_norm

  num_fsm_states = 0
  if is_use_fsm:
    print(f'[INFO] Number of FSM states to generate (default: {num_tasks_norm}): ', end='')
    val = input().strip()
    num_fsm_states = int(val) if val != '' else num_tasks_norm

  # TRẢ VỀ CÁC GIÁ TRỊ ĐÃ NHẬP
  return num_tasks_norm, num_tasks_poll, num_signals, is_use_fsm, is_use_tsm, num_tsm_states, num_fsm_states

# Function to generate blank task declarations based on user input
# Remember that task generation auto set the ID from 0xE6u, 0xE7u, ... to 0xEFu
def task_norm_declaration(num_tasks_norm, num_tsm_states, num_fsm_states):
  # Generate task declarations in Kconfig format
  kconfig_content = []
  kconfig_content.append('menu "Task Norm configuration"\n')

  for i in range(1, num_tasks_norm + 1):
    kconfig_content.append(f'\tmenu \"Task #{i} configuration\"')

    kconfig_content.append(f'\t\tconfig TASK_NORM_{i}_NAME') # Config name
    kconfig_content.append(f'\t\t\tstring "Name of task #{i}"')
    kconfig_content.append(f'\t\t\tdefault "TASK_NORM_{i}_ID"\n') 

    kconfig_content.append(f'\t\tconfig TASK_NORM_{i}_PRIO') # Config priority
    kconfig_content.append(f'\t\t\tstring "Priority of task #{i}"')
    kconfig_content.append(f'\t\t\tdefault "UEDP_TASK_PRI_LEVEL_0"')
    kconfig_content.append(f'\t\t\tdepends on TASK_NORM_{i}_NAME != ""\n')

    kconfig_content.append(f'\t\tconfig MSG_QUEUE_{i}_NAME') # Config message queue name
    kconfig_content.append(f'\t\t\tstring "Name of message queue #{i}"')
    kconfig_content.append(f'\t\t\tdefault "MSG_QUEUE_{i}_ID"')
    kconfig_content.append(f'\t\t\tdepends on TASK_NORM_{i}_NAME != ""\n')

    kconfig_content.append(f'\t\tconfig HANDLER_{i}_NAME') # Config handler name
    kconfig_content.append(f'\t\t\tstring "Name of handler #{i}"')
    kconfig_content.append(f'\t\t\tdefault "HANDLER_{i}_ID"')
    kconfig_content.append(f'\t\t\tdepends on TASK_NORM_{i}_NAME != ""\n')

    kconfig_content.append(f'\t\tconfig TASK_NORM_{i}_USE_TSM') # Config TSM flag
    kconfig_content.append(f'\t\t\tbool "Use TSM for task #{i}"')
    kconfig_content.append(f'\t\t\tdefault n')
    kconfig_content.append(f'\t\t\tdepends on TASK_NORM_{i}_NAME != ""\n')

    kconfig_content.append(f'\t\tconfig TSM_TASK_{i}') # Config TSM name
    kconfig_content.append(f'\t\t\tstring "Name of TSM task #{i}"')
    kconfig_content.append(f'\t\t\tdefault "TSM_TASK_{i}_ID"')
    kconfig_content.append(f'\t\t\tdepends on TASK_NORM_{i}_NAME != "" && TASK_NORM_{i}_USE_TSM \n')

    for j in range(1, num_tsm_states + 1):
      kconfig_content.append(f'\t\tconfig TSM_TASK_{i}_STATE_{j}') # Config TSM state name
      kconfig_content.append(f'\t\t\tstring "Name of TSM task #{i} state #{j}"')
      kconfig_content.append(f'\t\t\tdefault "TSM_TASK_{i}_STATE_{j}_ID"')
      kconfig_content.append(f'\t\t\tdepends on TASK_NORM_{i}_NAME != "" && TASK_NORM_{i}_USE_TSM \n')

    kconfig_content.append(f'\t\tconfig TASK_NORM_{i}_USE_FSM') # Config FSM flag
    kconfig_content.append(f'\t\t\tbool "Use FSM for task #{i}"')
    kconfig_content.append(f'\t\t\tdefault n')
    kconfig_content.append(f'\t\t\tdepends on TASK_NORM_{i}_NAME != ""\n')

    kconfig_content.append(f'\t\tconfig FSM_TASK_{i}') # Config FSM name
    kconfig_content.append(f'\t\t\tstring "Name of FSM task #{i}"')
    kconfig_content.append(f'\t\t\tdefault "FSM_TASK_{i}_ID"')
    kconfig_content.append(f'\t\t\tdepends on TASK_NORM_{i}_NAME != "" && TASK_NORM_{i}_USE_FSM \n')

    for j in range(1, num_fsm_states + 1):
      kconfig_content.append(f'\t\tconfig FSM_TASK_{i}_STATE_{j}') # Config FSM state name
      kconfig_content.append(f'\t\t\tstring "Name of FSM task #{i} state #{j}"')
      kconfig_content.append(f'\t\t\tdefault "FSM_TASK_{i}_STATE_{j}_ID"')
      kconfig_content.append(f'\t\t\tdepends on TASK_NORM_{i}_NAME != "" && TASK_NORM_{i}_USE_FSM \n')

    kconfig_content.append(f'\tendmenu\n')

  kconfig_content.append(f'endmenu\n')
  
  with open("sources/app/kconfig/decl.kconfig", "w", encoding="utf-8") as f:
    f.write("\n".join(kconfig_content))

def task_poll_declaration(num_tasks_poll):
  # Generate task declarations in Kconfig format
  kconfig_content = []
  kconfig_content.append('menu "Task Poll configuration"\n')

  for i in range(1, num_tasks_poll + 1):
    kconfig_content.append(f'\tmenu \"Task Poll #{i} configuration\"')

    kconfig_content.append(f'\t\tconfig TASK_POLL_{i}_NAME') # Config name
    kconfig_content.append(f'\t\t\tstring "Name of task poll #{i}"')
    kconfig_content.append(f'\t\t\tdefault "TASK_POLL_{i}_ID"\n') 

    kconfig_content.append(f'\t\tconfig TASK_POLL_{i}_ABILITY') # Config ability
    kconfig_content.append(f'\t\t\tbool "Ability of task poll #{i}"')
    kconfig_content.append(f'\t\t\tdefault "n"')
    kconfig_content.append(f'\t\t\tdepends on TASK_POLL_{i}_NAME != ""\n')

    kconfig_content.append(f'\tendmenu\n')

  kconfig_content.append(f'endmenu\n')
  
  with open("sources/app/kconfig/decl.kconfig", "a", encoding="utf-8") as f:
    f.write("\n".join(kconfig_content))

# Function to generate blank signal declarations based on user input
# Remember that signal generation auto set the value from 0x01u, 0x02u, ... to 0xFFu
def signal_declaration(num_signals):
  # Generate signal declarations in Kconfig format
  kconfig_content = []
  kconfig_content.append('menu "Signal configuration"\n')

  for i in range(1, num_signals + 1):
    kconfig_content.append(f'\tmenu \"Signal #{i} configuration\"')
    kconfig_content.append(f'\t\tconfig SIG_TSK_{i}_NAME')
    kconfig_content.append(f'\t\t\tstring "Name of signal #{i}"')
    kconfig_content.append(f'\t\t\tdefault "SIG_TSK_{i}_ID"\n')
    kconfig_content.append(f'\tendmenu\n')

  kconfig_content.append(f'endmenu\n')
  
  with open("sources/app/kconfig/decl.kconfig", "a", encoding="utf-8") as f:
    f.write("\n".join(kconfig_content))

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
  (n_norm, n_poll, n_sig, use_fsm, use_tsm, n_tsm_st, n_fsm_st) = user_input()

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
  
  header_target = os.path.join("sources", "app", "config", "core_cfg.h")
  if update_core_cfg_header(kconf, header_target):
      print(f"\n[SUCCESS] Cấu hình đã được chèn thành công vào {header_target}!")

if __name__ == "__main__":
  main()