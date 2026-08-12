import yaml
from pprint import PrettyPrinter

with open('sources/app/lstaxizer.yaml', 'r') as file:
  data = yaml.safe_load(file)
pp = PrettyPrinter(
  indent=2,       # Number of spaces for indentation
  width=60,       # Max characters per line before wrapping
  depth=None,     # Limit nesting depth (None = no limit)
  sort_dicts=False # Sort dictionary keys alphabetically
)
pp.pprint(data)