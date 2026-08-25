from pydantic import BaseModel, field_validator
from typing import List, Optional, Dict
from .symresolv_lstaxer import symresolv_load, symresolv_debug
from .strucjec_lstaxer import strucjec_calib

# NOTE - Validation Strategy for lstaxer.vlid 
"""
1. Dangling Alias Check
2. Structure Validation
3. Context-Type Match
4. UST cross Reference
5. Post Resource Existance
6. Policy Alignment Check
"""

# NOTE - Dangling Alias Check 
"""
b4 lstaxer.symresolv is called,
ustab has already entered the yaml file to resolve data,
therefore, if there is any dangling alias, 
it will be detected by the yaml loader and raise an error.
So DAC can be skipped in lstaxer.vlid.
"""

# NOTE - Hardcode path to the YAML file for processing
with open('sources/app/lstaxizer.yaml', 'r', encoding='utf-8') as f:
  yaml_content = f.read()

# NOTE - Structure Validation
"""
after ustab finish DAC along with yaml loader, 
the yaml content is already loaded into memory.
Therefore, we can use the loaded content to perform structure validation.
"""

strucjec_calib(yaml_content)

# NOTE - Context-Type Match

"""
ANCHOR          | DEFINITION TRACE PATH
------------------------------------------------------------
sig1            | sigs -> 1

With anchor, source-side is the first element in the trace path

ALIAS           | FULL TRACE CONTEXT
------------------------------------------------------------
*sig1           | tlist -> task:TASK_USR -> tsm -> id:STATE_USR_IDLE-> trans -> [0] -> sig

With alias, target-side is the last element in the trace path
If both are not the same, then it is a context-type mismatch error. (CTM code)

However, exceptions are allowed for some special cases:

- glbda.var is able to be alive in `data` context.
- tnormX-ctrl is able to be alive in `<<` context.
- sig is able to be alive in `on_sig` context.

These exception are allowed but they are not interfered with each other, 
so if the source-side is `sig` and the target-side is `data`,
or if the glbda.var is used in `sig` context, then it is still a CTM error.

Another special case is that the target-side is `sig` context,
but the source-side is `sigs` context,
then it is valid because the sigs context is a superset of sig context.
"""

anchors, trace_results = symresolv_load(yaml_content)
symresolv_debug(anchors, trace_results)
symresolv_error = 0

for r in trace_results:
  alias_name = r['alias']
  full_trace = r['path']
  trace_elements = full_trace.split(' -> ')
  
  # Target-side is the last element in the alias trace path
  target_side = trace_elements[-1]
  
  # Source-side is the first element in the anchor trace path
  for name, tag in anchors.items():
    if name == alias_name:
      source_side = tag.split(' -> ')[0]
      break

  # NOTE - Debug to print the source-side and target-side for each alias
  # print(source_side, target_side)

  if target_side in ['data', '<<', 'on_sig']:
    # NOTE - Special case noted above
    # NOTE - not use break in this, this is diff from C-style switch-case
    match target_side:
      case 'data':
        if source_side != 'glbda':
          print(f"[ERROR] Alias *{alias_name} is defined in {source_side} context but used in {target_side} context.")
          symresolv_error += 1
      case '<<':
        if source_side != 'tnorms' and source_side != 'tpolls':
          print(f"[ERROR] Alias *{alias_name} is defined in {source_side} context but used in {target_side} context.")
          symresolv_error += 1
      case 'on_sig':
        if source_side != 'sigs':
          print(f"[ERROR] Alias *{alias_name} is defined in {source_side} context but used in {target_side} context.")
          symresolv_error += 1
      case _:
        print(f"[ERROR] Alias *{alias_name} is not exception case but used in {target_side} context.")
        symresolv_error += 1
  else:
    if source_side == 'sigs' and target_side == 'sig':
      # NOTE - Special case noted above
      continue
    else:
      # NOTE - Normal case
      if source_side != target_side:
        print(f"[ERROR] Alias *{alias_name} is defined in {source_side} context but used in {target_side} context.")
        symresolv_error += 1

if symresolv_error == 0:
  print("[INFO] All aliases are used in the correct context.")
else:
  # NOTE - Summary of errors
  print(f"[INFO] Total {symresolv_error} context-type mismatch errors found.")
  print("[INFO] Please check the above errors and fix them in the YAML file.")
  print("[INFO] Exiting with error.")
  # NOTE - Exit with error code
  exit(1)

# NOTE - UST cross Reference

"""
b4 users define logic in the YAML file,
ustab has already loaded the UST data into memory.
Therefore, data has already been guaranteed to be cross-referenced with UST data.
So UST cross reference check can be skipped in lstaxer.vlid.
However, UST can be considered to be implemented in the future
as it a good practice but not necessary for now.
"""

# NOTE - Post Resource Existance

"""
This part is to check if after the YAML file is loaded into memory,
all the resources defined in the YAML file are still exist in the UST data.
However, since the UST data is loaded into memory before the YAML file is loaded,
if any resource is deleted in the UST data, it will be detected by the YAML loader and raise an error.
So PRE can be skipped in lstaxer.vlid.
However, PRE can be considered to be implemented in the future
as it a good practice but not necessary for now.
"""