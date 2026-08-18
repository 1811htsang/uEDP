import yaml

def extract_yaml_metadata(yaml_text):
    """
    Trích xuất danh sách Anchor kèm theo đường dẫn định nghĩa chi tiết (Trace Path).
    """
    events = yaml.parse(yaml_text)
    
    anchors = {}         # Kết quả: {tên_anchor: trace_path}
    path_stack = []      # Lưu chuỗi các key/index đang duyệt
    seq_index_stack = [] # Lưu chỉ số index nếu đang ở trong list
    
    current_key = None
    is_key_turn = True   # Flag xác định Scalar tiếp theo là Key hay Value

    for event in events:
        # 1. Xử lý BẮT ĐẦU Mapping (Object)
        if isinstance(event, yaml.MappingStartEvent):
            # Cập nhật đường dẫn dựa trên key cha hoặc chỉ số mảng
            if seq_index_stack:
                path_stack.append(f"[{seq_index_stack[-1]}]")
            elif current_key:
                path_stack.append(current_key)
            
            # Nếu object này có Anchor (ví dụ: &tnorm1-ctrl)
            if event.anchor:
                anchors[event.anchor] = " -> ".join(path_stack) if path_stack else "ROOT"
            
            current_key = None
            is_key_turn = True

        # 2. Xử lý KẾT THÚC Mapping
        elif isinstance(event, yaml.MappingEndEvent):
            if path_stack:
                path_stack.pop()
            if seq_index_stack:
                seq_index_stack[-1] += 1
            is_key_turn = True

        # 3. Xử lý BẮT ĐẦU Sequence (List)
        elif isinstance(event, yaml.SequenceStartEvent):
            if current_key:
                path_stack.append(current_key)
            
            # Nếu list này có Anchor
            if event.anchor:
                anchors[event.anchor] = " -> ".join(path_stack) if path_stack else "ROOT"
                
            seq_index_stack.append(0)
            current_key = None
            is_key_turn = True

        # 4. Xử lý KẾT THÚC Sequence
        elif isinstance(event, yaml.SequenceEndEvent):
            if path_stack:
                path_stack.pop()
            seq_index_stack.pop()
            if seq_index_stack:
                seq_index_stack[-1] += 1
            is_key_turn = True

        # 5. Xử lý Scalar (Dữ liệu đơn lẻ)
        elif isinstance(event, yaml.ScalarEvent):
            if is_key_turn:
                # Đây là một KEY
                current_key = event.value
                # Nếu Anchor nằm ở vị trí KEY
                if event.anchor:
                    anchors[event.anchor] = " -> ".join(path_stack + [current_key])
                is_key_turn = False
            else:
                # Đây là một VALUE
                # Logic định danh context: cập nhật task:NAME hoặc id:NAME vào path_stack
                if current_key in ['task', 'id'] and path_stack:
                    path_stack[-1] = f"{current_key}:{event.value}"
                
                # Nếu Anchor nằm ở vị trí VALUE (ví dụ: data: &gda_status ...)
                if event.anchor:
                    anchors[event.anchor] = " -> ".join(path_stack + [current_key])
                
                current_key = None
                is_key_turn = True

        # 6. Alias (Dấu *) - Không xử lý trong hàm này vì ta chỉ cần tìm nơi Định nghĩa (&)
        elif isinstance(event, yaml.AliasEvent):
            is_key_turn = True

    return anchors

def trace_yaml_context(yaml_text):
    events = yaml.parse(yaml_text)
    
    path_stack = []        # Lưu chuỗi các tag (key hoặc [index])
    parent_type_stack = [] # Lưu loại node cha ('MAP' hoặc 'SEQ')
    seq_index_stack = []   # Chỉ số index dành riêng cho List
    results = []
    
    current_key = None
    is_key_turn = True

    for event in events:
        # --- BẮT ĐẦU MAPPING (Dictionary) ---
        if isinstance(event, yaml.MappingStartEvent):
            # Nếu cha trực tiếp là một List -> thêm [index] vào path
            if parent_type_stack and parent_type_stack[-1] == 'SEQ':
                path_stack.append(f"[{seq_index_stack[-1]}]")
            # Nếu cha trực tiếp là một Dict -> thêm tên key vào path
            elif current_key:
                path_stack.append(current_key)
            
            parent_type_stack.append('MAP')
            current_key = None
            is_key_turn = True

        # --- KẾT THÚC MAPPING ---
        elif isinstance(event, yaml.MappingEndEvent):
            if path_stack:
                path_stack.pop()
            parent_type_stack.pop()
            # Nếu vừa xong một phần tử trong List, tăng index của List đó lên
            if parent_type_stack and parent_type_stack[-1] == 'SEQ':
                seq_index_stack[-1] += 1
            is_key_turn = True

        # --- BẮT ĐẦU SEQUENCE (List) ---
        elif isinstance(event, yaml.SequenceStartEvent):
            if parent_type_stack and parent_type_stack[-1] == 'SEQ':
                path_stack.append(f"[{seq_index_stack[-1]}]")
            elif current_key:
                path_stack.append(current_key)
            
            parent_type_stack.append('SEQ')
            seq_index_stack.append(0) # Khởi tạo bộ đếm cho List mới
            current_key = None
            is_key_turn = True

        # --- KẾT THÚC SEQUENCE ---
        elif isinstance(event, yaml.SequenceEndEvent):
            if path_stack:
                path_stack.pop()
            parent_type_stack.pop()
            seq_index_stack.pop()
            # Nếu List này nằm trong một List cha, tăng index của List cha
            if parent_type_stack and parent_type_stack[-1] == 'SEQ':
                seq_index_stack[-1] += 1
            is_key_turn = True

        # --- DỮ LIỆU ĐƠN (Scalar) ---
        elif isinstance(event, yaml.ScalarEvent):
            if is_key_turn:
                current_key = event.value
                is_key_turn = False
            else:
                # Logic định danh đặc biệt: task:NAME hoặc id:NAME
                if current_key in ['task', 'id'] and path_stack:
                    path_stack[-1] = f"{current_key}:{event.value}"
                
                # Nếu Scalar này là một phần tử đơn trong List (không phải key-value)
                if parent_type_stack and parent_type_stack[-1] == 'SEQ':
                    seq_index_stack[-1] += 1
                        
                current_key = None
                is_key_turn = True

        # --- ALIAS (Dấu *) ---
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

