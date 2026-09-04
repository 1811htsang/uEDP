#!/bin/bash
# Stop on first error
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
echo "[ENTRY] call pre-logicdef generator"
python -m pltf.testspec.generators.pre_logicdef.tsgen
echo "[ENTRY] call post-logicdef app.c generator"
python -m pltf.testspec.generators.post_logicdef.tsgen \
  --yaml sources/app/lstaxizer.yaml \
  --output sources/app/app.c
echo "[ENTRY] call testspec.ustab.custab"
python -m pltf.testspec.ustab.custab
# Change ownership of all files in the /uedp-libs and /uedp-test directories to the new user
chown -R $USER_ID:$GROUP_ID /uedp-libs/*
echo -e "[DONE]"
echo -e "You can:"
echo -e "\t[cd /uedp-test] for PLTF development"
echo -e "\t[exit] for logic development"
exec gosu uedp_user bash