import os
import sys
from ..cfparsers import dotcfg_cfp

def main():
  cur_trm_dir = os.path.dirname("uEDP")
  config_dir = os.path.join(cur_trm_dir, ".config")
  context = dotcfg_cfp.cfp_parse_dotcfg(config_dir)
  return context