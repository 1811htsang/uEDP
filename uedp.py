import os
import sys

# Tự động tìm và thêm đường dẫn của thư mục sources/common/kconfiglib vào Python path
current_dir = os.path.dirname(os.path.abspath(__file__))
kconfig_dir = os.path.join(current_dir, "sources", "common", "kconfiglib")
sys.path.insert(0, kconfig_dir)

# Bây giờ bạn có thể import thoải mái mà không sợ máy người dùng thiếu thư viện
import kconfiglib
import menuconfig
import argparse

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
  # Giả lập xử lý đối số menuconfig giống câu trước
  os.environ["KCONFIG_CONFIG"] = ".config"
  
  # Khởi tạo kconfig
  kconf = kconfiglib.Kconfig("Kconfig")
  
  # Nếu có file .config cũ thì load lên trước
  if os.path.exists(".config"):
    kconf.load_config(".config")

  # Mở giao diện menuconfig cho người dùng chỉnh sửa
  menuconfig.menuconfig(kconf)
  
  # Sau khi người dùng Lưu và Thoát:
  kconf.write_config(".config") # Vẫn lưu .config để giữ trạng thái cho lần sau
  
  # Gọi hàm tự chế để ghi vào đúng vị trí trong core_cfg.h
  header_target = os.path.join("sources", "app", "config", "core_cfg.h")
  if update_core_cfg_header(kconf, header_target):
    print(f"\n[SUCCESS] Cấu hình đã được chèn thành công vào {header_target}!")

if __name__ == "__main__":
  main()