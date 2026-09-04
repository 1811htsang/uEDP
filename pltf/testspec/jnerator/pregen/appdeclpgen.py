import os

from jinja2 import Environment, FileSystemLoader


def main(context):
  cur_trm_dir = os.path.dirname("uEDP")
  env = Environment(loader = FileSystemLoader('./pltf/templates'))
  template = env.get_template('appdeclh.txt')
  output = template.render(
    current_date = context["current_date"],
    tasknorm_defs = context['tasknorm_defs'], 
    taskpoll_defs = context['taskpoll_defs'],
    sig_defs = context['sig_defs'],
    msgq_defs = context['msgq_defs'],
    normhler_lists = context['normhler_lists'], 
    pollhler_lists = context['pollhler_lists'],
    tsmio_lists = context['tsmio_lists'],
    fsmio_lists = context['fsmio_lists']
  )
  # For debug
    # print(output)
  # Create file
  output_dir = os.path.join(cur_trm_dir, "sources", "app", "declaration")
  with open(output_dir + "/app_decl.h", "w", encoding="utf-8") as f:
    f.write(output)