#!/usr/bin/env bash

set -exo pipefail

source_dir=${1}
build_dir=${2}

export MAGISTRATE=${source_dir}
export MAGISTRATE_BUILD=${build_dir}/magistrate
pushd "$MAGISTRATE_BUILD"

ctest --output-on-failure | tee cmake-output.log

if test "${MAGISTRATE_CODE_COVERAGE:-0}" -eq 1
then
    lcov --capture --directory . --output-file coverage.info
    lcov --remove coverage.info '/usr/*' --output-file coverage.info
    lcov --list coverage.info
fi

popd
