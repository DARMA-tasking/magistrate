#!/usr/bin/env bash

set -exo pipefail

if test $# -lt 3
then
    echo "usage: ./$0 <kokkos-version> <build-dir> <openmp-enabled>"
    exit 1
fi

kokkos_version=$1
kokkos_zip_name=${kokkos_version}.zip
build_dir=$2
openmp=$3

echo "${kokkos_version}"
echo "${kokkos_zip_name}"

wget "http://github.com/kokkos/kokkos/archive/${kokkos_zip_name}"

unzip "${kokkos_zip_name}"

mkdir -p "${build_dir}"
pushd "${build_dir}"

export kokkos=/kokkos-${kokkos_version}
export kokkos_build=${build_dir}/kokkos
mkdir -p "$kokkos_build"
cd "$kokkos_build"
mkdir build
cd build
cmake -G "${CMAKE_GENERATOR:-Ninja}" \
      -DKokkos_ENABLE_OPENMP:BOOL="${openmp}" \
      -DCMAKE_INSTALL_PREFIX="$kokkos_build/install" \
      "$kokkos"
cmake --build . --target install
