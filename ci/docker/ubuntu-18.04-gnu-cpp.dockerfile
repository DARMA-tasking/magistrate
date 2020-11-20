
ARG arch=amd64
ARG distro=18.04
FROM ${arch}/ubuntu:${distro} as base

ARG proxy=""
ARG compiler=gcc-7

ENV https_proxy=${proxy} \
    http_proxy=${proxy}

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update -y -q && \
    apt-get install -y -q --no-install-recommends \
    g++-$(echo ${compiler} | cut -d- -f2) \
    ca-certificates \
    less \
    curl \
    git \
    wget \
    ${compiler} \
    zlib1g \
    zlib1g-dev \
    unzip \
    ninja-build \
    valgrind \
    make-guile \
    libomp5 \
    ccache && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

RUN ln -s \
    "$(which g++-$(echo ${compiler}  | cut -d- -f2))" \
    /usr/bin/g++

RUN ln -s \
    "$(which gcc-$(echo ${compiler}  | cut -d- -f2))" \
    /usr/bin/gcc

ENV CC=gcc \
    CXX=g++

COPY ./ci/deps/cmake.sh cmake.sh
RUN ./cmake.sh 3.18.4

COPY ./ci/deps/mpich.sh mpich.sh
RUN ./mpich.sh 3.3.2 -j4

ENV PATH=/cmake/bin/:$PATH
ENV LESSCHARSET=utf-8

COPY ./ci/deps/gtest.sh gtest.sh
RUN ./gtest.sh 1.8.1 /build
ENV GTEST_ROOT=/build/gtest/install

COPY ./ci/deps/kokkos.sh kokkos.sh
RUN ./kokkos.sh 3.1.01 /build 1
ENV KOKKOS_ROOT=/build/kokkos/install/lib

COPY ./ci/deps/kokkos-kernels.sh kokkos-kernels.sh
RUN ./kokkos-kernels.sh 3.2.00 /build
ENV KOKKOS_KERNELS_ROOT=/build/kokkos-kernels/install/lib

ENV MPI_EXTRA_FLAGS="" \
    PATH=/usr/lib/ccache/:$PATH

RUN apt-get update -y -q && \
    apt-get install -y -q --no-install-recommends \
    lcov && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

FROM base as build
COPY . /checkpoint

ARG CHECKPOINT_DOXYGEN_ENABLED
ARG CHECKPOINT_TESTS_ENABLED
ARG CHECKPOINT_EXAMPLES_ENABLED
ARG CHECKPOINT_WARNINGS_AS_ERRORS
ARG CHECKPOINT_MPI_ENABLED
ARG CHECKPOINT_ASAN_ENABLED

ENV CHECKPOINT_DOXYGEN_ENABLED=${CHECKPOINT_DOXYGEN_ENABLED} \
    CHECKPOINT_TESTS_ENABLED=${CHECKPOINT_TESTS_ENABLED} \
    CHECKPOINT_EXAMPLES_ENABLED=${CHECKPOINT_EXAMPLES_ENABLED} \
    CHECKPOINT_WARNINGS_AS_ERRORS=${CHECKPOINT_WARNINGS_AS_ERRORS} \
    CHECKPOINT_MPI_ENABLED=${CHECKPOINT_MPI_ENABLED} \
    CHECKPOINT_ASAN_ENABLED=${CHECKPOINT_ASAN_ENABLED} \
    CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}

RUN /checkpoint/ci/build_cpp.sh /checkpoint /build

FROM build as test
RUN /checkpoint/ci/test_cpp.sh /checkpoint /build
