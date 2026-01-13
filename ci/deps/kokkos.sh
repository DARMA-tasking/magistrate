#!/usr/bin/env bash

set -exo pipefail

if test $# -lt 3
then
    echo "usage: ./$0 <kokkos-version> <build-dir> <openmp-enabled>"
    exit 1
fi

kokkos_version=$1
build_dir=$2
openmp=$3

echo "kokkos version: ${kokkos_version}"

git clone https://github.com/kokkos/kokkos.git
cd kokkos
kokkos_dir=`pwd`
git checkout ${kokkos_version}

mkdir -p "${build_dir}"
pushd "${build_dir}"

export kokkos="$kokkos_dir"
export kokkos_build=${build_dir}/kokkos
export kokkos_install=${build_dir}/kokkos-install
mkdir -p "$kokkos_build"
cd "$kokkos_build"
mkdir build
cd build
cmake -G "${CMAKE_GENERATOR:-Ninja}" \
      -DKokkos_ENABLE_OPENMP:BOOL="${openmp}" \
      -DCMAKE_INSTALL_PREFIX="$kokkos_install" \
      "$kokkos"
cmake --build . --target install

export kokkos_DIR="$kokkos_install"
