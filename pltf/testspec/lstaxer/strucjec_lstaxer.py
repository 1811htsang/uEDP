import yaml

DEBUG_FLAG = True

def strucjec_target_tlist(yaml_text):
  events = yaml.parse(yaml_text)
    
  path_stack = []      
  errors = []
  
  in_tlist = False
  current_task = ""
  current_sub_item = {} # Lưu thông tin item của tsm/fsm đang xét
  
  # Trạng thái điều hướng
  context_type = None # 'TSM' hoặc 'FSM' hoặc None
  waiting_for_task_name = False
  waiting_for_on_recv_val = False

  for event in events:
    # --- QUẢN LÝ CẤU TRÚC STACK ---
    if isinstance(event, yaml.SequenceStartEvent):
      path_stack.append("SEQ")
    elif isinstance(event, yaml.MappingStartEvent):
      path_stack.append("MAP")
      
      # Cấp độ 4: Bắt đầu 1 object bên trong list tsm hoặc fsm
      if in_tlist and context_type in ['TSM', 'FSM'] and len(path_stack) == 5:
        current_sub_item = {
          'id': "Unknown ID",
          'found_tags': set(),
          'line': event.start_mark.line + 1,
          'on_recv_is_list': False
        }
      
      # Nếu on_recv bắt đầu bằng một mapping (không phải NULL)
      if waiting_for_on_recv_val:
        current_sub_item['on_recv_is_list'] = True # Coi như hợp lệ vì không phải NULL
        waiting_for_on_recv_val = False

    elif isinstance(event, yaml.MappingEndEvent):
      # Kết thúc 1 item của tsm/fsm -> Validate ngay
      if in_tlist and context_type in ['TSM', 'FSM'] and len(path_stack) == 5:
        strucjec_target_tlist_item(current_task, context_type, current_sub_item, errors)
      
      # Thoát khỏi vùng tsm/fsm list
      if len(path_stack) == 4: context_type = None
      path_stack.pop()
        
    elif isinstance(event, yaml.SequenceEndEvent):
      path_stack.pop()

    # --- XỬ LÝ DỮ LIỆU (SCALAR) ---
    elif isinstance(event, yaml.ScalarEvent):
      val = event.value
      
      # 1. Nhận diện task name
      if len(path_stack) == 1 and val == 'tlist': in_tlist = True
      if in_tlist and len(path_stack) == 3:
        if val == 'tnorm' or val == 'tpoll': waiting_for_task_name = True
        elif waiting_for_task_name:
          current_task = val
          waiting_for_task_name = False
      
      # 2. Nhận diện vùng TSM / FSM
      if in_tlist and len(path_stack) == 3:
        if val == 'tsm': context_type = 'TSM'
        if val == 'fsm': context_type = 'FSM'

      # 3. Thu thập tag trong item của TSM/FSM
      if in_tlist and context_type in ['TSM', 'FSM'] and len(path_stack) == 5:
        current_sub_item['found_tags'].add(val)
        if val == 'id': # Lấy ID để báo lỗi cho rõ
          # Giả định Scalar tiếp theo là giá trị ID
          pass 
        
        if val == 'on_recv':
          waiting_for_on_recv_val = True
        elif waiting_for_on_recv_val:
          # Nếu sau on_recv là một Scalar (như NULL), kiểm tra lỗi
          if val is None or val.upper() == 'NULL':
            current_sub_item['on_recv_is_list'] = False
          waiting_for_on_recv_val = False

    # --- XỬ LÝ SEQUENCE (Cho on_recv) ---
    elif isinstance(event, yaml.SequenceStartEvent):
      if waiting_for_on_recv_val:
        current_sub_item['on_recv_is_list'] = True
        waiting_for_on_recv_val = False

  return errors

def strucjec_target_tlist_item(task, type, item, errors):
  tags = item['found_tags']
  loc = f"Task: {task} -> {type} Item (Line:{item['line']})"
  
  if type == 'TSM':
    required = ['trans', 'on_ntry', 'on_actv', 'on_exit']
    for r in required:
      if r not in tags:
        errors.append({'loc': loc, 'msg': f"TSM item missing required tag: '{r}'"})
  
  elif type == 'FSM':
    if 'on_recv' not in tags:
      errors.append({'loc': loc, 'msg': "FSM item missing required tag: 'on_recv'"})
    elif not item.get('on_recv_is_list', False):
      errors.append({'loc': loc, 'msg': "Tag 'on_recv' in FSM must be a LIST (not NULL)."})

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