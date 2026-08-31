import yaml

DEBUG_FLAG = True

def strucjec_target_tlist(yaml_text):
  events = yaml.parse(yaml_text)
  
  path_stack = []
  errors = []
  
  # Theo dõi Task hiện tại
  in_tlist = False
  cur_task = {"name": "Unknown", "type": None, "tags": set(), "line": 0, "has_anchor": False}
  
  # Ngăn xếp theo dõi các Mapping (để xử lý actv lồng nhau)
  # Mỗi phần tử: {"keys": set(), "is_actobj": False, "line": 0}
  map_stack = []
  
  # Context cho TSM/FSM/EXEC
  sub_type = None 
  waiting_for_val = None 

  for event in events:
    # --- BẮT ĐẦU MAPPING ---
    if isinstance(event, yaml.MappingStartEvent):
      path_stack.append("MAP")
      
      # 1. Khởi tạo tracker cho Mapping mới
      map_info = {
        "keys": set(),
        "is_actobj": False,
        "line": event.start_mark.line + 1
      }
      map_stack.append(map_info)

      # 2. Nhận diện bắt đầu Task trong tlist
      if in_tlist and len(path_stack) == 3:
        cur_task = {"name": "Unknown", "type": None, "tags": set(), 
              "line": event.start_mark.line + 1, "has_anchor": False}
        if event.anchor: cur_task["has_anchor"] = True

    # --- KẾT THÚC MAPPING ---
    elif isinstance(event, yaml.MappingEndEvent):
      # Lấy thông tin mapping vừa kết thúc
      finished_map = map_stack.pop()
      
      # 1. Nếu là Action Object -> Validate cấu trúc actv
      if finished_map["is_actobj"]:
        strucjec_target_atcvobj(cur_task, finished_map, errors)
      
      # 2. Nếu là Task trong tlist -> Validate cấu trúc Task
      if in_tlist and len(path_stack) == 3:
        strucjec_target_tnode(cur_task, errors)
        
      # 3. Nếu là Item trong TSM/FSM -> Validate các tag đặc thù (id, trans...)
      if in_tlist and len(path_stack) == 5:
        strucjec_target_sub_tnode(cur_task, sub_type, finished_map, errors)

      path_stack.pop()

    # --- BẮT ĐẦU SEQUENCE ---
    elif isinstance(event, yaml.SequenceStartEvent):
      path_stack.append("SEQ")

    # --- KẾT THÚC SEQUENCE ---
    elif isinstance(event, yaml.SequenceEndEvent):
      path_stack.pop()
      if in_tlist and len(path_stack) == 1: in_tlist = False

    # --- DỮ LIỆU (SCALAR) ---
    elif isinstance(event, yaml.ScalarEvent):
      val = event.value
      if len(path_stack) == 1 and val == 'tlist': in_tlist = True

      if in_tlist:
        # Ghi nhận key cho Mapping hiện tại
        if map_stack:
          map_stack[-1]["keys"].add(val)
          if val == 'actv': map_stack[-1]["is_actobj"] = True

        # Logic lấy tên Task và loại Task (tnorm/tpoll)
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

        # Check giá trị NULL cho tsm/fsm/exec
        if waiting_for_val and waiting_for_val.startswith("check_null_"):
          tag = waiting_for_val.split("_")[-1]
          if val is None or val.upper() == "NULL":
            cur_task["tags"].add(f"{tag}_IS_NULL")
          waiting_for_val = None

    # --- THAM CHIẾU ALIAS (*) ---
    elif isinstance(event, yaml.AliasEvent):
      if in_tlist and len(path_stack) == 3: cur_task["has_anchor"] = True

  return errors

def strucjec_target_atcvobj(task, act_map, errors):
  """Kiểm tra cấu trúc bắt buộc của một Action Object"""
  tags = act_map["keys"]
  required = ['actv', 'to', 'sig', 'data', 'ptype']
  missing = [r for r in required if r not in tags]
  
  if missing:
    loc = f"Task: {task['name']} -> Action (Line:{act_map['line']})"
    errors.append({'loc': loc, 'msg': f"Action object missing tags: {', '.join(missing)}"})

def strucjec_target_tnode(task, errors):
  t_label = f"{task['type'].upper()}: {task['name']} (L:{task['line']})"
  tags = task['tags']

  if not task['has_anchor']:
    errors.append({'loc': t_label, 'msg': "Missing anchor definition or alias (<<: *)."})

  if task['type'] == 'tnorm':
    # TNORM phải có logic: (tsm/fsm không NULL) HOẶC (exec không NULL)
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
  print(f"{'TYPE':<10} | {'LOCATION':<45} | {'MESSAGE'}")
  print("-" * 105)
  if not errors: print("SUCCESS  | Structure is fully valid.")
  else:
    for e in errors: print(f"{'ERROR':<10} | {e['loc']:<45} | {e['msg']}")

  print("\n")

