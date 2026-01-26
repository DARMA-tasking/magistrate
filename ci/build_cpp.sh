#!/usr/bin/env bash

set -ex

source_dir=${1}
build_dir=${2}
target=${3:-install}

echo -e "===\n=== ccache statistics before build\n==="
ccache -s

mkdir -p "${build_dir}"
pushd "${build_dir}"

export MAGISTRATE=${source_dir}
export MAGISTRATE_BUILD=${build_dir}/magistrate
mkdir -p "$MAGISTRATE_BUILD"
cd "$MAGISTRATE_BUILD"
rm -Rf ./*

cmake_build_type="${CMAKE_BUILD_TYPE:-Release}"
is_debug=0
if test cmake_build_type = "Debug" || test cmake_build_type = "RelWithDebInfo"
then
    is_debug=1
fi

echo "kokkos: ${MAGISTRATE_KOKKOS_ENABLED}"

if test "${MAGISTRATE_KOKKOS_ENABLED:-0}" -eq 1
then
    "$MAGISTRATE/ci/deps/kokkos.sh" "5.0.1" "$MAGISTRATE_BUILD/kokkos_build" 0
    export KOKKOS_ROOT="$MAGISTRATE_BUILD/kokkos_build/kokkos-install"
    "$MAGISTRATE/ci/deps/kokkos-kernels.sh" "5.0.1" "$MAGISTRATE_BUILD/kokkos_kernels_build" "$MAGISTRATE_BUILD/kokkos_build/kokkos-install"
    export KOKKOS_KERNELS_ROOT="$MAGISTRATE_BUILD/kokkos_kernels_build/kokkos-kernels/kokkos-kernels-install"
fi

cmake -G "${CMAKE_GENERATOR:-Ninja}" \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
      -Dmagistrate_code_coverage="${MAGISTRATE_CODE_COVERAGE:-0}" \
      -Dmagistrate_doxygen_enabled="${MAGISTRATE_DOXYGEN_ENABLED:-0}" \
      -Dmagistrate_tests_enabled="${MAGISTRATE_TESTS_ENABLED:-1}" \
      -Dmagistrate_examples_enabled="${MAGISTRATE_EXAMPLES_ENABLED:-1}" \
      -Dmagistrate_warnings_as_errors="${MAGISTRATE_WARNINGS_AS_ERRORS:-0}" \
      -Dmagistrate_mpi_enabled="${MAGISTRATE_MPI_ENABLED:-1}" \
      -Dmagistrate_asan_enabled="${MAGISTRATE_ASAN_ENABLED:-0}" \
      -Dmagistrate_ubsan_enabled="${MAGISTRATE_UBSAN_ENABLED:-0}" \
      -Dmagistrate_serialization_error_checking_enabled="${MAGISTRATE_SERIALIZATION_ERROR_CHECKING_ENABLED:-$is_debug}" \
      -DCMAKE_BUILD_TYPE="${cmake_build_type}" \
      -DCMAKE_CXX_COMPILER="${CXX:-c++}" \
      -DCMAKE_C_COMPILER="${CC:-cc}" \
      -DCMAKE_EXE_LINKER_FLAGS="${CMAKE_EXE_LINKER_FLAGS:-}" \
      -DCMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH:-}" \
      -DCMAKE_INSTALL_PREFIX="$MAGISTRATE_BUILD/install" \
      -DGTEST_ROOT="${GTEST_ROOT}" \
      -DKokkos_ROOT="${KOKKOS_ROOT}" \
      -DKokkosKernels_ROOT="${KOKKOS_KERNELS_ROOT}" \
      "$MAGISTRATE"
cmake_conf_ret=$?

if test "${MAGISTRATE_DOXYGEN_ENABLED:-0}" -eq 1
then
    MCSS=$PWD/m.css
    GHPAGE=$PWD/DARMA-tasking.github.io

    git clone --depth=1 "https://x-access-token:${GITHUB_TOKEN}@github.com/DARMA-tasking/DARMA-tasking.github.io"
    git clone https://github.com/mosra/m.css
    cd m.css
    git checkout 699abdd5
    cd ../
    "$MCSS/documentation/doxygen.py" Doxyfile-mcss

    if test "${GIT_BRANCH:-}" = "develop"
    then
        CKPT_NAME=magistrate_docs
        mv docs "$CKPT_NAME"
        cp  -R "$CKPT_NAME" "$GHPAGE"
        cd "$GHPAGE"
        git config --global user.email "jliffla@sandia.gov"
        git config --global user.name "Jonathan Lifflander"
        git add "$CKPT_NAME"
        git commit --allow-empty -m "Update magistrate_docs (auto-build)"
        git push origin master
    fi
elif test "${VT_CI_BUILD:-0}" -eq 1
then
    # Generate output file with compilation warnings and errors
    GENERATOR=$(cmake -L . | grep USED_CMAKE_GENERATOR:STRING | cut -d"=" -f2)
    OUTPUT="$VT_BUILD"/compilation_errors_warnings.out
    OUTPUT_TMP="$OUTPUT".tmp
    WARNS_ERRS=""

    # Unfortunately Ninja doesn't output compilation warnings and errors to stderr
    # so it needs special treatment
    if test "$GENERATOR" = "Ninja"
    then
        # To easily tell if compilation of given file succeeded special progress bar is used
        # (controlled by variable NINJA_STATUS)
        export NINJA_STATUS="[ninja][%f/%t] "
        time cmake --build . --target "${target}" | tee "$OUTPUT_TMP"
        compilation_ret=${PIPESTATUS[0]}
        sed -i '/ninja: build stopped:/d' "$OUTPUT_TMP"

        # Now every line that doesn't start with [ninja][number]/[number] is an error or a warning
        WARNS_ERRS=$(grep -Ev '^(\[ninja\]\[[[:digit:]]+\/[[:digit:]]+\])|(--) .*$' "$OUTPUT_TMP" || true)
    elif test "$GENERATOR" = "Unix Makefiles"
    then
        # Gcc outputs warnings and errors to stderr, so there's not much to do
        time cmake --build . --target "${target}" 2> >(tee "$OUTPUT_TMP")
        compilation_ret=$?
        WARNS_ERRS=$(cat "$OUTPUT_TMP")
    fi

    echo "$WARNS_ERRS" > "$OUTPUT"
else
    time cmake --build . --target "${target}"
    compilation_ret=$?
fi

echo -e "===\n=== ccache statistics after build\n==="
ccache -s


# Exit with error code if there was any
if test "$cmake_conf_ret" -ne 0
then
    echo "There was an error during CMake configuration"
    exit "$cmake_conf_ret"
elif test "$compilation_ret" -ne 0
then
    echo "There was an error during compilation"
    exit "$compilation_ret"
fi
