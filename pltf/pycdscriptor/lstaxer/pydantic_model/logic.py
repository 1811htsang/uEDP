from pydantic import BaseModel, field_validator, Field, ConfigDict
from typing import List, Optional, Dict, Union
from .resrc import C_tnorm_resrc_obj, C_tpoll_resrc_obj, C_gda_resrc_obj, C_sig_obj

# LINK - sources/app/lstaxizer.yaml
# NOTE - This file is used to check against lstaxer.vlid

# NOTE - General Pydantic configuration to understand both alias names and variable names
model_config = ConfigDict(populate_by_name=True)

# CRITICAL 
"""
`exec` keyword with `object` is a special keyword in Python, therefore.
using `kwexec` instead of `exec` to avoid conflict with Python's reserved keyword.
using `kwobject` instead of `object` to avoid conflict with Python's reserved keyword.
"""

# DEPRECATED - C_data_obj will be removed from section of actv-obj-post removal

class C_act_obj(BaseModel):
  actv: str
  code: Optional[str] = None
  function: Optional[str] = None
  args: List[str] = Field(default_factory=list)

# STATUS - actv-obj-post removal update 
'''
C_act_obj with only actv, code, function, args.
kind will be handle seperatedly by other module,
therefore it will not be included in C_act_obj.
'''

class C_act_list_obj(BaseModel):
  steps: List[C_act_obj]
  single_act: Optional[C_act_obj] = None
  # NOTE 
  '''
  single_act is used when there is only one action, and steps is used when there are multiple actions. 
  Therefore, we can use either steps or single_act, but not both. 
  If both are present, we will use steps and ignore single_act.
  '''

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
  # NOTE
  '''
  steps in on_rcev is mandatory, but also
  it overlaps the definition of steps in C_act_list_obj, 
  so we can use C_act_list_obj for steps in on_recv.
  '''

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
  # NOTE
  '''
  steps in kwexec is mandatory, but also
  it overlaps the definition of steps in C_act_list_obj,
  so we can use C_act_list_obj for steps in kwexec.
  '''

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

class C_tpoll_obj(BaseModel):
  tpoll: str
  kwexec: List[C_act_obj]
