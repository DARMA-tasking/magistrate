#!/usr/bin/env bash
# Check if there were any dockerfiles modified in the latest pull request.

diff_latest() {
  git diff --name-only origin/develop...HEAD
}

# Copy the diff output to stderr to list the modified files
if diff_latest | tee >(cat >&2) | grep -i dockerfile > /dev/null
then
    echo "docker-compose build --pull"
else
    echo "docker-compose pull --ignore-pull-failures"
fi
