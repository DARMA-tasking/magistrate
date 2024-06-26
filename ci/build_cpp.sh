#!/usr/bin/env bash

set -ex

source_dir=${1}
build_dir=${2}

if test "${CHECKPOINT_DOXYGEN_ENABLED:-0}" -eq 1
then
    token=${3}
else
    target=${3:-install}
fi

echo -e "===\n=== ccache statistics before build\n==="
ccache -s

mkdir -p "${build_dir}"
pushd "${build_dir}"

export CHECKPOINT=${source_dir}
export CHECKPOINT_BUILD=${build_dir}/checkpoint
mkdir -p "$CHECKPOINT_BUILD"
cd "$CHECKPOINT_BUILD"
rm -Rf ./*

cmake_build_type="${CMAKE_BUILD_TYPE:-Release}"
is_debug=0
if test cmake_build_type = "Debug" || test cmake_build_type = "RelWithDebInfo"
then
    is_debug=1
fi

cmake -G "${CMAKE_GENERATOR:-Ninja}" \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
      -Dcheckpoint_doxygen_enabled="${CHECKPOINT_DOXYGEN_ENABLED:-0}" \
      -Dcheckpoint_tests_enabled="${CHECKPOINT_TESTS_ENABLED:-1}" \
      -Dcheckpoint_examples_enabled="${CHECKPOINT_EXAMPLES_ENABLED:-1}" \
      -Dcheckpoint_warnings_as_errors="${CHECKPOINT_WARNINGS_AS_ERRORS:-0}" \
      -Dcheckpoint_mpi_enabled="${CHECKPOINT_MPI_ENABLED:-1}" \
      -Dcheckpoint_asan_enabled="${CHECKPOINT_ASAN_ENABLED:-0}" \
      -Dcheckpoint_ubsan_enabled="${CHECKPOINT_UBSAN_ENABLED:-0}" \
      -Dcheckpoint_serialization_error_checking_enabled="${CHECKPOINT_SERIALIZATION_ERROR_CHECKING_ENABLED:-$is_debug}" \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
      -DCMAKE_BUILD_TYPE="${cmake_build_type}" \
      -DCMAKE_CXX_COMPILER="${CXX:-c++}" \
      -DCMAKE_C_COMPILER="${CC:-cc}" \
      -DCMAKE_EXE_LINKER_FLAGS="${CMAKE_EXE_LINKER_FLAGS:-}" \
      -DCODE_COVERAGE="${CODE_COVERAGE:-0}" \
      -DCMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH:-}" \
      -DCMAKE_INSTALL_PREFIX="$CHECKPOINT_BUILD/install" \
      -DGTEST_ROOT="${GTEST_ROOT}" \
      -DKokkos_ROOT="${KOKKOS_ROOT}" \
      -DKokkosKernels_ROOT="${KOKKOS_KERNELS_ROOT}" \
      "$CHECKPOINT"

if test "${CHECKPOINT_DOXYGEN_ENABLED:-0}" -eq 1
then
    MCSS=$PWD/m.css
    GHPAGE=$PWD/DARMA-tasking.github.io
    git clone "https://${token}@github.com/DARMA-tasking/DARMA-tasking.github.io"
    git clone https://github.com/mosra/m.css
    cd m.css
    git checkout 6eefd92c2aa3e0a257503d31b1a469867dfff8b6
    cd ../
    "$MCSS/documentation/doxygen.py" Doxyfile-mcss
    CKPT_NAME=checkpoint_docs
    mv docs "$CKPT_NAME"
    cp  -R "$CKPT_NAME" "$GHPAGE"
    cd "$GHPAGE"
    git config --global user.email "jliffla@sandia.gov"
    git config --global user.name "Jonathan Lifflander"
    git add "$CKPT_NAME"
    git commit -m "Update checkpoint_docs (auto-build)"
    git push origin master
else
    time cmake --build . --target "${target}"
fi

echo -e "===\n=== ccache statistics after build\n==="
ccache -s
