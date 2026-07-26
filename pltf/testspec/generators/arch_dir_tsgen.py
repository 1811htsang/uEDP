import sys
import os
from jinja2 import Environment, FileSystemLoader
def main():
  cur_trm_dir = os.path.dirname("uEDP")
  config_dir = os.path.join(cur_trm_dir, ".config")
  cur_file_dir = os.path.dirname(os.path.abspath(__file__))
  parent_file_dir = os.path.dirname(cur_file_dir)
  arch_dir = "sources/pal/arch"
  sys.path.insert(0, config_dir)
  if parent_file_dir not in sys.path:
    sys.path.append(parent_file_dir)
  from cfparsers import dotcfg_cfp
  context = dotcfg_cfp.parse_config(config_dir)
  # Create folder for arch
  try:
    os.makedirs(arch_dir + f"/{context["arch_name"]}", exist_ok=True)
  except PermissionError:
    print(f"[ERROR] Permission denied: Cannot create directory")
  except OSError as e:
    print(f"[ERROR] Error creating directory: {e}")