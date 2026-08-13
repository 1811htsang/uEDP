import os
import sys

try:
  from ..cfparsers import dotcfg_cfp
except ImportError:
  current_dir = os.path.dirname(os.path.abspath(__file__))
  parent_dir = os.path.dirname(current_dir)
  if parent_dir not in sys.path:
    sys.path.insert(0, parent_dir)
  from cfparsers import dotcfg_cfp


def main():
  cur_trm_dir = os.path.dirname("uEDP")
  config_dir = os.path.join(cur_trm_dir, ".config")
  context = dotcfg_cfp.parse_config(config_dir)
  return context