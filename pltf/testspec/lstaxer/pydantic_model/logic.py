from pydantic import BaseModel, field_validator
from typing import List, Optional, Dict

# LINK - sources/app/lstaxizer.yaml
# NOTE - This file is used to check against lstaxer.vlid

'''
# STUB - TNORM Logic Stub with TSM for YAML file
- task: TASK_USR -> str
  tsm: -> must have 'id', 'trans', 'on_ntry', 'on_actv', 'on_exit'
  - id: STATE_USR_IDLE -> str
    trans: -> must have 'sig', 'goto' = Dict[str, str]
    - sig: *sig1 -> str
      goto: STATE_USR_RUNNING -> str
    on_ntry: # NOTE - logic "khởi tạo" (Entry) -> can be 'steps' or 'actv'
      steps: -> If use steps, then it must have 'actv', 'to', 'sig', 'data' = List[Dict[str, Optional[str], Optional[str], Optional[str]]]. 'to', 'sig', 'data' can be None, but 'actv' must have value.
      - actv: uedp_itnlog_init() -> str
        to: NULL -> Optional[str] = None
        sig: NULL -> Optional[str] = None
        data: NULL -> Optional[str] = None
      - actv: uedp_itnlog_set_output(pal_logdp_dispatch)
        to: NULL -> Optional[str] = None
        sig: NULL -> Optional[str] = None
        data: NULL -> Optional[str] = None
    on_actv: #NOTE - logic "chạy" (Active) -> Same as on_ntry, can be 'steps' or 'actv'
      steps:
      - actv: uedp_itnlog_log
        to: NULL
        sig: NULL
        data: "[USR][IDLE] Starting sequence..."
      - actv: uedp_task_norm_post_msg
        to: TASK_A
        sig: *sig1
    on_exit: # NOTE - logic "kết thúc" (Exit) -> In this case, it is only actv, but must have 'actv', 'to', 'sig', 'data' = Dict[str, Optional[str], Optional[str], Optional[str]]. 'to', 'sig', 'data' can be None, but 'actv' must have value.
      actv: uedp_itnlog_log -> str
      to: NULL -> Optional[str] = None
      sig: NULL -> Optional[str] = None
      data: "[USR][IDLE] Sequence finished." -> Optional[str] = None
  - id: STATE_USR_RUNNING -> Same as STATE_USR_IDLE above.
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
  << : *tnorm1-ctrl -> str. Check for anchor reference to tnorm1-ctrl, which is defined in the YAML file.
Concentrate:
TNORM Logic with TSM should have type below:
- task: str
- tsm -> Can be None, but if it is present:
  - id: str
    - trans: Dict[str, str]
    - on_ntry/on_actv/on_exit: A = List[Dict[str, Optional[str], Optional[str], Optional[str]]] or B = Dict[str, Optional[str], Optional[str], Optional[str]]
  -> Therefore, Dict[str, Dict[str, str], A or B] = tsm_item
-> Therefore, List[tsm_item] = tsm_list = tsm
Add Optional to tsm, because it can be None if not used.
-> Therefore, Optional[tsm_list] = tsm
<< : *tnorm1-ctrl -> str
# ANCHOR - -> Therefore, Dict[str, tsm, str] = tnorm_item
'''
'''
# STUB - TNORM Logic Stub with FSM for YAML file
- task: TASK_B -> str
  fsm: List[fsm_item] = fsm_list
  - id: STATE_B_IDLE -> str
    on_recv: -> List[on_recv_item] = on_recv_list
    - sig: *sig5 -> str
      goto: STATE_B_BUSY -> str
      steps: -> List[step_item] = steps_list
      - actv: uedp_task_norm_post_msg -> str
        to: TASK_A -> Optional[str] = None
        sig: *sig3 -> Optional[str] = None
      - actv: uedp_task_norm_post_msg -> str
        to: TASK_A -> Optional[str] = None
        sig: *sig4 -> Optional[str] = None
        -> Dict[str, Optional[str], Optional[str]] = step_item
    -> Dict[str, str, steps_list] = on_recv_item
  -> Dict[str, on_recv_list] = fsm_item
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
        ptype: VAL        # Validator check: Copy nội dung (memcpy) -> Optional[str] = None. Add to ptype to indicate that it is a value copy, not a reference.
        -> step_item.add(data = Optional[str] = None, ptype = Optional[str] = None)
  << : *tnorm3-ctrl
Concentrate:
TNORM Logic with FSM should have type below:
- task: str
- fsm -> List[Dict[str, on_recv_list]] = fsm_list = fsm -> Optional[fsm] = None
  - id: str
    - on_recv: -> Dict[str, str, steps_list] = on_recv_item
      - sig: str
        goto: str
        - steps: -> List[step_item] = steps_list
          - actv: str
            to: Optional[str] = None
            sig: Optional[str] = None
            data: Optional[str] = None
            ptype: Optional[str] = None
            -> Dict[str, Optional[str], Optional[str], Optional[str], Optional[str]] = step_item
  -> Therefore, Dict[str, Dict[str, str], A or B] = tsm_item
<< : *tnorm1-ctrl -> str
# ANCHOR - -> Therefore, Dict[str, fsm, str] = tnorm_item
'''
'''
# STUB - TNORM Logic Stub with non-HSMC for YAML file
- task: KID_TASK_SIMPLE
  exec: -> List[Dict[str, List[Dict[str | actv, Optional[str] | to, Optional[str] | sig, Optional[str] | data, Optional[str] | ptype]]]]] = exec_list
  - on_sig: SIG_A
    steps: -> Step is same as above.
    - actv: post_msg
      to: KID_TASK_B
      sig: SIG_B
      data: NULL
      //NOTE - add ptype key.
    - actv: log
      to: KID_TASK_SIMPLE
      sig: SIG_LOG
      data: "Task Simple received SIG_A and sent SIG_B to Task B."
-> Dict[str, exec_list] = tnorm_item
<< : *tnorm1-ctrl -> str
# ANCHOR - -> Therefore, Dict[str, exec_list, str] = tnorm_item
'''
'''
# STUB - Final TNORM Stub for YAML file
TNORM can have key type:
- fsm/tsm
- escal
- exec
- anchor
'''
class A(BaseModel):
  index: str
  hex_val: str
  id_symbol: str
  anchor: str = None

