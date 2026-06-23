# [1] Add lib
import os
import sys

# [2] Config specifier
current_dir = os.path.dirname(os.path.abspath(__file__))
kconfig_dir = os.path.join(current_dir, "sources", "common", "kconfiglib")
pyspec_dir = os.path.join(current_dir, "sources", "common", "pyspec")

# [3] Python inserter
sys.path.insert(0, kconfig_dir)
sys.path.insert(0, pyspec_dir)

# [4] Import kconfiglib và menuconfig after add to sys.path
from sources.common.kconfiglib import kconfiglib
from sources.common.kconfiglib import menuconfig
import argparse

# [5] Define marker for app_cfg.h
KCONFIG_APPCFG_STATE_TRANS_START = "// KCONFIG_APPCFG_STATE_TRANS_START"
KCONFIG_APPCFG_STATE_TRANS_END = "// KCONFIG_APPCFG_STATE_TRANS_END"

KCONFIG_APPCFG_TSM_TABLE_START = "// KCONFIG_APPCFG_TSM_TABLE_START"
KCONFIG_APPCFG_TSM_TABLE_END = "// KCONFIG_APPCFG_TSM_TABLE_END"

KCONFIG_APPCFG_TSM_OJB_START = "// KCONFIG_APPCFG_TSM_OJB_START"
KCONFIG_APPCFG_TSM_OJB_END = "// KCONFIG_APPCFG_TSM_OJB_END"

KCONFIG_APPCFG_FSM_OJB_START = "// KCONFIG_APPCFG_FSM_OJB_START"
KCONFIG_APPCFG_FSM_OJB_END = "// KCONFIG_APPCFG_FSM_OJB_END"

def tsm_obj_gen(kconf, target):
  # 1. Find the target file
  if not os.path.exists(target):
    print(f"[ERROR] File not found at sources/app/config: {target}")
    return False

  # 2. Generate appcfg content based on Kconfig configuration
  appcfg_tsmobj_kconfig_lines = []
  
  # 3. Define indentation (here using 2 spaces, can be changed to "\t" if preferred)
  indent = "\t" 

  # 4. Generate state transition content based on Kconfig configuration
  for sym in kconf.unique_defined_syms:
    # Ignore blank or undefined symbols
    if sym.config_string == "" or sym.name.startswith("_"):
      continue

    # Only process symbols that start with "APPCFG_TSM_TASK_"
    if not sym.name.startswith("APPCFG_TSM_TASK_"):
      continue

    # Extract the relevant part of the symbol name after "APPCFG_"
    relevant_name = sym.name[len("APPCFG_"):]

    # Generate the line content based on the symbol type and value
    # Must be like APPCFG_TSM_TASK_{i}, If APPCFG_TSM_TASK_{i}_STATE_{j} then skip
    if "_STATE_" in relevant_name:
      continue  # Skip state symbols, only process task symbols
    if sym.type == kconfiglib.STRING:
      #line content is like "extern uedp_tsm_t {sym.str_value};"
      line_content = f"{indent}extern uedp_tsm_t {sym.str_value.lower()}_obj;"
    else:
      continue  # Skip unsupported types

    # Append the generated line to the list
    appcfg_tsmobj_kconfig_lines.append(line_content)

  # 5. Join the lines into a single string to insert into the target file
  kconfig_content = "\n".join(appcfg_tsmobj_kconfig_lines)

  # 6. Find the position to insert in the target file
  with open(target, "r", encoding="utf-8") as f:
    file_content = f.read()

  if KCONFIG_APPCFG_TSM_OJB_START not in file_content or KCONFIG_APPCFG_TSM_OJB_END not in file_content:
    print(f"[ERROR] Marker {KCONFIG_APPCFG_TSM_OJB_START} not found in file {target}!")
    return False
  
  # 7. Split the file content into three parts: before the start marker, the new content, and after the end marker
  parts = file_content.split(KCONFIG_APPCFG_TSM_OJB_START)
  before_part = parts[0] + KCONFIG_APPCFG_TSM_OJB_START
  after_part = parts[1].split(KCONFIG_APPCFG_TSM_OJB_END)[1]

  # 8. Join the parts back together to form the new file content
  new_file_content = f"{before_part}\n\n{kconfig_content}\n\n{indent}{KCONFIG_APPCFG_TSM_OJB_END}{after_part}"

  # 9. Write the updated content back to the file
  with open(target, "w", encoding="utf-8") as f:
    f.write(new_file_content)

  return True

