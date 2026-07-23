#!/bin/bash
# Dừng script nếu có lỗi xảy ra
set -e
echo "[ENTRY] call menuconfig"
python uedp.py menuconfig
echo "[ENTRY] call testspec.generator"
python pltf/testspec/generators/tsgen.py
echo -e "[DONE]"
echo -e "You can:"
echo -e "\t[cd /uedp-test] for PLTF development"
echo -e "\t[exit] for logic development"
exec bash