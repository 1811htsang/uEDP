#!/bin/bash

# NOTE - Set UUID and GID for the user to run the container as, defaulting to 1000 if not provided
fn_docker() {
  set -e
  USER_ID=${MY_UID:-1000}
  GROUP_ID=${MY_GID:-1000}
  echo "[INFO] Starting entrypoint.sh with UID: $USER_ID and GID: $GROUP_ID"
  # Setup user and group with the specified IDs
  if ! id -u uedp_user >/dev/null 2>&1; then
    groupadd -g $GROUP_ID uedp_group 2>/dev/null || true
    useradd --shell /bin/bash -u $USER_ID -g $GROUP_ID -o -c "" -m uedp_user
  fi
  # Change ownership of the necessary directories to the new user
  chown $USER_ID:$GROUP_ID /uedp-libs
  chown $USER_ID:$GROUP_ID /uedp-test
  export HOME=/home/uedp_user
  # Load the ESP-IDF environment for the new user
  echo "source $IDF_PATH/export.sh > /dev/null 2>&1" >> /home/uedp_user/.bashrc
  echo "[INFO] Running as uedp_user (UID: $USER_ID)"
  # Run the Python scripts as the new user
  echo "[ENTRY] call menuconfig"
  python uedp.py menuconfig
  echo "[ENTRY] call pycdscriptor.jnerator.pregen.fpregen"
  python -m pltf.pycdscriptor.jnerator.pregen.fpregen
  echo "[ENTRY] call pycdscriptor.ustab.custab"
  python -m pltf.pycdscriptor.ustab.custab
  echo "[ENTRY] call pycdscriptor.ustab.ankorpin"
  python -m pltf.pycdscriptor.ustab.ankorpin
  # Change ownership of all files in the /uedp-libs and /uedp-test directories to the new user
  chown -R $USER_ID:$GROUP_ID /uedp-libs/*
  echo -e "[DONE]"
  echo -e "You can:"
  echo -e "\t[cd /uedp-test] for PLTF development"
  echo -e "\t[exit] for logic development"
  exec gosu uedp_user bash
}

fn_interactive() {
  echo "[ENTRY] call menuconfig"
  python uedp.py menuconfig
  echo "[ENTRY] call pycdscriptor.jnerator.pregen.fpregen"
  python -m pltf.pycdscriptor.jnerator.pregen.fpregen
  echo "[ENTRY] call pycdscriptor.ustab.custab"
  python -m pltf.pycdscriptor.ustab.custab
  echo "[ENTRY] call pycdscriptor.ustab.ankorpin"
  python -m pltf.pycdscriptor.ustab.ankorpin
}

fn_non_interactive() {
  echo "[ENTRY] call pycdscriptor.jnerator.pregen.fpregen"
  python -m pltf.pycdscriptor.jnerator.pregen.fpregen
  echo "[ENTRY] call pycdscriptor.ustab.custab"
  python -m pltf.pycdscriptor.ustab.custab
  echo "[ENTRY] call pycdscriptor.ustab.ankorpin"
  python -m pltf.pycdscriptor.ustab.ankorpin
}

parameter=$1
# NOTE - Check if parameter is `--it` (interactive) or `--n-it` (non-interactive)
if [ "$parameter" == "--it" ]; then
  echo "[INFO] Running in interactive mode"
  fn_interactive
elif [ "$parameter" == "--n-it" ]; then
  echo "[INFO] Running in non-interactive mode"
  fn_non_interactive
elif [ "$parameter" == "--docker" ]; then
  echo "[INFO] Running in docker mode"
  fn_docker
else
  echo "[ERROR] Invalid parameter. Use --it for interactive or --n-it for non-interactive."
  exit 1
fi