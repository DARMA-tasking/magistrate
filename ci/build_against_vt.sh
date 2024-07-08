#!/usr/bin/env bash

set -ex

vt_source_dir="${1}"
vt_build_dir="${2}"
checkpoint_source_dir="${3}"

mkdir -p "$vt_source_dir"
cd "$vt_source_dir"
rm -Rf ./*
git clone -b develop https://github.com/DARMA-tasking/vt.git .

target="${4:-install}"

if hash ccache &>/dev/null
then
    use_ccache=true
fi

if test "${use_ccache}"
then
    { echo -e "===\n=== ccache statistics before build\n==="; } 2>/dev/null
    ccache -s
else
    { echo -e "===\n=== ccache not found, compiling without it\n==="; } 2>/dev/null
fi

mkdir -p "${vt_build_dir}"
cd "${vt_build_dir}"
rm -Rf ./*

export CHECKPOINT=${checkpoint_source_dir}
export CHECKPOINT_BUILD=${vt_build_dir}/checkpoint
mkdir -p "${CHECKPOINT_BUILD}"
cd "${CHECKPOINT_BUILD}"
rm -Rf ./*
mkdir -p build
cd build
rm -Rf ./*
cmake -G "${CMAKE_GENERATOR:-Ninja}" \
        -DCMAKE_INSTALL_PREFIX="${CHECKPOINT_BUILD}/install" \
        -Dcheckpoint_asan_enabled="${MAGISTRATE_ASAN_ENABLED:-1}" \
        -Dcheckpoint_ubsan_enabled="${MAGISTRATE_UBSAN_ENABLED:-1}" \
        -Dcheckpoint_serialization_error_checking_enabled="${MAGISTRATE_SERIALIZATION_ERROR_CHECKING_ENABLED:-1}" \
        "${CHECKPOINT}"
cmake --build . --target install

if test "${VT_ZOLTAN_ENABLED:-0}" -eq 1
then
    export Zoltan_DIR=${ZOLTAN_DIR:-""}
fi

export VT=${vt_source_dir}
export VT_BUILD=${vt_build_dir}/vt
mkdir -p "${VT_BUILD}"
cd "${VT_BUILD}"
rm -Rf ./*
cmake -G "${CMAKE_GENERATOR:-Ninja}" \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
      -Dvt_test_trace_runtime_enabled="${VT_TRACE_RUNTIME_ENABLED:-0}" \
      -Dvt_lb_enabled="${VT_LB_ENABLED:-1}" \
      -Dvt_trace_enabled="${VT_TRACE_ENABLED:-0}" \
      -Dvt_trace_only="${VT_BUILD_TRACE_ONLY:-0}" \
      -Dvt_doxygen_enabled="${VT_DOXYGEN_ENABLED:-0}" \
      -Dvt_mimalloc_enabled="${VT_MIMALLOC_ENABLED:-0}" \
      -Dvt_asan_enabled="${VT_ASAN_ENABLED:-1}" \
      -Dvt_werror_enabled="${VT_WERROR_ENABLED:-0}" \
      -Dvt_pool_enabled="${VT_POOL_ENABLED:-1}" \
      -Dvt_build_extended_tests="${VT_EXTENDED_TESTS_ENABLED:-1}" \
      -Dvt_zoltan_enabled="${VT_ZOLTAN_ENABLED:-0}" \
      -Dvt_production_build_enabled="${VT_PRODUCTION_BUILD_ENABLED:-0}" \
      -Dvt_unity_build_enabled="${VT_UNITY_BUILD_ENABLED:-0}" \
      -Dvt_diagnostics_enabled="${VT_DIAGNOSTICS_ENABLED:-1}" \
      -Dvt_diagnostics_runtime_enabled="${VT_DIAGNOSTICS_RUNTIME_ENABLED:-0}" \
      -Dvt_fcontext_enabled="${VT_FCONTEXT_ENABLED:-0}" \
      -Dvt_fcontext_build_tests_examples="${VT_FCONTEXT_BUILD_TESTS_EXAMPLES:-0}" \
      -DUSE_OPENMP="${VT_USE_OPENMP:-0}" \
      -DUSE_STD_THREAD="${VT_USE_STD_THREAD:-0}" \
      -DCODE_COVERAGE="${CODE_COVERAGE:-0}" \
      -DMI_INTERPOSE:BOOL=ON \
      -DMI_OVERRIDE:BOOL=ON \
      -Dvt_mpi_guards="${VT_MPI_GUARD_ENABLED:-0}" \
      -DMPI_EXTRA_FLAGS="${MPI_EXTRA_FLAGS:-}" \
      -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE:-Release}" \
      -DMPI_C_COMPILER="${MPICC:-mpicc}" \
      -DMPI_CXX_COMPILER="${MPICXX:-mpicxx}" \
      -DCMAKE_CXX_COMPILER="${CXX:-c++}" \
      -DCMAKE_C_COMPILER="${CC:-cc}" \
      -DCMAKE_EXE_LINKER_FLAGS="${CMAKE_EXE_LINKER_FLAGS:-}" \
      -Dcheckpoint_ROOT="${CHECKPOINT_BUILD}/install" \
      -DCMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH:-}" \
      -DCMAKE_INSTALL_PREFIX="${VT_BUILD}/install" \
      -Dvt_ci_build="${VT_CI_BUILD:-1}" \
      -Dvt_debug_verbose="${VT_DEBUG_VERBOSE:-}" \
      -Dvt_tests_num_nodes="${VT_TESTS_NUM_NODES:-}" \
      "${VT}"
time cmake --build . --target "${target}"

if test "${use_ccache}"
then
    { echo -e "===\n=== ccache statistics after build\n==="; } 2>/dev/null
    ccache -s
fi
