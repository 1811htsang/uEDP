import yaml

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

        # Kiểm tra nếu key là 'task'
        if event.value == 'task':
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
  if 'task' not in tags:
    errors.append({'loc': f"Line {item['line']}", 'msg': "Missing 'task' identifier tag."})

  # 2. Check bắt buộc: tsm/fsm vs exec
  has_logic = any(t in tags for t in ['tsm', 'fsm'])
  if not has_logic and 'exec' not in tags:
    errors.append({'loc': task_label, 'msg': "Must have either 'tsm', 'fsm', or 'exec' tag."})

  # 3. Check bắt buộc: anchor/alias
  if not item['has_anchor']:
    errors.append({'loc': task_label, 'msg': "Missing anchor definition or alias reference (e.g., <<: *anchor)."})
  
  # 4. Warnings: escal
  if 'escal' not in tags:
    warnings.append({'loc': task_label, 'msg': "Missing 'escal' tag. If not use, please set NULL"})

def strucjec_debug_tlist(errors, warnings):
  print(f"{'-'*30} strucjec `tlist` param  {'-'*30}\n")
  print(f"{'TYPE':<10} | {'LOCATION':<30} | {'MESSAGE'}")
  print("-" * 85)
  
  if not errors and not warnings:
    print(f"{'SUCCESS':<10} | {'All tasks':<30} | No issues found.")
  else:
    for err in errors:
      print(f"ERROR      | {err['loc']:<30} | {err['msg']}")
    for warn in warnings:
      print(f"WARNING    | {warn['loc']:<30} | {warn['msg']}")

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

def strucjec_debug_glbda(errors):
  print(f"{'-'*30} strucjec `glbda` param  {'-'*30}\n")
  print(f"{'TYPE':<10} | {'LOCATION':<30} | {'MESSAGE'}")
  print("-" * 85)
  
  if not errors:
    print(f"{'SUCCESS':<10} | {'Global Data Area':<30} | All GDA items are valid.")
  else:
    for err in errors:
      print(f"ERROR      | {err['loc']:<30} | {err['msg']}")

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

# STUB - sample usage to validate output
with open('sources/app/lstaxizer.yaml', 'r', encoding='utf-8') as f:
  yaml_sample = f.read()
errors_tlist, warnings_tlist = strucjec_target_tlist(yaml_sample)
strucjec_debug_tlist(errors_tlist, warnings_tlist)
errors_glbda = strucjec_target_glbda(yaml_sample)
strucjec_debug_glbda(errors_glbda)