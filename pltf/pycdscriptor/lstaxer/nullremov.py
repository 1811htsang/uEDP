import pprint

def nullremov_null_tags(data):
  if isinstance(data, dict):
    # Tạo dictionary mới chỉ chứa các key có value khác NULL
    return {
      k: nullremov_null_tags(v) 
      for k, v in data.items() 
      if v is not None and str(v).upper() != "NULL"
    }
  elif isinstance(data, list):
    # Nếu là list, duyệt qua từng phần tử để làm sạch
    return [nullremov_null_tags(item) for item in data]
  else:
    # Nếu là giá trị đơn (string, int, bool...), giữ nguyên
    return data

def nullremov_resc_data(data):
  # NOTE - remove toàn bộ các cấu hình `sigs`, `tnorms` và `tpolls` sau khi đã load phân giải và xóa các giá trị NULL
  if isinstance(data, dict):
    # Tạo dictionary mới chỉ chứa các key có value khác NULL
    return {
      k: nullremov_null_tags(v) 
      for k, v in data.items() 
      if v is not None and str(v).upper() != "NULL" and k not in ['sigs', 'tnorms', 'tpolls']
    }


# --- Cách sử dụng ---
import yaml

with open('sources/app/lstaxizer.yaml', 'r', encoding='utf-8') as f:
  config_data = yaml.safe_load(f)
clean_data = nullremov_null_tags(config_data)
clean_data = nullremov_resc_data(clean_data)
with open('sources/app/lstaxizer_cleaned.yaml', 'w', encoding='utf-8') as f:
  # NOTE - export to file with perserve the original formatting order and structure
  yaml.dump(clean_data, f, default_flow_style=False, sort_keys=False, allow_unicode=True)