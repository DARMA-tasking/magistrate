
ARG compiler=11.2.2
ARG arch=amd64
ARG ubuntu=20.04
FROM --platform=${arch} nvidia/cuda:${compiler}-devel-ubuntu${ubuntu} as base

ARG proxy=""

ENV https_proxy=${proxy} \
    http_proxy=${proxy}

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update -y -q && \
    apt-get install -y -q --no-install-recommends \
    ca-certificates \
    curl \
    less \
    git \
    wget \
    zlib1g \
    zlib1g-dev \
    ninja-build \
    gnupg \
    make-guile \
    unzip \
    libomp5 \
    valgrind \
    ccache && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

ENV CC=gcc \
    CXX=g++

COPY ./ci/deps/cmake.sh cmake.sh
RUN ./cmake.sh 3.23.4 ${arch}

COPY ./ci/deps/mpich.sh mpich.sh
RUN ./mpich.sh 3.3.2 -j4

ENV PATH=/cmake/bin/:$PATH
ENV LESSCHARSET=utf-8

COPY ./ci/deps/gtest.sh gtest.sh
RUN ./gtest.sh 1.12.1 /pkgs
ENV GTEST_ROOT=/pkgs/gtest/install

COPY ./ci/deps/kokkos.sh kokkos.sh
RUN ./kokkos.sh 4.1.00 /pkgs 0
ENV KOKKOS_ROOT=/pkgs/kokkos/install

RUN mkdir -p /nvcc_wrapper/build && \
    wget https://raw.githubusercontent.com/kokkos/kokkos/master/bin/nvcc_wrapper -P /nvcc_wrapper/build && \
    chmod +x /nvcc_wrapper/build/nvcc_wrapper

ENV MPI_EXTRA_FLAGS="" \
    PATH=/usr/lib/ccache/:/nvcc_wrapper/build:$PATH \
    CXX=nvcc_wrapper

COPY ./ci/deps/kokkos-kernels.sh kokkos-kernels.sh
RUN ./kokkos-kernels.sh 4.1.00 /pkgs
ENV KOKKOS_KERNELS_ROOT=/pkgs/kokkos-kernels/install

FROM base as build
COPY . /checkpoint

ARG CHECKPOINT_DOXYGEN_ENABLED
ARG CHECKPOINT_TESTS_ENABLED
ARG CHECKPOINT_EXAMPLES_ENABLED
ARG CHECKPOINT_WARNINGS_AS_ERRORS
ARG CHECKPOINT_MPI_ENABLED
ARG CHECKPOINT_ASAN_ENABLED
ARG CHECKPOINT_UBSAN_ENABLED
ARG CHECKPOINT_SERIALIZATION_ERROR_CHECKING_ENABLED

ENV CHECKPOINT_DOXYGEN_ENABLED=${CHECKPOINT_DOXYGEN_ENABLED} \
    CHECKPOINT_TESTS_ENABLED=${CHECKPOINT_TESTS_ENABLED} \
    CHECKPOINT_EXAMPLES_ENABLED=${CHECKPOINT_EXAMPLES_ENABLED} \
    CHECKPOINT_WARNINGS_AS_ERRORS=${CHECKPOINT_WARNINGS_AS_ERRORS} \
    CHECKPOINT_MPI_ENABLED=${CHECKPOINT_MPI_ENABLED} \
    CHECKPOINT_ASAN_ENABLED=${CHECKPOINT_ASAN_ENABLED} \
    CHECKPOINT_UBSAN_ENABLED=${CHECKPOINT_UBSAN_ENABLED} \
    CHECKPOINT_SERIALIZATION_ERROR_CHECKING_ENABLED=${CHECKPOINT_SERIALIZATION_ERROR_CHECKING_ENABLED} \
    CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}

RUN /checkpoint/ci/build_cpp.sh /checkpoint /build

FROM build as test
RUN /checkpoint/ci/test_cpp.sh /checkpoint /build
