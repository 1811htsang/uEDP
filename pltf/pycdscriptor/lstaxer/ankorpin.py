import yaml
import pprint
from yaml.events import ScalarEvent, MappingStartEvent

def ankorpin_process_and_remap(input_path, output_path):
  # 1. Đọc nội dung file
  with open(input_path, 'r', encoding='utf-8') as f:
    yaml_text = f.read()

  # 2. Parse toàn bộ events vào một list để có thể duyệt nhiều lần hoặc xử lý tuần tự
  # Không sử dụng generator trực tiếp nếu muốn xử lý phức tạp
  events = list(yaml.parse(yaml_text))
  modified_events = []

  # Trạng thái theo dõi
  current_section = None
  pending_index = None
  
  # Cấu trúc prefix cho từng section
  prefixes = {
    'tnorms': 'tnorm',
    'tpolls': 'tpoll',
    'sigs': 'sig',
    'glbda': 'gda'
  }

  for event in events:
    # Nhận diện Section (tnorms, tpolls, sigs, glbda)
    if isinstance(event, ScalarEvent):
      if event.value in prefixes:
        current_section = event.value
      # Nhận diện Index (ví dụ '1', '2'...) bên trong các section mục tiêu
      elif current_section and event.value.isdigit():
        pending_index = event.value

    # Khi gặp MappingStartEvent ngay sau một Index
    elif isinstance(event, MappingStartEvent) and pending_index:
      prefix = prefixes[current_section]
      # Tạo Anchor name (ví dụ: tnorm1-ctrl, sig2-ctrl)
      # Bạn có thể tùy chỉnh logic đặt tên ở đây
      anchor_name = f"{prefix}{pending_index}-ank"
      
      # Tạo Event mới với Anchor được chèn vào (Event cũ là immutable)
      event = MappingStartEvent(
        anchor=anchor_name,
        tag=event.tag,
        implicit=event.implicit,
        start_mark=event.start_mark,
        end_mark=event.end_mark,
        flow_style=event.flow_style
      )
      # Reset index sau khi đã gán cho Mapping
      pending_index = None

    modified_events.append(event)

  # 3. Ghi lại nội dung vào file bằng emit
  with open(output_path, 'w', encoding='utf-8') as f:
    yaml.emit(modified_events, f)
  
  return modified_events

def ankorpin_map():
  input_file = 'sources/app/lstaxizer.yaml'
  output_file = 'sources/app/lstaxizer.yaml_anchored.yaml'

  try:
    final_events = ankorpin_process_and_remap(input_file, output_file)
    print(f"Successfully processed YAML. Output saved to: {output_file}")
    # ANCHOR - replace output file to input file
    with open(input_file, 'w', encoding='utf-8') as f:
      yaml.emit(final_events, f)
    # ANCHOR - delete the temporary output file
    import os
    os.remove(output_file)
  except Exception as e:
    print(f"Error processing YAML: {e}")

if __name__ == "__main__":
  ankorpin_map()