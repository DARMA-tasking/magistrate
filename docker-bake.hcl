variable "REPO" {
  default = "lifflander1/vt"
}

variable "GIT_BRANCH" {}

function "arch" {
  params = [item]
  result = lookup(item, "arch", "amd64")
}

function "magistrate_asan" {
  params = [item]
  result = lookup(item, "magistrate_asan", "1")
}

function "magistrate_ubsan" {
  params = [item]
  result = lookup(item, "magistrate_ubsan", "0")
}

function "magistrate_code_coverage" {
  params = [item]
  result = lookup(item, "magistrate_code_coverage", "0")
}

function "magistrate_tests" {
  params = [item]
  result = lookup(item, "magistrate_tests", "1")
}

function "magistrate_examples" {
  params = [item]
  result = lookup(item, "magistrate_examples", "1")
}

function "magistrate_mpi" {
  params = [item]
  result = lookup(item, "magistrate_mpi", "1")
}

function "magistrate_warnings_as_errors" {
  params = [item]
  result = lookup(item, "magistrate_warnings_as_errors", "1")
}

function "magistrate_docs" {
  params = [item]
  result = lookup(item, "magistrate_docs", "0")
}

function "magistrate_serialization_error_checking" {
  params = [item]
  result = lookup(item, "magistrate_serialization_error_checking", "1")
}

function "magistrate_build_against_vt" {
  params = [item]
  result = lookup(item, "magistrate_build_against_vt", "0")
}

function "magistrate_asan_options" {
  params = [item]
  result = lookup(item, "magistrate_asan_options", "")
}

function "cmake_build_type" {
  params = [item]
  result = lookup(item, "cmake_build_type", "Release")
}

function "magistrate_code_coverage" {
  params = [item]
  result = lookup(item, "magistrate_code_coverage", "0")
}

function "variant" {
  params = [item]
  result = lookup(item, "variant", "")
}

function "target_suffix" {
  params = [item]
  result = variant(item) == "" ? "" : "-${variant(item)}"
}

target "magistrate-build" {
  target = "build"
  context = "."
  dockerfile = "ci/docker/dockerfile"

  platforms = [
    "linux/amd64"
  ]
  ulimits = [
    "core=0"
  ]

  secret = ["id=GITHUB_TOKEN,env=GITHUB_TOKEN"]
}

target "magistrate-build-all" {
  name = "magistrate-build-${replace(item.image, ".", "-")}${target_suffix(item)}"
  inherits = ["magistrate-build"]
  tags = ["${REPO}:magistrate-${item.image}"]

  args = {
    ARCH = arch(item)
    GIT_BRANCH = "${GIT_BRANCH}"
    IMAGE = "wf-${item.image}"
    REPO = REPO
    CMAKE_BUILD_TYPE = cmake_build_type(item)
    MAGISTRATE_TESTS_ENABLED = magistrate_tests(item)
    MAGISTRATE_EXAMPLES_ENABLED = magistrate_examples(item)
    MAGISTRATE_MPI_ENABLED = magistrate_mpi(item)
    MAGISTRATE_WARNINGS_AS_ERRORS = magistrate_warnings_as_errors(item)
    MAGISTRATE_DOXYGEN_ENABLED = magistrate_docs(item)
    MAGISTRATE_ASAN_ENABLED = magistrate_asan(item)
    MAGISTRATE_UBSAN_ENABLED = magistrate_ubsan(item)
    MAGISTRATE_SERIALIZATION_ERROR_CHECKING_ENABLED = magistrate_serialization_error_checking(item)
    MAGISTRATE_BUILD_AGAINST_VT = magistrate_build_against_vt(item)
    MAGISTRATE_ASAN_OPTIONS = magistrate_asan_options(item)
    MAGISTRATE_CODE_COVERAGE = magistrate_code_coverage(item)
  }

  matrix = {
    item = [
      {
        image = "amd64-ubuntu-20.04-clang-9-cpp"
      },
      {
        image = "amd64-ubuntu-20.04-clang-10-cpp"
      },
      {
        image = "amd64-ubuntu-22.04-clang-11-cpp",
        magistrate_asan_options = "use_sigaltstack=false"
      },
      {
        image = "amd64-ubuntu-22.04-clang-12-cpp",
        magistrate_build_against_vt = 1,
        variant = "vt",
        magistrate_ubsan = 1
      },
      {
        image = "amd64-ubuntu-22.04-clang-13-cpp"
      },
      {
        image = "amd64-ubuntu-22.04-clang-14-cpp"
      },
      {
        image = "amd64-ubuntu-20.04-gcc-10-cpp"
      },
      {
        image = "amd64-ubuntu-20.04-gcc-9-cpp",
        magistrate_code_coverage = 1,
        magistrate_serialization_error_checking = 0
      },
      {
        image = "amd64-ubuntu-20.04-gcc-9-cpp",
        magistrate_docs = 1,
        variant = "docs",
      },
      {
        image = "amd64-ubuntu-20.04-gcc-9-cuda-11.4.3-cpp",
        magistrate_asan = 0,
        magistrate_serialization_error_checking = 0
      }
    ]
  }
}
