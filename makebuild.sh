#!/bin/bash
COMPONENT=checkpoint
SRC_DIR=../$COMPONENT
INSTALL_DIR=../$COMPONENT-install
BUILD_DIR=../$COMPONENT-build

mdkir -p $INSTALL_DIR
mkdir -p $BUILD_DIR

rm -fRv $BUILD_DIR/*

pushd $BUILD_DIR

cmake -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
	-Ddetector_DIR=../detector-install \
	$SRC_DIR

popd
