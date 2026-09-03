import os

from jinja2 import Environment, FileSystemLoader


def main(context):
  cur_trm_dir = os.path.dirname("uEDP")
  env = Environment(loader = FileSystemLoader('./pltf/templates'))
  template = env.get_template('appcfg_tmpl.txt')
  output = template.render(
    current_date = context["current_date"], 
    appcfg_libs = context["appcfg_libs"], 
    appcfg_tsm_state_trans = context["appcfg_tsm_state_trans"],
    appcfg_tsm_tables = context["appcfg_tsm_tables"],
    appcfg_tsm_objects = context["appcfg_tsm_objects"],
    appcfg_fsm_objects = context["appcfg_fsm_objects"]
  )
  # For debug
    # print(output)
  # Create file
  output_dir = os.path.join(cur_trm_dir, "sources", "app", "config")
  with open(output_dir + "/app_cfg.h", "w", encoding="utf-8") as f:
    f.write(output)
