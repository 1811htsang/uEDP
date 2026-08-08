import yaml
import json
import pprint
from .gnnerate_ustab import generate_ustab_from_kconfig

class ExplicitAnchorDumper(yaml.SafeDumper):
    """Dumper tùy chỉnh để ép buộc ghi Anchor dựa trên bản đồ ID"""
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.anchor_map = {}

    def represent_data(self, data):
        # Gọi representer mặc định để tạo node
        node = super().represent_data(data)
        
        # Nếu ID của đối tượng Python này nằm trong bản đồ Anchor, hãy gán nhãn cho node
        obj_id = id(data)
        if obj_id in self.anchor_map:
            node.anchor = self.anchor_map[obj_id]
        return node

def build_anchor_map(data):
    """Hàm duyệt dữ liệu và tạo bản đồ: id(object) -> 'tên_anchor'"""
    amap = {}
    
    # 1. Anchors cho Sigs
    amap[id(data["sigs"])] = 'siglst'
    for k, v in data['sigs'].items():
      amap[id(v)] = f'sig{k}'
        
    # 2. Anchors cho Tnorms và các Object lồng nhau
    amap[id(data['tnorms'])] = 'tnormlst'
    for k, v in data['tnorms'].items():
      amap[id(v)] = f'tnorm{k}'
      if 'fsm' in v:
        amap[id(v['fsm'])] = f'tnorm{k}-fsmobj'
        amap[id(v['fsm']['states'])] = f'tnorm{k}-fsmstates'
      if 'tsm' in v:
        amap[id(v['tsm'])] = f'tnorm{k}-tsmobj'
        amap[id(v['tsm']['states'])] = f'tnorm{k}-tsmstates'
          
    # 3. Anchors cho Tpolls
    amap[id(data['tpolls'])] = 'tpollslst'
    for k, v in data['tpolls'].items():
      amap[id(v)] = f'tpoll{k}'
        
    return amap

def build_reverse_anchor_map(anchor_map):
    """Hàm tạo bản đồ ngược: 'tên_anchor' -> id(object)"""
    return {v: k for k, v in anchor_map.items()}

def gen_final_map(data):
    """Hàm tạo bản đồ cuối cùng: 'object' -> tên_anchor"""
    anchor_mapping = build_anchor_map(data)
    reverse_anchor_mapping = build_reverse_anchor_map(anchor_mapping)
    final_map = {}
    # từng key trong reverse_anchor_mapping, tìm object tương ứng từ id và gán vào final_map
    for anchor_name, obj_id in reverse_anchor_mapping.items():
        # Tìm object từ id
        for obj in [data, data['sigs'], data['tnorms'], data['tpolls']]:
            if id(obj) == obj_id:
                final_map[obj] = anchor_name
                break
    return final_map

# Thực thi
dumper = ExplicitAnchorDumper
# Xây dựng bản đồ Anchor từ dữ liệu thực tế trong bộ nhớ
data = generate_ustab_from_kconfig(".config")
# anchor_mapping = build_anchor_map(data)
# reverse_anchor_mapping = build_reverse_anchor_map(anchor_mapping)
final_anchor_map = gen_final_map(data)
pprint.pprint(final_anchor_map)

# Inject bản đồ vào Dumper (Kỹ thuật khéo léo để bypass init)
class FinalDumper(ExplicitAnchorDumper):
  def __init__(self, *args, **kwargs):
    super().__init__(*args, **kwargs)
    self.anchor_map = final_anchor_map

# Dump ra file YAML
output_file = 'config_with_anchors.yaml'
with open(output_file, 'w') as f:
    yaml.dump(data, f, Dumper=FinalDumper, default_flow_style=False, sort_keys=False)

print(f"Done! File saved to {output_file}")