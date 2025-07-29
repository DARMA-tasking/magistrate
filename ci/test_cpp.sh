#!/usr/bin/env bash

set -exo pipefail

source_dir=${1}
build_dir=${2}

export CHECKPOINT=${source_dir}
export CHECKPOINT_BUILD=${build_dir}/checkpoint
pushd "$CHECKPOINT_BUILD"

ctest --output-on-failure | tee cmake-output.log

if test "${MAGISTRATE_CODE_COVERAGE:-0}" -eq 1
then
    export CODECOV_TOKEN="$CODECOV_TOKEN"

    lcov --capture --directory . --output-file coverage.info
    lcov --remove coverage.info '/usr/*' --output-file coverage.info
    lcov --list coverage.info
    pushd "$CHECKPOINT"

    apt-get update && apt-get install -y python3-pip
    python3 -m pip install --upgrade codecov-cli

    codecovcli --verbose upload-process \
      --disable-search \
      -f "${CHECKPOINT_BUILD}/coverage.info" \
      --commit-sha "$GITHUB_SHA" \
      --slug DARMA-tasking/magistrate \
      --git-service github \
      --token "$CODECOV_TOKEN"

    popd
fi

popd
