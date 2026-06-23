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
KCONFIG_DECL_TASK_NORM_START = "// KCONFIG_DECL_TASK_NORM_START"
KCONFIG_DECL_TASK_NORM_END = "// KCONFIG_DECL_TASK_NORM_END"

KCONFIG_DECL_TASK_POLL_START = "// KCONFIG_DECL_TASK_POLL_START"
KCONFIG_DECL_TASK_POLL_END = "// KCONFIG_DECL_TASK_POLL_END"

KCONFIG_DECL_SIGNAL_START = "// KCONFIG_DECL_SIGNAL_START"
KCONFIG_DECL_SIGNAL_END = "// KCONFIG_DECL_SIGNAL_END"

KCONFIG_DECL_MSG_QUEUE_START = "// KCONFIG_DECL_MSG_QUEUE_START"
KCONFIG_DECL_MSG_QUEUE_END = "// KCONFIG_DECL_MSG_QUEUE_END"

KCONFIG_DECL_TASK_NORM_HANDLER_START = "// KCONFIG_DECL_TASK_NORM_HANDLER_START"
KCONFIG_DECL_TASK_NORM_HANDLER_END = "// KCONFIG_DECL_TASK_NORM_HANDLER_END"

KCONFIG_DECL_TASK_POLL_HANDLER_START = "// KCONFIG_DECL_TASK_POLL_HANDLER_START"
KCONFIG_DECL_TASK_POLL_HANDLER_END = "// KCONFIG_DECL_TASK_POLL_HANDLER_END"

def tsk_norm_gen(kconf, target):
  # 1. Find the target file
  if not os.path.exists(target):
    print(f"[ERROR] File not found at sources/app/config: {target}")
    return False

  # 2. Generate appcfg content based on Kconfig configuration
  appdecl_tsk_norm_kconfig_lines = []
  
  # 3. Define indentation (here using 2 spaces, can be changed to "\t" if preferred)
  indent = "\t" 

  # 4. Define start id range from 0xE6 to 0xFF for task norm
  start_id = 0xE6
  end_id = 0xFF
  index = 0

  # 5. Generate state transition content based on Kconfig configuration
  for sym in kconf.unique_defined_syms:

    # Ignore blank or undefined symbols
    if sym.config_string == "" or sym.name.startswith("_"):
      continue

    # Only process symbols that start with "DECL_TASK_NORM_"
    if not sym.name.startswith("DECL_TASK_NORM_"):
      continue

    # Extract the relevant part of the symbol name after "DECL_TASK_NORM_"
    relevant_name = sym.name[len("DECL_"):]

    # Generate the line content based on the symbol type and value
    # Must be like DECL_TASK_NORM_{i}_NAME, DECL_TASK_NORM_{i}_PRIO
    if not "_NAME" in relevant_name:
      continue  # Skip if the symbol name does not contain "_NAME_"
    if sym.type == kconfiglib.STRING:
      #line content is like "#define {sym.str_value.upper()} (value from range 0xE6 to 0xFF)"
      line_content = f"{indent}#define {sym.str_value.upper()} (0x{start_id + index:02X}u)"
    else:
      continue  # Skip unsupported types

    # Append the generated line to the list
    appdecl_tsk_norm_kconfig_lines.append(line_content)
    index += 1

  # 5. Join the lines into a single string to insert into the target file
  kconfig_content = "\n".join(appdecl_tsk_norm_kconfig_lines)

  # 6. Find the position to insert in the target file
  with open(target, "r", encoding="utf-8") as f:
    file_content = f.read()

  if KCONFIG_DECL_TASK_NORM_START not in file_content or KCONFIG_DECL_TASK_NORM_END not in file_content:
    print(f"[ERROR] Marker {KCONFIG_DECL_TASK_NORM_START} not found in file {target}!")
    return False
  
  # 7. Split the file content into three parts: before the start marker, the new content, and after the end marker
  parts = file_content.split(KCONFIG_DECL_TASK_NORM_START)
  before_part = parts[0] + KCONFIG_DECL_TASK_NORM_START
  after_part = parts[1].split(KCONFIG_DECL_TASK_NORM_END)[1]

  # 8. Join the parts back together to form the new file content
  new_file_content = f"{before_part}\n\n{kconfig_content}\n\n{indent}{KCONFIG_DECL_TASK_NORM_END}{after_part}"

  # 9. Write the updated content back to the file
  with open(target, "w", encoding="utf-8") as f:
    f.write(new_file_content)

  return True

