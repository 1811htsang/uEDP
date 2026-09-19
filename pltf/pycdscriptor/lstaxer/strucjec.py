import yaml
import pprint

DEBUG_FLAG = True

# DEPRECATED - Old TASK - Bổ sung cân nhắc thay thế việc sử dụng pytest + pydantic model để kiểm tra tối ưu hơn.
# STATUS - strucjec bắt buộc kiểm tra trước khi luukupmodel, vì strucjec sẽ kiểm tra cấu trúc syntax, còn luukupmodel sẽ kiểm tra logic và mapping.
def strucjec_target_tlist(yaml_text):
  events = yaml.parse(yaml_text)
  
  path_stack = []
  errors = []
  
  in_tlist = False
  cur_task = {"name": "Unknown", "type": None, "tags": set(), "line": 0, "has_anchor": False}
  
  map_stack = []
  
  sub_type = None 
  waiting_for_val = None 

  for event in events:
    if isinstance(event, yaml.MappingStartEvent):
      path_stack.append("MAP")
      
      map_info = {
        "keys": set(),
        "is_actobj": False,
        "line": event.start_mark.line + 1
      }
      map_stack.append(map_info)

      if in_tlist and len(path_stack) == 3:
        cur_task = {"name": "Unknown", "type": None, "tags": set(), 
              "line": event.start_mark.line + 1, "has_anchor": False}
        if event.anchor: cur_task["has_anchor"] = True

    elif isinstance(event, yaml.MappingEndEvent):
      finished_map = map_stack.pop()
      
      if finished_map["is_actobj"] and (
        finished_map["keys"] & {'to', 'sig', 'data', 'ptype'}
      ):
        strucjec_target_atcvobj(cur_task, finished_map, errors)
      
      if in_tlist and len(path_stack) == 3:
        strucjec_target_tnode(cur_task, errors)
        
      if in_tlist and len(path_stack) == 5:
        strucjec_target_sub_tnode(cur_task, sub_type, finished_map, errors)

      path_stack.pop()

    elif isinstance(event, yaml.SequenceStartEvent):
      path_stack.append("SEQ")

    elif isinstance(event, yaml.SequenceEndEvent):
      path_stack.pop()
      if in_tlist and len(path_stack) == 1: in_tlist = False

    elif isinstance(event, yaml.ScalarEvent):
      val = event.value
      if len(path_stack) == 1 and val == 'tlist': in_tlist = True

      if in_tlist:
        if map_stack:
          map_stack[-1]["keys"].add(val)
          if val == 'actv': map_stack[-1]["is_actobj"] = True

        if len(path_stack) == 3:
          if val in ['tnorm', 'tpoll']:
            cur_task["type"] = val
            waiting_for_val = "task_name"
          elif waiting_for_val == "task_name":
            cur_task["name"] = val
            waiting_for_val = None
          
          if val in ['tsm', 'fsm', 'exec']:
            cur_task["tags"].add(val)
            sub_type = val.upper()
            waiting_for_val = f"check_null_{val}"
          
          if val == '<<' or event.anchor: cur_task["has_anchor"] = True

        if waiting_for_val and waiting_for_val.startswith("check_null_"):
          tag = waiting_for_val.split("_")[-1]
          if val is None or val.upper() == "NULL":
            cur_task["tags"].add(f"{tag}_IS_NULL")
          waiting_for_val = None

    elif isinstance(event, yaml.AliasEvent):
      if in_tlist and len(path_stack) == 3: cur_task["has_anchor"] = True

  errors.extend(strucjec_validate_action_syntax(yaml_text))
  return errors

def strucjec_target_atcvobj(task, act_map, errors):
  tags = act_map["keys"]
  required = ['actv', 'to', 'sig', 'data', 'ptype']
  missing = [r for r in required if r not in tags]
  
  if missing:
    loc = f"Task: {task['name']} -> Action (Line:{act_map['line']})"
    errors.append({'loc': loc, 'msg': f"Action object missing tags: {', '.join(missing)}"})

def strucjec_validate_action_syntax(yaml_text):
  payload = yaml.safe_load(yaml_text) or {}
  errors = []

  for task in payload.get('tlist', []) or []:
    if not isinstance(task, dict):
      continue
    task_name = task.get('tnorm', task.get('tpoll', 'Unknown'))
    for action in _iter_action_objects(task):
      actv = action.get('actv')
      if isinstance(actv, dict):
        if 'kind' not in actv:
          errors.append({
            'loc': f"Task: {task_name} -> actv",
            'msg': "actv object missing required 'kind' field",
          })
        kind = actv.get('kind', actv.get('type'))
        if kind == 'c_stmt':
          if not isinstance(actv.get('code'), str) or not actv['code'].strip():
            errors.append({
              'loc': f"Task: {task_name} -> c_stmt",
              'msg': "c_stmt requires a non-empty 'code' field",
            })
        elif kind == 'c_call':
          if not isinstance(actv.get('function'), str) or not actv['function'].strip():
            errors.append({
              'loc': f"Task: {task_name} -> c_call",
              'msg': "c_call requires a non-empty 'function' field",
            })
          if 'args' in actv and not isinstance(actv['args'], list):
            errors.append({
              'loc': f"Task: {task_name} -> c_call",
              'msg': "c_call 'args' must be a list",
            })
      elif actv in ('c_stmt', 'c_call'):
        required = ['code'] if actv == 'c_stmt' else ['function']
        missing = [key for key in required if not action.get(key)]
        if missing:
          errors.append({
            'loc': f"Task: {task_name} -> {actv}",
            'msg': f"{actv} requires: {', '.join(missing)}",
          })

  return errors

