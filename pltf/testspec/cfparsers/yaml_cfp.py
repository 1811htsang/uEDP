import yaml
from pprint import PrettyPrinter

with open('pltf/testspec/cfparsers/test.yaml', 'r') as file:
  data = yaml.safe_load(file)
pp = PrettyPrinter(
  indent=2,      # Number of spaces for indentation
  width=60,      # Max characters per line before wrapping
  depth=None,    # Limit nesting depth (None = no limit)
  sort_dicts=False  # Sort dictionary keys alphabetically
)
print("\nTask USR info:")
pp.pprint(data['applg'][2])