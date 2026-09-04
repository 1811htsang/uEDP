"""Entry points for post-logicdef code generation."""

from pathlib import Path
from typing import Any
import argparse

from ...lstaxer.kre8 import build_generator_context
from .appc_tsgen import generate_appc


def load_context(yaml_path: str | Path) -> dict[str, Any]:
  """Build a generator context from a validated post-logicdef YAML file."""
  path = Path(yaml_path)
  return build_generator_context(path.read_text(encoding='utf-8'))


def build_context(yaml_text: str) -> dict[str, Any]:
  """Build a generator context directly from post-logicdef YAML text."""
  return build_generator_context(yaml_text)


def main(yaml_path: str | Path = 'sources/app/lstaxizer.yaml', output_path: str | Path = 'sources/app/app.c') -> Path:
  """Run the post-logicdef pipeline and generate the application source."""
  return generate_appc(yaml_path, output_path)


__all__ = ['build_context', 'generate_appc', 'load_context']


if __name__ == '__main__':
  parser = argparse.ArgumentParser(description='Generate app.c from post-logicdef YAML')
  parser.add_argument('--yaml', default='sources/app/lstaxizer.yaml')
  parser.add_argument('--output', default='sources/app/app.c')
  args = parser.parse_args()
  output = main(args.yaml, args.output)
  print(f'[INFO] generated {output}')
