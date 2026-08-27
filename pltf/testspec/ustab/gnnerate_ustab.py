import re
import json
from .cvert_ustab import ustab_convert_yaml

class gnnerate_ustab:
  def __init__(self):
    self.ust = {
      "tnorms": {},
      "tpolls": {},
      "sigs": {}
    }
    # Định nghĩa các base offset theo thiết kế HES của μE-OS
    # RESOLVED - Đã xử lý task default removal (uedp_core.h): chỉ TIM/IF/DBG (norm)
    # và WDG/SYSLF/MEMRP/IDLE (poll) bị xóa vì không còn được dùng ở đâu.
    # SYS_ID, USR_ID, IDLE_ID (norm) được giữ lại do vẫn có ý nghĩa chức năng thật.
    # OFFSET và MIN_ID không đổi (norm: 0xE0 + 0x06, poll: 0xD0 + 0x04) nên các
    # giá trị offset auto-counter bên dưới vẫn chính xác, không cần chỉnh sửa.
    self.OFFSETS = {
      "NORM": 0xE6,
      "POLL": 0xD4,
      "SIG": 0x01
    }
  def ustab_parse_kconfig(self, filepath):
    with open(filepath, 'r') as f:
      lines = f.readlines()
    for line in lines:
      line = line.strip()
      if not line or line.startswith("#"):
        continue
      m = re.match(r'CONFIG_DECL_TASK_NORM_(\d+)_NAME="(.+)"', line)
      if m: self._get_norm(m.group(1))["id_symbol"] = m.group(2) + "_IDS"      
      m = re.match(r'CONFIG_DECL_MSG_QUEUE_(\d+)_NAME="(.+)"', line)
      if m: self._get_norm(m.group(1))["queue_name"] = m.group(2) + "_msgq"
      m = re.match(r'CONFIG_DECL_NORM_HANDLER_(\d+)_NAME="(.+)"', line)
      if m: self._get_norm(m.group(1))["handler"] = m.group(2) + "_nhler"
      m = re.match(r'CONFIG_APPCFG_TSM_TASK_(\d+)="(.+)"', line)
      if m: 
        self._get_norm(m.group(1))["tsm_resrc"]["object"] = m.group(2)
        self._get_norm(m.group(1))["tsm_resrc"]["table"] = m.group(2) + "_tbl"
      m = re.match(r'CONFIG_APPCFG_TSM_TASK_(\d+)_STATE_(\d+)="(.+)"', line)
      if m: 
        self._get_norm(m.group(1))["tsm_resrc"]["states"].append(m.group(3))
        self._get_norm(m.group(1))["tsm_resrc"]["state_trans"].append(m.group(3) + "_trans")
      m = re.match(r'CONFIG_APPCFG_FSM_TASK_(\d+)="(.+)"', line)
      if m: self._get_norm(m.group(1))["fsm_resrc"]["object"] = m.group(2)
      m = re.match(r'CONFIG_APPCFG_FSM_TASK_(\d+)_STATE_(\d+)="(.+)"', line)
      if m: self._get_norm(m.group(1))["fsm_resrc"]["states"].append(m.group(3))
      m = re.match(r'CONFIG_DECL_TASK_POLL_(\d+)_NAME="(.+)"', line)
      if m: self._get_poll(m.group(1))["id_symbol"] = m.group(2)     
      m = re.match(r'CONFIG_DECL_POLL_HANDLER_(\d+)_NAME="(.+)"', line)
      if m: self._get_poll(m.group(1))["handler"] = m.group(2)
      m = re.match(r'CONFIG_DECL_SIG_(\d+)_NAME="(.+)"', line)
      if m: self._get_sig(m.group(1))["id_symbol"] = m.group(2)
    self._apply_hex_values()
    return self.ust
  def _get_norm(self, idx):
    if idx not in self.ust["tnorms"]:
      self.ust["tnorms"][idx] = {"tsm_resrc": {"states": [], "state_trans": []}, "fsm_resrc": {"states": []}}
    return self.ust["tnorms"][idx]
  def _get_poll(self, idx):
    if idx not in self.ust["tpolls"]:
      self.ust["tpolls"][idx] = {}
    return self.ust["tpolls"][idx]
  def _get_sig(self, idx):
    if idx not in self.ust["sigs"]:
      self.ust["sigs"][idx] = {}
    return self.ust["sigs"][idx]
  def _apply_hex_values(self):
    for idx, data in self.ust["tnorms"].items():
      data["hex_val"] = hex(self.OFFSETS["NORM"] + int(idx) - 1)
    for idx, data in self.ust["tpolls"].items():
      data["hex_val"] = hex(self.OFFSETS["POLL"] + int(idx) - 1)
    for idx, data in self.ust["sigs"].items():
      data["hex_val"] = hex(self.OFFSETS["SIG"] + int(idx) - 1)
# Calling
def generate_ustab_from_kconfig(kconfig_path):
  generator = gnnerate_ustab()
  ustab_data = generator.ustab_parse_kconfig(kconfig_path)
  yaml_output = ustab_convert_yaml(ustab_data)
  return yaml_output
# data = generate_ustab_from_kconfig(".config")