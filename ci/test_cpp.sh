#!/usr/bin/env bash

set -exo pipefail

source_dir=${1}
build_dir=${2}

export CHECKPOINT=${source_dir}
export CHECKPOINT_BUILD=${build_dir}/checkpoint
pushd "$CHECKPOINT_BUILD"

ctest --output-on-failure | tee cmake-output.log

if test "${CODE_COVERAGE:-0}" -eq 1
then
    export CODECOV_TOKEN="33a2f0dd-2962-48fa-807f-1a861e432596"
    lcov --capture --directory . --output-file coverage.info
    lcov --remove coverage.info '/usr/*' --output-file coverage.info
    lcov --list coverage.info
    pushd "$CHECKPOINT"
    bash <(curl -s https://codecov.io/bash) -f "${CHECKPOINT_BUILD}/coverage.info" || echo "Codecov did not collect coverage reports"
    popd
fi

popd
