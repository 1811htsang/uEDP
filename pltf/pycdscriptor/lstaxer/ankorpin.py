import yaml
import os
import pprint
with open('pltf/pycdscriptor/lstaxer/testobj_ankorpin.yaml', 'r', encoding='utf-8') as f:
  yaml_sample = f.read()
events = yaml.parse(yaml_sample)
pprint.pprint(list(events))

print('\n')
# DOC
'''
Follow from lstaxer.lukupmodel documentation
and task list,
we will use sample yaml file to test
the idea for the ankorpin module.
'''

anchor_pivot_list = ['tnorms', 'tpolls', 'sigs', 'glbda']

# ANCHOR
'''
Find any ScalarEvent with value in anchor_pivot_list 
or value is an incremental index
'''

def ankorpin_add_anchor(type, event):

def ankorpin_lukup_tnorm_pivot(yaml_text):
  payload = yaml.safe_load(yaml_text) or {}
  entries = payload.get('tnorms', {})
  if not isinstance(entries, dict):
    return []

  # Extract anchors when they are present in the source YAML. This mirrors the
  # notes in the file: the object anchor is encountered right after the numeric
  # index and before the nested resource blocks are mapped.
  anchor_map = {}
  current_index = None
  in_tnorms = False
  for event in yaml.parse(yaml_text):
    if isinstance(event, yaml.ScalarEvent):
      if event.value == 'tnorms':
        in_tnorms = True
      elif in_tnorms and event.value.isdigit():
        current_index = event.value
        # ANCHOR - add anchor for `ankorpin_add_anchor` function
    elif isinstance(event, yaml.MappingStartEvent) and in_tnorms and event.anchor:
      if current_index is not None:
        anchor_map[current_index] = event.anchor

  result = []
  for index, entry in entries.items():
    anchor = anchor_map.get(index)
    result.append({
      'index': index,
      'anchor': anchor
    })
  return result

def ankorpin_lukup_tpoll_pivot(yaml_text):
  payload = yaml.safe_load(yaml_text) or {}
  entries = payload.get('tpolls', {})
  if not isinstance(entries, dict):
    return []

  # Extract anchors when they are present in the source YAML. This mirrors the
  # notes in the file: the object anchor is encountered right after the numeric
  # index and before the nested resource blocks are mapped.
  anchor_map = {}
  current_index = None
  in_tpolls = False
  for event in yaml.parse(yaml_text):
    if isinstance(event, yaml.ScalarEvent):
      if event.value == 'tpolls':
        in_tpolls = True
      elif in_tpolls and event.value.isdigit():
        current_index = event.value
    elif isinstance(event, yaml.MappingStartEvent) and in_tpolls and event.anchor:
      if current_index is not None:
        anchor_map[current_index] = event.anchor

  result = []
  for index, entry in entries.items():
    anchor = anchor_map.get(index)
    result.append({
      'index': index,
      'anchor': anchor
    })
  return result

def ankorpin_lukup_sigs_pivot(yaml_text):
  payload = yaml.safe_load(yaml_text) or {}
  entries = payload.get('sigs', {})
  if not isinstance(entries, dict):
    return []

  # Extract anchors when they are present in the source YAML. This mirrors the
  # notes in the file: the object anchor is encountered right after the numeric
  # index and before the nested resource blocks are mapped.
  anchor_map = {}
  current_index = None
  in_sigs = False
  for event in yaml.parse(yaml_text):
    if isinstance(event, yaml.ScalarEvent):
      if event.value == 'sigs':
        in_sigs = True
      elif in_sigs and event.value.isdigit():
        current_index = event.value
    elif isinstance(event, yaml.MappingStartEvent) and in_sigs and event.anchor:
      if current_index is not None:
        anchor_map[current_index] = event.anchor

  result = []
  for index, entry in entries.items():
    anchor = anchor_map.get(index)
    result.append({
      'index': index,
      'anchor': anchor
    })
  return result

pprint.pprint(ankorpin_lukup_tnorm_pivot(yaml_sample))
pprint.pprint(ankorpin_lukup_tpoll_pivot(yaml_sample))
pprint.pprint(ankorpin_lukup_sigs_pivot(yaml_sample))