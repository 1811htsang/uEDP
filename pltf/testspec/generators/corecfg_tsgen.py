import os

from jinja2 import Environment, FileSystemLoader


def main(context):
  cur_trm_dir = os.path.dirname("uEDP")
  env = Environment(loader = FileSystemLoader('./pltf/templates'))
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