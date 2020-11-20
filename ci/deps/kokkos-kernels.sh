#!/usr/bin/env bash

set -exo pipefail

if test $# -lt 2
then
    echo "usage: ./$0 <kokkos-kernels-version> <build-dir>"
    exit 1
fi

kokkos_version=$1
kokkos_zip_name=${kokkos_version}.zip

build_dir=$2

echo "${kokkos_version}"
echo "${kokkos_zip_name}"

wget http://github.com/kokkos/kokkos-kernels/archive/${kokkos_zip_name}

unzip ${kokkos_zip_name}

mkdir -p "${build_dir}"
pushd "${build_dir}"

export kokkos=/kokkos-kernels-${kokkos_version}
export kokkos_build=${build_dir}/kokkos-kernels
mkdir -p "$kokkos_build"
cd "$kokkos_build"
mkdir build
cd build
ls ${KOKKOS_ROOT}
ls ${KOKKOS_ROOT}/cmake

cmake -G "${CMAKE_GENERATOR:-Ninja}" \
      -DCMAKE_PREFIX_PATH="${KOKKOS_ROOT}/cmake" \
      -DCMAKE_INSTALL_PREFIX="$kokkos_build/install" \
      "$kokkos"
cmake --build . --target install