def tsk_poll_gen(kconf, target):
  # 1. Find the target file
  if not os.path.exists(target):
    print(f"[ERROR] File not found at sources/app/config: {target}")
    return False

  # 2. Generate appcfg content based on Kconfig configuration
  appdecl_tsk_poll_kconfig_lines = []
  
  # 3. Define indentation (here using 2 spaces, can be changed to "\t" if preferred)
  indent = "\t" 

  # 4. Define start id range from 0xD4 to 0xDF for task poll
  start_id = 0xD4
  end_id = 0xDF
  index = 0

  # 5. Generate state transition content based on Kconfig configuration
  for sym in kconf.unique_defined_syms:

    # Ignore blank or undefined symbols
    if sym.config_string == "" or sym.name.startswith("_"):
      continue

    # Only process symbols that start with "DECL_TASK_POLL_"
    if not sym.name.startswith("DECL_TASK_POLL_"):
      continue

    # Extract the relevant part of the symbol name after "DECL_TASK_POLL_"
    relevant_name = sym.name[len("DECL_"):]

    # Generate the line content based on the symbol type and value
    # Must be like DECL_TASK_POLL_{i}_NAME, DECL_TASK_POLL_{i}_PRIO
    if not "_NAME" in relevant_name:
      continue  # Skip if the symbol name does not contain "_NAME_"
    if sym.type == kconfiglib.STRING:
      #line content is like "#define {sym.str_value.upper()} (value from range 0xD4 to 0xDF)"
      line_content = f"{indent}#define {sym.str_value.upper()} (0x{start_id + index:02X}u)"
    else:
      continue  # Skip unsupported types

    # Append the generated line to the list
    appdecl_tsk_poll_kconfig_lines.append(line_content)
    index += 1

  # 5. Join the lines into a single string to insert into the target file
  kconfig_content = "\n".join(appdecl_tsk_poll_kconfig_lines)

  # 6. Find the position to insert in the target file
  with open(target, "r", encoding="utf-8") as f:
    file_content = f.read()

  if KCONFIG_DECL_TASK_POLL_START not in file_content or KCONFIG_DECL_TASK_POLL_END not in file_content:
    print(f"[ERROR] Marker {KCONFIG_DECL_TASK_POLL_START} not found in file {target}!")
    return False
  
  # 7. Split the file content into three parts: before the start marker, the new content, and after the end marker
  parts = file_content.split(KCONFIG_DECL_TASK_POLL_START)
  before_part = parts[0] + KCONFIG_DECL_TASK_POLL_START
  after_part = parts[1].split(KCONFIG_DECL_TASK_POLL_END)[1]

  # 8. Join the parts back together to form the new file content
  new_file_content = f"{before_part}\n\n{kconfig_content}\n\n{indent}{KCONFIG_DECL_TASK_POLL_END}{after_part}"

  # 9. Write the updated content back to the file
  with open(target, "w", encoding="utf-8") as f:
    f.write(new_file_content)

  return True

def sig_gen(kconf, target):
  # 1. Find the target file
  if not os.path.exists(target):
    print(f"[ERROR] File not found at sources/app/config: {target}")
    return False

  # 2. Generate appcfg content based on Kconfig configuration
  appdecl_sig_kconfig_lines = []
  
  # 3. Define indentation (here using 2 spaces, can be changed to "\t" if preferred)
  indent = "\t" 

  # 4. Define start id range from 0xD4 to 0xDF for task poll
  start_id = 0x01
  end_id = 0xFF
  index = 0

  # 5. Generate signal declarations based on Kconfig configuration
  for sym in kconf.unique_defined_syms:

    # Ignore blank or undefined symbols
    if sym.config_string == "" or sym.name.startswith("_"):
      continue

    # Only process symbols that start with "DECL_SIG_"
    if not sym.name.startswith("DECL_SIG_"):
      continue

    # Extract the relevant part of the symbol name after "DECL_SIG_"
    relevant_name = sym.name[len("DECL_"):]

    # Generate the line content based on the symbol type and value
    # Must be like DECL_SIG_{i}_NAME
    if not "_NAME" in relevant_name:
      continue  # Skip if the symbol name does not contain "_NAME_"
    if sym.type == kconfiglib.STRING:
      #line content is like "#define {sym.str_value.upper()} (value from range 0x01 to 0xFF)"
      line_content = f"{indent}#define {sym.str_value.upper()} (0x{start_id + index:02X}u)"
    else:
      continue  # Skip unsupported types

    # Append the generated line to the list
    appdecl_sig_kconfig_lines.append(line_content)
    index += 1

  # 5. Join the lines into a single string to insert into the target file
  kconfig_content = "\n".join(appdecl_sig_kconfig_lines)

  # 6. Find the position to insert in the target file
  with open(target, "r", encoding="utf-8") as f:
    file_content = f.read()

  if KCONFIG_DECL_SIGNAL_START not in file_content or KCONFIG_DECL_SIGNAL_END not in file_content:
    print(f"[ERROR] Marker {KCONFIG_DECL_SIGNAL_START} not found in file {target}!")
    return False
  
  # 7. Split the file content into three parts: before the start marker, the new content, and after the end marker
  parts = file_content.split(KCONFIG_DECL_SIGNAL_START)
  before_part = parts[0] + KCONFIG_DECL_SIGNAL_START
  after_part = parts[1].split(KCONFIG_DECL_SIGNAL_END)[1]

  # 8. Join the parts back together to form the new file content
  new_file_content = f"{before_part}\n\n{kconfig_content}\n\n{indent}{KCONFIG_DECL_SIGNAL_END}{after_part}"

  # 9. Write the updated content back to the file
  with open(target, "w", encoding="utf-8") as f:
    f.write(new_file_content)

  return True

