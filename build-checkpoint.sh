#!/bin/bash

if test $# -lt 2
then
    echo "usage $0 <build-mode> <has-detector> [detector-path] [gtest-path]"
    exit 1;
fi

build_mode=$1
has_detector=$2

if test ${has_detector} -gt 0
then
    if test $# -gt 2
    then
        detector_path=$3
    else
        detector_path=/Users/jliffla/codes/vt/detector-install
    fi
else
    detector_path=
fi

if test $# -gt 3
then
    gtest_directory=$4
else
    gtest_directory=/Users/jliffla/codes/gtest/gtest-install
fi

compiler_c=clang-mp-3.9
compiler_cxx=clang++-mp-3.9

if test $# -gt 4; then compiler_c=$5; fi
if test $# -gt 5; then compiler_cxx=$6; fi

cmake ../checkpoint                                                    \
      -DCMAKE_INSTALL_PREFIX=../checkpoint-install                     \
      -DCMAKE_CXX_COMPILER=${compiler_cxx}                             \
      -DCMAKE_C_COMPILER=${compiler_c}                                 \
      -Ddetector_DIR=${detector_path}                                  \
      -DCHECKPOINT_BUILD_TESTS:bool=ON                                 \
      -DCHECKPOINT_BUILD_EXAMPLES:bool=ON                              \
      -DGTEST_DIR=${gtest_directory}                                   \
      -DCMAKE_BUILD_TYPE=${build_mode}                                 \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=true
