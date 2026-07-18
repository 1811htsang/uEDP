from jinja2 import Environment, FileSystemLoader

env = Environment(loader = FileSystemLoader('./sources/common/testspec/templates'))
template = env.get_template('corecfg_tmpl.txt')

output = template.render(
  core_configs = ['EXIT 10', 'MSG 100'] 
)
print(output)