def msg_queue_gen(kconf, target):
  # 1. Find the target file
  if not os.path.exists(target):
    print(f"[ERROR] File not found at sources/app/config: {target}")
    return False

  # 2. Generate appcfg content based on Kconfig configuration
  appdecl_msg_queue_kconfig_lines = []
  
  # 3. Define indentation (here using 2 spaces, can be changed to "\t" if preferred)
  indent = "\t" 

  # 4. Generate state transition content based on Kconfig configuration
  for sym in kconf.unique_defined_syms:

    # Ignore blank or undefined symbols
    if sym.config_string == "" or sym.name.startswith("_"):
      continue

    if sym.name == "CORE_UEDP_TASK_MSG_QUEUE_SIZE":
      if sym.type == kconfiglib.INT or sym.type == kconfiglib.HEX:
        queue_size = sym.str_value  # Lấy ra chuỗi số (ví dụ: "8")

    # Only process symbols that start with "DECL_MSG_QUEUE_"
    if not sym.name.startswith("DECL_MSG_QUEUE_"):
      continue

    # Extract the relevant part of the symbol name after "DECL_MSG_QUEUE_"
    relevant_name = sym.name[len("DECL_"):]

    # Generate the line content based on the symbol type and value
    # Must be like DECL_MSG_QUEUE_{i}_NAME, DECL_MSG_QUEUE_{i}_SIZE
    if not "_NAME" in relevant_name:
      continue  # Skip if the symbol name does not contain "_NAME_"
    if sym.type == kconfiglib.STRING:
      #line content is like "extern uedp_msg_t* b_q_mem[8];"
      line_content = f"{indent}extern uedp_msg_t* {sym.str_value.lower()}[{queue_size}];"
    else:
      continue  # Skip unsupported types

    # Append the generated line to the list
    appdecl_msg_queue_kconfig_lines.append(line_content)

  # 5. Join the lines into a single string to insert into the target file
  kconfig_content = "\n".join(appdecl_msg_queue_kconfig_lines)

  # 6. Find the position to insert in the target file
  with open(target, "r", encoding="utf-8") as f:
    file_content = f.read()

  if KCONFIG_DECL_MSG_QUEUE_START not in file_content or KCONFIG_DECL_MSG_QUEUE_END not in file_content:
    print(f"[ERROR] Marker {KCONFIG_DECL_MSG_QUEUE_START} not found in file {target}!")
    return False
  
  # 7. Split the file content into three parts: before the start marker, the new content, and after the end marker
  parts = file_content.split(KCONFIG_DECL_MSG_QUEUE_START)
  before_part = parts[0] + KCONFIG_DECL_MSG_QUEUE_START
  after_part = parts[1].split(KCONFIG_DECL_MSG_QUEUE_END)[1]

  # 8. Join the parts back together to form the new file content
  new_file_content = f"{before_part}\n\n{kconfig_content}\n\n{indent}{KCONFIG_DECL_MSG_QUEUE_END}{after_part}"

  # 9. Write the updated content back to the file
  with open(target, "w", encoding="utf-8") as f:
    f.write(new_file_content)

  return True