def fsm_obj_gen(kconf, target):
  # 1. Find the target file
  if not os.path.exists(target):
    print(f"[ERROR] File not found at sources/app/config: {target}")
    return False

  # 2. Generate appcfg content based on Kconfig configuration
  appcfg_fsmobj_kconfig_lines = []
  
  # 3. Define indentation (here using 2 spaces, can be changed to "\t" if preferred)
  indent = "\t" 

  # 4. Generate state transition content based on Kconfig configuration
  for sym in kconf.unique_defined_syms:
    # Ignore blank or undefined symbols
    if sym.config_string == "" or sym.name.startswith("_"):
      continue

    # Only process symbols that start with "APPCFG_FSM_TASK_"
    if not sym.name.startswith("APPCFG_FSM_TASK_"):
      continue

    # Extract the relevant part of the symbol name after "APPCFG_"
    relevant_name = sym.name[len("APPCFG_"):]

    # Generate the line content based on the symbol type and value
    # Must be like APPCFG_FSM_TASK_{i}, If APPCFG_FSM_TASK_{i}_STATE_{j} then skip
    if "_STATE_" in relevant_name:
      continue  # Skip state symbols, only process task symbols
    if sym.type == kconfiglib.STRING:
      #line content is like "extern uedp_fsm_t {sym.str_value};"
      line_content = f"{indent}extern uedp_fsm_t {sym.str_value.lower()}_obj;"
    else:
      continue  # Skip unsupported types

    # Append the generated line to the list
    appcfg_fsmobj_kconfig_lines.append(line_content)

  # 5. Join the lines into a single string to insert into the target file
  kconfig_content = "\n".join(appcfg_fsmobj_kconfig_lines)

  # 6. Find the position to insert in the target file
  with open(target, "r", encoding="utf-8") as f:
    file_content = f.read()

  if KCONFIG_APPCFG_FSM_OJB_START not in file_content or KCONFIG_APPCFG_FSM_OJB_END not in file_content:
    print(f"[ERROR] Marker {KCONFIG_APPCFG_FSM_OJB_START} not found in file {target}!")
    return False
  
  # 7. Split the file content into three parts: before the start marker, the new content, and after the end marker
  parts = file_content.split(KCONFIG_APPCFG_FSM_OJB_START)
  before_part = parts[0] + KCONFIG_APPCFG_FSM_OJB_START
  after_part = parts[1].split(KCONFIG_APPCFG_FSM_OJB_END)[1]

  # 8. Join the parts back together to form the new file content
  new_file_content = f"{before_part}\n\n{kconfig_content}\n\n{indent}{KCONFIG_APPCFG_FSM_OJB_END}{after_part}"

  # 9. Write the updated content back to the file
  with open(target, "w", encoding="utf-8") as f:
    f.write(new_file_content)

  return True

def tsm_obj_table_gen(kconf, target):
  # 1. Find the target file
  if not os.path.exists(target):
    print(f"[ERROR] File not found at sources/app/config: {target}")
    return False

  # 2. Generate appcfg content based on Kconfig configuration
  appcfg_tsmobjtable_kconfig_lines = []
  
  # 3. Define indentation (here using 2 spaces, can be changed to "\t" if preferred)
  indent = "\t" 

  # 4. Generate state transition content based on Kconfig configuration
  for sym in kconf.unique_defined_syms:
    # Ignore blank or undefined symbols
    if sym.config_string == "" or sym.name.startswith("_"):
      continue

    # Only process symbols that start with "APPCFG_TSM_TASK_"
    if not sym.name.startswith("APPCFG_TSM_TASK_"):
      continue

    # Extract the relevant part of the symbol name after "APPCFG_"
    relevant_name = sym.name[len("APPCFG_"):]

    # Generate the line content based on the symbol type and value
    # Must be like APPCFG_TSM_TASK_{i}, If APPCFG_TSM_TASK_{i}_STATE_{j} then skip
    if "_STATE_" in relevant_name:
      continue  # Skip state symbols, only process task symbols
    if sym.type == kconfiglib.STRING:
      #line content is like "extern const tsm_state_desc_t {sym.str_value}[];"
      line_content = f"{indent}extern const tsm_state_desc_t {sym.str_value.lower()}_state_desc[];"
    else:
      continue  # Skip unsupported types

    # Append the generated line to the list
    appcfg_tsmobjtable_kconfig_lines.append(line_content)

  # 5. Join the lines into a single string to insert into the target file
  kconfig_content = "\n".join(appcfg_tsmobjtable_kconfig_lines)

  # 6. Find the position to insert in the target file
  with open(target, "r", encoding="utf-8") as f:
    file_content = f.read()

  if KCONFIG_APPCFG_TSM_TABLE_START not in file_content or KCONFIG_APPCFG_TSM_TABLE_END not in file_content:
    print(f"[ERROR] Marker {KCONFIG_APPCFG_TSM_TABLE_START} not found in file {target}!")
    return False
  
  # 7. Split the file content into three parts: before the start marker, the new content, and after the end marker
  parts = file_content.split(KCONFIG_APPCFG_TSM_TABLE_START)
  before_part = parts[0] + KCONFIG_APPCFG_TSM_TABLE_START
  after_part = parts[1].split(KCONFIG_APPCFG_TSM_TABLE_END)[1]

  # 8. Join the parts back together to form the new file content
  new_file_content = f"{before_part}\n\n{kconfig_content}\n\n{indent}{KCONFIG_APPCFG_TSM_TABLE_END}{after_part}"

  # 9. Write the updated content back to the file
  with open(target, "w", encoding="utf-8") as f:
    f.write(new_file_content)

  return True

