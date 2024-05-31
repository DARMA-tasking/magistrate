#!/usr/bin/env bash

path_to_magistrate=${1}
cd "$path_to_magistrate" || exit 1

for sub_dir in "src" "tests" "examples"
do
  "$path_to_magistrate/scripts/add-license-perl.pl" "$path_to_magistrate/$sub_dir" "$path_to_magistrate/scripts/license-template"
done

result=$(git diff --name-only)

if [ -n "$result" ]; then
  echo -e "Following files have incorrect license!\n"
  echo "$result"
  exit 1
fi