def tsk_norm_handler_gen(kconf, target):
  # 1. Find the target file
  if not os.path.exists(target):
    print(f"[ERROR] File not found at sources/app/config: {target}")
    return False

  # 2. Generate appcfg content based on Kconfig configuration
  appdecl_tsk_handler_kconfig_lines = []
  
  # 3. Define indentation (here using 2 spaces, can be changed to "\t" if preferred)
  indent = "\t" 

  # 4. Generate state transition content based on Kconfig configuration
  for sym in kconf.unique_defined_syms:

    # Ignore blank or undefined symbols
    if sym.config_string == "" or sym.name.startswith("_"):
      continue

    # Only process symbols that start with "DECL_TSK_HANDLER_"
    if not sym.name.startswith("DECL_NORM_HANDLER_"):
      continue

    # Extract the relevant part of the symbol name after "DECL_MSG_QUEUE_"
    relevant_name = sym.name[len("DECL_"):]

    # Generate the line content based on the symbol type and value
    # Must be like DECL_MSG_QUEUE_{i}_NAME, DECL_MSG_QUEUE_{i}_SIZE
    if not "_NAME" in relevant_name:
      continue  # Skip if the symbol name does not contain "_NAME_"
    if sym.type == kconfiglib.STRING:
      #line content is like "void task_norm_x_handler(uedp_msg_t* msg)"
      line_content = f"{indent}void {sym.str_value.lower()}(uedp_msg_t* msg);"
    else:
      continue  # Skip unsupported types

    # Append the generated line to the list
    appdecl_tsk_handler_kconfig_lines.append(line_content)

  # 5. Join the lines into a single string to insert into the target file
  kconfig_content = "\n".join(appdecl_tsk_handler_kconfig_lines)

  # 6. Find the position to insert in the target file
  with open(target, "r", encoding="utf-8") as f:
    file_content = f.read()

  if KCONFIG_DECL_TASK_NORM_HANDLER_START not in file_content or KCONFIG_DECL_TASK_NORM_HANDLER_END not in file_content:
    print(f"[ERROR] Marker {KCONFIG_DECL_TASK_NORM_HANDLER_START} not found in file {target}!")
    return False
  
  # 7. Split the file content into three parts: before the start marker, the new content, and after the end marker
  parts = file_content.split(KCONFIG_DECL_TASK_NORM_HANDLER_START)
  before_part = parts[0] + KCONFIG_DECL_TASK_NORM_HANDLER_START
  after_part = parts[1].split(KCONFIG_DECL_TASK_NORM_HANDLER_END)[1]

  # 8. Join the parts back together to form the new file content
  new_file_content = f"{before_part}\n\n{kconfig_content}\n\n{indent}{KCONFIG_DECL_TASK_NORM_HANDLER_END}{after_part}"

  # 9. Write the updated content back to the file
  with open(target, "w", encoding="utf-8") as f:
    f.write(new_file_content)

  return True

def tsk_poll_handler_gen(kconf, target):
  # 1. Find the target file
  if not os.path.exists(target):
    print(f"[ERROR] File not found at sources/app/config: {target}")
    return False

  # 2. Generate appcfg content based on Kconfig configuration
  appdecl_tsk_handler_kconfig_lines = []
  
  # 3. Define indentation (here using 2 spaces, can be changed to "\t" if preferred)
  indent = "\t" 

  # 4. Generate state transition content based on Kconfig configuration
  for sym in kconf.unique_defined_syms:

    # Ignore blank or undefined symbols
    if sym.config_string == "" or sym.name.startswith("_"):
      continue

    # Only process symbols that start with "DECL_TSK_HANDLER_"
    if not sym.name.startswith("DECL_POLL_HANDLER_"):
      continue

    # Extract the relevant part of the symbol name after "DECL_MSG_QUEUE_"
    relevant_name = sym.name[len("DECL_"):]

    # Generate the line content based on the symbol type and value
    # Must be like DECL_MSG_QUEUE_{i}_NAME, DECL_MSG_QUEUE_{i}_SIZE
    if not "_NAME" in relevant_name:
      continue  # Skip if the symbol name does not contain "_NAME_"
    if sym.type == kconfiglib.STRING:
      #line content is like "void task_poll_x_handler()"
      line_content = f"{indent}void {sym.str_value.lower()}();"
    else:
      continue  # Skip unsupported types

    # Append the generated line to the list
    appdecl_tsk_handler_kconfig_lines.append(line_content)

  # 5. Join the lines into a single string to insert into the target file
  kconfig_content = "\n".join(appdecl_tsk_handler_kconfig_lines)

  # 6. Find the position to insert in the target file
  with open(target, "r", encoding="utf-8") as f:
    file_content = f.read()

  if KCONFIG_DECL_TASK_POLL_HANDLER_START not in file_content or KCONFIG_DECL_TASK_POLL_HANDLER_END not in file_content:
    print(f"[ERROR] Marker {KCONFIG_DECL_TASK_POLL_HANDLER_START} not found in file {target}!")
    return False
  
  # 7. Split the file content into three parts: before the start marker, the new content, and after the end marker
  parts = file_content.split(KCONFIG_DECL_TASK_POLL_HANDLER_START)
  before_part = parts[0] + KCONFIG_DECL_TASK_POLL_HANDLER_START
  after_part = parts[1].split(KCONFIG_DECL_TASK_POLL_HANDLER_END)[1]

  # 8. Join the parts back together to form the new file content
  new_file_content = f"{before_part}\n\n{kconfig_content}\n\n{indent}{KCONFIG_DECL_TASK_POLL_HANDLER_END}{after_part}"

  # 9. Write the updated content back to the file
  with open(target, "w", encoding="utf-8") as f:
    f.write(new_file_content)

  return True

# Function to generate app_cfg.h declaration based on Kconfig configuration
def app_decl_gen(kconf, target):
  
  return True