def strucjec_target_glbda(yaml_text):
  """
  Hàm kiểm tra cấu trúc cho Global Data Area (glbda).
  Yêu cầu mỗi item phải có: name, type, initial_value.
  """
  events = yaml.parse(yaml_text)
  
  path_stack = []
  current_item = {}
  errors = []
  
  in_glbda = False
  waiting_for_val_of = None # Theo dõi để lấy giá trị của name/type...

  for event in events:
    # 1. Quản lý cấp độ (Stack)
    if isinstance(event, yaml.SequenceStartEvent):
      path_stack.append("SEQ")
    elif isinstance(event, yaml.MappingStartEvent):
      path_stack.append("MAP")
        
      # Nếu bắt đầu một item trong glbda (ROOT -> glbda -> Item)
      if in_glbda and len(path_stack) == 3:
        current_item = {
          'display_name': "Unknown GDA",
          'found_tags': set(),
          'line': event.start_mark.line + 1
        }
  
    elif isinstance(event, yaml.MappingEndEvent):
      # Kết thúc một item -> Validate
      if in_glbda and len(path_stack) == 3:
        strucjec_target_glbda_item(current_item, errors)
      path_stack.pop()
        
    elif isinstance(event, yaml.SequenceEndEvent):
      path_stack.pop()
      # Thoát khỏi vùng glbda
      if in_glbda and len(path_stack) == 1:
        in_glbda = False

    # 2. Xử lý dữ liệu (Scalar)
    elif isinstance(event, yaml.ScalarEvent):
      # Nhận diện tag glbda ở cấp gốc
      if len(path_stack) == 1 and event.value == 'glbda':
        in_glbda = True
        continue

      if in_glbda and len(path_stack) == 3:
        # Nếu đang chờ lấy giá trị cho một key cụ thể (ví dụ lấy tên của biến GDA)
        if waiting_for_val_of == 'name':
          current_item['display_name'] = event.value
          waiting_for_val_of = None
        
        # Lưu key vào bộ nhớ kiểm tra
        current_item['found_tags'].add(event.value)

        # Đánh dấu nếu key là 'name' để lượt sau lấy giá trị làm nhãn báo lỗi
        if event.value == 'name':
          waiting_for_val_of = 'name'
  return errors        

def strucjec_target_glbda_item(item, errors):
  """Logic kiểm tra chi tiết cho từng item GDA"""
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

def strucjec_target_isr(yaml_text):
  """
  Hàm kiểm tra cấu trúc cho Interrupt Service Routine (isr).
  Yêu cầu mỗi item phải có: id, to, sig.
  """
  events = yaml.parse(yaml_text)
  
  path_stack = []
  current_item = None
  errors = []
  
  in_isr = False
  waiting_for_id_val = False # Để lấy giá trị của trường 'id' làm nhãn báo lỗi
  is_key_turn = True         # Flag để phân biệt Key và Value trong Mapping

  for event in events:
    # 1. Quản lý cấp độ (Stack)
    if isinstance(event, yaml.SequenceStartEvent):
      path_stack.append("SEQ")
    elif isinstance(event, yaml.MappingStartEvent):
      path_stack.append("MAP")
        
      # Nếu bắt đầu một item trong isr (ROOT -> isr -> Item)
      if in_isr and len(path_stack) == 3:
        current_item = {
          'isr_id': "Unknown ISR",
          'found_keys': set(),
          'line': event.start_mark.line + 1
        }
        is_key_turn = True # Item mới luôn bắt đầu bằng một Key
    
    elif isinstance(event, yaml.MappingEndEvent):
      # Kết thúc một item -> Thực hiện Validate
      if in_isr and len(path_stack) == 3 and current_item:
        strucjec_target_isr_item(current_item, errors)
      path_stack.pop()
      
    elif isinstance(event, yaml.SequenceEndEvent):
      path_stack.pop()
      # Thoát khỏi vùng isr
      if in_isr and len(path_stack) == 1:
        in_isr = False

    # 2. Xử lý dữ liệu (Scalar)
    elif isinstance(event, yaml.ScalarEvent):
      # Nhận diện tag 'isr' ở cấp độ gốc (level 1)
      if len(path_stack) == 1 and event.value == 'isr':
        in_isr = True
        continue

      if in_isr and len(path_stack) == 3:
        if is_key_turn:
          # Đây là một KEY
          key_name = event.value
          current_item['found_keys'].add(key_name)
          
          if key_name == 'id':
            waiting_for_id_val = True
          
          is_key_turn = False # Sau Key sẽ là Value
        else:
          # Đây là một VALUE
          if waiting_for_id_val:
            current_item['isr_id'] = event.value
            waiting_for_id_val = False
          
          is_key_turn = True # Sau Value sẽ quay lại Key

    # 3. Xử lý Alias (Trường hợp sig: *sig7)
    elif isinstance(event, yaml.AliasEvent):
      if in_isr and len(path_stack) == 3:
        # Alias luôn đóng vai trò là một Value, nên tiếp theo sẽ là Key
        is_key_turn = True

  return errors

