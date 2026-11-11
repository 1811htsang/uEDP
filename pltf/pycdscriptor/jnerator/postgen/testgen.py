from pathlib import Path
from typing import Any
from jinja2 import Environment, FileSystemLoader

from ...lstaxer.kre8 import build_generator_context
from .modalcvert import build_appc_context

_TEMPLATE_DIR = Path(__file__).resolve().parents[3] / 'templates'
_TEMPLATE_NAME = 'testc.txt'
_LEVELS = ('ut', 'ct', 'st', 'it')

def _gda_types(context: dict[str, Any]) -> dict[str, str]:
  return {item['name']: item['type'] for item in context.get('glbda_defs', [])}

def _tsm_objects(appc: dict[str, Any]) -> dict[str, str]:
  return {task['task']: task['tsm_object'] for task in appc.get('tnorm_codegen', []) if task.get('tsm_states')}

def _check_scenario(scenario: dict[str, Any], tsm_objects: dict[str, str], gda_types: dict[str, str]) -> None:
  for check in scenario['expect']:
    if check.get('tsm_state'):
      task = check['tsm_state']['task']
      if task not in tsm_objects:
        raise ValueError(f"tsd '{scenario['name']}': task '{task}' has no TSM")
    if check.get('gda_value'):
      name = check['gda_value']['name']
      if name not in gda_types:
        raise ValueError(f"tsd '{scenario['name']}': gda '{name}' is not declared in glbda")

def build_testc_context(yaml_text: str, level: str | None = None) -> dict[str, Any]:
  context = build_generator_context(yaml_text)
  appc = build_appc_context(yaml_text)
  gda_types = _gda_types(context)
  tsm_objects = _tsm_objects(appc)

  scenarios = []
  for scenario in context.get('tsd', []) or []:
    if level is not None and scenario['level'] != level:
      continue
    _check_scenario(scenario, tsm_objects, gda_types)
    scenarios.append(scenario)

  return {
    'level': level or 'all',
    'scenarios': scenarios,
    'gda_names': [{'name': name, 'type': gda_types[name]} for name in sorted({
      check['gda_value']['name'] for s in scenarios for check in s['expect'] if check.get('gda_value')
    })],
    'tsm_objects': tsm_objects,
  }

def render_testc(yaml_text: str, level: str | None = None) -> str:
  if level is not None and level not in _LEVELS:
    raise ValueError(f"invalid level '{level}', expected one of {_LEVELS}")
  env = Environment(
    loader=FileSystemLoader(str(_TEMPLATE_DIR)),
    keep_trailing_newline=True,
  )
  return env.get_template(_TEMPLATE_NAME).render(**build_testc_context(yaml_text, level))

def generate_testc(yaml_path: str | Path, output_path: str | Path, level: str | None = None) -> Path:
  output = Path(output_path)
  output.parent.mkdir(parents=True, exist_ok=True)
  yaml_text = Path(yaml_path).read_text(encoding='utf-8')
  output.write_text(render_testc(yaml_text, level), encoding='utf-8')
  return output
