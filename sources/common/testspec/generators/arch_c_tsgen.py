from jinja2 import Environment, FileSystemLoader

env = Environment(loader = FileSystemLoader('./sources/common/testspec/templates'))
template = env.get_template('arch_c_tmpl.txt')

output = template.render(
  arch_name = 'stm32h723',
  arch_apis = ['init_env', 'sleep', 'get_hardfault_reason']
)
print(output)