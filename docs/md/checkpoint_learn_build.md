\page ckpt_learn_build How to Build

checkpoint can be built with `cmake`.

\section ckpt_how-to-build Building

To build *checkpoint*, one must obtain the following dependencies:

\subsection ckpt_required-deps Required

  - [detector](https://github.com/DARMA-tasking/detector)

\subsection ckpt_optional-deps Optional

  - gtest,   (if testing *checkpoint* is enabled)
  - MPI,   (if MPI tests are enabled)
  - [Kokkos](https://github.com/kokkos/kokkos),  (if Kokkos views need to be serialized)
  - [KokkosKernels](https://github.com/kokkos/kokkos-kernels),  (if Kokkos kernel data structures need to be serialized)

\subsection ckpt_use-cmake-directly-vars Using cmake directly

One may use `cmake` to build *checkpoint* once the dependencies are installed.
The following are some options that can be provided to `cmake` to change the
build configuration:

| CMake Variable                                       | Default Value | Description                                   |
| ---------------------------------------------------- | ------------- | --------------------------------------------- |
| `detector_DIR`                                       |               | Install directory for detector                |
| `gtest_DIR`                                          |               | Install directory for googletest              |
| `kokkos_DIR`                                         |               | Install directory for kokkos                  |
| `KokkosKernels_DIR`                                  |               | Install directory for kokkoskernels           |
| `checkpoint_tests_enabled`                           | 0             | Build *checkpoint* tests                      |
| `checkpoint_mpi_enabled`                             | 0             | Build *checkpoint* with MPI for tests         |
| `checkpoint_examples_enabled`                        | 0             | Build *checkpoint* examples                   |
| `checkpoint_warnings_as_errors`                      | 0             | Make all warnings errors during build         |
| `checkpoint_doxygen_enabled`                         | 0             | Enable doxygen generation                     |
| `checkpoint_asan_enabled`                            | 0             | Enable address sanitizer                      |
| `checkpoint_ubsan_enabled`                           | 0             | Enable undefined behavior sanitizer           |
| `checkpoint_serialization_error_checking_enabled(*)` | 0             | Enable extensive serialization error checking |
| `CODE_COVERAGE`                                      | 0             | Generate code coverage report                 |

* note that if `checkpoint_serialization_error_checking_enabled` is not explicitly enabled or disabled, it will be **enabled** for `Debug` and `RelWithDebInfo` builds and disabled for others.

\subsection using-the-build-script Using the Build Script

Instead of running `cmake`, one may invoke the `checkpoint/ci/build_cpp.sh`
script which will run `cmake` for *checkpoint* with environment variables for
most configuration parameters.

\subsubsection building-environment-variables Build Script Environment Variables

| Variable                                          | Default Value | Description                                       |
| ------------------------------------------------- | ------------- | ------------------------------------------------- |
| `CMAKE_BUILD_TYPE`                                | Release       | The `cmake` build type                            |
| `CODE_COVERAGE`                                   | 0             | Enable code coverage reporting                    |
| `GTEST_ROOT`                                      | <empty>       | Install directory for googletest                  |
| `KOKKOS_ROOT`                                     | <empty>       | Install directory for kokkos                      |
| `KOKKOS_KERNELS_ROOT`                             | <empty>       | Install directory for kokkos-kernels              |
| `CHECKPOINT_DOXYGEN_ENABLED`                      | 0             | Enable doxygen generation                         |
| `CHECKPOINT_ASAN_ENABLED`                         | 0             | Enable building with address sanitizer            |
| `CHECKPOINT_UBSAN_ENABLED`                        | 0             | Enable building with undefined behavior sanitizer |
| `CHECKPOINT_TESTS_ENABLED`                        | 1             | Enable checkpoint tests                           |
| `CHECKPOINT_EXAMPLES_ENABLED`                     | 1             | Enable checkpoint examples                        |
| `CHECKPOINT_WARNINGS_AS_ERRORS`                   | 0             | Make all warnings errors during build             |
| `CHECKPOINT_SERIALIZATION_ERROR_CHECKING_ENABLED` | 0             | Enable extensive error checking of serialization  |
| `CHECKPOINT_MPI_ENABLED`                          | 1             | Enable checkpoint MPI for testing                 |

* note that if `CHECKPOINT_SERIALIZATION_ERROR_CHECKING_ENABLED` is not explicitly enabled or disabled, it will be **enabled** for `Debug` and `RelWithDebInfo` builds and disabled for others.

With these set, invoke the script with two arguments: the path to the
*checkpoint* root directory and the build path. Here's an example assuming that
*checkpoint* is cloned into `/usr/src/checkpoint` with trace enabled in debug mode.

**Usage for building:**

```bash
$ checkpoint/ci/build_cpp.sh <full-path-to-checkpoint-source> <full-path-to-build-dir>
```

\subsection docker-build Building with docker containerization

The easiest way to build *checkpoint* is by using `docker` with the available
containers that contain the proper compilers, MPI, and all other
dependencies. First, install `docker` on the system. On some systems,
`docker-compose` might also need to be installed.

The `docker` builds are configured through `docker-compose` to use a shared,
cached filesystem mount with the host for `ccache` to enable fast re-builds.

For `docker-compose`, the following variables can be set to configure the
build. One may configure the architecture, compiler type (GNU, Clang, Intel,
Nvidia) and compiler version, Linux distro (ubuntu or alpine), and distro
version.

The default set of the docker configuration options is located in
`checkpoint/.env`, which `docker-compose` will read.

```
# Variables:
#   ARCH={amd64, arm64v8, ...}
#   COMPILER_TYPE={gnu, clang, intel, nvidia}
#   COMPILER={gcc-5, gcc-6, gcc-7, gcc-8, gcc-9, gcc-10,
#             clang-3.9, clang-4.0, clang-5.0, clang-6.0, clang-7, clang-8,
#             clang-9, clang-10,
#             icc-18, icc-19,
#             nvcc-10, nvcc-11}
#   REPO=lifflander1/checkpoint
#   UBUNTU={18.04, 20.04}
#   ULIMIT_CORE=0
#
# DARMA/checkpoint Configuration Variables:
#   CHECKPOINT_TESTS=1                        # Enable checkpoint tests
#   CHECKPOINT_EXAMPLES=1                     # Enable checkpoint examples
#   CHECKPOINT_MPI=1                          # Enable checkpoint MPI tests
#   CHECKPOINT_WARNINGS_AS_ERRORS=0           # Treat warnings as errors in compilation
#   CHECKPOINT_ASAN=0                         # Enable address sanitizer in build
#   CHECKPOINT_UBSAN=0                        # Enable undefined behavior sanitizer in build
#   CHECKPOINT_SERIALIZATION_ERROR_CHECKING=0 # Enable extensive serialization error checking
#   CHECKPOINT_DOCS=0                         # Enable doxygen build
#   BUILD_TYPE=release                        # CMake build type
#   CODE_COVERAGE=0                           # Enable generation of code coverage reports
```

With these set, one may run the following for a non-interactive build with
ubuntu. Or, to speed up the build process, the base container can be pulled for
many of the common configurations: `docker-compose pull ubuntu-cpp`.

```bash
$ cd checkpoint
$ docker-compose run -e BUILD_TYPE=debug ubuntu-cpp
```

For an interactive build with ubuntu, where one can build, debug, and run
`valgrind`, etc:

```bash
$ cd checkpoint
$ docker-compose run -e BUILD_TYPE=debug ubuntu-cpp-interactive
# /checkpoint/ci/build_cpp.sh /checkpoint /build
# /checkpoint/ci/test_cpp.sh /checkpoint /build
```
