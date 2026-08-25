from jinja2 import Environment, FileSystemLoader
import cfpcall_tsgen
import os
cur_trm_dir = os.path.dirname("uEDP")
env = Environment(loader = FileSystemLoader('./pltf/templates'))
template = env.get_template('app_c_tmpl.txt')
context = cfpcall_tsgen.main()
output = template.render(
  msgq_defs = context["msgq_defs"],
  tsmobj_defs = context["appcfg_tsm_objects"],
  tsmio_lists = context["tsmio_lists"],
  fsmobj_defs = context["appcfg_fsm_objects"],
  appcfg_tsm_state_trans = context["appcfg_tsm_state_trans"],
  appcfg_tsm_tables = context["appcfg_tsm_tables"],
  normhler_lists = context["normhler_lists"],
  pollhler_lists = context["pollhler_lists"],
  fsmio_lists = context["fsmio_lists"]
)
# print(output)
output_dir = os.path.join(cur_trm_dir, "sources", "app")
with open(output_dir + "/app.c", "w", encoding="utf-8") as f:
  f.write(output)