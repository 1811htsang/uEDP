import os

def main(context):
  cur_trm_dir = os.path.dirname("uEDP")
  arch_dir = "sources/pal/arch"
  # Create folder for arch
  try:
    # NOTE - Add checking if the directory is already exists, if not, create it. If it exists, check if it's empty or not. If it's not empty, print a warning message.
    if os.path.exists(arch_dir + f"/{context['arch_name']}"):
      if os.listdir(arch_dir + f"/{context['arch_name']}"):
        print(f"[WARNING] Directory {arch_dir}/{context['arch_name']} is not empty. Reject creation.")
        return
      else:
        print(f"[INFO] Directory {arch_dir}/{context['arch_name']} already exists and is empty. Proceeding with creation.")
    else:
      print(f"[INFO] Directory {arch_dir}/{context['arch_name']} does not exist. Creating it.")
      os.makedirs(arch_dir + f"/{context['arch_name']}", exist_ok=True)
  except PermissionError:
    print(f"[ERROR] Permission denied: Cannot create directory")
  except OSError as e:
    print(f"[ERROR] Error creating directory: {e}")