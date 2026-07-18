from jinja2 import Environment, FileSystemLoader

env = Environment(loader = FileSystemLoader('./sources/common/testspec/templates'))
template = env.get_template('appdecl_tmpl.txt')

output = template.render(
  tasknorm_defs = ['TASK_NORM_A_ID (0xE6u)', 'TASK_NORM_B_ID (0xE7u)', 'TASK_NORM_C_ID (0xE8u)'], 
  taskpoll_defs = ['TASK_POLL_A_ID (0xD4u)', 'TASK_POLL_B_ID (0xD5u)'],
  sig_defs = ['SIG_USR_START (0x01u)', 'SIG_USR_STOP (0x02u)', 'SIG_TSK_A_TO_B (0x03u)', 'SIG_TSK_B_TO_A (0x04u)'],
  msgq_defs = ['usr', 'a', 'b'],
  normhler_lists = ['usr', 'a', 'b'], 
  pollhler_lists = ['memrp', 'logdp'],
  tsmio_lists = ['usr_st_a', 'usr_st_b', 'usr_st_b'],
  fsmio_lists = ['usr', 'a', 'b']
)
print(output)