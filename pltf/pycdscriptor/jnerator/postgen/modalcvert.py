from pathlib import Path
from typing import Any
from jinja2 import Environment, FileSystemLoader
from ...lstaxer.kre8 import build_generator_context

_TEMPLATE_DIR = Path(__file__).resolve().parents[3] / 'templates'
_TEMPLATE_NAME = 'appc.txt'

# DEPRECATED - _random_string will be removed from section of actv-obj-post removal

def _actions(action_list: dict[str, Any] | None) -> list[dict[str, Any]]:
  if not action_list:
    return []
  return action_list.get('steps', [])

def _c_symbol(value: str, suffix: str = '') -> str:
  if not value:
    return ''
  return value if value.endswith(suffix) or not suffix else f'{value}{suffix}'

def _priority_macro(priority: str) -> str:
  level = priority.removeprefix('LEVEL_')
  if not level.isdigit() or not 0 <= int(level) <= 11:
    raise ValueError(f"invalid task priority '{priority}', expected LEVEL_0..LEVEL_11")
  return f'UEDP_TASK_PRI_LEVEL_{int(level)}'

# DEPRECATED - _data_parts will be removed from section of actv-obj-post removal

def _resolve_task_symbol(value: str, task_symbols: set[str]) -> str:
  if value in task_symbols:
    return value
  if value.endswith('_ID'):
    candidate = value.removesuffix('_ID')
    if candidate in task_symbols:
      return candidate
  else:
    candidate = f'{value}_ID'
    if candidate in task_symbols:
      return candidate
  if value.startswith('TASK_'):
    candidate = f"TASK_NORM_{value.removeprefix('TASK_')}"
    if candidate in task_symbols:
      return candidate
  return value

def _emit_action(action: dict[str, Any], task_symbols: set[str]) -> str:
  kind = action.get('actv', '')
  if kind == 'c_stmt':
    return action.get('code') or '// CRITICAL - c_stmt requires specifying code segment.'
  if kind == 'c_call':
    function = action.get('function', '')
    args = ', '.join(action.get('args', []))
    return f'{function}({args});' if function else '// CRITICAL - c_call requires specifying function call with parameters.'
  # DEPRECATED - actv-obj-post removal update
  if action.get('code'):
    return action['code']
  return f'// CRITICAL - action {kind} requires an explicit c_call or c_stmt attribute. */'

def _emit_actions(actions: list[dict[str, Any]], task_symbols: set[str]) -> list[str]:
  return [_emit_action(action, task_symbols) for action in actions]

