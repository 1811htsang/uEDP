import os
import sys
import subprocess
import pwd
import grp

def run_cmd(cmd):
  # NOTE - Thực thi lệnh và in log ra màn hình
  print(f"[ENTRY] Running: {' '.join(cmd)}")
  subprocess.check_call(cmd)

def setup_user():
  # NOTE - Tương đương fn_docker: Thiết lập user/group và phân quyền
  uid = int(os.getenv('MY_UID', 1000))
  gid = int(os.getenv('MY_GID', 1000))
  
  print(f"[INFO] Running in docker mode as UID: {uid}, GID: {gid}")
  
  # NOTE - Giả lập lệnh useradd/groupadd (Chỉ chạy nếu có quyền root)
  if os.geteuid() == 0:
    try:
      grp.getgrgid(gid)
    except KeyError:
      subprocess.run(["groupadd", "-g", str(gid), "uedp_group"], check=True)
      
    try:
      pwd.getpwuid(uid)
    except KeyError:
      subprocess.run(["useradd", "--shell", "/bin/bash", "-u", str(uid), 
              "-g", str(gid), "-o", "-c", "", "-m", "uedp_user"], check=True)
      
    # NOTE - Chown các thư mục
    for d in ['/uedp-libs', '/uedp-test']:
      os.chown(d, uid, gid)
      
  # NOTE - Thêm source vào bashrc
  idf_path = os.getenv('IDF_PATH', '/opt/esp/idf')
  bashrc = "/home/uedp_user/.bashrc"
  with open(bashrc, "a") as f:
    f.write(f"\nsource {idf_path}/export.sh > /dev/null 2>&1\n")

def run_pipeline(include_menuconfig=True):
  # NOTE - Danh sách các tác vụ cần chạy
  if include_menuconfig:
    run_cmd(["python", "uedp.py", "menuconfig"])
  
  tasks = [
    ["python", "-m", "pltf.pycdscriptor.jnerator.pregen.fpregen"],
    ["python", "-m", "pltf.pycdscriptor.ustab.custab"],
    ["python", "-m", "pltf.pycdscriptor.ustab.ankorpin"]
  ]
  
  for task in tasks:
    run_cmd(task)

def main():
  if len(sys.argv) < 2:
    print("[ERROR] No parameter. Use --it, --n-it, or --docker.")
    sys.exit(1)

  mode = sys.argv[1]

  if mode == "--docker":
    setup_user()
    run_pipeline(include_menuconfig=True)
    # NOTE - Chown lại libs sau khi chạy
    subprocess.run(["chown", "-R", f"{os.getenv('MY_UID', 1000)}:{os.getenv('MY_GID', 1000)}", "/uedp-libs/"], check=True)
    print("[DONE]\nYou can:\n\t[cd /uedp-test] for PLTF development\n\t[exit] for logic development")
    os.execvp("gosu", ["gosu", "uedp_user", "bash"])

  elif mode == "--it":
    print("[INFO] Running in interactive mode")
    run_pipeline(include_menuconfig=True)

  elif mode == "--n-it":
    print("[INFO] Running in non-interactive mode")
    run_pipeline(include_menuconfig=False)
    
  else:
    print(f"[ERROR] Invalid parameter: {mode}")
    sys.exit(1)

if __name__ == "__main__":
  main()