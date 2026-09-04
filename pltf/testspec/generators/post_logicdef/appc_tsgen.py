"""Generate application C source from post-logicdef models."""

from pathlib import Path
from typing import Any

from jinja2 import Environment, FileSystemLoader

from ...lstaxer.kre8_lstaxer import build_generator_context


_TEMPLATE_DIR = Path(__file__).resolve().parents[3] / 'templates'
_TEMPLATE_NAME = 'appc.txt'


def _actions(action_list: dict[str, Any] | None) -> list[dict[str, Any]]:
  if not action_list:
    return []
  return action_list.get('steps', [])


def _c_symbol(value: str, suffix: str = '') -> str:
  if not value:
    return ''
  return value if value.endswith(suffix) or not suffix else f'{value}{suffix}'


def _data_parts(action: dict[str, Any]) -> tuple[str, str | None, str | None]:
  data = action.get('data')
  if isinstance(data, dict):
    return data.get('value', ''), data.get('type'), data.get('mode')
  return data or '', None, action.get('ptype')


def _resolve_task_symbol(value: str, task_symbols: set[str]) -> str:
  if value in task_symbols:
    return value
  if value.startswith('TASK_'):
    candidate = f"TASK_NORM_{value.removeprefix('TASK_')}"
    if candidate in task_symbols:
      return candidate
  return value


def _emit_action(action: dict[str, Any], task_symbols: set[str]) -> str:
  kind = action.get('actv', '')
  if kind == 'c_stmt':
    return action.get('code') or '/* c_stmt requires code. */'
  if kind == 'c_call':
    function = action.get('function', '')
    args = ', '.join(action.get('args', []))
    return f'{function}({args});' if function else '/* c_call requires function. */'
  if kind in ('post_msg', 'uedp_post_msg', 'uedp_task_norm_post_msg'):
    target = _resolve_task_symbol(action.get('to', ''), task_symbols)
    signal = action.get('sig', '')
    value, data_type, mode = _data_parts(action)
    if not target or not signal:
      return '/* post_msg requires to and sig. */'
    if not value:
      return '{ uedp_msg_t* msg = uedp_msg_alloc(%s, %s, 0u); if (msg) { uedp_task_norm_post_msg(%s, msg); } }' % (target, signal, target)
    if str(mode).upper() == 'REF':
      size = 'sizeof(void*)'
      setter = f'uedp_msg_set_data_ref(msg, (void*)&{value});'
    else:
      size = f'sizeof({value})' if not data_type else f'sizeof({value})'
      setter = f'uedp_msg_set_data(msg, (const ui8*)&{value}, (ui8){size});'
    return '{ uedp_msg_t* msg = uedp_msg_alloc(%s, %s, %s); if (msg) { %s uedp_task_norm_post_msg(%s, msg); } }' % (target, signal, size, setter, target)
  if action.get('code'):
    return action['code']
  return f'/* action {kind} requires an explicit c_call or c_stmt schema. */'


def _emit_actions(actions: list[dict[str, Any]], task_symbols: set[str]) -> list[str]:
  return [_emit_action(action, task_symbols) for action in actions]


def build_appc_context(yaml_text: str) -> dict[str, Any]:
  """Build the template context for the post-logicdef app.c artifact."""
  context = build_generator_context(yaml_text)
  resources = {
    item['id_symbol']: item for item in context['tnorm_resources']
  }
  task_symbols = set(resources)
  tnorm_codegen = []

  for logic in context['tnorm_logic']:
    resource = resources.get(logic['task'], {})
    tsm_states = []
    for state_index, state in enumerate(logic.get('tsm', {}).get('tsm_list', [])):
      state_name = state['id']
      prefix = f"{state_name}"
      tsm_states.append({
        'id': state_name,
        'state_id': f'{state_name}_ID',
        'state_value': f'(UEDP_TSM_STATE_MIN + UEDP_TSM_STATE_OFFSET + {state_index}u)',
        'entry_name': f'{prefix}_ntry',
        'active_name': f'{prefix}_onst',
        'exit_name': f'{prefix}_exit',
        'transitions': [
          {
            'sig': transition['sig'],
            'goto': transition['goto'],
            'goto_value': (
              'UEDP_TSM_STATE_STAY'
              if transition['goto'] == 'STAY'
              else f"({transition['goto']}_ID)"
            ),
          }
          for transition in state.get('trans', {}).get('trans', [])
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
    task_resource_items.append({
      'id': resource['id_symbol'],
      'priority': f'UEDP_TASK_PRI_LEVEL_{max(0, 8 - index)}',
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
  return context


def render_appc(yaml_text: str) -> str:
  """Render post-logicdef YAML into complete app.c source text."""
  env = Environment(
    loader=FileSystemLoader(str(_TEMPLATE_DIR)),
    keep_trailing_newline=True,
  )
  return env.get_template(_TEMPLATE_NAME).render(**build_appc_context(yaml_text))


def generate_appc(yaml_path: str | Path, output_path: str | Path) -> Path:
  """Generate app.c from YAML without modifying the pre-logicdef pipeline."""
  output = Path(output_path)
  output.parent.mkdir(parents=True, exist_ok=True)
  yaml_text = Path(yaml_path).read_text(encoding='utf-8')
  output.write_text(render_appc(yaml_text), encoding='utf-8')
  return output
