# [1] Add lib
import os
import sys
# [2] Config specifier
current_dir = os.path.dirname(os.path.abspath(__file__))
kconfig_dir = os.path.join(current_dir, "sources", "common", "kconfiglib")
pyspec_dir = os.path.join(current_dir, "sources", "common", "pyspec")
# [3] Python inserter
sys.path.insert(0, kconfig_dir)
sys.path.insert(1, pyspec_dir)
# [4] Import kconfiglib và menuconfig after add to sys.path
import kconfiglib
import menuconfig
import argparse
# [5] Import user input function from pyspec
from pltf.pyspec import user_input, task_norm_declaration, task_poll_declaration, signal_declaration, hardware_api_declaration
# [6] Global variables to hold user input values (if needed)
DEFAULT_VALS = {
  "num_tasks_norm": 8,
  "num_tasks_poll": 8,
  "num_signals": 10,
  "is_use_fsm": False,
  "is_use_tsm": False,
  "num_tsm_states": 0,
  "num_fsm_states": 0, 
  "num_hw_api": 0
}
def main():
  os.environ["KCONFIG_CONFIG"] = ".config"
  os.environ["MENUCONFIG_STYLE"] = "aquatic"
  # Input holder from collector api
  (n_norm, n_poll, n_sig, use_fsm, use_tsm, n_tsm_st, n_fsm_st, n_hw_api) = user_input(DEFAULT_VALS)
  # Override decl file for new config
  open("sources/app/kconfig/decl.kconfig", "w").close()
  # Generate declaration for uEDP
  task_norm_declaration(n_norm, n_tsm_st, n_fsm_st, use_tsm, use_fsm)
  task_poll_declaration(n_poll)
  signal_declaration(n_sig)
  hardware_api_declaration(n_hw_api)
  # Kconfig call
  kconf = kconfiglib.Kconfig("Kconfig")
  if os.path.exists(".config"):
    kconf.load_config(".config")
  menuconfig.menuconfig(kconf)
  kconf.write_config(".config")
if __name__ == "__main__":
  main()