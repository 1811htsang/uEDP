from pydantic import BaseModel, field_validator
from typing import List, Optional, Dict

# LINK - sources/app/lstaxizer.yaml
# NOTE - This file is used to check against lstaxer.vlid

# DEPRECATED - Old TASK
'''
Loại bỏ toàn bộ model ISR vì bản thân `process-syntax` đã có thể xử lý syntax C-type với `actv: c_stmt` hoặc `actv: c_call`.
#STATUS - DONE
'''
  
# DEPRECATED - Old CRITICAL
'''
Thông qua các vòng review và đánh giá thiết kế syntax,
ISR đã được xác định là một tính năng không cần thiết 
và cho phép loại bỏ khỏi μE-LS.
Task đã được assign vào task list để loại bỏ ISR support trong syntax, bao gồm pydantic_model, example của docs, pycdscriptor.
#STATUS - DONE
'''

# STUB - OCE Stub for YAML file
'''
outexec:
- name: OCE_ITNLOG_DUMP -> str
  handler: itnlog_dump_handler -> str
  context: NULL -> Optional[str] = None
  state: READY -> str
'''

class C_outexec_obj(BaseModel):
  name: str
  handler: str
  context: Optional[str] = None
  state: str

class C_outexec_list_obj(BaseModel):
  outexec_list: Optional[List[C_outexec_obj]] = None