def _iter_action_objects(value):
  if isinstance(value, dict):
    if 'actv' in value:
      yield value
    for child in value.values():
      yield from _iter_action_objects(child)
  elif isinstance(value, list):
    for child in value:
      yield from _iter_action_objects(child)

def strucjec_debug_action_syntax(errors):
  print(f"{'-'*30} strucjec `action syntax` param  {'-'*22}\n")
  print(f"{'TYPE':<10} | {'LOCATION':<30} | {'MESSAGE'}")
  print("-" * 85)
  
  if not errors:
    print(f"{'SUCCESS':<10} | {'Action Syntax Validation':<30} | All action objects are valid.")
  else:
    for err in errors:
      print(f"ERROR      | {err['loc']:<30} | {err['msg']}")

  print("\n")

def strucjec_target_tnode(task, errors):
  t_label = f"{task['type'].upper()}: {task['name']} (L:{task['line']})"
  tags = task['tags']

  if not task['has_anchor']:
    errors.append({'loc': t_label, 'msg': "Missing anchor definition or alias (<<: *)."})

  if task['type'] == 'tnorm':
    # NOTE - TNORM phải có logic: (tsm/fsm không NULL) HOẶC (exec không NULL)
    has_sm = ('tsm' in tags and 'tsm_IS_NULL' not in tags) or ('fsm' in tags and 'fsm_IS_NULL' not in tags)
    has_exec = 'exec' in tags and 'exec_IS_NULL' not in tags
    if not (has_sm or has_exec):
      errors.append({'loc': t_label, 'msg': "TNORM must have TSM/FSM or a valid 'exec' list."})
  
  elif task['type'] == 'tpoll':
    if 'exec' not in tags or 'exec_IS_NULL' in tags:
      errors.append({'loc': t_label, 'msg': "TPOLL must have a valid 'exec' list."})

def strucjec_target_sub_tnode(task, s_type, item, errors):
  tags = item["keys"]
  loc = f"Task {task['name']} -> {s_type} Item (L:{item['line']})"
  if s_type == 'TSM':
    for r in ['id', 'trans', 'on_ntry', 'on_actv', 'on_exit']:
      if r not in tags: errors.append({'loc': loc, 'msg': f"TSM missing '{r}'"})
  elif s_type == 'FSM':
    if 'id' not in tags or 'on_recv' not in tags:
      errors.append({'loc': loc, 'msg': f"FSM missing 'id' or 'on_recv'"})

def strucjec_debug_tlist(errors):
  print(f"{'TYPE':<10} | {'LOCATION':<30} | {'MESSAGE'}")
  print("-" * 85)
  if not errors: print(f"{'SUCCESS':<10} | Structure is fully valid.")
  else:
    for e in errors: print(f"{'ERROR':<10} | {e['loc']:<45} | {e['msg']}")

  print("\n")

def strucjec_target_glbda(yaml_text):
  events = yaml.parse(yaml_text)
  
  path_stack = []
  current_item = {}
  errors = []
  
  in_glbda = False
  waiting_for_val_of = None

  for event in events:
    if isinstance(event, yaml.SequenceStartEvent):
      path_stack.append("SEQ")
    elif isinstance(event, yaml.MappingStartEvent):
      path_stack.append("MAP")
        
      if in_glbda and len(path_stack) == 3:
        current_item = {
          'display_name': "Unknown GDA",
          'found_tags': set(),
          'line': event.start_mark.line + 1
        }
  
    elif isinstance(event, yaml.MappingEndEvent):
      if in_glbda and len(path_stack) == 3:
        strucjec_target_glbda_item(current_item, errors)
      path_stack.pop()
        
    elif isinstance(event, yaml.SequenceEndEvent):
      path_stack.pop()
      if in_glbda and len(path_stack) == 1:
        in_glbda = False

    elif isinstance(event, yaml.ScalarEvent):
      if len(path_stack) == 1 and event.value == 'glbda':
        in_glbda = True
        continue

      if in_glbda and len(path_stack) == 3:
        if waiting_for_val_of == 'name':
          current_item['display_name'] = event.value
          waiting_for_val_of = None
        
        current_item['found_tags'].add(event.value)

        if event.value == 'name':
          waiting_for_val_of = 'name'
  return errors        

def strucjec_target_glbda_item(item, errors):
  tags = item['found_tags']
  gda_label = f"GDA: {item['display_name']} (L:{item['line']})"  
  required_fields = ['name', 'type', 'initial_value']
  
  for field in required_fields:
    if field not in tags:
      errors.append({
        'loc': gda_label,
        'msg': f"Missing required field: '{field}'"
      })