def strucjec_target_isr_item(item, errors):
  """Logic kiểm tra các trường bắt buộc của ISR"""
  keys = item['found_keys']
  isr_label = f"ISR: {item['isr_id']} (L:{item['line']})"
  
  required_fields = ['id', 'to', 'sig']
  
  for field in required_fields:
    if field not in keys:
      errors.append({
        'loc': isr_label,
        'msg': f"Missing required field: '{field}'"
      })

def strucjec_debug_isr(errors):
  print(f"{'-'*30} strucjec `isr` param  {'-'*32}\n")
  print(f"{'TYPE':<10} | {'LOCATION':<30} | {'MESSAGE'}")
  print("-" * 85)
  
  if not errors:
    print(f"{'SUCCESS':<10} | {'ISR Configuration':<30} | All ISR items are valid.")
  else:
    for err in errors:
      print(f"ERROR      | {err['loc']:<30} | {err['msg']}")

  print("\n")

def strucjec_target_outexec(yaml_text):
  """
  Hàm kiểm tra cấu trúc cho Out-Context Execution (outexec).
  Yêu cầu mỗi item phải có: name, handler, context, state.
  """
  events = yaml.parse(yaml_text)
  
  path_stack = []
  current_item = None
  errors = []
  
  in_outexec = False
  waiting_for_name_val = False 
  is_key_turn = True         

  for event in events:
    # 1. Quản lý cấp độ cấu trúc (Stack)
    if isinstance(event, yaml.SequenceStartEvent):
      path_stack.append("SEQ")
    elif isinstance(event, yaml.MappingStartEvent):
      path_stack.append("MAP")
        
      # Nếu bắt đầu một item trong outexec (ROOT -> outexec -> Item)
      if in_outexec and len(path_stack) == 3:
        current_item = {
          'oce_name': "Unknown OCE",
          'found_keys': set(),
          'line': event.start_mark.line + 1
        }
        is_key_turn = True
    
    elif isinstance(event, yaml.MappingEndEvent):
      # Kết thúc một item -> Thực hiện Validate
      if in_outexec and len(path_stack) == 3 and current_item:
        strucjec_target_outexec_item(current_item, errors)
      path_stack.pop()
        
    elif isinstance(event, yaml.SequenceEndEvent):
      path_stack.pop()
      # Thoát khỏi vùng outexec
      if in_outexec and len(path_stack) == 1:
        in_outexec = False

    # 2. Xử lý dữ liệu Scalar (Key/Value)
    elif isinstance(event, yaml.ScalarEvent):
      # Nhận diện tag 'outexec' ở cấp độ gốc
      if len(path_stack) == 1 and event.value == 'outexec':
        in_outexec = True
        continue

      if in_outexec and len(path_stack) == 3:
        if is_key_turn:
          # Đang ở vị trí KEY
          key_name = event.value
          current_item['found_keys'].add(key_name)
          
          if key_name == 'name':
            waiting_for_name_val = True
          
          is_key_turn = False
        else:
          # Đang ở vị trí VALUE
          if waiting_for_name_val:
            current_item['oce_name'] = event.value
            waiting_for_name_val = False
          
          is_key_turn = True

    # 3. Xử lý Alias (Nếu có tham chiếu *)
    elif isinstance(event, yaml.AliasEvent):
      if in_outexec and len(path_stack) == 3:
        is_key_turn = True

  return errors

def strucjec_target_outexec_item(item, errors):
  """Logic kiểm tra các trường bắt buộc cho OutExec"""
  keys = item['found_keys']
  oce_label = f"OCE: {item['oce_name']} (L:{item['line']})"
  
  # Danh sách các trường bắt buộc theo yêu cầu
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
  errors_isr = strucjec_target_isr(yaml_sample)
  errors_outexec = strucjec_target_outexec(yaml_sample)
  if DEBUG_FLAG:
    strucjec_debug_glbda(errors_glbda)
    strucjec_debug_tlist(errors_tlist)
    strucjec_debug_isr(errors_isr)
    strucjec_debug_outexec(errors_outexec)
  if errors_tlist or errors_glbda or errors_isr or errors_outexec:
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