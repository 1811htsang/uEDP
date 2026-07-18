from jinja2 import Environment, FileSystemLoader

env = Environment(loader = FileSystemLoader('./sources/common/testspec/templates'))
template = env.get_template('arch_h_tmpl.txt')
arch_name = 'stm32h723'

output = template.render(
  arch_name = arch_name,
  arch_name_upperc = arch_name.upper(),
  arch_apis = ['init_env', 'sleep', 'get_hardfault_reason'] 
)
print(output)