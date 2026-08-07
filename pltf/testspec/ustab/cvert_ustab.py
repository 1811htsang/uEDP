import os
import pprint
import yaml
import sys
from ..cfparsers.dotcfg_cfp import parse_config

def convert_yaml():
  print("[INFO] testspec.ust_cfp is called")
  print("[INFO] generating context from .config")
  cur_trm_dir = os.path.dirname("uEDP")
  config_dir = os.path.join(cur_trm_dir, ".config")
  context = parse_config(config_dir)
  print("[INFO] context is generated, concentrate to yaml file")
  # NOTE - 1. Chuyển toàn bộ context thành chuỗi YAML trong bộ nhớ
  yaml_string = yaml.dump(context, default_flow_style=False)
  # NOTE - 2. Thay thế chuỗi "- - " thành "- "
  # Lưu ý: replace 2 lần nếu có dấu cách dư thừa hoặc thụt lề
  fixed_yaml = yaml_string.replace("- - ", "- ")
  # NOTE - 3. Ghi một lần duy nhất vào file
  with open("config.yaml", "w", encoding='utf-8') as f:
    f.write(fixed_yaml)

  return fixed_yaml

# if __name__ == "__main__":
#   convert_yaml()