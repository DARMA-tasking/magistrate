#!/usr/bin/env bash

set -exo pipefail

if test $# -lt 3
then
    echo "usage: ./$0 <kokkos-kernels-version> <build-dir> <kokkos-root>"
    exit 1
fi

kokkos_kernels_version=$1
kokkos_DIR=$3

build_dir=$2

echo "${kokkos_kernels_version}"

git clone https://github.com/kokkos/kokkos-kernels.git
cd kokkos-kernels
kokkos_kernels_dir=`pwd`
git checkout ${kokkos_kernels_version}

mkdir -p "${build_dir}"
pushd "${build_dir}"

export kokkos_kernels="$kokkos_kernels_dir"
export kokkos_kernels_build=${build_dir}/kokkos-kernels
mkdir -p "$kokkos_kernels_build"
cd "$kokkos_kernels_build"
mkdir build
cd build

cmake -G "${CMAKE_GENERATOR:-Ninja}" \
      -DCMAKE_INSTALL_PREFIX="$kokkos_kernels_build/kokkos-kernels-install" \
      -DKokkos_ROOT="${kokkos_DIR}" \
      "$kokkos_kernels"
cmake --build . --target install
