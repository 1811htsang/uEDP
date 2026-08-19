import yaml

def validate_tlist_structure(yaml_text):
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
        validate_item(current_item, errors, warnings)
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

  # --- IN KẾT QUẢ ---
  print(f"{'TYPE':<10} | {'LOCATION':<30} | {'MESSAGE'}")
  print("-" * 85)
  
  if not errors and not warnings:
    print(f"{'SUCCESS':<10} | {'All tasks':<30} | No issues found.")
  else:
    for err in errors:
      print(f"ERROR      | {err['loc']:<30} | {err['msg']}")
    for warn in warnings:
      print(f"WARNING    | {warn['loc']:<30} | {warn['msg']}")

def validate_item(item, errors, warnings):
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
    warnings.append({'loc': task_label, 'msg': "Missing 'escal' tag for emergency handling. If not use, please set NULL"})

with open('sources/app/lstaxizer.yaml', 'r', encoding='utf-8') as f:
  yaml_sample = f.read()
validate_tlist_structure(yaml_sample)