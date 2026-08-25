import yaml

DEBUG_FLAG = True

def strucjec_target_tlist(yaml_text):
  events = yaml.parse(yaml_text)
  
  path_stack = []      
  current_item = {}    
  errors = []
  warnings = []
  
  in_tlist = False
  # Biến hỗ trợ lấy tên Task
  waiting_for_task_name = False

  for event in events:
    # 1. Theo dõi cấu trúc
    if isinstance(event, yaml.SequenceStartEvent):
      path_stack.append("SEQ")
    elif isinstance(event, yaml.MappingStartEvent):
      path_stack.append("MAP")
      # Bắt đầu một Task mới trong tlist (ROOT -> tlist -> Item)
      if in_tlist and len(path_stack) == 3:
        current_item = {
          'task_name': "Unknown Task", # Mặc định nếu không tìm thấy key task
          'found_tags': set(),
          'has_anchor': False,
          'line': event.start_mark.line + 1
        }
        # Nếu chính Mapping có Anchor (&...)
        if event.anchor:
          current_item['has_anchor'] = True

    elif isinstance(event, yaml.MappingEndEvent):
      # Kết thúc một Task -> Tiến hành VALIDATE
      if in_tlist and len(path_stack) == 3:
        strucjec_target_tlist_item(current_item, errors, warnings)
      path_stack.pop()
        
    elif isinstance(event, yaml.SequenceEndEvent):
      path_stack.pop()
      if in_tlist and len(path_stack) == 1:
        in_tlist = False

    # 2. Xử lý dữ liệu (Scalar)
    elif isinstance(event, yaml.ScalarEvent):
      # Nhận diện vùng tlist
      if len(path_stack) == 1 and event.value == 'tlist':
        in_tlist = True
      
      # Nếu đang ở cấp độ thuộc tính của Task
      if in_tlist and len(path_stack) == 3:
        # Nếu vừa đọc được key 'task', thì Scalar này chính là giá trị tên Task
        if waiting_for_task_name:
          current_item['task_name'] = event.value
          waiting_for_task_name = False
        
        # Lưu các key để kiểm tra cấu trúc
        current_item['found_tags'].add(event.value)

        # Kiểm tra nếu key là 'tnorm' hoặc 'tpoll' 
        if event.value == 'tnorm' or event.value == 'tpoll':
          waiting_for_task_name = True
        
        # Kiểm tra merge key '<<' hoặc anchor trên scalar
        if event.value == '<<' or event.anchor:
          current_item['has_anchor'] = True

    # 3. Xử lý Alias (*)
    elif isinstance(event, yaml.AliasEvent):
      if in_tlist and len(path_stack) == 3:
        current_item['has_anchor'] = True

  return errors, warnings

def strucjec_target_tlist_item(item, errors, warnings):
  tags = item['found_tags']
  task_label = f"Task: {item['task_name']} (Line: {item['line']})"
  
  # 1. Check bắt buộc: task
  if 'tnorm' not in tags and 'tpoll' not in tags:
    errors.append({'loc': f"Line {item['line']}", 'msg': "Missing 'tnorm' or 'tpoll' identifier tag."})

  # 2. Check bắt buộc: tsm/fsm vs exec
  if 'tnorm' in tags:
    has_logic = any(t in tags for t in ['tsm', 'fsm'])
    if not has_logic and 'exec' not in tags:
      errors.append({'loc': task_label, 'msg': "tnorm is present but missing 'tsm'/'fsm' or 'exec' tag."})
  if 'tpoll' in tags:
    has_logic = any(t in tags for t in ['tsm', 'fsm'])
    if not has_logic and 'exec' not in tags:
      errors.append({'loc': task_label, 'msg': "tpoll is present but missing 'exec' tag."})

  # 3. Check bắt buộc: anchor/alias
  if not item['has_anchor']:
    errors.append({'loc': task_label, 'msg': "Missing anchor definition or alias reference (e.g., <<: *anchor)."})
  
  # 4. Warnings: escal
  if 'tnorm' in tags and 'escal' not in tags:
    warnings.append({'loc': task_label, 'msg': "Missing 'escal' tag. If not use, please set NULL"})

def strucjec_debug_tlist(errors, warnings):
  print(f"{'-'*30} strucjec `tlist` param  {'-'*30}\n")
  print(f"{'TYPE':<10} | {'LOCATION':<40} | {'MESSAGE'}")
  print("-" * 85)
  
  if not errors and not warnings:
    print(f"{'SUCCESS':<10} | {'All tasks':<40} | No issues found.")
  else:
    for err in errors:
      print(f"ERROR      | {err['loc']:<40} | {err['msg']}")
    for warn in warnings:
      print(f"WARNING    | {warn['loc']:<40} | {warn['msg']}")

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
  errors_tlist, warnings_tlist = strucjec_target_tlist(yaml_sample)
  errors_glbda = strucjec_target_glbda(yaml_sample)
  errors_isr = strucjec_target_isr(yaml_sample)
  errors_outexec = strucjec_target_outexec(yaml_sample)
  if DEBUG_FLAG:
    strucjec_debug_glbda(errors_glbda)
    strucjec_debug_tlist(errors_tlist, warnings_tlist)
    strucjec_debug_isr(errors_isr)
    strucjec_debug_outexec(errors_outexec)
  if warnings_tlist:
    print("[INFO] Structure validation completed with warnings.")
    print("[INFO] Please check the above warnings and consider fixing them in the YAML file.\n")
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