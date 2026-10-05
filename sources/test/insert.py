import os
import sys
import argparse

def insert_test_object():
  # 1. Cấu hình đối số
  parser = argparse.ArgumentParser(description="Insert test object into lstaxizer.yaml")
  parser.add_argument("target", help="The name of the test object file (without .yaml extension)")
  args = parser.parse_args()

  target_name = args.target
  source_obj_path = f"./testobj/{target_name}.yaml"
  output_yaml_path = "../app/lstaxizer.yaml"

  # 2. Kiểm tra thư mục hiện tại
  if not os.path.isdir("./testobj"):
    print("[NOTICE] Please run this script inside /sources/test/, not in root directory of project.")
    sys.exit(1)

  print(f"[NOTICE] target: {target_name}")

  # 3. Kiểm tra file output
  if not os.path.exists(output_yaml_path) or os.path.getsize(output_yaml_path) == 0:
    print(f"[ERROR] {output_yaml_path} is empty or missing.")
    print("\tPlease generate logic-resc using ./entrypoint.py before running this script.")
    sys.exit(1)

  # 4. Kiểm tra file nguồn
  if not os.path.exists(source_obj_path):
    print(f"[ERROR] Source file {source_obj_path} does not exist.")
    sys.exit(1)

  # 5. Thực hiện append
  try:
    with open(source_obj_path, 'r', encoding='utf-8') as src:
      content = src.read()
      
    with open(output_yaml_path, 'a', encoding='utf-8') as dst:
      # Đảm bảo có dòng trống giữa file cũ và nội dung mới nếu cần
      dst.write("\n") 
      dst.write(content)
      
    print(f"[NOTICE] Successfully appended ./testobj/{target_name}.yaml to {output_yaml_path}.")
    
  except Exception as e:
    print(f"[ERROR] Failed to insert content: {e}")
    sys.exit(1)

if __name__ == "__main__":
  insert_test_object()