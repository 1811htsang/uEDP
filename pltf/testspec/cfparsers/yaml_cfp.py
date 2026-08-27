import yaml
from pprint import PrettyPrinter

def cfp_parse_yaml(context):
  return context["glbda"]

# STUB - Called sample to check the output of the cfp_parse_dotcfg function
# with open('sources/app/lstaxizer.yaml', 'r') as file:
#   data = yaml.safe_load(file)
# pp = PrettyPrinter(
#   indent=0,       # Number of spaces for indentation
#   width=60,       # Max characters per line before wrapping
#   depth=None,     # Limit nesting depth (None = no limit)
#   sort_dicts=False # Sort dictionary keys alphabetically
# )
# # NOTE - Extract glbda
# pp.pprint(data["glbda"])
# NOTE - Redirect to return context["glbda_defs"] in cfparsers/yaml_cfp.py
# Return data["glbda"] ? It seems like fitting the required structure for context["glbda_defs"] in the template rendering.
# return data["glbda"]