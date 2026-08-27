from jinja2 import Environment, FileSystemLoader
from . import cfpcall_tsgen
from ..cfparsers import yaml_cfp
import os
import yaml

# NOTE - Add generators for GDA as requested

"""
In template, GDA is temporal set as
  {% for item in glbda_defs %}
  {%- set is_string = 'char' in item.type -%}
  {{item.type}} {{ item.name }} = {% if is_string %}"{{ item.initial_value }}"{% else %}{{ item.initial_value }}{% endif %};
  {% endfor %}
So the generator should be like this:
  glbda_defs = context["glbda_defs"]
with context["glbda_defs"] contains a dict of {"name": "gda_name", "type": "gda_type", "initial_value": "gda_initial_value"}
for example, if the config file has:
  CONFIG_GDA_1_NAME="gda1"
  CONFIG_GDA_1_TYPE="int"
  CONFIG_GDA_1_INITIAL_VALUE=10
  CONFIG_GDA_2_NAME="gda2"
The generator should produce:
  int gda1 = 10;
while the context["glbda_defs"] will be:
  context["glbda_defs"] = [
    {"name": "gda1", "type": "int", "initial_value":10}
  ]
As this step is post definition of logic in lstaxizer.yaml
Therefore, cfparsers is also in charged of generating the context["glbda_defs"] from the lstaxizer.yaml file, 
and the template will be rendered with the context["glbda_defs"] to produce the GDA definitions in the output C file.
"""

# STUB - Called sample to check the output of the template rendering
cur_trm_dir = os.path.dirname("uEDP")
env = Environment(loader = FileSystemLoader('./pltf/templates'))
template = env.get_template('app_c_tmpl.txt')
context = cfpcall_tsgen.main()
with open('sources/app/lstaxizer.yaml', 'r') as file:
  data = yaml.safe_load(file)
gda_items = yaml_cfp.cfp_parse_yaml(data)
output = template.render(
  msgq_defs = context["msgq_defs"],
  glbda_defs = gda_items,
  tsmobj_defs = context["appcfg_tsm_objects"],
  tsmio_lists = context["tsmio_lists"],
  fsmobj_defs = context["appcfg_fsm_objects"],
  appcfg_tsm_state_trans = context["appcfg_tsm_state_trans"],
  appcfg_tsm_tables = context["appcfg_tsm_tables"],
  normhler_lists = context["normhler_lists"],
  pollhler_lists = context["pollhler_lists"],
  fsmio_lists = context["fsmio_lists"]
)
output_dir = os.path.join(cur_trm_dir, "sources", "app")
with open(output_dir + "/app.c", "w", encoding="utf-8") as f:
  f.write(output)