def strucjec_debug_glbda(errors):
  print(f"{'-'*30} strucjec `glbda` param  {'-'*30}\n")
  print(f"{'TYPE':<10} | {'LOCATION':<30} | {'MESSAGE'}")
  print("-" * 85)
  
  if not errors:
    print(f"{'SUCCESS':<10} | {'Global Data Area':<30} | All GDA items are valid.")
  else:
    for err in errors:
      print(f"ERROR      | {err['loc']:<30} | {err['msg']}")
  
  print("\n")

# DEPRECATED - Old TASK - Remove ISR support in syntax and generator.
# STATUS - DONE

def strucjec_target_outexec(yaml_text):
  events = yaml.parse(yaml_text)
  
  path_stack = []
  current_item = None
  errors = []
  
  in_outexec = False
  waiting_for_name_val = False 
  is_key_turn = True         

  for event in events:
    if isinstance(event, yaml.SequenceStartEvent):
      path_stack.append("SEQ")
    elif isinstance(event, yaml.MappingStartEvent):
      path_stack.append("MAP")
        
      if in_outexec and len(path_stack) == 3:
        current_item = {
          'oce_name': "Unknown OCE",
          'found_keys': set(),
          'line': event.start_mark.line + 1
        }
        is_key_turn = True
    
    elif isinstance(event, yaml.MappingEndEvent):
      if in_outexec and len(path_stack) == 3 and current_item:
        strucjec_target_outexec_item(current_item, errors)
      path_stack.pop()
        
    elif isinstance(event, yaml.SequenceEndEvent):
      path_stack.pop()
      if in_outexec and len(path_stack) == 1:
        in_outexec = False

    elif isinstance(event, yaml.ScalarEvent):
      if len(path_stack) == 1 and event.value == 'outexec':
        in_outexec = True
        continue

      if in_outexec and len(path_stack) == 3:
        if is_key_turn:
          key_name = event.value
          current_item['found_keys'].add(key_name)
          
          if key_name == 'name':
            waiting_for_name_val = True
          
          is_key_turn = False
        else:
          if waiting_for_name_val:
            current_item['oce_name'] = event.value
            waiting_for_name_val = False
          
          is_key_turn = True

    elif isinstance(event, yaml.AliasEvent):
      if in_outexec and len(path_stack) == 3:
        is_key_turn = True

  return errors

def strucjec_target_outexec_item(item, errors):
  keys = item['found_keys']
  oce_label = f"OCE: {item['oce_name']} (L:{item['line']})"
  required_fields = ['name', 'handler', 'context', 'state']
  
  for field in required_fields:
    if field not in keys:
      errors.append({
        'loc': oce_label,
        'msg': f"Missing required field: '{field}'"
      })

def strucjec_debug_outexec(errors):
  print(f"{'-'*30} strucjec `outexec` param  {'-'*28}\n")
  print(f"{'TYPE':<10} | {'LOCATION':<30} | {'MESSAGE'}")
  print("-" * 85)
  
  if not errors:
    print(f"{'SUCCESS':<10} | {'OutExec Configuration':<30} | All OutExec items are valid.")
  else:
    for err in errors:
      print(f"ERROR      | {err['loc']:<30} | {err['msg']}")

  print("\n")

# NOTE - Outer function to call all structure validation functions
def strucjec_calib(yaml_sample):
  errors_tlist = strucjec_target_tlist(yaml_sample)
  errors_glbda = strucjec_target_glbda(yaml_sample)
  # DEPRECATED - Old TASK - Remove ISR support in syntax and generator.
  errors_outexec = strucjec_target_outexec(yaml_sample)
  errors_action_syntax = strucjec_validate_action_syntax(yaml_sample)
  if DEBUG_FLAG:
    strucjec_debug_glbda(errors_glbda)
    strucjec_debug_tlist(errors_tlist)
    # DEPRECATED - Old TASK - Remove ISR support in syntax and generator.
    strucjec_debug_outexec(errors_outexec)
    strucjec_debug_action_syntax(errors_action_syntax)
  if errors_tlist or errors_glbda or errors_outexec:
    print("[INFO] Structure validation completed with errors.")
    print("[INFO] Please check the above errors and fix them in the YAML file.")
    print("[INFO] Exiting with error.")
    # NOTE - Exit with error code
    exit(1)

# STUB - sample usage to validate output
# with open('sources/app/lstaxizer.yaml', 'r', encoding='utf-8') as f:
#   yaml_sample = f.read()
# errors_tlist, warnings_tlist = strucjec_target_tlist(yaml_sample)
# strucjec_debug_tlist(errors_tlist, warnings_tlist)
# errors_glbda = strucjec_target_glbda(yaml_sample)
# strucjec_debug_glbda(errors_glbda)
# errors_isr = strucjec_target_isr(yaml_sample)
# strucjec_debug_isr(errors_isr)
# errors_outexec = strucjec_target_outexec(yaml_sample)
# strucjec_debug_outexec(errors_outexec)