import os
import sys
from ...attribarse import dotcfg

def main():
  cur_trm_dir = os.path.dirname("uEDP")
  config_dir = os.path.join(cur_trm_dir, ".config")
  context = dotcfg.cfp_parse_dotcfg(config_dir)
  return context