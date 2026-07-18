from jinja2 import Environment, FileSystemLoader

env = Environment(loader = FileSystemLoader('./sources/common/testspec/templates'))
template = env.get_template('appcfg_tmpl.txt')

output = template.render(current_date='16 May 2025')
print(output)