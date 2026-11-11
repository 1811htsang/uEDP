from pydantic import BaseModel
from typing import Any, List, Optional

class C_tsd_post_isr_obj(BaseModel):
  to: str
  sig: str

class C_tsd_step_obj(BaseModel):
  post_isr: Optional[C_tsd_post_isr_obj] = None
  advance_tick: Optional[int] = None

class C_tsd_expect_tsm_state_obj(BaseModel):
  task: str
  state: str

class C_tsd_expect_gda_value_obj(BaseModel):
  name: str
  equals: str

class C_tsd_expect_obj(BaseModel):
  tsm_state: Optional[C_tsd_expect_tsm_state_obj] = None
  gda_value: Optional[C_tsd_expect_gda_value_obj] = None

class C_tsd_scenario_obj(BaseModel):
  name: str
  level: str
  steps: List[C_tsd_step_obj]
  expect: List[C_tsd_expect_obj]
