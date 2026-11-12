from pathlib import Path
from typing import Any
from jinja2 import Environment, FileSystemLoader

from ...lstaxer.kre8 import build_generator_context
from .modalcvert import build_appc_context

_TEMPLATE_DIR = Path(__file__).resolve().parents[3] / 'templates'
_TEMPLATE_NAME = 'testc.txt'
_LEVELS = ('ut', 'ct', 'st', 'it')

_ASSERT_BY_TYPE = {
  'const char*': 'TEST_ASSERT_EQUAL_STRING',
  'char*': 'TEST_ASSERT_EQUAL_STRING',
  'bool': 'TEST_ASSERT_TRUE',
  'uint8_t': 'TEST_ASSERT_EQUAL_UINT8',
  'ui8': 'TEST_ASSERT_EQUAL_UINT8',
  'uint16_t': 'TEST_ASSERT_EQUAL_UINT16',
  'ui16': 'TEST_ASSERT_EQUAL_UINT16',
  'uint32_t': 'TEST_ASSERT_EQUAL_UINT32',
  'ui32': 'TEST_ASSERT_EQUAL_UINT32',
  'int8_t': 'TEST_ASSERT_EQUAL_INT8',
  'i8': 'TEST_ASSERT_EQUAL_INT8',
  'int16_t': 'TEST_ASSERT_EQUAL_INT16',
  'i16': 'TEST_ASSERT_EQUAL_INT16',
  'int32_t': 'TEST_ASSERT_EQUAL_INT32',
  'int': 'TEST_ASSERT_EQUAL_INT32',
  'i32': 'TEST_ASSERT_EQUAL_INT32',
}

def assert_macro_for(gda_type: str, expected: str) -> str:
  if gda_type == 'bool':
    if expected not in ('true', 'false'):
      raise ValueError(f"bool gda expects 'true' or 'false', got '{expected}'")
    return 'TEST_ASSERT_TRUE' if expected == 'true' else 'TEST_ASSERT_FALSE'
  if gda_type in _ASSERT_BY_TYPE:
    return _ASSERT_BY_TYPE[gda_type]
  if 'char' in gda_type and '*' in gda_type:
    return 'TEST_ASSERT_EQUAL_STRING'
  raise ValueError(f"unsupported glbda type '{gda_type}' for TSD expect; add a mapping in _ASSERT_BY_TYPE")

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
      check['gda_value']['assert_macro'] = assert_macro_for(gda_types[name], check['gda_value']['equals'])

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
