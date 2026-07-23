import yaml
import pprint

with open('pltf/testspec/cfparsers/test.yaml', 'r') as file:
  data = yaml.safe_load(file)

print("Data read from 'test.yaml':")
pprint.pprint(data)
print("\nTask USR info:")
pprint.pprint(data['application_logic'][0])