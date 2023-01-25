
ARG cuda=11.4
ARG arch=amd64
FROM ${arch}/ubuntu:20.04 as base

ARG proxy=""
ARG compiler=nvcc-11.2

ENV https_proxy=${proxy} \
    http_proxy=${proxy}

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update -y -q && \
    apt-get install -y -q --no-install-recommends \
    ca-certificates \
    g++-7 \
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

RUN if test ${compiler} = "nvcc-12"; then \
      wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2004/x86_64/cuda-ubuntu2004.pin && \
      mv cuda-ubuntu2004.pin /etc/apt/preferences.d/cuda-repository-pin-600 && \
      wget https://developer.download.nvidia.com/compute/cuda/12.0.0/local_installers/cuda-repo-ubuntu2004-12-0-local_12.0.0-525.60.13-1_amd64.deb && \
      dpkg -i cuda-repo-ubuntu2004-12-0-local_12.0.0-525.60.13-1_amd64.deb && \
      cp /var/cuda-repo-ubuntu2004-12-0-local/cuda-*-keyring.gpg /usr/share/keyrings/ && \
      apt-get update && \
      apt-get -y install cuda && \
      apt-get clean && \
      rm -rf /var/lib/apt/lists/* && \
      rm -rf cuda-repo-ubuntu1804-10-1-local-10.1.243-418.87.00_1.0-1_amd64.deb && \
      ln -s /usr/local/cuda-12 /usr/local/cuda-versioned; \
    elif test ${compiler} = "nvcc-11.4"; then \
      wget http://developer.download.nvidia.com/compute/cuda/repos/ubuntu2004/x86_64/cuda-ubuntu2004.pin && \
      mv cuda-ubuntu2004.pin /etc/apt/preferences.d/cuda-repository-pin-600 && \
      wget http://developer.download.nvidia.com/compute/cuda/11.4.1/local_installers/cuda-repo-ubuntu2004-11-4-local_11.4.1-470.57.02-1_amd64.deb && \
      dpkg -i cuda-repo-ubuntu2004-11-4-local_11.4.1-470.57.02-1_amd64.deb && \
      apt-key add /var/cuda-repo-ubuntu2004-11-4-local/7fa2af80.pub && \
      apt-get update && \
      apt-get -y install cuda && \
      apt-get clean && \
      rm -rf /var/lib/apt/lists/* && \
      rm -rf cuda-repo-ubuntu2004-11-4-local_11.4.1-470.57.02-1_amd64.deb && \
      ln -s /usr/local/cuda-11.4 /usr/local/cuda-versioned; \
    elif test ${compiler} = "nvcc-11.2"; then \
      wget http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1804/x86_64/cuda-ubuntu1804.pin && \
      mv cuda-ubuntu1804.pin /etc/apt/preferences.d/cuda-repository-pin-600 && \
      wget https://developer.download.nvidia.com/compute/cuda/11.2.0/local_installers/cuda-repo-ubuntu1804-11-2-local_11.2.0-460.27.04-1_amd64.deb && \
      dpkg -i cuda-repo-ubuntu1804-11-2-local_11.2.0-460.27.04-1_amd64.deb && \
      apt-key add /var/cuda-repo-ubuntu1804-11-2-local/7fa2af80.pub && \
      apt-get update && \
      apt-get -y install cuda-nvcc-11-2 && \
      apt-get clean && \
      rm -rf /var/lib/apt/lists/* && \
      rm -rf cuda-repo-ubuntu1804-11-2-local_11.2.0-460.27.04-1_amd64.deb && \
      ln -s /usr/local/cuda-11.2 /usr/local/cuda-versioned; \
    fi

ENV CC=gcc \
    CXX=g++

COPY ./ci/deps/cmake.sh cmake.sh
RUN ./cmake.sh 3.18.4

COPY ./ci/deps/mpich.sh mpich.sh
RUN ./mpich.sh 3.3.2 -j4

ENV PATH=/cmake/bin/:$PATH
ENV LESSCHARSET=utf-8

COPY ./ci/deps/gtest.sh gtest.sh
RUN ./gtest.sh 1.8.1 /pkgs
ENV GTEST_ROOT=/pkgs/gtest/install

ENV CUDACXX=/usr/local/cuda-versioned/bin/nvcc
ENV PATH=/usr/local/cuda-versioned/bin/:$PATH

COPY ./ci/deps/kokkos.sh kokkos.sh
RUN ./kokkos.sh 3.6.00 /pkgs 0
ENV KOKKOS_ROOT=/pkgs/kokkos/install/lib

RUN git clone https://github.com/kokkos/nvcc_wrapper.git && \
    cd nvcc_wrapper && \
    mkdir build && \
    cd build && \
    cmake ../

ENV MPI_EXTRA_FLAGS="" \
    CXX=/nvcc_wrapper/build/nvcc_wrapper \
    PATH=/usr/lib/ccache/:$PATH

COPY ./ci/deps/kokkos-kernels.sh kokkos-kernels.sh
RUN ./kokkos-kernels.sh 3.6.00 /pkgs
ENV KOKKOS_KERNELS_ROOT=/pkgs/kokkos-kernels/install/lib

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
