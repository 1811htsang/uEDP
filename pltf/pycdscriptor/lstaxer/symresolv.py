import yaml

def extract_yaml_metadata(yaml_text):
  events = yaml.parse(yaml_text)
  
  anchors = {}       
  path_stack = []    
  seq_index_stack = []
  
  current_key = None
  is_key_turn = True  

  for event in events:
    if isinstance(event, yaml.MappingStartEvent):
      if seq_index_stack:
        path_stack.append(f"[{seq_index_stack[-1]}]")
      elif current_key:
        path_stack.append(current_key)
      
      if event.anchor:
        anchors[event.anchor] = " -> ".join(path_stack) if path_stack else "ROOT"
      
      current_key = None
      is_key_turn = True

    elif isinstance(event, yaml.MappingEndEvent):
      if path_stack:
        path_stack.pop()
      if seq_index_stack:
        seq_index_stack[-1] += 1
      is_key_turn = True

    elif isinstance(event, yaml.SequenceStartEvent):
      if current_key:
        path_stack.append(current_key)
        
      if event.anchor:
        anchors[event.anchor] = " -> ".join(path_stack) if path_stack else "ROOT"
          
      seq_index_stack.append(0)
      current_key = None
      is_key_turn = True

    elif isinstance(event, yaml.SequenceEndEvent):
      if path_stack:
        path_stack.pop()
      seq_index_stack.pop()
      if seq_index_stack:
        seq_index_stack[-1] += 1
      is_key_turn = True

    elif isinstance(event, yaml.ScalarEvent):
      if is_key_turn:
        current_key = event.value
        if event.anchor:
          anchors[event.anchor] = " -> ".join(path_stack + [current_key])
        is_key_turn = False
      else:
        if current_key in ['task', 'id'] and path_stack:
          path_stack[-1] = f"{current_key}:{event.value}"
        
        if event.anchor:
          anchors[event.anchor] = " -> ".join(path_stack + [current_key])
        
        current_key = None
        is_key_turn = True

    elif isinstance(event, yaml.AliasEvent):
      is_key_turn = True

  return anchors

def trace_yaml_context(yaml_text):
  events = yaml.parse(yaml_text)
  
  path_stack = []        
  parent_type_stack = [] 
  seq_index_stack = []   
  results = []
  
  current_key = None
  is_key_turn = True

  for event in events:
    if isinstance(event, yaml.MappingStartEvent):
      if parent_type_stack and parent_type_stack[-1] == 'SEQ':
        path_stack.append(f"[{seq_index_stack[-1]}]")
      elif current_key:
        path_stack.append(current_key)
      
      parent_type_stack.append('MAP')
      current_key = None
      is_key_turn = True

    elif isinstance(event, yaml.MappingEndEvent):
      if path_stack:
        path_stack.pop()
      parent_type_stack.pop()
      if parent_type_stack and parent_type_stack[-1] == 'SEQ':
        seq_index_stack[-1] += 1
      is_key_turn = True

    elif isinstance(event, yaml.SequenceStartEvent):
      if parent_type_stack and parent_type_stack[-1] == 'SEQ':
        path_stack.append(f"[{seq_index_stack[-1]}]")
      elif current_key:
        path_stack.append(current_key)
      
      parent_type_stack.append('SEQ')
      seq_index_stack.append(0)
      current_key = None
      is_key_turn = True

    elif isinstance(event, yaml.SequenceEndEvent):
      if path_stack:
        path_stack.pop()
      parent_type_stack.pop()
      seq_index_stack.pop()
      if parent_type_stack and parent_type_stack[-1] == 'SEQ':
        seq_index_stack[-1] += 1
      is_key_turn = True

    elif isinstance(event, yaml.ScalarEvent):
      if is_key_turn:
        current_key = event.value
        is_key_turn = False
      else:
        # Logic định danh đặc biệt: task:NAME hoặc id:NAME
        # NOTE - cập nhật mới với task:NAME được phân tách thành tnorm:NAME hoặc tpoll:NAME
        if current_key in ['tnorm', 'tpoll', 'id'] and path_stack:
          path_stack[-1] = f"{current_key}:{event.value}"
        
        # Nếu Scalar này là một phần tử đơn trong List (không phải key-value)
        if parent_type_stack and parent_type_stack[-1] == 'SEQ':
          seq_index_stack[-1] += 1
                
        current_key = None
        is_key_turn = True

    elif isinstance(event, yaml.AliasEvent):
      full_trace = " -> ".join(path_stack)
      final_path = f"{full_trace} -> {current_key}" if current_key else full_trace
      
      results.append({
          'alias': event.anchor,
          'path': final_path
      })
      
      # Alias đóng vai trò là một Value, nếu nó nằm trong List thì tăng index
      if parent_type_stack and parent_type_stack[-1] == 'SEQ':
        seq_index_stack[-1] += 1
      is_key_turn = True

  return results

def symresolv_load(yaml_content):

  # NOTE - Reference resolution
  '''
  To resolve anchors and aliases, we should you normal I/O read
  instead of yaml loader, because the loader will resolve them automatically 
  and we won't be able to trace them.
  '''

  trace_results = trace_yaml_context(yaml_content)
  anchors = extract_yaml_metadata(yaml_content)
  return anchors, trace_results

def symresolv_debug(anchors, trace_results):
  print(f"{'-'*30} symresolv `anchor` param  {'-'*28}\n")
  print(f"\n{'INDEX':<6} | {'ANCHOR':<15} | {'DEFINITION TRACE PATH'}")
  print("-" * 60)
  i = 0
  for name, tag in anchors.items():
    print(f"i = {i:<2} | {name:<15} | {tag}")
    i += 1

  print('\n')
  print(f"{'-'*30} symresolv `alias` param  {'-'*30}\n")
  print(f"{'INDEX':<6} | {'ALIAS':<15} | {'FULL TRACE CONTEXT'}")
  print("-" * 60)
  i = 0
  for r in trace_results:
    print(f"i = {i:<2} | *{r['alias']:<14} | {r['path']}")
    i += 1

  print(f"\nTotal Anchors: {len(anchors)}")
  print(f"Total Aliases: {len(trace_results)}\n")