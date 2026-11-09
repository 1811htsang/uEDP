#!/bin/bash
echo -e '[NOTICE] Please run this script inside /sources/test/, \n\t not in root directory of project.'
target=$1
if [ -z "$target" ]; then
  echo -e '[ERROR] Please provide a target name in ./testobj \n\t as the first argument.'
  exit 1
fi
echo -e "[NOTICE] target: $target"
output_dir='../app/lstaxizer.yaml'
# NOTE - check if app/lstaxizer.yaml not empty, if empty, raise an error and exit
if [ ! -s "$output_dir" ]; then
  echo -e "[ERROR] $output_dir is empty. \n\t Please generate logic-resc \n\t using ./entrypoint.sh and ./jainerator.sh before running this script."
  exit 1
else
  echo -e "[NOTICE] $output_dir is not empty. \n\t Proceeding with the test object insertion."
  # NOTE - append content of ./testobj/$target to the end of app/lstaxizer.yaml
  cat "./testobj/$target.yaml" >> "$output_dir"
  echo -e "[NOTICE] Successfully appended ./testobj/$target to $output_dir."
fi