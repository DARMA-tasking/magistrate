
macro(require_pkg_directory pkg_name pkg_user_name)
  get_directory_property(hasParent PARENT_DIRECTORY)
  if(hasParent)
    # Skip this logic when this macro was not invoked from the
    # top-level CMakeLists.txt file under the assumption that this
    # package was dropped into another build system using add_subdirectory().
    # Note that this will also skip if you call this macro from
    # a subdirectory in your own package, so just don't do it!

    #message(STATUS "skipping find_package for ${pkg_name}")
  else()
    #message(STATUS "require_directory: name=${pkg_name}")
    option(${pkg_name}_DIR "Root folder for ${pkg_user_name} installation" OFF)
    if (NOT ${pkg_name}_DIR)
      message(
        FATAL_ERROR "Please specify ${pkg_user_name} library installation root"
        " with -D${pkg_name}_DIR="
      )
    endif()
  endif()
endmacro(require_pkg_directory)

macro(find_package_local pkg_name pkg_other_name)
  get_directory_property(hasParent PARENT_DIRECTORY)
  if(hasParent)
    # Skip this logic when this macro was not invoked from the
    # top-level CMakeLists.txt file under the assumption that this
    # package was dropped into another build system using add_subdirectory().
    # Note that this will also skip if you call this macro from
    # a subdirectory in your own package, so just don't do it!

    #message(STATUS "skipping find_package for ${pkg_name}")
  else()
    if(DEFINED ${pkg_name}_DIR AND NOT DEFINED ${pkg_name}_ROOT)
      set(${pkg_name}_ROOT "${${pkg_name}_DIR}"
                           "${${pkg_name}_DIR}/cmake/${pkg_name}/"
                           "${${pkg_name}_DIR}/cmake/${pkg_other_name}/"
                           "${${pkg_name}_DIR}/CMake/${pkg_name}/"
                           "${${pkg_name}_DIR}/CMake/${pkg_other_name}/"
      )
    endif()

    message(
      STATUS "find_package_local: pkg name=\"${pkg_name}\", ${pkg_name}_ROOT=\"${${pkg_name}_ROOT}\""
    )

    # Search locally only for package based on the user's supplied path;
    find_package(
      ${pkg_name}
      NAMES ${pkg_name} ${pkg_other_name}
      NO_CMAKE_PACKAGE_REGISTRY
      NO_CMAKE_BUILDS_PATH
      NO_CMAKE_SYSTEM_PATH
      NO_CMAKE_SYSTEM_PACKAGE_REGISTRY
      NO_SYSTEM_ENVIRONMENT_PATH
      QUIET
      REQUIRED
    )
  endif()
endmacro(find_package_local)

macro(optional_pkg_directory pkg_name pkg_user_name assume_found_if_hasparent)
  get_directory_property(hasParent PARENT_DIRECTORY)
  if(hasParent AND NOT (${CMAKE_PROJECT_NAME} STREQUAL "vt"))
    # Skip MOST of this logic when this macro was not invoked from the
    # top-level CMakeLists.txt file under the assumption that this
    # package was dropped into another build system using add_subdirectory().
    # Note that this will also skip if you call this macro from
    # a subdirectory in your own package, so just don't do it!

    if(${assume_found_if_hasparent})
      # Assume that the package is available even if the directory wasn't specified
      set(${pkg_name}_DIR_FOUND 1)
    endif()
  else()
    #message(STATUS "optional_pkg_directory: name=${pkg_name}")
    if (NOT ${pkg_name}_DIR AND NOT ${pkg_name}_ROOT)
      message(
        STATUS
        "Path for ${pkg_user_name} library (optional) not specified "
        "with -D${pkg_name}_DIR= or -D${pkg_name}_ROOT="
      )
      message(
        STATUS
        "Building without ${pkg_user_name} library"
      )
      set(${pkg_name}_DIR_FOUND 0)
    else()
      message(
        STATUS
        "Path for ${pkg_user_name} library (optional) specified "
        "with -D${pkg_name}_DIR=${${pkg_name}_DIR} -D${pkg_name}_ROOT=${${pkg_name}_ROOT}"
      )
      set(${pkg_name}_DIR_FOUND 1)
    endif()
  endif()
endmacro(optional_pkg_directory)
