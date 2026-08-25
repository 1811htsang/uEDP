import os
from datetime import datetime
import pprint

# [1] Config specifier
current_dir = os.path.dirname("uEDP")
config_dir = os.path.join(current_dir, ".config")
arch_dir = "sources/pal/arch"

counters = {"norm": 0xE6, "poll": 0xD4, "sig": 0x01}

def parse_config(config_path):
  # Jinja2 data structure
  context = {
    "current_date": datetime.now().strftime("%Y-%m-%d"),
    "core_configs": [],
    "pal_configs": [],
    "tasknorm_defs": [],
    "taskpoll_defs": [],
    "sig_defs": [],
    "msgq_defs": [],
    "normhler_lists": [],
    "pollhler_lists": [],
    "tsmio_lists": set(),
    "fsmio_lists": set(),
    "appcfg_libs": ["uedp_core.h", "uedp_task.h"],
    "appcfg_tsm_state_trans": [],
    "appcfg_tsm_tables": [],
    "appcfg_tsm_objects": [],
    "appcfg_fsm_objects": [],
    "arch_name": "pal_arch",
    "arch_apis": [],
    "task_tsm": [], # This will be filled with the task name followed by the state name list, used for μE-LS user syntax support
    "task_fsm": [] # This will be filled with the task name followed by the state name list, used for μE-LS user syntax support
  }

  # Auto ID counter
  # FIXME - Chỗ này sẽ được Minh sửa follow theo task default removal
  counters = {"norm": 0xE6, "poll": 0xD4, "sig": 0x01}
  task_tsm_map = {}
  task_fsm_map = {}

  with open(config_path, 'r', encoding='utf-8') as f:
    for line in f:
      line = line.strip().removeprefix("CONFIG_")
      if not line or line.startswith('#') or "=" not in line:
        continue
      
      key, val = line.split("=", 1)
      val = val.strip('"')

      # 1. CORE_
      if key.startswith("CORE_"):
        name = key.removeprefix("CONFIG_").removeprefix("CORE_")
        context["core_configs"].append(f"{name:<30} ({val}u)")

      # 2. PAL_
      elif key.startswith("PAL_"):
        if "NAME" in key and not "API" in key: context["arch_name"] = val
        elif "API" in key: context["arch_apis"].append(val)
        else:
          name = key.removeprefix("PAL_")
          context["pal_configs"].append(f"{name:<25} ({val}u)")

      # 3. DECL_ (Task, Signal, Handler)
      elif key.startswith("DECL_"):
        if "TASK_NORM" in key:
          context["tasknorm_defs"].append(f"{val:<20} (0x{counters['norm']:02X}u)")
          counters["norm"] += 1
        elif "TASK_POLL" in key:
          context["taskpoll_defs"].append(f"{val:<20} (0x{counters['poll']:02X}u)")
          counters["poll"] += 1
        elif "SIG_" in key:
          context["sig_defs"].append(f"{val:<20} (0x{counters['sig']:02X}u)")
          counters["sig"] += 1
        elif "MSG_QUEUE" in key:
          context["msgq_defs"].append(val.lower().replace("_ID", ""))
        elif "NORM_HANDLER" in key:
          context["normhler_lists"].append(val.replace("_ID", "").lower())
        elif "POLL_HANDLER" in key:
          context["pollhler_lists"].append(val.replace("_ID", "").lower())

      # 4. APPCFG_ (TSM/FSM)
      elif key.startswith("APPCFG_"):
        if "TSM_TASK" in key and "STATE" not in key:
          obj = val.replace("_ID", "").lower() # This line will parse to "tsm_task_1" from "TSM_TASK_1_ID"
          context["appcfg_tsm_objects"].append(f"{obj}")
          context["appcfg_tsm_tables"].append(f"{obj}_tbl")
          task_tsm_map[key] = [obj]
        elif "STATE" in key:
          state_clean = val.replace("_ID", "").lower()
          if "TSM" in key:
            context["appcfg_tsm_state_trans"].append(f"{state_clean}_trans")
            context["tsmio_lists"].add(state_clean)
            task_key = key.split("_STATE_", 1)[0]
            if task_key not in task_tsm_map:
              task_tsm_map[task_key] = []
            task_tsm_map[task_key].append(state_clean)
          else:
            context["fsmio_lists"].add(state_clean)
            task_key = key.split("_STATE_", 1)[0]
            if task_key not in task_fsm_map:
              task_fsm_map[task_key] = []
            task_fsm_map[task_key].append(state_clean)
        elif "FSM_TASK" in key:
          obj = val.replace("_ID", "").lower()
          context["appcfg_fsm_objects"].append(f"{obj}")
          task_fsm_map[key] = [obj]

  # List formatize
  context["tsmio_lists"] = sorted(list(context["tsmio_lists"]))
  context["fsmio_lists"] = sorted(list(context["fsmio_lists"]))
  context["task_tsm"] = list(task_tsm_map.values())
  context["task_fsm"] = list(task_fsm_map.values())
  context["arch_name_upperc"] = context["arch_name"].upper()
  
  return context

# STUB - Sample usage to test the parse_config function
# pprint.pprint(parse_config(config_dir))
# print("\n\n")