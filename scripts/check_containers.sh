#!/usr/bin/env bash
# Decide whether we should pull or build the Docker images in CI.
# This is consistent with GitHub (actions/checkout@v3).

diff_latest() {
  printf "Files modified in the latest pull request:\n" >&2
  local base_ref
  if [[ $(git branch --show-current) == "develop" ]]
  then
    base_ref=$(git log --skip=1 -1 --merges --pretty=format:%H)
  else
    base_ref="origin/develop"
  fi
  git diff --name-only "$base_ref"...HEAD
}

# Check if there were any dockerfiles modified in the latest pull request.
# Print the modified files list to stderr as well.
if diff_latest | tee >(cat >&2) | grep -i -q dockerfile
then
    echo "docker-compose build --pull"
else
    echo "docker-compose pull --ignore-pull-failures"
fi