def tsm_trans_gen(kconf, target):
  # 1. Find the target file
  if not os.path.exists(target):
    print(f"[ERROR] File not found at sources/app/config: {target}")
    return False

  # 2. Generate appcfg content based on Kconfig configuration
  appcfg_tsmobjtable_kconfig_lines = []
  
  # 3. Define indentation (here using 2 spaces, can be changed to "\t" if preferred)
  indent = "\t" 

  # 4. Generate state transition content based on Kconfig configuration
  for sym in kconf.unique_defined_syms:
    # Ignore blank or undefined symbols
    if sym.config_string == "" or sym.name.startswith("_"):
      continue

    # Only process symbols that start with "APPCFG_TSM_TASK_"
    if not sym.name.startswith("APPCFG_TSM_TASK_"):
      continue

    # Extract the relevant part of the symbol name after "APPCFG_"
    relevant_name = sym.name[len("APPCFG_"):]

    # Generate the line content based on the symbol type and value
    # Must be like APPCFG_TSM_TASK_{i}, If APPCFG_TSM_TASK_{i}_STATE_{j} then skip
    if not "_STATE_" in relevant_name:
      continue  # Skip state symbols, only process task symbols
    if sym.type == kconfiglib.STRING:
      #line content is like "extern const tsm_trans_t {sym.str_value}[];"
      line_content = f"{indent}extern const tsm_trans_t {sym.str_value.lower()}_trans_desc[];"
    else:
      continue  # Skip unsupported types

    # Append the generated line to the list
    appcfg_tsmobjtable_kconfig_lines.append(line_content)

  # 5. Join the lines into a single string to insert into the target file
  kconfig_content = "\n".join(appcfg_tsmobjtable_kconfig_lines)

  # 6. Find the position to insert in the target file
  with open(target, "r", encoding="utf-8") as f:
    file_content = f.read()

  if KCONFIG_APPCFG_STATE_TRANS_START not in file_content or KCONFIG_APPCFG_STATE_TRANS_END not in file_content:
    print(f"[ERROR] Marker {KCONFIG_APPCFG_STATE_TRANS_START} not found in file {target}!")
    return False
  
  # 7. Split the file content into three parts: before the start marker, the new content, and after the end marker
  parts = file_content.split(KCONFIG_APPCFG_STATE_TRANS_START)
  before_part = parts[0] + KCONFIG_APPCFG_STATE_TRANS_START
  after_part = parts[1].split(KCONFIG_APPCFG_STATE_TRANS_END)[1]

  # 8. Join the parts back together to form the new file content
  new_file_content = f"{before_part}\n\n{kconfig_content}\n\n{indent}{KCONFIG_APPCFG_STATE_TRANS_END}{after_part}"

  # 9. Write the updated content back to the file
  with open(target, "w", encoding="utf-8") as f:
    f.write(new_file_content)

  return True

# Function to generate app_cfg.h declaration based on Kconfig configuration
def app_cfg_gen(kconf, target):
  if fsm_obj_gen(kconf, target):
    print(f"\n[SUCCESS] FSM object has been generated in {target}!")
  if tsm_obj_gen(kconf, target):
    print(f"\n[SUCCESS] TSM object has been generated in {target}!")
  if tsm_obj_table_gen(kconf, target):
    print(f"\n[SUCCESS] TSM object table (state descriptor) has been generated in {target}!")
  if tsm_trans_gen(kconf, target):
    print(f"\n[SUCCESS] TSM transitions table (transition descriptors) have been generated in {target}!")

  return True