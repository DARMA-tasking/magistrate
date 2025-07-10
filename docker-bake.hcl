variable "REPO" {
  default = "lifflander1/vt"
}

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
}

target "magistrate-build-all" {
  name = "magistrate-build-${replace(item.image, ".", "-")}"
  inherits = ["magistrate-build"]
  tags = ["${REPO}:vt-${item.image}"]
  cache-from = [
    "type=local,src=.buildx-cache/${item.image}"
  ]
  cache-to = [
    "type=local,dest=.buildx-cache/${item.image},mode=max"
  ]

  args = {
    ARCH = arch(item)
    IMAGE = "wf-${item.image}"
    REPO = REPO
    MAGISTRATE_TESTS = magistrate_tests(item)
    MAGISTRATE_EXAMPLES = magistrate_examples(item)
    MAGISTRATE_MPI = magistrate_mpi(item)
    MAGISTRATE_WARNINGS_AS_ERRORS = magistrate_warnings_as_errors(item)
    MAGISTRATE_DOCS = magistrate_docs(item)
    MAGISTRATE_ASAN = magistrate_asan(item)
    MAGISTRATE_UBSAN = magistrate_ubsan(item)
    MAGISTRATE_SERIALIZATION_ERROR_CHECKING = magistrate_serialization_error_checking(item)
  }

  # to get the list of available images from DARMA-tasking/workflows:
  # workflows > docker buildx bake --print magistrate-build-all | grep "lifflander1/vt:"
  matrix = {
    item = [
      {
        image = "amd64-ubuntu-20.04-clang-9-cpp"
      },
      {
        image = "amd64-ubuntu-20.04-clang-10-cpp"
      },
      {
        image = "amd64-ubuntu-22.04-clang-11-cpp"
      },
      {
        image = "amd64-ubuntu-22.04-clang-12-cpp"
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
        image = "amd64-ubuntu-20.04-gcc-9-cpp"
        magistrate_code_coverage = 1
        magistrate_serialization_error_checking = 0
      },
      {
        image = "amd64-ubuntu-20.04-gcc-9-cuda-11.4.3-cpp"
        magistrate_asan = 0
        magistrate_serialization_error_checking = 0
      },
    ]
  }
}
