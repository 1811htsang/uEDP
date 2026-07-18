from jinja2 import Environment, FileSystemLoader

env = Environment(loader = FileSystemLoader('./sources/common/testspec/templates'))
template = env.get_template('palcfg_tmpl.txt')

output = template.render(
  pal_configs = ['EXIT 10', 'MSG 100'] 
)
print(output)