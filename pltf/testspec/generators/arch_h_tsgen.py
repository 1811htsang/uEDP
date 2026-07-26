import sys
import os
from jinja2 import Environment, FileSystemLoader
def main():
  cur_trm_dir = os.path.dirname("uEDP")
  config_dir = os.path.join(cur_trm_dir, ".config")
  cur_file_dir = os.path.dirname(os.path.abspath(__file__))
  parent_file_dir = os.path.dirname(cur_file_dir)
  sys.path.insert(0, config_dir)
  if parent_file_dir not in sys.path:
    sys.path.append(parent_file_dir)
  from cfparsers import dotcfg_cfp
  context = dotcfg_cfp.parse_config(config_dir)
  env = Environment(loader = FileSystemLoader('./pltf/templates'))
  template = env.get_template('arch_h_tmpl.txt')
  output = template.render(
    current_date = context["current_date"],
    arch_name = context['arch_name'],
    arch_name_upperc = context['arch_name_upperc'],
    arch_apis = context['arch_apis'] 
  )
  # For debug
    # print(output)
  # Create file
  output_dir = os.path.join(cur_trm_dir, "sources", "pal", "arch", context['arch_name'])
  with open(output_dir + "/" + context['arch_name'] + "_arch.h", "w", encoding="utf-8") as f:
    f.write(output)