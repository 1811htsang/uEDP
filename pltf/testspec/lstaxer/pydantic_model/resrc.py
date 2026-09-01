from pydantic import BaseModel, field_validator, Field, ConfigDict
from typing import List, Optional, Dict, Union

# LINK - sources/app/lstaxizer.yaml
# NOTE - This file is used to check against lstaxer.vlid

# General Pydantic configuration to understand both alias names and variable names
model_config = ConfigDict(populate_by_name=True)

'''
# STUB - Signal Stub for YAML file
sigs:
  '1': &sig1 -> index
    hex_val: '0x1' -> str
    id_symbol: SIG_USR_START -> str
  '2': &sig2
    hex_val: '0x2'
    id_symbol: SIG_USR_STOP
In YAML file, sigs are defined as a mapping of string keys to objects, where each object has two properties: hex_val and id_symbol. The keys '1' and '2' are used to reference the objects using anchors (&sig1 and &sig2). The hex_val is a string representing a hexadecimal value, and id_symbol is a string representing an identifier symbol.
Therefore, the class present should have
- First key is a string, which is the key of the mapping.
- Second key is a string, which is the hex_val.
- Third key is a string, which is the id_symbol.
Add anchor to mark the anchor to check the logic after that against this anchor. 
The anchor is must have in the class.
'''
class C_sig_obj(BaseModel):
  model_config = model_config
  index: str
  hex_val: str
  id_symbol: str

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
class C_tnorm_tsm_resrc_obj(BaseModel):
  model_config = model_config
  kwobject: str = Field(alias="object")  # Use alias to avoid conflict with Python's built-in object
  state_trans: List[str]
  states: List[str]
  table: str

class C_tnorm_fsm_resrc_obj(BaseModel):
  model_config = model_config
  kwobject: str
  states: List[str]

class C_tnorm_resrc_obj(BaseModel):
  model_config = model_config
  index: str
  handler: str
  hex_val: str
  id_symbol: str
  queue_name: str
  anchor: str
  # tsm_resrc must have 'object', 'state_trans', 'states'
  # NOTE - default value is None, because tsm_resrc is not present in tnorm1, but it is present in tnorm3. Therefore, dev has chosen tsm_resrc from tnorm3 instead.
  tsm_resrc: Optional[C_tnorm_tsm_resrc_obj] = None
  # fsm_resrc must have 'object', 'states'
  fsm_resrc: Optional[C_tnorm_fsm_resrc_obj] = None

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
class C_tpoll_resrc_obj(BaseModel):
  model_config = model_config
  index: str
  hex_val: str
  handler: str
  id_symbol: str
  anchor: str

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
class C_gda_resrc_obj(BaseModel):
  model_config = model_config
  index: str
  name: str
  kwtype: str = Field(alias="type")  # Use alias to avoid conflict with Python's built-in type
  # NOTE - initial_value can be int or str, so we use Optional[int] = str to allow both types. it can be int or default value is str
  initial_value: Union[int, str, bool]
  anchor: str