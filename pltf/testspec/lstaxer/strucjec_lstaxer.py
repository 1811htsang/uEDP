import yaml

DEBUG_FLAG = True

def strucjec_target_tlist(yaml_text):
  events = yaml.parse(yaml_text)
    
  path_stack = []        # ROOT -> tlist -> [idx] -> task_map
  errors = []
  
  # State tracking
  in_tlist = False
  current_task = "Unknown"
  
  # Task level containers
  task_metadata = {
    'has_tsm': False, 'tsm_null': True,
    'has_fsm': False, 'fsm_null': True,
    'has_exec': False, 'exec_is_list': False,
    'line': 0
  }
  
  # Sub-item tracking (TSM/FSM states)
  current_sub_item = {'keys': set(), 'line': 0, 'type': None}
  
  # on_recv tracking
  in_on_recv = False
  on_recv_item = {'keys': set(), 'has_steps': False, 'has_actv': False}

  for event in events:
    # --- QUẢN LÝ CẤU TRÚC (STACK) ---
    if isinstance(event, yaml.SequenceStartEvent):
      path_stack.append("SEQ")
    elif isinstance(event, yaml.SequenceEndEvent):
      path_stack.pop()
    elif isinstance(event, yaml.MappingStartEvent):
      path_stack.append("MAP")
      depth = len(path_stack)
      
      # Khởi tạo item trong tlist
      if in_tlist and depth == 3:
        task_metadata = {'has_tsm': False, 'tsm_null': True, 'has_fsm': False, 
                          'fsm_null': True, 'has_exec': False, 'exec_is_list': False, 
                          'line': event.start_mark.line + 1}
      
      # Khởi tạo item trong tsm/fsm list (Level 5)
      if in_tlist and depth == 5:
        current_sub_item = {'keys': set(), 'line': event.start_mark.line + 1}
      
      # Khởi tạo item trong on_recv list (Level 7)
      if in_on_recv and depth == 7:
        on_recv_item = {'keys': set(), 'has_steps': False, 'has_actv': False}

    elif isinstance(event, yaml.MappingEndEvent):
      depth = len(path_stack)
      # Kết thúc task -> Kiểm tra ràng buộc tsm/fsm/exec
      if in_tlist and depth == 3:
        strucjec_target_tlogic(current_task, task_metadata, errors)
      
      # Kết thúc tsm/fsm item
      if in_tlist and depth == 5:
        strucjec_target_sub_tlogic(current_task, current_sub_item, errors)
      
      # Kết thúc on_recv item
      if in_on_recv and depth == 7:
        strucjec_target_sub_tlogic_onrecv(current_task, on_recv_item, event.start_mark.line + 1, errors)
          
      path_stack.pop()

    # --- XỬ LÝ DỮ LIỆU (SCALAR) ---
    elif isinstance(event, yaml.ScalarEvent):
      val = event.value
      depth = len(path_stack)

      # Nhận diện vùng tlist
      if depth == 1 and val == 'tlist': in_tlist = True
      
      # Lấy tên Task (tnorm hoặc tpoll)
      if in_tlist and depth == 3:
        if val in ['tnorm', 'tpoll']: 
          # Giả định event tiếp theo là giá trị tên task
          pass 
        else: 
          # Nếu đây là VALUE của tnorm/tpoll
          if not any(k in path_stack for k in ['SEQ']): # Tránh nhầm với list con
            current_task = val

      # Kiểm tra tag tại task level
      if in_tlist and depth == 3:
        if val == 'tsm': task_metadata['has_tsm'] = True
        if val == 'fsm': task_metadata['has_fsm'] = True
        if val == 'exec': task_metadata['has_exec'] = True
        
        # Check nếu tsm/fsm có data (không phải NULL)
        if val in ['tsm', 'fsm', 'exec']:
          # Chúng ta sẽ check sự kiện tiếp theo để xem nó có phải SequenceStart không
          pass

      # Thu thập keys cho TSM/FSM state
      if in_tlist and depth == 5:
        current_sub_item['keys'].add(val)
        if val == 'on_recv': in_on_recv = True

      # Thu thập keys cho on_recv item
      if in_on_recv and depth == 7:
        on_recv_item['keys'].add(val)
        if val == 'steps': on_recv_item['has_steps'] = True
        if val == 'actv': on_recv_item['has_actv'] = True

    # --- KIỂM TRA KIỂU DỮ LIỆU (SEQUENCE VS NULL) ---
    elif isinstance(event, (yaml.SequenceStartEvent, yaml.ScalarEvent)):
      # Tự động nhận diện nếu tsm/fsm là List hay NULL dựa trên event tiếp theo
      pass 

  return errors

def strucjec_target_tlogic(task, meta, errors):
  loc = f"Task: {task} (Line:{meta['line']})"
  
  # Ràng buộc 1: Nếu không có tsm/fsm (hoặc NULL) -> Bắt buộc có exec
  # (Lưu ý: Logic kiểm tra NULL cần bắt qua ScalarEvent tiếp theo sau key tsm/fsm)
  if not meta['has_tsm'] and not meta['has_fsm']:
    if not meta['has_exec']:
      errors.append({'loc': loc, 'msg': "Must define 'exec' if 'tsm' and 'fsm' are absent."})

def strucjec_target_sub_tlogic(task, item, errors):
  keys = item['keys']
  loc = f"Task: {task} (Line:{item['line']})"
  
  # Kiểm tra TSM Item
  if 'on_actv' in keys or 'on_ntry' in keys: # Nhận diện đây là TSM
    for req in ['trans', 'on_ntry', 'on_actv', 'on_exit']:
      if req not in keys:
        errors.append({'loc': loc, 'msg': f"TSM State missing '{req}'"})
              
  # Kiểm tra FSM Item
  if 'on_recv' in keys:
    pass # Ràng buộc on_recv được check ở hàm riêng

def strucjec_target_sub_tlogic_onrecv(task, item, line, errors):
  keys = item['keys']
  loc = f"Task: {task} -> on_recv (Line:{line})"
  
  # Check sig, goto
  for req in ['sig', 'goto']:
    if req not in keys:
      errors.append({'loc': loc, 'msg': f"on_recv item missing '{req}'"})
  
  # Check steps XOR actv
  if not item['has_steps'] and not item['has_actv']:
    errors.append({'loc': loc, 'msg': "on_recv item must have either 'steps' (list) or 'actv' (scalar)."})

def strucjec_debug_tlist(errors):
  print(f"{'-'*30} strucjec `tlist` param  {'-'*30}\n")
  print(f"{'TYPE':<10} | {'LOCATION':<40} | {'MESSAGE'}")
  print("-" * 85)
  
  if not errors:
    print(f"{'SUCCESS':<10} | {'All tasks':<40} | No issues found.")
  else:
    for err in errors:
      print(f"ERROR      | {err['loc']:<40} | {err['msg']}")

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