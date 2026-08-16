from pydantic import BaseModel, field_validator
from typing import List, Optional, Dict

# LINK - sources/app/lstaxizer.yaml
# NOTE - This file is used to check against lstaxer.vlid

"""
# STUB - ISR Stub for YAML file
isr: -> List[Dict[str, str, str]]
- id: ISR_HARD_STOP -> str # Act as handler name, not the ISR name, as the handler name has already cover the ISR name.
  to: TASK_A -> str
  sig: *sig7 -> str
"""

class C_isr_obj(BaseModel):
  id: str
  to: str
  sig: str

class C_isr_list_obj(BaseModel):
  isr_list: Optional[List[C_isr_obj]] = None

"""
# STUB - OCE Stub for YAML file
outexec:
- name: OCE_ITNLOG_DUMP -> str
  handler: itnlog_dump_handler -> str
  context: NULL -> Optional[str] = None
  state: READY -> str
"""

class C_outexec_obj(BaseModel):
  name: str
  handler: str
  context: Optional[str] = None
  state: str

class C_outexec_list_obj(BaseModel):
  outexec_list: Optional[List[C_outexec_obj]] = None