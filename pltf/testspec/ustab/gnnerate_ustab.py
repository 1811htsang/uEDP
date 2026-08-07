import yaml
import re
from .cvert_ustab import convert_yaml

class USTManager:
  def __init__(self, yaml_content):
    # Load YAML gốc
    self.raw_data = yaml.safe_load(yaml_content)
    self.task_registry = {}
    self.signal_map = {}

  def _clean_value(self, val_str):
    """Hàm phụ trợ để tách ID và giá trị Hex (ví dụ: 'SIG_1_ID (0x01u)' -> 0x01)"""
    match = re.search(r'\((0x[0-9A-Fa-f]+)u\)', val_str)
    if match:
      return match.group(1)
    return val_str

  def build_registry(self):
    """
    Hệ thống lõi để ánh xạ các danh sách rời rạc thành các đối tượng Task tập trung.
    Logic: Dựa trên chỉ số index của các danh sách để đảm bảo tính nhất quán 
    mà không cần quan tâm đến tên gọi (task_1 hay task_A).
    """
    
    # 1. Xử lý Task Norm (Lấy số lượng từ tasknorm_defs)
    tasks = self.raw_data.get('tasknorm_defs', [])
    handlers = self.raw_data.get('normhler_lists', [])
    fsm_objs = self.raw_data.get('appcfg_fsm_objects', [])
    tsm_objs = self.raw_data.get('appcfg_tsm_objects', [])
    tsm_state_trans = self.raw_data.get('appcfg_tsm_state_trans', [])
    msg_queues = self.raw_data.get('msgq_defs', [])

    for i in range(len(tasks)):
      # Tách lấy Key ID (ví dụ: TASK_NORM_1_ID)
      task_key = tasks[i].split()[0]
      hex_val = self._clean_value(tasks[i])

      # Tạo Object Task tập trung
      self.task_registry[task_key] = {
        "hex": hex_val,
        "handler": handlers[i] if i < len(handlers) else "NULL",
        "fsm": fsm_objs[i] if i < len(fsm_objs) else "NULL",
        "tsm": tsm_objs[i] if i < len(tsm_objs) else "NULL",
        "msg_queue": msg_queues[i] if i < len(msg_queues) else "NULL"
      }

    # 2. Xử lý Signal
    signals = self.raw_data.get('sig_defs', [])
    for sig in signals:
      sig_key = sig.split()[0]
      sig_hex = self._clean_value(sig)
      self.signal_map[sig_key] = sig_hex

  def get_task_context(self, task_id):
    """Truy vấn nhanh mọi thông tin của 1 Task bất kỳ"""
    return self.task_registry.get(task_id, {})

  def export_unified_context(self):
    """Xuất dữ liệu đã được 'concentrate' để đẩy vào Jinja2"""
    return {
        "tasks": self.task_registry,
        "signals": self.signal_map,
        "total_tasks": len(self.task_registry)
    }

# --- TEST MODULE ---
if __name__ == "__main__":
  # NOTE - Giả sử đây là nội dung YAML từ cfparsers
  yaml_content = convert_yaml()

  ust = USTManager(yaml_content)
  ust.build_registry()
  
  # Lấy thông tin tập trung
  unified_context = ust.export_unified_context()
  
  import json
  import pprint
  pprint.pprint(unified_context)
  # print(json.dumps(unified_context))