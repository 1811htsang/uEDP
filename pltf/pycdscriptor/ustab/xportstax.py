import os
# NOTE - hardcode path, do not change it, because the script 
# will be executed in the root directory of the project
def ustab_xport():
  # NOTE - kiểm tra xem có file config.yaml đã tồn tại chưa, nếu chưa thì báo lỗi
  if not os.path.exists("config.yaml"):
    raise FileNotFoundError("[ERROR] File config.yaml not found. Please run the script to generate it first.")
  # NOTE - kiểm tra xem file sources/app/lstaxizer.yaml có tồn tại chưa, nếu chưa thì warning và tạo một file trống
  if not os.path.exists("sources/app/lstaxizer.yaml"):
    print("[WARNING] File sources/app/lstaxizer.yaml not found. Creating an empty file.")
    os.makedirs("sources/app", exist_ok=True)
    with open("sources/app/lstaxizer.yaml", "w") as f:
      f.write("")
  # NOTE - import toàn bộ nội dung của config.yaml vào sources/app/lstaxizer.yaml
  with open("config.yaml", "r") as f:
    config_content = f.read()
  with open("sources/app/lstaxizer.yaml", "w") as f:
    f.write(config_content)