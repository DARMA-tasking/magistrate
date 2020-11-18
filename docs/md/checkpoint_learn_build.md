\page ckpt_learn_build How to Build

checkpoint can be built with `cmake`.

\section ckpt_how-to-build Building

To build *checkpoint*, one must obtain the following dependencies:

\subsection ckpt_required-deps Required
  - [detector](https://github.com/DARMA-tasking/detector)

\subsection ckpt_optional-deps Optional

  - gtest,   (if testing *checkpoint* is enabled)
     - MPI,   (if MPI tests are enabled)
  - [Kokkos](https://github.com/kokkos/kokkos),  (if Kokkos kernels need to be serialized)

\subsection ckpt_use-cmake-directly-vars Using cmake directly

One may use `cmake` as normal on *checkpoint*, with detector installed.
The following are some custom configuration build options that can be
provided to `cmake` to change the build configuration:


| CMake Variable                   | Default Value   | Description |
| ------------------               | --------------- | ----------- |
| `detector_DIR`                    | | Directory where the *compiled* detector library is installed |
| `gtest_DIR`                       | | Directory where the *compiled* GoogleTest library is installed |
| `CHECKPOINT_BUILD_TESTS`          | OFF | Build *checkpoint* tests |
| `CHECKPOINT_BUILD_TESTS_WITH_MPI` | OFF | Build *checkpoint* tests with MPI |
| `CHECKPOINT_BUILD_EXAMPLES`       | OFF | Build *checkpoint* examples |
| `KOKKOS_KERNELS_ENABLED`          | ON | |
| `checkpoint_doxygen_enabled`      | 0               | Enable doxygen generation |

Here is an example of a custom `cmake` command to configure *checkpoint*:

```bash
$ cmake ../checkpoint                                                  \
      -DCMAKE_INSTALL_PREFIX=../checkpoint-install                     \
      -DCMAKE_CXX_COMPILER=${compiler_cxx}                             \
      -DCMAKE_C_COMPILER=${compiler_c}                                 \
      -Ddetector_DIR=${detector_path}                                  \
      -DCHECKPOINT_BUILD_TESTS:bool=${build_tests}                     \
      -DCHECKPOINT_BUILD_EXAMPLES:bool=${build_examples}               \
      -DCHECKPOINT_BUILD_TESTS_WITH_MPI:bool=${build_tests_mpi}        \
      -DKOKKOS_KERNELS_ENABLED:bool=${enable_kokkos_serializer}        \
      -Dgtest_DIR=${gtest_directory}                                   \
      -DCMAKE_BUILD_TYPE=${build_mode}                                 \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=true
```