# --- Kiểm tra ---
yaml_sample = """
sigs:
  '1': &sig1
    hex_val: '0x1'
    id_symbol: SIG_USR_START
  '2': &sig2
    hex_val: '0x2'
    id_symbol: SIG_USR_STOP
  '3': &sig3
    hex_val: '0x3'
    id_symbol: SIG_0X34
  '4': &sig4
    hex_val: '0x4'
    id_symbol: SIG_0XFF
  '5': &sig5
    hex_val: '0x5'
    id_symbol: SIG_0X12
  '6': &sig6
    hex_val: '0x6'
    id_symbol: SIG_0XAA
  '7': &sig7
    hex_val: '0x7'
    id_symbol: SIG_HALT_NOW
tnorms:
  '1': &tnorm1-ctrl
    fsm_resrc: NULL
    handler: tnorm_usr_nhler
    hex_val: '0xe6'
    id_symbol: TASK_USR_IDS
    queue_name: tnorm_usr_msgq
    tsm_resrc:
      object: tnorm_usr_tsmobj
      state_trans:
      - tnorm_usr_state_idle_trans
      - tnorm_usr_state_running_trans
      states:
      - tnorm_usr_state_idle
      - tnorm_usr_state_running
      table: tnorm_usr_tsmobj_tbl
  '2': &tnorm2-ctrl
    fsm_resrc: NULL
    handler: tnorm_a_nhler
    hex_val: '0xe7'
    id_symbol: TASK_A_IDS
    queue_name: tnorm_a_msgq
    tsm_resrc:
      object: tnorm_a_tsmobj
      state_trans:
      - tnorm_a_idle_trans
      - tnorm_a_state_waitb_trans
      states:
      - tnorm_a_idle
      - tnorm_a_state_waitb
      table: tnorm_a_tsmobj_tbl
  '3': &tnorm3-ctrl
    fsm_resrc:
      object: tnorm_b_tsmobj
      states:
      - tnorm_b_state_idle
      - tnorm_b_state_busy
    tsm_resrc: NULL
    handler: tnorm_b_nhler
    hex_val: '0xe8'
    id_symbol: TASK_B_IDS
    queue_name: tnorm_b_msgq
tpolls: {}

# Anchor: sử dụng &<str> để định danh các phần tử 
# >> sigs: &siglst
# >>  '1', '2' of sigs: &sig1, &sig2, ...
# >> tnorms: &tnormlst
# >>  '1', '2' of tnorms: &tnorm1, &tnorm2, ...
# >>  fsm_resrc of '1', '2': &tnorm1-fsmobj, &tnorm2-fsmobj, ...
# >>    states of '1'[fsm_resrc], '2'[fsm_resrc]: &tnorm1-fsmstates, &tnorm2-fsmstates, ...
# >>  tsm_resrc of '1', '2': &tnorm1-tsmobj, &tnorm2-tsmobj, ...
# >>    states of '1'[tsm_resrc], '2'[tsm_resrc]: &tnorm1-tsmstates, &tnorm2-tsmstates, ...
# >>    state trans of '1', '2': &tnorm1-tsmtrans, &tnorm2-tsmtrans, ...
# >> tpolls: &tpolllst
# >>  '1', '2' of tpolls: &tpoll1, &tpoll2, ...
# Alias: sử dụng *<str> để tham chiếu đến các phần tử đã định danh

# NOTE - Triển khai logic bắt đầu từ dòng này

pject: "uEDP_PingPong_Full_Validation"
versh: "1.2.0-pre"

# ANCHOR - Global Data Area (GDA) - Biến toàn cục

glbda:
- '1': &gda_status
  name: GDA_SYSTEM_STATUS
  type: "const char*"
  initial_value: "STATUS: INITIALIZING"

# ANCHOR - Task List (TLIST)

tlist:
- task: TASK_USR
  tsm:
  - id: STATE_USR_IDLE
    trans:
    - sig: *sig1
      goto: STATE_USR_RUNNING
    on_ntry: # NOTE - logic "khởi tạo" (Entry)
      steps:
      - actv: uedp_itnlog_init()
        to: NULL
        sig: NULL
        data: NULL
      - actv: uedp_itnlog_set_output(pal_logdp_dispatch)
        to: NULL
        sig: NULL
        data: NULL
    on_actv: #NOTE - logic "chạy" (Active)
      steps:
      - actv: uedp_itnlog_log
        to: NULL
        sig: NULL
        data: "[USR][IDLE] Starting sequence..."
      - actv: uedp_task_norm_post_msg
        to: TASK_A
        sig: *sig1
    on_exit: # NOTE - logic "kết thúc" (Exit)
      actv: uedp_itnlog_log
      to: NULL
      sig: NULL
      data: "[USR][IDLE] Sequence finished."
  - id: STATE_USR_RUNNING
    trans:
    - sig: *sig2
      goto: STATE_USR_IDLE
    on_ntry:
      actv: uedp_itnlog_log
      to: NULL
      sig: NULL
      data: "[USR][RUNNING] Sequence started."
    on_actv:
      steps:
      - actv: uedp_itnlog_log
        to: NULL
        sig: NULL
        data: "[USR][RUNNING] Posting message to TASK_A..."
      - actv: uedp_task_norm_post_msg
        to: TASK_A
        sig: *sig1
        data: *gda_status   # Validator check: tham chiếu anchor hợp lệ
        ptype: REF          # D2MP: Truyền địa chỉ chuỗi status
    on_exit:
      steps:
      - actv: ocesvc_register
        to: NULL
        sig: NULL
        data: itnlog_dump_svc
  << : *tnorm1-ctrl
- task: TASK_A
  tsm:
  - id: STATE_A_IDLE
    trans:
    - sig: *sig1
      goto: STATE_A_WAITING_B
    on_ntry: NULL
    on_actv: NULL
    on_exit: NULL
  - id: STATE_A_WAITING_B
    trans:
    - sig: *sig3
      goto: STAY            # Validator check: logic "bỏ qua" (Stay)
    - sig: *sig4
      goto: STATE_A_IDLE
    on_ntry:
      actv: uedp_post_msg
      to: TASK_B
      sig: *sig5
      data: NULL
    on_actv:
      steps:                # Chuyển trạng thái kèm hành động phức tạp
      - actv: uedp_post_msg
        to: TASK_B
        sig: *sig6
        data: NULL
      - actv: uedp_itnlog_log
        data: "[TASK_A][WAITING_B] Posted SIG_0xAA to TASK_B."
    on_exit: NULL
  escal: # Cấu hình APE để xử lý khẩn cấp (nếu có)
    mode: slnf
    trigger:
    - on_sig: *sig7
      post_urgent: NULL       # Chạy tiếp queue hiện tại với ưu tiên cao
  << : *tnorm2-ctrl
- task: TASK_B
  fsm:
  - id: STATE_B_IDLE
    on_recv:
    - sig: *sig5
      goto: STATE_B_BUSY
      steps:
      - actv: uedp_task_norm_post_msg
        to: TASK_A
        sig: *sig3
      - actv: uedp_task_norm_post_msg
        to: TASK_A
        sig: *sig4
  - id: STATE_B_BUSY
    on_recv:
    - sig: *sig6
      goto: STATE_B_IDLE
      steps:
      - actv: uedp_task_norm_post_msg
        to: TASK_USR
        sig: *sig2
      - actv: uedp_itnlog_log
        data: *gda_status # Ghi log nội dung từ biến toàn cục
        ptype: VAL        # Validator check: Copy nội dung (memcpy)
  << : *tnorm3-ctrl

# ANCHOR - Interrupt Service Routine (ISR) - Xử lý ngắt

isr:
- id: ISR_HARD_STOP
  to: TASK_A
  sig: *sig7

# ANCHOR - Out-Context Execution (OUTEXEC) - Thực thi ngoài ngữ cảnh

outexec:
- name: OCE_ITNLOG_DUMP
  handler: itnlog_dump_handler
  context: NULL
  state: READY
"""

trace_results = trace_yaml_context(yaml_sample)
anchors = extract_yaml_metadata(yaml_sample)

print("--- ANCHORS ---")
for name, tag in anchors.items():
    print(f"{name:<15} | {tag}")

print(f"\n{'ALIAS':<15} | {'FULL TRACE CONTEXT'}")
print("-" * 60)
for r in trace_results:
    print(f"*{r['alias']:<14} | {r['path']}")