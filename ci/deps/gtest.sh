#!/usr/bin/env bash

set -exo pipefail

if test $# -lt 2
then
    echo "usage: ./$0 <gtest-version> <build-dir>"
    exit 1
fi

gtest_version=$1
gtest_zip_name=release-${gtest_version}.zip

build_dir=$2

echo "${gtest_version}"
echo "${gtest_zip_name}"

wget http://github.com/google/googletest/archive/${gtest_zip_name}

unzip ${gtest_zip_name}

mkdir -p "${build_dir}"
pushd "${build_dir}"

export gtest=/googletest-release-${gtest_version}
export gtest_build=${build_dir}/gtest
mkdir -p "$gtest_build"
cd "$gtest_build"
mkdir build
cd build
cmake -G "${CMAKE_GENERATOR:-Ninja}" \
      -DCMAKE_INSTALL_PREFIX="$gtest_build/install" \
      "$gtest"
cmake --build . --target install
