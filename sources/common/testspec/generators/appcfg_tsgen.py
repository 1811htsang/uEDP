from jinja2 import Environment, FileSystemLoader

env = Environment(loader = FileSystemLoader('./sources/common/testspec/templates'))
template = env.get_template('appcfg_tmpl.txt')

output = template.render(
  current_date='16 May 2025', 
  appcfg_libs = ['stdio.h', 'stdint.h', 'math.h'], 
  appcfg_tsm_state_trans = ['blink_active_trans', 'idle_active_trans'],
  appcfg_tsm_tables = ['blinker_tsm_table', 'idle_tsm_table'],
  appcfg_tsm_objects = ['blinker_tsm', 'idle_tsm'],
  appcfg_fsm_objects = ['blinker_fsm', 'idle_fsm']
)
print(output)