
ARG compiler=icc-18
FROM lifflander1/${compiler} as base

ARG proxy=""

ENV https_proxy=${proxy} \
    http_proxy=${proxy}

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update -y -q && \
    apt-get install -y -q --no-install-recommends \
    ca-certificates \
    less \
    curl \
    git \
    wget \
    zlib1g \
    zlib1g-dev \
    ninja-build \
    valgrind \
    make-guile \
    libomp5 \
    unzip \
    ccache && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

RUN ln -s \
    /opt/intel/install/bin/icpc \
    /opt/intel/install/bin/g++

RUN ln -s \
    /opt/intel/install/bin/icc \
    /opt/intel/install/bin/gcc

ARG arch

COPY ./ci/deps/cmake.sh cmake.sh
RUN ./cmake.sh 3.23.4 ${arch}
ENV PATH=/cmake/bin/:$PATH

COPY ./ci/deps/gtest.sh gtest.sh
RUN ./gtest.sh 1.8.1 /pkgs
ENV GTEST_ROOT=/pkgs/gtest/install

COPY ./ci/deps/mpich.sh mpich.sh
RUN ./mpich.sh 3.3.2 -j4

ENV LESSCHARSET=utf-8

ENV CC=/opt/intel/install/bin/icc \
    CXX=/opt/intel/install/bin/icpc

COPY ./ci/deps/kokkos.sh kokkos.sh
RUN ./kokkos.sh 4.1.00 /pkgs 1
ENV KOKKOS_ROOT=/pkgs/kokkos/install

COPY ./ci/deps/kokkos-kernels.sh kokkos-kernels.sh
RUN ./kokkos-kernels.sh 4.1.00 /pkgs
ENV KOKKOS_KERNELS_ROOT=/pkgs/kokkos-kernels/install

ENV MPI_EXTRA_FLAGS="" \
    PATH=/usr/lib/ccache/:$PATH \
    LD_LIBRARY_PATH=/opt/intel/ld_library_path

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
