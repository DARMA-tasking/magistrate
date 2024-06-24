
ARG arch=amd64
FROM ${arch}/ubuntu:20.04 as base

ARG proxy=""
ARG compiler=gcc-8
ARG token

ENV https_proxy=${proxy} \
    http_proxy=${proxy}

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update -y -q && \
    apt-get install -y -q --no-install-recommends \
    ca-certificates \
    curl \
    cmake \
    git \
    mpich \
    libmpich-dev \
    wget \
    ${compiler} \
    zlib1g \
    zlib1g-dev \
    ninja-build \
    doxygen \
    unzip \
    python3 \
    python3-jinja2 \
    python3-pygments \
    texlive-font-utils \
    ghostscript \
    ccache && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

COPY ./ci/deps/cmake.sh cmake.sh
RUN ./cmake.sh 3.23.4 ${arch}

ENV PATH=/cmake/bin/:$PATH
ENV LESSCHARSET=utf-8

COPY ./ci/deps/gtest.sh gtest.sh
RUN ./gtest.sh 1.8.1 /pkgs
ENV GTEST_ROOT=/pkgs/gtest/install

COPY ./ci/deps/kokkos.sh kokkos.sh
RUN ./kokkos.sh 4.1.00 /pkgs 1
ENV KOKKOS_ROOT=/pkgs/kokkos/install/lib

COPY ./ci/deps/kokkos-kernels.sh kokkos-kernels.sh
RUN ./kokkos-kernels.sh 4.1.00 /pkgs
ENV KOKKOS_KERNELS_ROOT=/pkgs/kokkos-kernels/install/lib

ENV MPI_EXTRA_FLAGS="" \
    CMAKE_PREFIX_PATH="/lib/x86_64-linux-gnu/" \
    CC=mpicc \
    CXX=mpicxx \
    PATH=/usr/lib/ccache/:$PATH

FROM base as build
COPY . /checkpoint

ARG token
ARG CHECKPOINT_DOXYGEN_ENABLED
ARG CMAKE_BUILD_TYPE

ENV CHECKPOINT_DOXYGEN_ENABLED=${CHECKPOINT_DOXYGEN_ENABLED} \
    CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}

RUN /checkpoint/ci/build_cpp.sh /checkpoint /build "${token}"
