import os

from jinja2 import Environment, FileSystemLoader


def main(context):
  cur_trm_dir = os.path.dirname("uEDP")
  env = Environment(loader = FileSystemLoader('./pltf/templates'))
  template = env.get_template('archc.txt')
  output = template.render(
    current_date = context["current_date"],
    arch_name = context['arch_name'],
    arch_apis = context['arch_apis']
  )
  # For debug
    # print(output)
  # Create file
  output_dir = os.path.join(cur_trm_dir, "sources", "pal", "arch", context['arch_name'])
  with open(output_dir + "/" + context['arch_name'] + "_arch.c", "w", encoding="utf-8") as f:
    f.write(output)