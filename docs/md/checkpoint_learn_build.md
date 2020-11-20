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

One may use `cmake` as normal on *checkpoint* once the dependencies are
installed.  The following are some configuration build options that can be
provided to `cmake` to change the build configuration:

| CMake Variable                    | Default Value   | Description                           |
| ------------------                | --------------- | -----------                           |
| `detector_DIR`                    |                 | Install directory for detector        |
| `gtest_DIR`                       |                 | Install directory for googletest      |
| `kokkos_DIR`                      |                 | Install directory for kokkos          |
| `KokkosKernels_DIR`               |                 | Install directory for kokkoskernels   |
| `checkpoint_tests_enabled`        | 0               | Build *checkpoint* tests              |
| `checkpoint_mpi_enabled`          | 0               | Build *checkpoint* with MPI for tests |
| `checkpoint_examples_enabled`     | 0               | Build *checkpoint* examples           |
| `checkpoint_warnings_as_errors`   | 0               | Make all warnings errors during build |
| `checkpoint_doxygen_enabled`      | 0               | Enable doxygen generation             |
| `checkpoint_asan_enabled`         | 0               | Enable address sanitizer              |
| `CODE_COVERAGE`                   | 0               | Generate code coverage report         |
