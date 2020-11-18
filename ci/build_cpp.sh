#!/usr/bin/env bash

set -ex

source_dir=${1}
build_dir=${2}

# Dependency versions, when fetched via git.
detector_rev=master

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

if test -d "detector"
then
    rm -Rf detector
fi

if test -d "${source_dir}/lib/detector"
then
    echo "Detector already in lib... not downloading, building, and installing"
else
    git clone -b "${detector_rev}" --depth 1 https://github.com/DARMA-tasking/detector.git
    export DETECTOR=$PWD/detector
    export DETECTOR_BUILD=${build_dir}/detector
    mkdir -p "$DETECTOR_BUILD"
    cd "$DETECTOR_BUILD"
    mkdir build
    cd build
    cmake -G "${CMAKE_GENERATOR:-Ninja}" \
          -DCMAKE_INSTALL_PREFIX="$DETECTOR_BUILD/install" \
          "$DETECTOR"
    cmake --build . --target install
fi

export CHECKPOINT=${source_dir}
export CHECKPOINT_BUILD=${build_dir}/checkpoint
mkdir -p "$CHECKPOINT_BUILD"
cd "$CHECKPOINT_BUILD"
rm -Rf ./*
cmake -G "${CMAKE_GENERATOR:-Ninja}" \
      -Dcheckpoint_doxygen_enabled="${CHECKPOINT_DOXYGEN_ENABLED:-0}" \
      -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE:-Release}" \
      -DCMAKE_CXX_COMPILER="${CXX:-c++}" \
      -DCMAKE_C_COMPILER="${CC:-cc}" \
      -DCMAKE_EXE_LINKER_FLAGS="${CMAKE_EXE_LINKER_FLAGS:-}" \
      -Ddetector_DIR="$DETECTOR_BUILD/install" \
      -DCMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH:-}" \
      -DCMAKE_INSTALL_PREFIX="$CHECKPOINT_BUILD/install" \
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
