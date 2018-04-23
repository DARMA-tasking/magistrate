#!/bin/bash

if test $# -lt 2
then
    echo "usage $0 <build-mode> <has-detector>"
    exit 1;
fi

build_mode=$1
has_detector=$2

gtest_directory=/Users/jliffla/codes/gtest/gtest-install

if test ${has_detector} -gt 0
then
    detector_path=/Users/jliffla/codes/vt/detector-install
else
    detector_path=
fi

cmake ../checkpoint \
      -DCMAKE_INSTALL_PREFIX=../checkpoint-install \
      -DCMAKE_CXX_COMPILER=clang++-mp-3.9 \
      -DCMAKE_C_COMPILER=clang-mp-3.9 \
      -Ddarma_detector_DIR=${detector_path} \
      -DCHECKPOINT_BUILD_TESTS:bool=ON \
      -DCHECKPOINT_BUILD_EXAMPLES:bool=ON \
      -DGTEST_DIR=${gtest_directory} \
      -DCMAKE_BUILD_TYPE=debug \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=true
