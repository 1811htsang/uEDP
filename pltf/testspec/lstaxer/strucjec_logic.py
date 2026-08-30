import yaml

def validate_uEDP_structure(yaml_text):
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
                validate_actv_node(cur_task, finished_map, errors)
            
            # 2. Nếu là Task trong tlist -> Validate cấu trúc Task
            if in_tlist and len(path_stack) == 3:
                validate_task_node(cur_task, errors)
                
            # 3. Nếu là Item trong TSM/FSM -> Validate các tag đặc thù (id, trans...)
            if in_tlist and len(path_stack) == 5:
                validate_sub_node(cur_task, sub_type, finished_map, errors)

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

    print_report(errors)

def validate_actv_node(task, act_map, errors):
    """Kiểm tra cấu trúc bắt buộc của một Action Object"""
    tags = act_map["keys"]
    required = ['actv', 'to', 'sig', 'data', 'ptype']
    missing = [r for r in required if r not in tags]
    
    if missing:
        loc = f"Task: {task['name']} -> Action (Line:{act_map['line']})"
        errors.append({'loc': loc, 'msg': f"Action object missing tags: {', '.join(missing)}"})

def validate_task_node(task, errors):
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

def validate_sub_node(task, s_type, item, errors):
    tags = item["keys"]
    loc = f"Task {task['name']} -> {s_type} Item (L:{item['line']})"
    if s_type == 'TSM':
        for r in ['id', 'trans', 'on_ntry', 'on_actv', 'on_exit']:
            if r not in tags: errors.append({'loc': loc, 'msg': f"TSM missing '{r}'"})
    elif s_type == 'FSM':
        if 'id' not in tags or 'on_recv' not in tags:
            errors.append({'loc': loc, 'msg': f"FSM missing 'id' or 'on_recv'"})

def print_report(errors):
    print(f"{'TYPE':<10} | {'LOCATION':<45} | {'MESSAGE'}")
    print("-" * 105)
    if not errors: print("SUCCESS    | Structure is fully valid.")
    else:
        for e in errors: print(f"ERROR      | {e['loc']:<45} | {e['msg']}")

# Thực thi
# validate_uEDP_structure(your_yaml_string)

with open('sources/app/lstaxizer.yaml', 'r', encoding='utf-8') as f:
  yaml_sample = f.read()
validate_uEDP_structure(yaml_sample)