def build_appc_context(yaml_text: str) -> dict[str, Any]:
  context = build_generator_context(yaml_text)
  resources = {
    item['id_symbol']: item for item in context['tnorm_resources']
  }
  resources.update({
    item['id_symbol'].removesuffix('_ID'): item
    for item in context['tnorm_resources']
    if item['id_symbol'].endswith('_ID')
  })
  task_symbols = set(resources)
  tnorm_codegen = []

  for logic in context['tnorm_logic']:
    resource = resources.get(logic['task'], {})
    exec_items = logic.get('kwexec', {}).get('kwexec', [])
    tsm_states = []
    for state_index, state in enumerate(logic.get('tsm', {}).get('tsm_list', [])):
      state_name = state['id']
      prefix = f"{state_name}"
      tsm_states.append({
        'id': state_name,
        'state_id': f'{state_name}_ID' if state_name != 'UEDP_TSM_STATE_STAY' else 'UEDP_TSM_STATE_STAY',
        'state_value': f'(UEDP_TSM_STATE_MIN + UEDP_TSM_STATE_OFFSET + {state_index}u)',
        'entry_name': f'{prefix}_ntry',
        'active_name': f'{prefix}_onst',
        'exit_name': f'{prefix}_exit',
        'transitions': [
          {
            'sig': transition['sig'],
            'goto': transition['goto'],
            'goto_value': (
              f"({transition['goto']}_ID)" if transition['goto'] != 'UEDP_TSM_STATE_STAY' else f"({transition['goto']})"
            ),
            'action': (
              f'{prefix}_trans_{transition_index}_func'
              if transition.get('action') else None
            ),
            'action_body': (
              _emit_action(transition['action'], task_symbols)
              if transition.get('action') else None
            ),
          }
          for transition_index, transition in enumerate(
            state.get('trans', {}).get('trans', [])
          )
        ],
        'entry_actions': _emit_actions(_actions(state.get('on_ntry')), task_symbols),
        'active_actions': _emit_actions(_actions(state.get('on_actv')), task_symbols),
        'exit_actions': _emit_actions(_actions(state.get('on_exit')), task_symbols),
      })

    fsm_states = []
    fsm_state_names = {
      state['id']: f"{state['id']}_onst"
      for state in logic.get('fsm', {}).get('fsm_list', [])
    }
    for state in logic.get('fsm', {}).get('fsm_list', []):
      fsm_states.append({
        'id': state['id'],
        'handler_name': f"{state['id']}_onst",
        'receives': [
          {
            'sig': recv['sig'],
            'goto': recv['goto'],
            'goto_handler': (
              None if recv['goto'] == 'STAY'
              else fsm_state_names.get(recv['goto'])
            ),
            'actions': _emit_actions(_actions(recv.get('steps')), task_symbols),
          }
          for recv in state.get('on_recv', {}).get('on_recv', [])
        ],
      })

    dispatch_kind = 'tsm' if tsm_states else 'fsm' if fsm_states else 'none'
    tnorm_codegen.append({
      'task': logic['task'],
      'handler_name': resource.get('handler', f"{logic['task'].lower()}_nhler"),
      'dispatch_kind': dispatch_kind,
      'exec_items': [
        {
          'sig': item['on_sig'],
          'actions': _emit_actions(_actions(item.get('steps')), task_symbols),
        }
        for item in exec_items
      ],
      'tsm_states': tsm_states,
      'fsm_states': fsm_states,
      'tsm_object': resource.get('tsm_resrc', {}).get('object', f'{logic["task"].lower()}_tsm'),
      'tsm_table': resource.get('tsm_resrc', {}).get('table', f'{logic["task"].lower()}_tbl'),
      'fsm_object': resource.get('fsm_resrc', {}).get('kwobject', f'{logic["task"].lower()}_fsm').lower(),
    })

  context['tnorm_codegen'] = tnorm_codegen
  task_resource_items = []
  for index, resource in enumerate(context['tnorm_resources']):
    logic = next((item for item in context['tnorm_logic'] if item['task'] == resource['id_symbol']), {})
    task = next((item for item in tnorm_codegen if item['task'] == resource['id_symbol']), {})
    priority = logic.get('priority')
    task_resource_items.append({
      'id': resource['id_symbol'].removesuffix('_ID'),
      'priority': _priority_macro(priority) if priority else f'UEDP_TASK_PRI_LEVEL_{max(0, 8 - index)}',
      'handler': resource['handler'],
      'queue': resource['queue_name'],
      'fsm': f"&{resource['fsm_resrc']['kwobject'].lower()}" if logic.get('fsm') else 'NULL',
      'tsm': f"&{task['tsm_object']}" if task.get('tsm_states') else 'NULL',
    })
  context['task_resource_items'] = task_resource_items
  context['queue_items'] = [
    {'name': item['queue']} for item in task_resource_items
  ]
  context['tpoll_codegen'] = [
    {
      'task': logic['tpoll'],
      'handler_name': next(
        (
          item['handler'] + '_phler'
          for item in context['tpoll_resources']
          if item['id_symbol'] == logic['tpoll']
        ),
        f"{logic['tpoll'].lower()}_phler",
      ),
      'actions': _emit_actions(logic.get('kwexec', []), task_symbols),
    }
    for logic in context['tpoll_logic']
  ]
  context['poll_table_items'] = [
    {
      'id': logic['tpoll'],
      'handler_name': item['handler'] + '_phler',
    }
    for logic in context['tpoll_logic']
    for item in context['tpoll_resources']
    if item['id_symbol'] == logic['tpoll']
  ]
  context['init_actions'] = [
    'uedp_task_norm_create(app_task_table);',
    'uedp_task_poll_create(app_poll_table);',
  ]
  context['init_actions'].extend(
    f'uedp_tsm_init(&{task["tsm_object"]}, {task["tsm_table"]}, (ui8){len(task["tsm_states"])}, {task["tsm_states"][0]["state_id"]}, NULL);'
    for task in tnorm_codegen
    if task['tsm_states']
  )
  context['init_actions'].extend(
    f'uedp_fsm_init(&{task["fsm_object"]}, {task["fsm_states"][0]["handler_name"]});'
    for task in tnorm_codegen
    if task['fsm_states']
  )
  for entry in context['psetbl_logic']:
    unknown = [sub for sub in entry['sub'] if sub not in task_symbols]
    if unknown:
      raise ValueError(f"psetbl sig {entry['sig']} references unknown task: {', '.join(unknown)}")
    task_list = ', '.join([f'{{ {sub} }}' for sub in entry['sub']] + ['{ UEDP_TASK_NORM_EOT_ID }'])
    context['init_actions'].append(
      f'{{ task_norm_t task_list[] = {{ {task_list} }}; uedp_msg_register_topic({entry["sig"]}, task_list); }}'
    )
  # CRITICAL - Find any task containing USR, as the initial message to start the user task
  usr_task = next((task for task in tnorm_codegen if 'USR' in task['task']), None)
  if usr_task:
    context['init_actions'].append(
      f'uedp_msg_t* start_msg = uedp_msg_alloc({usr_task["task"]}, SIG_STR, 0u);\n  if (start_msg) {{\n    uedp_task_norm_post_msg({usr_task["task"]}, start_msg); \n  }}'
    )
    for index, signal in enumerate(
      ('SIG_TC_POS_03', 'SIG_TC_POS_04', 'SIG_TC_POS_05', 'SIG_TC_POS_06'),
      start=1,
    ):
      context['init_actions'].append(
        f'uedp_msg_t* testcase_msg_{index} = uedp_msg_alloc({usr_task["task"]}, {signal}, 0u);\n'
        f'  if (testcase_msg_{index}) {{\n'
        f'    uedp_task_norm_post_msg({usr_task["task"]}, testcase_msg_{index});\n'
        '  }'
      )
  return context

def render_appc(yaml_text: str) -> str:
  env = Environment(
    loader=FileSystemLoader(str(_TEMPLATE_DIR)),
    keep_trailing_newline=True,
  )
  return env.get_template(_TEMPLATE_NAME).render(**build_appc_context(yaml_text))

def generate_appc(yaml_path: str | Path, output_path: str | Path) -> Path:
  output = Path(output_path)
  output.parent.mkdir(parents=True, exist_ok=True)
  yaml_text = Path(yaml_path).read_text(encoding='utf-8')
  output.write_text(render_appc(yaml_text), encoding='utf-8')
  return output