'''
# STUB - TNORM Stub for YAML file
tnorms:
  '1': &tnorm1-ctrl
    fsm_resrc: -> must have 'object', 'states'
      object: tnorm_b_tsmobj -> str
      states: -> str
      - tnorm_b_state_idle
      - tnorm_b_state_busy
    handler: tnorm_usr_nhler -> str
    hex_val: '0xe6' -> str
    id_symbol: TASK_USR_IDS -> str
    queue_name: tnorm_usr_msgq -> str
    tsm_resrc: -> must have 'object', 'state_trans', 'states'
      object: tnorm_usr_tsmobj
      state_trans: -> str
      - tnorm_usr_state_idle_trans
      - tnorm_usr_state_running_trans
      states: -> str
      - tnorm_usr_state_idle
      - tnorm_usr_state_running
      table: tnorm_usr_tsmobj_tbl -> str
tnorm is a mapping of string keys to obj ects, where each object has several properties, 
including fsm_resrc, handler, hex_val, id_symbol, queue_name, tsm_resrc. 
tsm_resrc is constrained to have Dict[str, List[str], List[str], str] type, which means it must have 'object', 'state_trans', and 'states' properties.
fsm_resrc is constrained to have Dict[str, List[str]] type, which means it must have 'object' and 'states' properties.
Default value of both tsm_resrc and fsm_resrc is None,
meaning whether FSM/TSM is used or not, it must have in the YAML file, but it can be None if not used. 
Therefore, dev has chosen tsm_resrc from tnorm3 instead of tnorm1.
Add anchor to mark the anchor to check the logic after that against this anchor. 
The anchor is must have in the class.
'''
class C_tnorm_obj(BaseModel):
  index: str
  handler: str
  hex_val: str
  id_symbol: str
  queue_name: str
  # tsm_resrc must have 'object', 'state_trans', 'states'
  # NOTE - default value is None, because tsm_resrc is not present in tnorm1, but it is present in tnorm3. Therefore, dev has chosen tsm_resrc from tnorm3 instead.
  tsm_resrc: Dict[str, List[str], List[str], str] = None
  # fsm_resrc must have 'object', 'states'
  fsm_resrc: Dict[str, List[str]] = None
  anchor: str = None

'''
# STUB - TPOLL Stub for YAML file
tpolls:
  '1': &tpoll1
    hex_val: '0xd4' -> str
    handler: tpoll_usr_nhler -> str
    id_symbol: TASK_USR_IDS -> str
Add anchor to mark the anchor to check the logic after that against this anchor. 
The anchor is must have in the class.
'''
class C_tpoll_obj(BaseModel):
  index: str
  hex_val: str
  handler: str
  id_symbol: str
  anchor: str = None

'''
# STUB - GDA Stub for YAML file
glbda:
- '1': &gda_status
  name: GDA_SYSTEM_STATUS
  type: "const char*"
  initial_value: "STATUS: INITIALIZING"
Add anchor to mark the anchor to check the logic after that against this anchor. 
The anchor is must have in the class.
'''
class C_gda_obj(BaseModel):
  index: str
  name: str
  type: str
  # NOTE - initial_value can be int or str, so we use Optional[int] = str to allow both types. it can be int or default value is str
  initial_value: Optional[int] = str
  anchor: str = None