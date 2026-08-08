import os
import pprint
import yaml
import sys
from ..cfparsers.dotcfg_cfp import parse_config

def ustab_convert_yaml(context):
  # NOTE - 1. Chuyển toàn bộ context thành chuỗi YAML trong bộ nhớ
  yaml_string = yaml.dump(context, default_flow_style=False)
  # NOTE - 2. Thay thế chuỗi "- - " thành "- "
  # Lưu ý: replace 2 lần nếu có dấu cách dư thừa hoặc thụt lề
  true_yaml = yaml_string.replace("- - ", "- ")
  # NOTE - 3. Ghi một lần duy nhất vào file
  with open("config.yaml", "w", encoding='utf-8') as f:
    f.write(true_yaml)
  return true_yaml

# if __name__ == "__main__":
#   ustab_convert_yaml()