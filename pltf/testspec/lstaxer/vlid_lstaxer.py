from pydantic import BaseModel, field_validator
from typing import List, Optional, Dict
from .symresolv_lstaxer import symresolv_load, symresolv_debug

# NOTE - Validation Strategy for lstaxer.vlid 
"""
1. Dangling Alias Check
2. Context-Type Match
3. UST cross Reference
4. Post Resource Existance
5. Policy Alignment Check
"""

# NOTE - Dangling Alias Check 
"""
b4 lstaxer.symresolv is called,
ustab has already entered the yaml file to resolve data,
therefore, if there is any dangling alias, 
it will be detected by the yaml loader and raise an error.
So DAC can be skipped in lstaxer.vlid.
"""

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

anchors, trace_results = symresolv_load()
symresolv_debug(anchors, trace_results)

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
      case '<<':
        if source_side != 'tnorms':
          print(f"[ERROR] Alias *{alias_name} is defined in {source_side} context but used in {target_side} context.")
      case 'on_sig':
        if source_side != 'sigs':
          print(f"[ERROR] Alias *{alias_name} is defined in {source_side} context but used in {target_side} context.")
      case _:
        print(f"[ERROR] Alias *{alias_name} is not exception case but used in {target_side} context.")
  else:
    if source_side == 'sigs' and target_side == 'sig':
      # NOTE - Special case noted above
      continue
    else:
      # NOTE - Normal case
      if source_side != target_side:
        print(f"[ERROR] Alias *{alias_name} is defined in {source_side} context but used in {target_side} context.")
  
