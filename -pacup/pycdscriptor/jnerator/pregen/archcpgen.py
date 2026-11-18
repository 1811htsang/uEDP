import os
from jinja2 import Environment, FileSystemLoader

def main(context):
  cur_trm_dir = os.path.dirname("uEDP")
  env = Environment(loader = FileSystemLoader('./pltf/templates'))
  template = env.get_template('archc.txt')
  output = template.render(
    current_date = context["current_date"],
    arch_name = context['arch_name'],
    arch_apis = context['arch_apis']
  )
  # For debug
    # print(output)
  # Create file
  
  #NOTE - Add checking if the file already exists, if it does, print a warning message and do not overwrite it. If it does not exist, create the file and write the output to it.
  arch_dir = "sources/pal/arch"
  try:
    if os.path.exists(arch_dir + f"/{context['arch_name']}/{context['arch_name']}.c"):
      print(f"[WARNING] File {arch_dir}/{context['arch_name']}/{context['arch_name']}.c already exists. Reject creation.")
      return
    else:
      print(f"[INFO] File {arch_dir}/{context['arch_name']}/{context['arch_name']}.c does not exist. Creating it.")
  except PermissionError:
    print(f"[ERROR] Permission denied: Cannot create file")
  except OSError as e:
    print(f"[ERROR] Error creating file: {e}")
  output_dir = os.path.join(cur_trm_dir, "sources", "pal", "arch", context['arch_name'])
  with open(output_dir + "/" + context['arch_name'] + ".c", "w", encoding="utf-8") as f:
    f.write(output)