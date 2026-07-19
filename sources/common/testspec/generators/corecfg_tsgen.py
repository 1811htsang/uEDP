import sys
import os
from jinja2 import Environment, FileSystemLoader
cur_trm_dir = os.path.dirname("uEDP")
config_dir = os.path.join(cur_trm_dir, ".config")
cur_file_dir = os.path.dirname(os.path.abspath(__file__))
parent_file_dir = os.path.dirname(cur_file_dir)
sys.path.insert(0, config_dir)
if parent_file_dir not in sys.path:
  sys.path.append(parent_file_dir)
from cfparsers import cfigf_cps
context = cfigf_cps.parse_config(config_dir)
env = Environment(loader = FileSystemLoader('./sources/common/testspec/templates'))
template = env.get_template('corecfg_tmpl.txt')
output = template.render(
  current_date = context["current_date"],
  core_configs = context['core_configs'] 
)
# For debug
  # print(output)
# Create file
output_dir = os.path.join(cur_trm_dir, "sources", "app", "config")
with open(output_dir + "/core_cfg.h", "w", encoding="utf-8") as f:
  f.write(output)