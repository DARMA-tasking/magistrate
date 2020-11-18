
ARG arch=amd64
FROM ${arch}/ubuntu:18.04 as base

ARG proxy=""
ARG compiler=gcc-7
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
    python3 \
    python3-jinja2 \
    python3-pygments \
    texlive-font-utils \
    ghostscript \
    ccache && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

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
