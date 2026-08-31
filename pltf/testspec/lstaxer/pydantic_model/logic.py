from pydantic import BaseModel, field_validator, Field, ConfigDict
from typing import List, Optional, Dict, Union

# LINK - sources/app/lstaxizer.yaml
# NOTE - This file is used to check against lstaxer.vlid

# General Pydantic configuration to understand both alias names and variable names
model_config = ConfigDict(populate_by_name=True)

# CRITICAL 
"""
Exec keyword is a special keyword in Python, therefore, 
using kwexec instead of exec to avoid conflict with Python's reserved keyword.
"""

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
  kwexec: -> List[Dict[str, List[Dict[str | actv, Optional[str] | to, Optional[str] | sig, Optional[str] | data, Optional[str] | ptype]]]]] = kwexec_list
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
-> Dict[str, kwexec_list] = tnorm_item
<< : *tnorm1-ctrl -> str
# ANCHOR - -> Therefore, Dict[str, kwexec_list, str] = tnorm_item
'''
'''
# STUB - TNORM Logic Stub with APE call for YAML file
escal: -> Optional[Dict[str, List[Dict[str, Optional[Dict[str, str, Optional[str], Optional[str]]]]]]] = escal_list
  mode: slnf -> str
  trigger: -> List[Dict[str, Optional[Dict[str, str, Optional[str], Optional[str]]]]] = trigger_list
  - on_sig: SIG_CALL_URGENT # Kích hoạt APE khi nhận signal này -> str
    post_urgent: # Tự gọi urgent message cho chính tnorm để thực thi hành vi ưu tiên -> Optional[Dict[str, str, Optional[str], Optional[str]]]
      to: KID_TASK_USR
      sig: SIG_kwexec_URGENT
      data: NULL
      # NOTE - ptype if needed.
'''
'''
# STUB - Final TNORM Stub for YAML file
TNORM can have key type:
- fsm/tsm -> optional
- escal -> optional
- kwexec -> must have if fsm/tsm is not present
- anchor -> must have in the class
'''

class C_act_obj(BaseModel):
  actv: str
  to: Optional[str] = None
  sig: Optional[str] = None
  data: Optional[str] = None
  ptype: Optional[str] = None

class C_act_list_obj(BaseModel):
  steps: List[C_act_obj]
  single_act: Optional[C_act_obj] = None
  # NOTE - single_act is used when there is only one action, and steps is used when there are multiple actions. 
  # Therefore, we can use either steps or single_act, but not both. 
  # If both are present, we will use steps and ignore single_act.

class C_trans_obj(BaseModel):
  sig: str
  goto: str

class C_trans_list_obj(BaseModel):
  trans: List[C_trans_obj]

class C_tsm_obj(BaseModel):
  id: str
  trans: C_trans_list_obj
  on_ntry: Optional[C_act_list_obj] = None
  on_actv: Optional[C_act_list_obj] = None
  on_exit: Optional[C_act_list_obj] = None

class C_tsm_list_obj(BaseModel):
  tsm_list: List[C_tsm_obj]
  # NOTE - final call is equipvalent to tsm

class C_onrecv_obj(BaseModel):
  sig: str
  goto: str
  steps: C_act_list_obj
  # NOTE - steps in on_rcev is mandatory, but also
  # it overlaps the definition of steps in C_act_list_obj, 
  # so we can use C_act_list_obj for steps in on_recv.

class C_onrecv_list_obj(BaseModel):
  on_recv: List[C_onrecv_obj]

class C_fsm_obj(BaseModel):
  id: str
  on_recv: C_onrecv_list_obj

class C_fsm_list_obj(BaseModel):
  fsm_list: List[C_fsm_obj]
  # NOTE - final call is equipvalent to fsm

class C_kwexec_obj(BaseModel):
  on_sig: str
  steps: C_act_list_obj
  # NOTE - steps in kwexec is mandatory, but also
  # it overlaps the definition of steps in C_act_list_obj,
  # so we can use C_act_list_obj for steps in kwexec.

class C_kwexec_list_obj(BaseModel):
  kwexec: List[C_kwexec_obj]
  # NOTE - final call is equipvalent to kwexec

class C_trig_obj(BaseModel):
  on_sig: str
  post_urgent: Optional[C_act_list_obj] = None

class C_trig_list_obj(BaseModel):
  trigger: List[C_trig_obj]

class C_escal_obj(BaseModel):
  mode: str
  trigger: C_trig_list_obj

class C_tnorm_obj(BaseModel):
  task: str
  tsm: Optional[C_tsm_list_obj] = None
  fsm: Optional[C_fsm_list_obj] = None
  kwexec: Optional[C_kwexec_list_obj] = None
  escal: Optional[C_escal_obj] = None
  anchor: Optional[str] = None

'''
# STUB - TPOLL Logic Stub for YAML file
- tpoll: TASK_POLL_MEMRP -> str
  kwexec: -> List[Dict[str, List[Dict[str | actv, Optional[str] | to, Optional[str] | sig, Optional[str] | data, Optional[str] | ptype]]]] = kwexec_list
  - actv: pal_memrp_report() -> str
    to: NULL -> Optional[str] = None
    sig: NULL -> Optional[str] = None
    data: NULL -> Optional[str] = None
    ptype: NULL -> Optional[str] = None
'''
class C_tpoll_obj(BaseModel):
  tpoll: str
  kwexec: List[C_act_obj]