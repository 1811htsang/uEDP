import subprocess
import argparse
import sys
import os

def run_module(module_name):
  # NOTE - Hàm chạy các module Python bằng -m
  print(f"[ENTRY] call {module_name}")
  try:
    subprocess.check_call([sys.executable, "-m", module_name])
  except subprocess.CalledProcessError as e:
    print(f"[ERROR] Failed to run {module_name}")
    sys.exit(e.returncode)

def run_postgen(yaml_path, output_path):
  # NOTE - Hàm chạy cgen với các tham số
  module = "pltf.pycdscriptor.jnerator.postgen.cgen"
  print(f"[ENTRY] call {module}")
  try:
    subprocess.check_call([
      sys.executable, "-m", module,
      "--yaml", yaml_path,
      "--output", output_path
    ])
    print("[INFO] jainerator has done")
  except subprocess.CalledProcessError as e:
    print(f"[ERROR] Failed to run {module}")
    sys.exit(e.returncode)

def main():
  # NOTE - Cấu hình các đối số đầu vào (tương đương với các cờ bash)
  parser = argparse.ArgumentParser(description="Jainerator Pipeline")
  parser.add_argument("--yaml", default="sources/app/lstaxizer.yaml", help="Path to yaml file")
  parser.add_argument("--output", default="sources/app/app.c", help="Path to output file")
  
  args = parser.parse_args()

  # NOTE - 1. Chạy validator
  run_module("pltf.pycdscriptor.lstaxer.vlid")
  
  # NOTE - 2. Chạy generator
  run_postgen(args.yaml, args.output)

if __name__ == "__main__":
  main()