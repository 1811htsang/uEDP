import os
import pprint
import yaml
import sys
from ..cfparsers.dotcfg_cfp import cfp_parse_dotcfg

def ustab_convert_yaml(context):
  # NOTE - 1. Chuyển toàn bộ context thành chuỗi YAML trong bộ nhớ
  yaml_string = yaml.dump(context, default_flow_style=False)
  # NOTE - 2. Thay thế chuỗi "- - " thành "- "
  # Lưu ý: replace 2 lần nếu có dấu cách dư thừa hoặc thụt lề
  true_yaml = yaml_string.replace("- - ", "- ")
  # NOTE - Bổ sung thêm các dòng trống ở cuối kèm hướng dẫn anchor
  true_yaml += """
# Anchor: sử dụng &<str> để định danh các phần tử 
# >> sigs: &siglst
# >>  '1', '2' of sigs: &sig1, &sig2, ...
# >> tnorms: &tnormlst
# >>  '1', '2' of tnorms: &tnorm1, &tnorm2, ...
# >>  fsm of '1', '2': &tnorm1-fsmobj, &tnorm2-fsmobj, ...
# >>    states of '1'[fsm], '2'[fsm]: &tnorm1-fsmstates, &tnorm2-fsmstates, ...
# >>  tsm of '1', '2': &tnorm1-tsmobj, &tnorm2-tsmobj, ...
# >>    states of '1'[tsm], '2'[tsm]: &tnorm1-tsmstates, &tnorm2-tsmstates, ...
# >>    state trans of '1', '2': &tnorm1-tsmtrans, &tnorm2-tsmtrans, ...
# >> tpolls: &tpolllst
# >>  '1', '2' of tpolls: &tpoll1, &tpoll2, ...
# Alias: sử dụng *<str> để tham chiếu đến các phần tử đã định danh

# NOTE - Triển khai logic bắt đầu từ dòng này
"""
  # NOTE - 3. Ghi một lần duy nhất vào file
  with open("config.yaml", "w", encoding='utf-8') as f:
    f.write(true_yaml)
  return true_yaml

# if __name__ == "__main__":
#   ustab_convert_yaml()