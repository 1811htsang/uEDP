from pathlib import Path
from typing import Any
import argparse
import re

import yaml
from jinja2 import Environment, FileSystemLoader

_TEMPLATE_DIR = Path(__file__).resolve().parents[3] / 'templates'
_TEMPLATE_NAME = 'testc.txt'
_LEVELS = ('ut', 'ct')
_DEFAULT_LEVEL = 'ct'

def _clean(text: Any) -> str:
  return ' '.join(str(text or '').split()).replace('*/', '* /')

def _ident(name: Any) -> str:
  return re.sub(r'[^A-Za-z0-9_]', '_', str(name))

def _steps(tezk: dict[str, Any]) -> list[dict[str, Any]]:
  steps = []
  for item in tezk.get('teztep', []) or []:
    if not isinstance(item, dict):
      continue
    for number, body in item.items():
      body = body or {}
      acts = [str(call.get('act', '')).rstrip() for call in body.get('call', []) or [] if isinstance(call, dict)]
      steps.append({
        'number': str(number),
        'desc': _clean(body.get('desc')),
        'acts': [act for act in acts if act.strip()],
      })
  return steps

def _checks(tezk: dict[str, Any]) -> list[str]:
  return [
    str(item['act']).rstrip()
    for item in tezk.get('xpect', []) or []
    if isinstance(item, dict) and str(item.get('act', '')).strip()
  ]

def build_testc_context(yaml_text: str, level: str | None = None) -> dict[str, Any]:
  payload = yaml.safe_load(yaml_text) or {}
  if 'tezuit' not in payload:
    raise ValueError("YAML does not have the key 'tezuit', need to define tezcal")

  scenarios = []
  for tezk in payload['tezuit'] or []:
    name = str(tezk.get('tezk', '')).strip()
    if not name:
      raise ValueError("tezk is missing its own key 'tezk' in tezuit")
    scenario_level = str(tezk.get('level', _DEFAULT_LEVEL))
    if scenario_level not in _LEVELS:
      raise ValueError(f"tezk '{name}': level '{scenario_level}' is not supported, only {_LEVELS}")
    if level is not None and scenario_level != level:
      continue
    scenarios.append({
      'name': _ident(name),
      'tezk': name,
      'level': scenario_level,
      'subidentif': _clean(tezk.get('subidentif')),
      'dscript': _clean(tezk.get('dscript')),
      'steps': _steps(tezk),
      'checks': _checks(tezk),
    })

  return {
    'identif': _clean(payload.get('identif')),
    'level': level or 'all',
    'scenarios': scenarios,
    'has_ct': any(s['level'] == 'ct' for s in scenarios),
  }

def render_testc(yaml_text: str, level: str | None = None) -> str:
  if level is not None and level not in _LEVELS:
    raise ValueError(f"invalid level '{level}', expected one of {_LEVELS}")
  env = Environment(
    loader=FileSystemLoader(str(_TEMPLATE_DIR)),
    keep_trailing_newline=True,
  )
  return env.get_template(_TEMPLATE_NAME).render(**build_testc_context(yaml_text, level))

def generate_testc(yaml_path: str | Path, output_path: str | Path, level: str | None = None) -> Path | None:
  yaml_text = Path(yaml_path).read_text(encoding='utf-8')
  if 'tezuit' not in (yaml.safe_load(yaml_text) or {}):
    return None
  output = Path(output_path)
  output.parent.mkdir(parents=True, exist_ok=True)
  output.write_text(render_testc(yaml_text, level), encoding='utf-8')
  return output

if __name__ == '__main__':
  parser = argparse.ArgumentParser(description='Generate the libunity test runner from a tezcal YAML')
  parser.add_argument('--yaml', required=True)
  parser.add_argument('--output', default='sources/test/tezprjec/tez.c')
  parser.add_argument('--level', choices=_LEVELS)
  args = parser.parse_args()
  output = generate_testc(args.yaml, args.output, args.level)
  if output is None:
    print(f'[INFO] skipped {args.yaml}: no tezuit section')
  else:
    print(f'[INFO] generated {output}')
