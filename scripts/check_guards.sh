#!/usr/bin/env bash

path_to_magistrate=${1}
show_fix_command=${2}
cd "$path_to_magistrate" || exit 1

for sub_dir in "src" "tests" "examples"
do
  python3 "${path_to_magistrate}/scripts/generate_header_guards_and_license.py" -s=${sub_dir} -l="${path_to_magistrate}/scripts/license-template"
done

# Check for modified files
modified_files=$(git ls-files -m)

if [ -n "$modified_files" ]; then
  echo "The following files have been modified by updating the license or header guards:"
  echo "$modified_files"

  if [ "$show_fix_command" == true ]; then
    echo ""
    echo "Please run the following command from main magistrate directory to fix them:"
    echo "./scripts/check_guards.sh ."
  fi
fi