import os


def main(context):
  cur_trm_dir = os.path.dirname("uEDP")
  arch_dir = "sources/pal/arch"
  # Create folder for arch
  try:
    os.makedirs(arch_dir + f"/{context['arch_name']}", exist_ok=True)
  except PermissionError:
    print(f"[ERROR] Permission denied: Cannot create directory")
  except OSError as e:
    print(f"[ERROR] Error creating directory: {e}")